#include "TextureManager.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <mutex>

#include "Log.hpp"
#include "Utils.hpp"
#include "d3dx12.h"

TextureManager::~TextureManager() {
    Unload();
}

DirectX::ScratchImage TextureManager::DecodeImage(const std::string& _fileName) const {
    DirectX::ScratchImage image {};
    std::string fullPath = folderPath_ + _fileName;
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
    {
        std::lock_guard<std::mutex> lock(mutex_);

        adapter_ = _adapter;
        srv_ = _srv;

        slots_.reserve(MAX_TEXTURE_SLOTS);

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
    }

    const TextureHandle fallback = Request(DEFAULT_TEXTURE);
    if (fallback.IsReady()) {
        defaultId_.store(fallback.GetId(), std::memory_order_release);
    } else {
        Log::Send(Log::Level::WARNING, std::format("TextureManager: default texture not available: {}", DEFAULT_TEXTURE));
    }

    Log::Send(Log::Level::INFO, "TextureManager Initialized");
}

std::string TextureManager::NormalizeName(const std::string& _fileName) const {
    std::string name = _fileName;
    size_t pos = 0;
    while ((pos = name.find(folderPath_, pos)) != std::string::npos){
        name.erase(pos, folderPath_.length());
    }
    return name;
}

uint32_t TextureManager::ResolveIdUnlocked(const std::string& _normalized) {
    if (const auto it = nameToId_.find(_normalized); it != nameToId_.end()) {
        return it->second;
    }

    if (slots_.size() >= MAX_TEXTURE_SLOTS) {
        Log::Send(Log::Level::ERR, std::format("TextureManager: slot capacity exceeded: {}", _normalized));
        Utils::Alert(std::format("TextureManager: slot capacity exceeded: {}", _normalized));
        return INVALID_SLOT_ID;
    }

    // GetSrvIndexOf()等がロックなしでslots_を添字参照するため、再確保を起こさせない
    if (slots_.capacity() < MAX_TEXTURE_SLOTS) {
        slots_.reserve(MAX_TEXTURE_SLOTS);
    }

    const uint32_t id = static_cast<uint32_t>(slots_.size());

    auto slot = std::make_unique<TextureSlot>();
    slot->name = _normalized;
    slots_.push_back(std::move(slot));

    nameToId_[_normalized] = id;
    slotCount_.store(static_cast<uint32_t>(slots_.size()), std::memory_order_release);

    return id;
}

bool TextureManager::EnsureLoadedUnlocked(const uint32_t _id) {
    if (_id == INVALID_SLOT_ID || _id >= slots_.size()) return false;

    TextureSlot& slot = *slots_[_id];

    switch (slot.state.load(std::memory_order_acquire)) {
        case SlotState::Ready:  return true;
        case SlotState::Failed: return false;
        default: break;
    }

    if (!adapter_ || !srv_) {
        Log::Send(Log::Level::ERR, std::format("TextureManager: not initialized: {}", slot.name));
        slot.state.store(SlotState::Failed, std::memory_order_release);
        return false;
    }

    slot.state.store(SlotState::Loading, std::memory_order_relaxed);

    DirectX::ScratchImage img = DecodeImage(slot.name);

    if (!img.GetImages() || img.GetImageCount() == 0) {
        Log::Send(Log::Level::ERR, std::format("TextureManager: failed to decode: {}", slot.name));
        slot.state.store(SlotState::Failed, std::memory_order_release);
        return false;
    }

    slot.state.store(SlotState::PendingUpload, std::memory_order_relaxed);

    if (srv_->IsFull()) {
        Log::Send(Log::Level::ERR, std::format("TextureManager: SRV heap is full: {}", slot.name));
        Utils::Alert(std::format("TextureManager: SRV heap is full: {}", slot.name));
        slot.state.store(SlotState::Failed, std::memory_order_release);
        return false;
    }

    slot.metadata = img.GetMetadata();
    slot.resource = adapter_->CreateTextureResource(slot.metadata);

    if (!slot.resource) {
        Log::Send(Log::Level::ERR, std::format("TextureManager: failed to create resource: {}", slot.name));
        slot.state.store(SlotState::Failed, std::memory_order_release);
        return false;
    }

    UploadTextureData(slot.resource.get(), img);

    slot.srvIndex = srv_->Allocate();
    slot.cpuHandle = srv_->GetCPUHandle(slot.srvIndex);
    slot.gpuHandle = srv_->GetGPUHandle(slot.srvIndex);

    if (slot.metadata.IsCubemap()) {
        srv_->CreateSRVForCubeMap(slot.srvIndex, slot.resource->Get(), slot.metadata.format);
    } else {
        srv_->CreateSRVForTexture2D(slot.srvIndex, slot.resource->Get(), slot.metadata.format, static_cast<UINT>(slot.metadata.mipLevels));
    }

    // Ready への遷移をreleaseにすることで、ロックなしの読み手がacquireで
    // srvIndex/gpuHandle を安全に読める
    slot.state.store(SlotState::Ready, std::memory_order_release);

    Log::Send(Log::Level::INFO, std::format("TextureManager::Load: {}", slot.name));
    return true;
}

TextureHandle TextureManager::Request(const std::string& _fileName) {
    const std::string name = NormalizeName(_fileName);

    std::lock_guard<std::mutex> lock(mutex_);

    const uint32_t id = ResolveIdUnlocked(name);
    if (id == INVALID_SLOT_ID) return {};

    EnsureLoadedUnlocked(id);
    return TextureHandle(id);
}

bool TextureManager::Load(const std::string& _fileName) {
    return Request(_fileName).IsReady();
}

const TextureManager::TextureSlot* TextureManager::FindReadySlot(const uint32_t _id) const {
    const uint32_t count = slotCount_.load(std::memory_order_acquire);

    if (_id < count) {
        const TextureSlot* slot = slots_[_id].get();
        if (slot->state.load(std::memory_order_acquire) == SlotState::Ready) return slot;
    }

    const uint32_t fallback = defaultId_.load(std::memory_order_acquire);
    if (fallback < count) {
        const TextureSlot* slot = slots_[fallback].get();
        if (slot->state.load(std::memory_order_acquire) == SlotState::Ready) return slot;
    }

    return nullptr;
}

uint32_t TextureManager::GetSrvIndexOf(const uint32_t _id) const {
    const TextureSlot* slot = FindReadySlot(_id);
    return slot ? slot->srvIndex.Get() : 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGpuHandleOf(const uint32_t _id) const {
    const TextureSlot* slot = FindReadySlot(_id);
    return slot ? slot->gpuHandle : D3D12_GPU_DESCRIPTOR_HANDLE{};
}

const DirectX::TexMetadata& TextureManager::GetMetadataOf(const uint32_t _id) const {
    static const DirectX::TexMetadata EMPTY{};

    const TextureSlot* slot = FindReadySlot(_id);
    return slot ? slot->metadata : EMPTY;
}

TextureState TextureManager::GetStateOf(const uint32_t _id) const {
    if (_id >= slotCount_.load(std::memory_order_acquire)) return TextureState::Failed;

    switch (slots_[_id]->state.load(std::memory_order_acquire)) {
        case SlotState::Ready:  return TextureState::Ready;
        case SlotState::Failed: return TextureState::Failed;
        default: return TextureState::NotReady;
    }
}

uint32_t TextureManager::GetDefaultSrvIndex() const {
    return GetSrvIndexOf(INVALID_SLOT_ID);
}

bool TextureManager::LoadFromRawPixels(const std::string& _name, const uint8_t* _pixels, uint32_t _width, uint32_t _height, DXGI_FORMAT _format) {
    const std::string name = NormalizeName(_name);

    std::lock_guard<std::mutex> lock(mutex_);

    if (!adapter_ || !srv_) {
        Log::Send(Log::Level::ERR, std::format("TextureManager::LoadFromRawPixels: not initialized: {}", name));
        return false;
    }

    const uint32_t id = ResolveIdUnlocked(name);
    if (id == INVALID_SLOT_ID) return false;

    TextureSlot& slot = *slots_[id];

    // 既存スロットはSRVスロットを使い回すため枯渇しない
    if (!slot.srvIndex.IsValid() && srv_->IsFull()) {
        Log::Send(Log::Level::ERR, std::format("TextureManager::LoadFromRawPixels: SRV heap is full: {}", name));
        Utils::Alert(std::format("TextureManager::LoadFromRawPixels: SRV heap is full: {}", name));
        return false;
    }

    // ScratchImage を確保してピクセルをコピー
    DirectX::ScratchImage img;
    HRESULT hr = img.Initialize2D(_format, _width, _height, 1, 1);
    if (FAILED(hr)) {
        Log::Send(Log::Level::ERR, std::format("TextureManager::LoadFromRawPixels: ScratchImage init failed: {}", name));
        return false;
    }

    const DirectX::Image* imageSlice = img.GetImage(0, 0, 0);
    if (!imageSlice) {
        Log::Send(Log::Level::ERR, std::format("TextureManager::LoadFromRawPixels: internal ScratchImage error: {}", name));
        return false;
    }

    // rowPitch が GPU アライメントで広い場合に対応して行ごとにコピー
    size_t srcRowPitch = static_cast<size_t>(_width) * DirectX::BitsPerPixel(_format) / 8;
    for (uint32_t row = 0; row < _height; ++row) {
        memcpy(imageSlice->pixels + row * imageSlice->rowPitch,
               _pixels           + row * srcRowPitch,
               srcRowPitch);
    }

    // 生成・アップロードが完了するまでは既存の内容に触れない
    std::unique_ptr<DX12Resource> resource = adapter_->CreateTextureResource(img.GetMetadata());
    if (!resource) {
        Log::Send(Log::Level::ERR, std::format("TextureManager::LoadFromRawPixels: CreateTextureResource failed: {}", name));
        return false;
    }

    UploadTextureData(resource.get(), img);

    // UploadTextureData 内でフェンス待ち済みのため旧リソースは解放してよい
    slot.metadata = img.GetMetadata();
    slot.resource = std::move(resource);

    if (!slot.srvIndex.IsValid()) {
        slot.srvIndex = srv_->Allocate();
        slot.cpuHandle = srv_->GetCPUHandle(slot.srvIndex);
        slot.gpuHandle = srv_->GetGPUHandle(slot.srvIndex);
    }

    srv_->CreateSRVForTexture2D(slot.srvIndex, slot.resource->Get(), _format, 1);

    slot.state.store(SlotState::Ready, std::memory_order_release);

    Log::Send(Log::Level::INFO, std::format("TextureManager::LoadFromRawPixels: {}", name));
    return true;
}

void TextureManager::Unload() {
    std::lock_guard<std::mutex> lock(mutex_);

    defaultId_.store(INVALID_SLOT_ID, std::memory_order_release);
    slotCount_.store(0, std::memory_order_release);

    nameToId_.clear();
    slots_.clear();
}

const DirectX::TexMetadata& TextureManager::GetTextureMetadata(const std::string& _fileName) {
    const std::string name = NormalizeName(_fileName);

    std::lock_guard<std::mutex> lock(mutex_);

    const uint32_t id = ResolveIdUnlocked(name);
    if (EnsureLoadedUnlocked(id)) {
        return slots_[id]->metadata;
    }

    Log::Send(Log::Level::ERR, std::format("TextureManager::GetTextureMetadata: {} not found", name));
    static const DirectX::TexMetadata EMPTY{};
    return EMPTY;
}

uint32_t TextureManager::GetSrvIndex(const std::string& _fileName) {
    const std::string name = NormalizeName(_fileName);

    std::lock_guard<std::mutex> lock(mutex_);

    if (const auto it = nameToId_.find(name); it != nameToId_.end()) {
        const TextureSlot& slot = *slots_[it->second];
        if (slot.state.load(std::memory_order_acquire) == SlotState::Ready) {
            return slot.srvIndex.Get();
        }
    }

    Log::Send(Log::Level::ERR, std::format("TextureManager::GetSrvIndex: {} not found", name));
    return GetDefaultSrvIndex();
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& _path) const {
    const std::string name = NormalizeName(_path);

    std::lock_guard<std::mutex> lock(mutex_);

    if (const auto it = nameToId_.find(name); it != nameToId_.end()) {
        const TextureSlot& slot = *slots_[it->second];
        if (slot.state.load(std::memory_order_acquire) == SlotState::Ready) {
            return slot.srvIndex.Get();
        }
    }

    Log::Send(Log::Level::ERR, std::format("TextureManager::GetTextureIndexByFilePath: {} not found", name));
    return GetDefaultSrvIndex();
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGPUHandle(const std::string& _fileName) {
    const std::string name = NormalizeName(_fileName);

    std::lock_guard<std::mutex> lock(mutex_);

    if (const auto it = nameToId_.find(name); it != nameToId_.end()) {
        const TextureSlot& slot = *slots_[it->second];
        if (slot.state.load(std::memory_order_acquire) == SlotState::Ready) {
            return slot.gpuHandle;
        }
    }

    Log::Send(Log::Level::ERR, std::format("TextureManager::GetGPUHandle: {} not found", name));
    return GetGpuHandleOf(INVALID_SLOT_ID);
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGPUHandle(const uint32_t _index) const {
    if (!srv_) return {};
    return srv_->GetGPUHandle(_index);
}

ID3D12Resource* TextureManager::GetResource(const std::string& _name) const {
    const std::string name = NormalizeName(_name);

    std::lock_guard<std::mutex> lock(mutex_);

    if (const auto it = nameToId_.find(name); it != nameToId_.end()) {
        const TextureSlot& slot = *slots_[it->second];
        if (slot.state.load(std::memory_order_acquire) == SlotState::Ready && slot.resource) {
            return slot.resource->Get();
        }
    }
    return nullptr;
}

std::vector<std::string> TextureManager::ListAvailableTextures() const {
    namespace fs = std::filesystem;

    std::vector<std::string> result;

    std::error_code ec;
    const fs::path root = folderPath_;
    if (!fs::exists(root, ec) || !fs::is_directory(root, ec)) return result;

    for (const auto& entry : fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec)) {
        if (ec) break;
        if (!entry.is_regular_file(ec)) continue;

        const std::string ext = entry.path().extension().string();
        const bool isImage = ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".dds" || ext == ".tga";
        if (!isImage) continue;

        const fs::path relative = fs::relative(entry.path(), root, ec);
        if (ec) continue;
        result.push_back(relative.generic_string());
    }

    std::sort(result.begin(), result.end());
    return result;
}

std::vector<std::string> TextureManager::GetLoadedTextureNames() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> result;
    result.reserve(slots_.size());

    for (const auto& slot : slots_) {
        if (slot->state.load(std::memory_order_acquire) == SlotState::Ready) {
            result.push_back(slot->name);
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}
