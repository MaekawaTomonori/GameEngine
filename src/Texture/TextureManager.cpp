#include "TextureManager.hpp"

#include <format>
#include <mutex>

#include "Log.hpp"
#include "Utils.hpp"
#include "vendor/DirectXTex/d3dx12.h"

TextureManager::~TextureManager() {
    Unload();
}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string& _filename) const {
    DirectX::ScratchImage image {};
    std::string fullPath = folderPath_ + _filename;
    std::wstring filePathW = Utils::Convert(fullPath);

    if (filePathW.ends_with(L".dds"))return LoadDDS(filePathW);

    [[maybe_unused]]HRESULT hr = LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);

    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR, std::format("Failed to load texture file: {}", fullPath));
        Utils::Alert(std::format("Failed to load texture file: {}", fullPath));
        return {};
    }

    DirectX::ScratchImage mipImages {};
    hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);

    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR, std::format("Failed to generate mipmaps for texture file: {}", fullPath));
        Utils::Alert(std::format("Failed to generate mipmaps for texture file: {}", fullPath));
        return {};
    }

    return mipImages;
}

void TextureManager::UploadTextureData(DX12Resource* _texture, const DirectX::ScratchImage& _mipImages) const {
    std::vector<D3D12_SUBRESOURCE_DATA> subResources;
    PrepareUpload(adapter_->GetDevice(), _mipImages.GetImages(), _mipImages.GetImageCount(), _mipImages.GetMetadata(), subResources);
    uint32_t intermediateSize = static_cast<uint32_t>(GetRequiredIntermediateSize(_texture->Get(), 0, static_cast<UINT>(subResources.size())));
    std::unique_ptr<DX12Resource> intermediateResource = adapter_->CreateBufferResource(intermediateSize);

    // 専用のコマンドアロケーターとコマンドリストをリセット
    if (FAILED(uploadCommandAllocator_->Reset())) {
        Utils::Alert("Failed to reset upload command allocator");
        return;
    }
    if (FAILED(uploadCommandList_->Reset(uploadCommandAllocator_.Get(), nullptr))) {
        Utils::Alert("Failed to reset upload command list");
        return;
    }

    UpdateSubresources(uploadCommandList_.Get(), _texture->Get(), intermediateResource->Get(), 0, 0, static_cast<UINT>(subResources.size()), subResources.data());
    _texture->ChangeState(uploadCommandList_.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    if (FAILED(uploadCommandList_->Close())) {
        Utils::Alert("Failed to close upload command list");
        return;
    }

    ID3D12CommandList* cls[] = { uploadCommandList_.Get() };
    adapter_->GetCommandQueue()->ExecuteCommandLists(_countof(cls), cls);

    // 適切なFence値管理を使用してコマンドキューの実行を待つ
    uint64_t fenceValue = adapter_->GetNextFenceValue();
    adapter_->GetCommandQueue()->Signal(adapter_->GetFence(), fenceValue);
    adapter_->WaitForFenceValue(fenceValue);
}

DirectX::ScratchImage TextureManager::LoadDDS(const std::wstring& _path) {
    DirectX::ScratchImage image{};
    HRESULT hr = LoadFromDDSFile(_path.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);

    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR, std::format("Failed to load DDS file: {}", Utils::Convert(_path)));
        Utils::Alert(std::format("Failed to load DDS file: {}", Utils::Convert(_path)));
        return {};
    }

    DirectX::ScratchImage mip;
    if (DirectX::IsCompressed(image.GetMetadata().format)) {
        mip = std::move(image);
    } else {
        hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mip);
        if (FAILED(hr)){
            Log::Send(Log::Level::ERR, std::format("Failed to generate mipmaps for DDS file: {}", Utils::Convert(_path)));
            Utils::Alert(std::format("Failed to generate mipmaps for DDS file: {}", Utils::Convert(_path)));
            return {};
        }
    }

    return mip;
}


void TextureManager::Initialize(DirectXAdapter* _adapter, SRVManager* _srv) {
    std::lock_guard<std::mutex> lock(mutex_);

    adapter_ = _adapter;
    srv_ = _srv;

    // テクスチャアップロード専用のコマンドアロケーターとコマンドリストを作成
    HRESULT hr = adapter_->GetDevice()->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&uploadCommandAllocator_)
    );
    if (FAILED(hr)) {
        Utils::Alert("Failed to create upload command allocator");
        return;
    }

    hr = adapter_->GetDevice()->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        uploadCommandAllocator_.Get(),
        nullptr,
        IID_PPV_ARGS(&uploadCommandList_)
    );
    if (FAILED(hr)) {
        Utils::Alert("Failed to create upload command list");
        return;
    }

    // 初期状態では閉じておく
    uploadCommandList_->Close();

    Log::Send(Log::Level::INFO, "TextureManager Initialized");
}

bool TextureManager::Load(const std::string& _fileName) {
    std::lock_guard<std::mutex> lock(mutex_);

    //Remove FolderPath
    std::string name = _fileName;
    size_t pos = 0;
    while((pos = name.find(folderPath_, pos)) != std::string::npos){
        name.erase(pos, folderPath_.length());
    }

    //Check if texture is already loaded
    if (textures_.contains(name)){
        return true;
    }

    assert(!srv_->IsFull());

    /// FLOW
    /// 1. Load Texture Data in CPU
    /// 2. Create TextureResource (VRAM)
    /// 3. Create UploadHeap Resource (IntermediateResource)
    /// 4. Upload IntermediateResource to CPU
    /// 5. Stack Command (3 -> 2) to CommandList
    /// 6. Execute Using CommandQueue
    /// 7. Wait


    //Load Texture
    DirectX::ScratchImage img = LoadTexture(name);

    if (!img.GetImages() || img.GetImageCount() == 0) {
        Log::Send(Log::Level::ERR, std::format("TextureManager::Load: Failed to load texture: {}", name));
        return false;
    }

    Texture texture;
    texture.metadata = img.GetMetadata();
    texture.resource = adapter_->CreateTextureResource(img.GetMetadata());

    if (!texture.resource) {
        Log::Send(Log::Level::ERR, std::format("TextureManager::Load: Failed to create texture resource: {}", name));
        return false;
    }

    UploadTextureData(texture.resource.get(), img);

    texture.srvIndex = srv_->Allocate();
    texture.cpuHandle = srv_->GetCPUHandle(texture.srvIndex);
    texture.gpuHandle = srv_->GetGPUHandle(texture.srvIndex);

    if (texture.metadata.IsCubemap()) {
        srv_->CreateSRVforCubemap(texture.srvIndex, texture.resource->Get(), texture.metadata.format);
    }else{
        srv_->CreateSRVforTexture2D(texture.srvIndex, texture.resource->Get(), texture.metadata.format, static_cast<UINT>(texture.metadata.mipLevels));
    }

    textures_[name] = std::move(texture);

    Log::Send(Log::Level::INFO, std::format("TextureManager::Load: {}", name));
    return true;
}

void TextureManager::Unload() {
    for (auto itr = textures_.begin(); itr != textures_.end(); ){
        itr->second.resource.reset();

        itr = textures_.erase(itr);
    }
    textures_.clear();
}

const DirectX::TexMetadata& TextureManager::GetTextureMetadata(const std::string& _fileName) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (textures_.contains(_fileName)){
        return textures_.at(_fileName).metadata;
    }

    Load(_fileName);
    Log::Send(Log::Level::ERR, std::format("TextureManager::GetTextureMetadata: {} not found", _fileName));
    Utils::Alert(std::format("TextureManager::GetTextureMetadata: {} not found", _fileName));
    return textures_.at(_fileName).metadata;
}

uint32_t TextureManager::GetSrvIndex(const std::string& _fileName) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (textures_.contains(_fileName)){
        return textures_.at(_fileName).srvIndex;
    }

    Log::Send(Log::Level::ERR, std::format("TextureManager::GetSrvIndex: {} not found", _fileName));
    assert(0);
    return 0;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& _path) const {
    if (textures_.contains(_path)){
        return textures_.at(_path).srvIndex;
    }

    Log::Send(Log::Level::ERR, std::format("TextureManager::GetTextureIndexByFilePath: {} not found", _path));
    assert(0);
    return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGPUHandle(const std::string& _fileName) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string name = _fileName;
    size_t pos = 0;
    while ((pos = name.find(folderPath_, pos)) != std::string::npos){
        name.erase(pos, folderPath_.length());
    }

    if (textures_.contains(name)){
        return textures_.at(name).gpuHandle;
    }

    Log::Send(Log::Level::ERR, std::format("TextureManager::GetGPUHandle: {} not found", name));
    assert(0);
    return {};
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGPUHandle(const uint32_t _index) const {
    assert(index <= textures_.size());
    Log::Send(Log::Level::INFO, std::format("TextureManager::GetGPUHandle: index {}", _index));
    return srv_->GetGPUHandle(_index);
}
