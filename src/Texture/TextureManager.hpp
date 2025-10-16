#pragma once
#include <d3d12.h>
#include <mutex>
#include <wrl/client.h>
#include <string>
#include <unordered_map>
#include <memory>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/Resource/DX12Resource.hpp"

#include "vendor/DirectXTex/DirectXTex.h"

/// <summary>
/// テクスチャ管理クラス
/// テクスチャの読み込み、キャッシュ、GPUアップロードを管理
/// </summary>
class TextureManager{
    /// <summary>
    /// テクスチャデータ
    /// </summary>
    struct Texture{
        uint32_t srvIndex;
        DirectX::TexMetadata metadata;
        std::unique_ptr<DX12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    };

private: //Variables
    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;

    // テクスチャアップロード専用のコマンドオブジェクト
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadCommandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCommandList_;

    std::mutex mutex_;

    std::string folderPath_ = "Assets/Resources/";

    std::unordered_map<std::string, Texture> textures_;

public:
    ~TextureManager();
    void Initialize(DirectXAdapter* _adapter, SRVManager* _srv);
    void Load(const std::string& fileName);

    //All Unload(Clear)
    void Unload();

    const DirectX::TexMetadata& GetTextureMetadata(const std::string& fileName);
    uint32_t GetSrvIndex(const std::string& fileName);

    uint32_t GetTextureIndexByFilePath(const std::string& path) const;

    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const std::string& fileName);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const uint32_t index) const;

private: //Methods
    DirectX::ScratchImage LoadTexture(const std::string& filename) const;
    void UploadTextureData(DX12Resource* _texture, const DirectX::ScratchImage& mipImages) const;

    static DirectX::ScratchImage LoadDDS(const std::wstring& _path);
};

