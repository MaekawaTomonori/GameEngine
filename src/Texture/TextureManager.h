#pragma once
#include <d3d12.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <wrl/client.h>

#include "DirectXTex.h"

class DirectXCommon;
class SRVManager;

class TextureManager{
	struct Texture{
        uint32_t srvIndex;
        DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};

private: //Variables
	static TextureManager* instance_;
    static std::once_flag onceFlag_;

    std::weak_ptr<DirectXCommon> dxCommon_;
    SRVManager* srvManager_ = nullptr;

	std::string folderPath_ = "assets/Resources/";

    std::unordered_map<std::string, Texture> textures_;

private: //Methods
	TextureManager() = default;
    ~TextureManager();

	DirectX::ScratchImage LoadTexture(const std::string& filename) const;
	ID3D12Resource* CreateTextureResource(const DirectX::TexMetadata& metadata) const;
	ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) const;

public:
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
	static TextureManager* GetInstance();
	static void Create();
	static void Destroy();

	void Initialize(const std::weak_ptr<DirectXCommon>& dxCommon, SRVManager* srvManager);
	void Load(const std::string& fileName);

	//All Unload(Clear)
    void Unload();

    const DirectX::TexMetadata& GetTextureMetadata(const std::string& fileName) const;
    uint32_t GetSrvIndex(const std::string& fileName) const;

	uint32_t GetTextureIndexByFilePath(const std::string& path) const;

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const std::string& fileName) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const uint32_t index) const;
};

