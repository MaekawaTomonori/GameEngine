#ifndef TEXTUREMANAGER_HPP_
#define TEXTUREMANAGER_HPP_

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

/** @brief テクスチャ管理クラス
 ** テクスチャの読み込み、キャッシュ、GPUアップロードを管理
 **/
class TextureManager{
    /** @brief テクスチャデータ
     **/
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

    /** @brief テクスチャマネージャーを初期化
     ** @param _adapter DirectXアダプター
     ** @param _srv SRVマネージャー
     **/
    void Initialize(DirectXAdapter* _adapter, SRVManager* _srv);

    /** @brief テクスチャを読み込み
     ** @param fileName ファイル名
     **/
    bool Load(const std::string& _fileName);

    /** @brief すべてのテクスチャをアンロード（クリア）
     **/
    void Unload();

    /** @brief テクスチャメタデータを取得
     ** @param fileName ファイル名
     ** @return テクスチャメタデータへの参照
     **/
    const DirectX::TexMetadata& GetTextureMetadata(const std::string& _fileName);

    /** @brief SRVインデックスを取得
     ** @param fileName ファイル名
     ** @return SRVインデックス
     **/
    uint32_t GetSrvIndex(const std::string& _fileName);

    /** @brief ファイルパスからテクスチャインデックスを取得
     ** @param path ファイルパス
     ** @return テクスチャインデックス
     **/
    uint32_t GetTextureIndexByFilePath(const std::string& _path) const;

    /** @brief GPUハンドルを取得（ファイル名指定）
     ** @param fileName ファイル名
     ** @return GPUディスクリプタハンドル
     **/
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const std::string& _fileName);

    /** @brief GPUハンドルを取得（インデックス指定）
     ** @param index インデックス
     ** @return GPUディスクリプタハンドル
     **/
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const uint32_t _index) const;

private: //Methods
    DirectX::ScratchImage LoadTexture(const std::string& _filename) const;
    void UploadTextureData(DX12Resource* _texture, const DirectX::ScratchImage& _mipImages) const;

    static DirectX::ScratchImage LoadDDS(const std::wstring& _path);
};

#endif // TEXTUREMANAGER_HPP_
