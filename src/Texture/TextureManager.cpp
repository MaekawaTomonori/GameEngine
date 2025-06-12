#include "TextureManager.h"

#include <format>

#include "DirectX/DirectXCommon.h"
#include "DirectX/Heap/Heap.h"
#include "DirectX/Heap/SRVManager.h"
#include "d3dx12.h"
#include "System/System.h"
#include "System/SingletonFinalizer/SingletonFinalizer.h"

TextureManager* TextureManager::instance_ = nullptr;
std::once_flag TextureManager::onceFlag_;

TextureManager* TextureManager::GetInstance() {
	call_once(onceFlag_, Create);
    assert(instance_);
    return instance_;
}
void TextureManager::Create() {
    instance_ = new TextureManager();
    SingletonFinalizer::AddFinalizer(&Destroy);
    System::Log(Log::Level::INFO, "TextureManager Enabled");
}

void TextureManager::Destroy() {
    delete instance_;
    instance_ = nullptr;
    System::Log(Log::Level::INFO, "TextureManager Disabled");
}

TextureManager::~TextureManager() {
    Unload();
}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filename) const {
    DirectX::ScratchImage image {};
    std::string fullPath = folderPath_ + filename;
    std::wstring filePathW = System::ConvertString(fullPath);
    HRESULT hr = LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
    assert(SUCCEEDED(hr));

    DirectX::ScratchImage mipImages {};
    hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
    assert(SUCCEEDED(hr));

    return mipImages;
}

ID3D12Resource* TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) const {
    /// FLOW  ///
	/// 1. Resource setting from metadata
	/// 2. Heap setting
	/// 3. Generate Resource
	///

	//Step1
	//Setting Resource from Metadata
    D3D12_RESOURCE_DESC resourceDesc {};
    resourceDesc.Width = static_cast<UINT>(metadata.width);
    resourceDesc.Height = static_cast<UINT>(metadata.height);
    resourceDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
    resourceDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
    resourceDesc.Format = metadata.format;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

    //Step2
    //HEAP SETTINGs
    D3D12_HEAP_PROPERTIES heapProperties {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    //heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    //heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

    //Step3
    //Generate Resource
    ID3D12Resource* resource = nullptr;

    #ifdef _DEBUG
    HRESULT hr =
        #endif  
        dxCommon_.lock()->GetDevice()->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&resource)
        );
    assert(SUCCEEDED(hr));

    return resource;
}

[[nodiscard]]
ID3D12Resource* TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) const {
    auto dxc = dxCommon_.lock();
    if (!dxc){
        System::Log(Log::Level::ERR, "SRVManager Initialize Failed");
        assert(0);
    }

	std::vector<D3D12_SUBRESOURCE_DATA> subResources;
    PrepareUpload(dxc->GetDevice().Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subResources);
    uint32_t intermediateSize = static_cast<uint32_t>(GetRequiredIntermediateSize(texture, 0, static_cast<UINT>(subResources.size())));
    ID3D12Resource* intermediateResource = DirectXCommon::CreateBufferResource(dxc->GetDevice(), intermediateSize).Get();
    UpdateSubresources(dxc->GetCommandList().Get(), texture, intermediateResource, 0, 0, static_cast<UINT>(subResources.size()), subResources.data());

    D3D12_RESOURCE_BARRIER barrier {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = texture;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
    dxc->GetCommandList()->ResourceBarrier(1, &barrier);
    return intermediateResource;
}


void TextureManager::Initialize(const std::weak_ptr<DirectXCommon>& dxCommon, SRVManager* srvManager) {
    dxCommon_ = dxCommon;
    srvManager_ = srvManager;
    System::Log(Log::Level::INFO, "TextureManager Initialized");
}

void TextureManager::Load(const std::string& fileName) {
    //Remove FolderPath
    std::string name = fileName;
    size_t pos = 0;
    while((pos = name.find(folderPath_, pos)) != std::string::npos){
        name.erase(pos, folderPath_.length());
    }
    
    //Check if texture is already loaded
	if (textures_.contains(name)){
        return;
    }

    assert(!srvManager_->IsFull());

    
    //Load Texture
    Texture& texture = textures_[name];

    DirectX::ScratchImage img = LoadTexture(name);

    texture.metadata = img.GetMetadata();
    texture.resource = CreateTextureResource(img.GetMetadata());
    texture.intermediateResource = UploadTextureData(texture.resource.Get(), img);

    texture.srvIndex = srvManager_->Allocate();
    texture.cpuHandle = srvManager_->GetCPUHandle(texture.srvIndex);
    texture.gpuHandle = srvManager_->GetGPUHandle(texture.srvIndex);

    srvManager_->CreateSRVforTexture2D(texture.srvIndex, texture.resource.Get(), texture.metadata.format, static_cast<UINT>(texture.metadata.mipLevels));

    System::Log(Log::Level::INFO, std::format("TextureManager::Load: {}", name));
}

void TextureManager::Unload() {
    for (auto itr = textures_.begin(); itr != textures_.end(); ){
        itr->second.resource->Release();
        itr->second.intermediateResource->Release();

        itr = textures_.erase(itr);
    }
    textures_.clear();
}

const DirectX::TexMetadata& TextureManager::GetTextureMetadata(const std::string& fileName) const {
    if (textures_.contains(fileName)){
        return textures_.at(fileName).metadata;
	}

    System::Log(Log::Level::ERR, std::format("TextureManager::GetTextureMetadata: {} not found", fileName));
    assert(0);
    return textures_.at("").metadata;
}

uint32_t TextureManager::GetSrvIndex(const std::string& fileName) const {
    if (textures_.contains(fileName)){
        return textures_.at(fileName).srvIndex;
    }

    System::Log(Log::Level::ERR, std::format("TextureManager::GetSrvIndex: {} not found", fileName));
    assert(0);
    return 0;
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& path) const {
    if (textures_.contains(path)){
        return textures_.at(path).srvIndex;
    }

    System::Log(Log::Level::ERR, std::format("TextureManager::GetTextureIndexByFilePath: {} not found", path));
    assert(0);
    return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGPUHandle(const std::string& fileName) const {
    std::string name = fileName;
    size_t pos = 0;
    while ((pos = name.find(folderPath_, pos)) != std::string::npos){
        name.erase(pos, folderPath_.length());
    }

    if (textures_.contains(name)){
        return textures_.at(name).gpuHandle;
    }

    System::Log(Log::Level::ERR, std::format("TextureManager::GetGPUHandle: {} not found", name));
    assert(0);
    return {};
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGPUHandle(const uint32_t index) const {
    assert(index <= textures_.size());
    System::Log(Log::Level::INFO, std::format("TextureManager::GetGPUHandle: index {}", index));
	return srvManager_->GetGPUHandle(index);
}
