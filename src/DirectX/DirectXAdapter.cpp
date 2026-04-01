#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "DirectXAdapter.hpp"

#include <cassert>

#include <d3dcompiler.h>

#include "Heap/Heap.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "LeakChecker/D3DResourceLeakChecker.hpp"

#include "include/DebugUI.hpp"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

DirectXAdapter::DirectXAdapter(const HWND _hWnd, size_t _width, size_t _height) :
    windowSize_(_width, _height), hWnd_(_hWnd), dsvHandle_() {
    Log::Send(Log::Level::INFO, "DirectXAdapter Created");
}

DirectXAdapter::~DirectXAdapter() {
    // Force cleanup before leak check
    if (fenceEvent_) {
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }

    // Force all resources to null before leak check
    swapChainResources_.clear();
    depthStencil_.reset();
    rtvHeap_.reset();
    dsvHeap_.reset();

    // Clear command objects
    cList_.Reset();
    cAllocator_.Reset();
    cQueue_.Reset();

    // Clear swap chain and device
    swapChain_.Reset();
    fence_.Reset();

    // Clear DXGI objects
    adapter_.Reset();
    factory_.Reset();

    // Clear device last (but before debug layer)
    device_.Reset();

    // Clear debug layer last
    debugLayer_.Reset();

    D3DResourceLeakChecker _lc;
}

void DirectXAdapter::Initialize() {
    EnableDebugLayer();
    if (!CreateDXGI())Utils::Alert("Failed to create DXGI");
    if (!InfoQueue()) Utils::Alert("Failed to create InfoQueue");
    if (!CreateCommand())Utils::Alert("Failed to create Command");
    if (!CreateSwapChain()) Utils::Alert("Failed to create SwapChain");
    if (!CreateFence()) Utils::Alert("Failed to create Fence");
    if (!CreateRTV()) Utils::Alert("Failed to create RTV");
    if (!CreateDSV()) Utils::Alert("Failed to create DSV");
    if (!CreateViewportAndScissor()) Utils::Alert("Failed to create Viewport and Scissor");
    if (!CreateLimiter()) Utils::Alert("Failed to create FrameRateLimiter");

    Log::Send(Log::Level::INFO, "DirectXAdapter Initialized");
}

std::unique_ptr<DX12Resource> DirectXAdapter::CreateBufferResource(const size_t _size) const {
    std::unique_ptr<DX12Resource> wrapper = std::make_unique<DX12Resource>();
    D3D12_HEAP_PROPERTIES properties{};
    properties.Type = D3D12_HEAP_TYPE_UPLOAD;

    //resource setting
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = _size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ID3D12Resource* resource = nullptr;

    HRESULT hR = device_->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));

    if (FAILED(hR)){
        Log::Send(Log::Level::ERR, "Failed to create buffer resource");
        Utils::Alert("Failed to create buffer resource");
        assert(false);
    }

    wrapper->Create(resource, D3D12_RESOURCE_STATE_GENERIC_READ);

    return std::move(wrapper);
}

std::unique_ptr<DX12Resource> DirectXAdapter::CreateTextureResource(const DirectX::TexMetadata& _metadata) const {
    /// FLOW  ///
    /// 1. Resource setting from metadata
    /// 2. Heap setting
    /// 3. Generate Resource
    ///

    std::unique_ptr<DX12Resource> wrapper = std::make_unique<DX12Resource>();

    //Step1
    //Setting Resource from Metadata
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = static_cast<UINT>(_metadata.width);
    resourceDesc.Height = static_cast<UINT>(_metadata.height);
    resourceDesc.MipLevels = static_cast<UINT16>(_metadata.mipLevels);
    resourceDesc.DepthOrArraySize = static_cast<UINT16>(_metadata.arraySize);
    resourceDesc.Format = _metadata.format;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(_metadata.dimension);

    //Step2
    //HEAP SETTINGs
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    //heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    //heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

    //Step3
    //Generate Resource
    ID3D12Resource* resource = nullptr;

    HRESULT hr = device_->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&resource)
    );

    if (FAILED(hr)){
        Log::Send(Log::Level::ERR, "Failed to create texture resource");
        Utils::Alert("Failed to create texture resource");
        assert(false);
    }

    wrapper->Create(resource, D3D12_RESOURCE_STATE_COPY_DEST);

    return std::move(wrapper);
}

std::unique_ptr<DX12Resource> DirectXAdapter::CreateDepthStencilResource(int32_t _width, int32_t _height) const {
    std::unique_ptr<DX12Resource> wrapper = std::make_unique<DX12Resource>();
    D3D12_RESOURCE_DESC desc{};
    desc.Width = _width;
    desc.Height = _height;
    desc.MipLevels = 1;
    desc.DepthOrArraySize = 1;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES properties{};
    properties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.DepthStencil.Depth = 1.f;
    clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    ID3D12Resource* resource = nullptr;
    HRESULT hr = device_->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&resource));
    if (FAILED(hr)){
        Log::Send(Log::Level::ERR, "Failed to create depth stencil resource");
        Utils::Alert("Failed to create depth stencil resource");
        assert(false);
    }

    wrapper->Create(resource, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    return std::move(wrapper);
}

std::unique_ptr<DX12Resource> DirectXAdapter::CreateRenderTextureResource(uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _cc) const {
    std::unique_ptr<DX12Resource> wrapper = std::make_unique<DX12Resource>();
    D3D12_RESOURCE_DESC desc{};
    desc.Width = _width;
    desc.Height = _height;
    desc.MipLevels = 1;
    desc.DepthOrArraySize = 1;
    desc.Format = _format;
    desc.SampleDesc.Count = 1;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_HEAP_PROPERTIES properties{};
    properties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Color[0] = _cc.x;
    clearValue.Color[1] = _cc.y;
    clearValue.Color[2] = _cc.z;
    clearValue.Color[3] = _cc.w;
    clearValue.Format = _format;

    ID3D12Resource* resource = nullptr;
    HRESULT hr = device_->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&resource));
    if (FAILED(hr)){
        Log::Send(Log::Level::ERR, "Failed to create render texture resource");
        Utils::Alert("Failed to create render texture resource");
        assert(false);
    }

    wrapper->Create(resource, D3D12_RESOURCE_STATE_RENDER_TARGET);

    return std::move(wrapper);
}

std::unique_ptr<DX12Resource> DirectXAdapter::CreateUnorderedAccessView() const {
    std::unique_ptr<DX12Resource> wrapper = std::make_unique<DX12Resource>();

    D3D12_RESOURCE_DESC desc {};
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_HEAP_PROPERTIES properties{};
    properties.Type = D3D12_HEAP_TYPE_DEFAULT;

    ID3D12Resource* resource = nullptr;
    HRESULT hr = device_->CreateCommittedResource(
        &properties,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(&resource)
    );
    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR, "Failed to create unordered access view");
        Utils::Alert("Failed to create unordered access view");
        throw std::runtime_error("Failed to create unordered access view");
    }

    wrapper->Create(resource, D3D12_RESOURCE_STATE_COMMON);
    return std::move(wrapper);
}

// without render target settings
void DirectXAdapter::PreProcess() const {
    cList_->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

    cList_->RSSetViewports(1, &viewport_);
    cList_->RSSetScissorRects(1, &scissorRect_);
}

void DirectXAdapter::BeginFrame() {
    isRunning_ = true;
    SetSwapChainRenderTarget();
}

void DirectXAdapter::EndFrame() {
    Present();
    isRunning_ = false;
}

void DirectXAdapter::DisplayFPS(const GESTD::WeakPtr<DebugUI>& _debug) const {
    _debug->RegisterCommand(
        "FPS",
        [this]() {
            ImGui::Begin("FPS");
            ImGui::ProgressBar(fpsLimiter_->GetCurrentFps() / fpsLimiter_->GetMaxFps());
            ImGui::End();
        }
    );
}

void DirectXAdapter::SetSwapChainRenderTarget() const {
    UINT bbi = swapChain_->GetCurrentBackBufferIndex();

    swapChainResources_[bbi]->ChangeState(cList_.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    cList_->OMSetRenderTargets(1, &rtvHandles_[bbi], false, &dsvHandle_);
    cList_->ClearRenderTargetView(rtvHandles_[bbi], &back_.x, 0, nullptr);
    cList_->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

    // デバッグ: 実際のSwapChainバッファサイズを確認
    static bool firstFrame = true;
    if (firstFrame) {
        D3D12_RESOURCE_DESC desc = swapChainResources_[bbi]->Get()->GetDesc();
        Log::Send(Log::Level::INFO,
            "[RENDER] SwapChain buffer actual size: " +
            std::to_string(desc.Width) + "x" + std::to_string(desc.Height));
        Log::Send(Log::Level::INFO,
            "[RENDER] Viewport size: " +
            std::to_string(static_cast<int>(viewport_.Width)) + "x" + std::to_string(static_cast<int>(viewport_.Height)));
        firstFrame = false;
    }

    cList_->RSSetViewports(1, &viewport_);
    cList_->RSSetScissorRects(1, &scissorRect_);
}

void DirectXAdapter::Present() {
    UINT bbi = swapChain_->GetCurrentBackBufferIndex();

    // PostDraw
    swapChainResources_[bbi]->ChangeState(cList_.Get(), D3D12_RESOURCE_STATE_PRESENT);

    if (FAILED(cList_->Close())){
        Utils::Alert("Failed to close command list");
        return;
    }
    ID3D12CommandList* commandLists[] = { cList_.Get() };
    cQueue_->ExecuteCommandLists(_countof(commandLists), commandLists);
    swapChain_->Present(1, 0);

    Wait();

    if (FAILED(cAllocator_->Reset())){
        Utils::Alert("Failed to reset command allocator");
        return;
    }
    if (FAILED(cList_->Reset(cAllocator_.Get(), nullptr))){
        Utils::Alert("Failed to reset command list");
        return;
    }
}

void DirectXAdapter::EnableDebugLayer() {
#ifdef _DEBUG
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer_)))){
        debugLayer_->EnableDebugLayer();
        debugLayer_->SetEnableGPUBasedValidation(true);
        debugLayer_->SetEnableSynchronizedCommandQueueValidation(true);
        Log::Send(Log::Level::INFO, "Debug Layer Enabled");
    } else{
        Log::Send(Log::Level::ERR, "Failed to enable debug layer");
    }
#endif
}

bool DirectXAdapter::CreateDXGI() {
    Log::Send(Log::Level::INFO, "Create DXGI");

    //Factory
    if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory_)))){
        return false;
    }

    Log::Send(Log::Level::INFO, "Factory Created");

    //Adaptor
    for (UINT i = 0; SUCCEEDED(factory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter_))); ++i){
        // Check if the adapter supports Direct3D 12
        DXGI_ADAPTER_DESC3 desc;
        if (FAILED(adapter_->GetDesc3(&desc))){
            adapter_ = nullptr;
        }
        if (!(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)){
            break;
        }
        adapter_ = nullptr;
    }

    if (adapter_ == nullptr)return false;

    Log::Send(Log::Level::INFO, "Adapter Created");

    //Device
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0
    };

    Log::Send(Log::Level::INFO, "Device Loop Begin");
    for (size_t i = 0; i < _countof(featureLevels); ++i){
        const char* featureLevelNames[] = {
            "12.2",
            "12.1",
            "12.0"
        };
        if (SUCCEEDED(D3D12CreateDevice(adapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_)))){
            Log::Send(Log::Level::INFO, "Device Created");
            Log::Send(Log::Level::INFO, "Feature Level: " + std::string(featureLevelNames[i]));
            break;
        }

        Log::Send(Log::Level::ERR, "Failed to create device with feature level: " + std::string(featureLevelNames[i]));
    }

    if (device_ == nullptr){
        Log::Send(Log::Level::ERR, "Failed to create device");
        return false;
    }

    Log::Send(Log::Level::INFO, "Complete create DXGI");
    return true;
}

bool DirectXAdapter::InfoQueue() const {
#ifdef _DEBUG
    {
        Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))){
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

            D3D12_MESSAGE_ID denyIds[] = { D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE };
            D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
            D3D12_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(denyIds);
            filter.DenyList.pIDList = denyIds;
            filter.DenyList.NumSeverities = _countof(severities);
            filter.DenyList.pSeverityList = severities;
            infoQueue->PushStorageFilter(&filter);
        }
    }
#endif
    return true;
}

bool DirectXAdapter::CreateCommand() {
    //Queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    if (FAILED(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cQueue_)))){
        Log::Send(Log::Level::ERR, "Failed to create command queue");
        return false;
    }
    Log::Send(Log::Level::INFO, "Command Queue Created");

    //Allocator
    if (FAILED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cAllocator_)))){
        Log::Send(Log::Level::ERR, "Failed to create command allocator");
        return false;
    }
    Log::Send(Log::Level::INFO, "Command Allocator Created");

    //List
    if (FAILED(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cAllocator_.Get(), nullptr, IID_PPV_ARGS(&cList_)))){
        Log::Send(Log::Level::ERR, "Failed to create command list");
        return false;
    }
    Log::Send(Log::Level::INFO, "Command List Created");

    Log::Send(Log::Level::INFO, "Complete create Commands");
    return true;
}

bool DirectXAdapter::CreateSwapChain() {
    Log::Send(Log::Level::INFO, "Create Swap Chain BEGIN");
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = static_cast<UINT>(windowSize_.first);
    desc.Height = static_cast<UINT>(windowSize_.second);
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    Log::Send(Log::Level::INFO, "CreateSwapChainForHwnd");
    if (FAILED(factory_->CreateSwapChainForHwnd(cQueue_.Get(), hWnd_, &desc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf())))){
        Log::Send(Log::Level::ERR, "Failed to create swap chain");
        return false;
    }

    Log::Send(Log::Level::INFO, "Swap Chain Created");

    return true;
}

bool DirectXAdapter::CreateFence() {
    if (FAILED(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)))){
        Log::Send(Log::Level::ERR, "Failed to create fence");
        return false;
    }
    fenceEvent_ = CreateEvent(nullptr, false, false, nullptr);

    Log::Send(Log::Level::INFO, "Fence Created");
    return true;
}

bool DirectXAdapter::CreateRTV() {
    // RTVヒープは初回のみ作成、リサイズ時は既存のものを再利用
    if (!rtvHeap_) {
        rtvHeap_ = std::make_unique<Heap>();
        if (!rtvHeap_->Create(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)){
            Log::Send(Log::Level::ERR, "Failed to create RTV Heap");
            return false;
        }
        Log::Send(Log::Level::INFO, "RTV Heap Created");
    }

    // SwapChainバッファを取得してラップ
    swapChainResources_.resize(2);
    for (UINT i = 0; i < 2; ++i){
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        if (FAILED(swapChain_->GetBuffer(i, IID_PPV_ARGS(&resource)))){
            Log::Send(Log::Level::ERR, "Failed to get swap chain buffer " + std::to_string(i));
            return false;
        }

        // 新しいバッファサイズを確認
        D3D12_RESOURCE_DESC desc = resource->GetDesc();
        Log::Send(Log::Level::INFO,
            "SwapChain buffer " + std::to_string(i) + " size: " +
            std::to_string(desc.Width) + "x" + std::to_string(desc.Height));

        swapChainResources_[i] = std::make_unique<DX12Resource>();
        swapChainResources_[i]->Create(resource);
        swapChainResources_[i]->Get()->SetName(std::wstring(L"SwapChain" + std::to_wstring(i)).c_str());
    }
    Log::Send(Log::Level::INFO, "Swap Chain Resources Created");

    // RTVディスクリプタを作成（既存のヒープに上書き）
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    rtvHandles_.resize(2);
    rtvHandles_[0] = rtvHeap_->Get()->GetCPUDescriptorHandleForHeapStart();
    device_->CreateRenderTargetView(swapChainResources_[0]->Get(), &rtvDesc, rtvHandles_[0]);

    rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    device_->CreateRenderTargetView(swapChainResources_[1]->Get(), &rtvDesc, rtvHandles_[1]);

    Log::Send(Log::Level::INFO, "RTVs Created");
    return true;
}

bool DirectXAdapter::CreateDSV() {
    // 新しいサイズで深度ステンシルリソースを作成
    depthStencil_ = CreateDepthStencilResource(static_cast<int32_t>(windowSize_.first), static_cast<int32_t>(windowSize_.second));

    if (!depthStencil_) {
        Log::Send(Log::Level::ERR, "Failed to create depth stencil resource");
        return false;
    }

    Log::Send(Log::Level::INFO,
        "Depth stencil buffer created: " + std::to_string(windowSize_.first) + "x" + std::to_string(windowSize_.second));

    // DSVヒープは初回のみ作成、リサイズ時は既存のものを再利用
    if (!dsvHeap_) {
        dsvHeap_ = std::make_unique<Heap>();
        if (!dsvHeap_->Create(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE)){
            Log::Send(Log::Level::ERR, "Failed to create DSV Heap");
            return false;
        }
        Log::Send(Log::Level::INFO, "DSV Heap Created");
    }

    // DSVディスクリプタを作成（既存のヒープに上書き）
    D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    device_->CreateDepthStencilView(depthStencil_->Get(), &desc, dsvHeap_->Get()->GetCPUDescriptorHandleForHeapStart());
    dsvHandle_ = dsvHeap_->GetCPUHandle(0);

    Log::Send(Log::Level::INFO, "DSV Created");
    return true;
}

bool DirectXAdapter::CreateViewportAndScissor() {
    viewport_.Width = static_cast<float>(windowSize_.first);
    viewport_.Height = static_cast<float>(windowSize_.second);
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
    viewport_.TopLeftX = 0.0f;
    viewport_.TopLeftY = 0.0f;

    scissorRect_.left = 0;
    scissorRect_.right = static_cast<LONG>(windowSize_.first);
    scissorRect_.top = 0;
    scissorRect_.bottom = static_cast<LONG>(windowSize_.second);
    Log::Send(Log::Level::INFO, "Viewport and Scissor Rect Created");
    return true;
}

void DirectXAdapter::SyncGPU() {
    // GPU同期（コマンドキューを完全にフラッシュ）
    uint64_t fenceValue = GetNextFenceValue();
    cQueue_->Signal(fence_.Get(), fenceValue);
    WaitForFenceValue(fenceValue);
    Log::Send(Log::Level::INFO, "GPU synchronized");
}

void DirectXAdapter::ReleaseSwapChainResources() {
    // SwapChainバッファへの参照を完全に解放
    for (auto& resource : swapChainResources_) {
        if (resource) {
            resource.reset();
        }
    }
    swapChainResources_.clear();
    rtvHandles_.clear();

    // 深度ステンシルバッファを解放
    if (depthStencil_) {
        depthStencil_.reset();
    }

    Log::Send(Log::Level::INFO, "SwapChain resources released");
}

void DirectXAdapter::UpdateWindowSize(size_t _width, size_t _height) {
    Log::Send(Log::Level::INFO,
        "UpdateWindowSize called: " + std::to_string(_width) + "x" + std::to_string(_height) +
        " (current: " + std::to_string(windowSize_.first) + "x" + std::to_string(windowSize_.second) + ")");

    // サイズが変わらない場合は何もしない
    if (_width == windowSize_.first && _height == windowSize_.second) {
        Log::Send(Log::Level::WARNING, "Window size _unchanged, skipping resize");
        return;
    }

    // 最小サイズ制約
    _width = std::max(_width, static_cast<size_t>(1));
    _height = std::max(_height, static_cast<size_t>(1));

    Log::Send(Log::Level::INFO,
        "*** STARTING RESIZE: " + std::to_string(windowSize_.first) + "x" + std::to_string(windowSize_.second) +
        " → " + std::to_string(_width) + "x" + std::to_string(_height) + " ***");

    // RAII化：GPU同期とリソース解放を専用メソッドで実行
    SyncGPU();
    ReleaseSwapChainResources();

    Log::Send(Log::Level::INFO, "Resizing swap chain...");

    // SwapChainをリサイズ
    HRESULT hr = swapChain_->ResizeBuffers(
        2,                              // バッファ数
        static_cast<UINT>(_width),      // 新しい幅
        static_cast<UINT>(_height),     // 新しい高さ
        DXGI_FORMAT_R8G8B8A8_UNORM,    // フォーマット
        0                               // フラグ
    );

    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR,
            "Failed to resize swap chain buffers - HRESULT: 0x" +
            std::to_string(static_cast<unsigned long>(hr)));

        // デバイス削除エラーの特別処理
        if (hr == DXGI_ERROR_DEVICE_REMOVED) {
            HRESULT reason = device_->GetDeviceRemovedReason();
            Log::Send(Log::Level::ERR,
                "Device removed! Reason: 0x" + std::to_string(static_cast<unsigned long>(reason)));
        }

        Utils::Alert("Failed to resize swap chain");
        return;
    }

    Log::Send(Log::Level::INFO, "SwapChain resized successfully");

    // ウィンドウサイズを更新
    windowSize_ = {_width, _height};

    // RTVを再作成
    if (!CreateRTV()) {
        Log::Send(Log::Level::ERR, "Failed to recreate RTV after resize");
        Utils::Alert("Failed to recreate RTV");
        return;
    }

    // 深度ステンシルバッファを再作成
    if (!CreateDSV()) {
        Log::Send(Log::Level::ERR, "Failed to recreate DSV after resize");
        Utils::Alert("Failed to recreate DSV");
        return;
    }

    // ビューポートとシザー矩形を更新
    viewport_.Width = static_cast<float>(_width);
    viewport_.Height = static_cast<float>(_height);
    scissorRect_.right = static_cast<LONG>(_width);
    scissorRect_.bottom = static_cast<LONG>(_height);

    Log::Send(Log::Level::INFO, "Window resize completed successfully");
}

bool DirectXAdapter::CreateLimiter() {
    fpsLimiter_ = std::make_unique<FrameRateLimiter>(static_cast<uint16_t>(60), true); // 60 FPS with VSync enabled

    return fpsLimiter_ != nullptr;
}

void DirectXAdapter::Wait() {
    uint64_t currentFenceValue = GetNextFenceValue();
    cQueue_->Signal(fence_.Get(), currentFenceValue);
    WaitForFenceValue(currentFenceValue);
    fpsLimiter_->WaitForNextFrame();
}

ID3D12Device* DirectXAdapter::GetDevice() const {
    return device_.Get();
}

ID3D12GraphicsCommandList* DirectXAdapter::GetCommandList() const {
    return cList_.Get();
}

size_t DirectXAdapter::GetWidth() const {
    return windowSize_.first;
}

size_t DirectXAdapter::GetHeight() const {
    return windowSize_.second;
}

ID3D12CommandQueue* DirectXAdapter::GetCommandQueue() const {
    return cQueue_.Get();
}

ID3D12CommandAllocator* DirectXAdapter::GetCommandAllocator() const {
    return cAllocator_.Get();
}

IDXGISwapChain4* DirectXAdapter::GetSwapChain() const {
    return swapChain_.Get();
}

ID3D12Fence* DirectXAdapter::GetFence() const {
    return fence_.Get();
}

HWND DirectXAdapter::GetWindowHandle() const {
    return hWnd_;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXAdapter::GetDSVHandle() const {
    return dsvHandle_;
}

uint64_t DirectXAdapter::GetNextFenceValue() {
    return ++fenceValue_;
}

void DirectXAdapter::WaitForFenceValue(const uint64_t _fenceValue) const {
    if (fence_->GetCompletedValue() < _fenceValue) {
        HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);
        fence_->SetEventOnCompletion(_fenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
        CloseHandle(fenceEvent);
    }
}
