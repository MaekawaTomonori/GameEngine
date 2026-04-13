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

#include "DirectXTex.h"

/** @brief テクスチャ管理クラス
 * テクスチャの読み込み、キャッシュ、GPUアップロードを管理
 */
class TextureManager{
    /** @brief テクスチャデータ
     */
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

    /** テクスチャアップロード専用のコマンドオブジェクト
     */
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadCommandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCommandList_;

    std::mutex mutex_;

    std::string folderPath_ = "Assets/Resources/";

    std::unordered_map<std::string, Texture> textures_;

public:
    ~TextureManager();

    /** @brief テクスチャマネージャーを初期化
     * @param _adapter DirectXアダプター
     * @param _srv SRVマネージャー
     */
    void Initialize(DirectXAdapter* _adapter, SRVManager* _srv);

    /** @brief テクスチャを読み込み
     * @param fileName ファイル名
     */
    bool Load(const std::string& _fileName);

    /** @brief 生ピクセルデータからテクスチャを登録
     * @param _name  テクスチャを識別するキー（ファイル名の代わり）
     * @param _pixels ピクセルデータ（フォーマットに合うバイト列）
     * @param _width  横幅
     * @param _height 縦幅
     * @param _format DXGI フォーマット（デフォルト RGBA8、SDF アトラスは R8_UNORM を指定）
     * @return 成功なら true（既登録キーは true を返してスキップ）
     */
    bool LoadFromRawPixels(const std::string& _name, const uint8_t* _pixels,
                           uint32_t _width, uint32_t _height,
                           DXGI_FORMAT _format = DXGI_FORMAT_R8G8B8A8_UNORM);

    /** @brief すべてのテクスチャをアンロード（クリア）
     */
    void Unload();

    /** @brief テクスチャメタデータを取得
     * @param fileName ファイル名
     * @return テクスチャメタデータへの参照
     */
    const DirectX::TexMetadata& GetTextureMetadata(const std::string& _fileName);

    /** @brief SRVインデックスを取得
     * @param fileName ファイル名
     * @return SRVインデックス
     */
    uint32_t GetSrvIndex(const std::string& _fileName);

    /** @brief ファイルパスからテクスチャインデックスを取得
     * @param path ファイルパス
     * @return テクスチャインデックス
     */
    uint32_t GetTextureIndexByFilePath(const std::string& _path) const;

    /** @brief GPUハンドルを取得（ファイル名指定）
     * @param fileName ファイル名
     * @return GPUディスクリプタハンドル
     */
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const std::string& _fileName);

    /** @brief GPUハンドルを取得（インデックス指定）
     * @param index インデックス
     * @return GPUディスクリプタハンドル
     */
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(const uint32_t _index) const;

    /** @brief 生 DX12 リソースポインタを返す（デバッグ表示用）
     * @param _name テクスチャキー
     * @return リソースポインタ。未登録の場合 nullptr
     */
    ID3D12Resource* GetResource(const std::string& _name) const;

private: //Methods
    DirectX::ScratchImage LoadTexture(const std::string& _filename) const;
    void UploadTextureData(DX12Resource* _texture, const DirectX::ScratchImage& _mipImages) const;

    static DirectX::ScratchImage LoadDDS(const std::wstring& _path);
};

#endif // TEXTUREMANAGER_HPP_
