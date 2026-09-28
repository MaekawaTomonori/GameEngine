#ifndef TEXTUREMANAGER_HPP_
#define TEXTUREMANAGER_HPP_

#include <atomic>
#include <d3d12.h>
#include <mutex>
#include <wrl/client.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "Texture/TextureHandle.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVHandle.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/Resource/DX12Resource.hpp"

#include "DirectXTex.h"

/** @brief テクスチャ管理クラス
 * テクスチャの読み込み、キャッシュ、GPUアップロードを管理
 */
class TextureManager{
    /** SRVヒープ容量が上限。slots_はこの数で予約し再確保させない */
    static constexpr uint32_t MAX_TEXTURE_SLOTS = 512;
    static constexpr uint32_t INVALID_SLOT_ID = UINT32_MAX;
    static constexpr const char* DEFAULT_TEXTURE = "white_x16.png";

    /** @brief スロットの内部状態（公開状態より細かい段階を持つ）
     */
    enum class SlotState{
        Queued,
        Loading,
        PendingUpload,
        Ready,
        Failed,
    };

    /** @brief テクスチャ1枚分のスロット
     * ハンドルのidがそのままslots_の添字になる
     */
    struct TextureSlot{
        std::string name;
        SRVHandle srvIndex;
        DirectX::TexMetadata metadata {};
        std::unique_ptr<DX12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle {};
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle {};
        std::atomic<SlotState> state {SlotState::Queued};
    };

private: //Variables
    DirectXAdapter* adapter_ = nullptr;
    SRVManager* srv_ = nullptr;

    /** テクスチャアップロード専用のコマンドオブジェクト
     */
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadCommandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCommandList_;

    mutable std::mutex mutex_;

    std::string folderPath_ = "Assets/Resources/";

    std::vector<std::unique_ptr<TextureSlot>> slots_;

    /** slots_の確定要素数。GetSrvIndexOf()がロックなしで境界を判定するために持つ */
    std::atomic<uint32_t> slotCount_ {0};

    std::unordered_map<std::string, uint32_t> nameToId_;

    /** 未Ready・失敗時の代替に使うスロット。ロックなしで読むためatomic */
    std::atomic<uint32_t> defaultId_ {INVALID_SLOT_ID};

public:
    ~TextureManager();

    /** @brief テクスチャマネージャーを初期化
     * @param _adapter DirectXアダプター
     * @param _srv SRVマネージャー
     */
    void Initialize(DirectXAdapter* _adapter, SRVManager* _srv);

    /** @brief テクスチャを要求しハンドルを取得（同一名は既存スロットを返す）
     * @param _fileName ファイル名
     * @return テクスチャハンドル
     */
    TextureHandle Request(const std::string& _fileName);

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
     * @return 成功なら true
     */
    bool LoadFromRawPixels(const std::string& _name, const uint8_t* _pixels,
                           uint32_t _width, uint32_t _height,
                           DXGI_FORMAT _format = DXGI_FORMAT_R8G8B8A8_UNORM);

    /** @brief すべてのテクスチャをアンロード（クリア）
     */
    void Unload();

    /** @brief ハンドルidからSRVインデックスを取得（TextureHandle経由で使う）
     * ロックを取らない。Readyでなければ既定テクスチャのインデックスを返す
     * @param _id スロットid
     * @return SRVインデックス
     */
    uint32_t GetSrvIndexOf(uint32_t _id) const;

    /** @brief ハンドルidからGPUハンドルを取得（ロックなし）
     * @param _id スロットid
     * @return GPUディスクリプタハンドル
     */
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandleOf(uint32_t _id) const;

    /** @brief ハンドルidからメタデータを取得（ロックなし）
     * @param _id スロットid
     * @return Readyなら実体、それ以外は既定テクスチャのメタデータ
     */
    const DirectX::TexMetadata& GetMetadataOf(uint32_t _id) const;

    /** @brief ハンドルidから読み込み状態を取得（TextureHandle経由で使う）
     * @param _id スロットid
     * @return 読み込み状態
     */
    TextureState GetStateOf(uint32_t _id) const;

    /** @brief 既定テクスチャのSRVインデックスを取得
     * @return SRVインデックス
     */
    uint32_t GetDefaultSrvIndex() const;

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

    /** @brief 読み込み可能な画像ファイルの一覧を取得（デバッグUIの選択肢表示用）
     * folderPath_ 配下を再帰的に走査し、対応拡張子（png/jpg/jpeg/dds/tga）のファイルを
     * folderPath_ からの相対パス（Load等にそのまま渡せるキー形式）で返す
     * @return ソート済みの相対パス一覧
     */
    std::vector<std::string> ListAvailableTextures() const;

    /** @brief 既に読み込み済み（GPUへアップロード済み）のテクスチャキー一覧を取得
     * ディスクを走査せず、現在使用可能なテクスチャだけを返す（デバッグUIの選択肢表示用）
     * @return ソート済みの読み込み済みテクスチャキー一覧
     */
    std::vector<std::string> GetLoadedTextureNames() const;

private: //Methods
    /** @brief folderPath_ を除去したキー形式へ正規化する */
    std::string NormalizeName(const std::string& _fileName) const;

    /** @brief 正規化済みキーのスロットidを取得（なければ作成）。mutex_保持が前提 */
    uint32_t ResolveIdUnlocked(const std::string& _normalized);

    /** @brief 未読み込みならこの場で読み込む。mutex_保持が前提 */
    bool EnsureLoadedUnlocked(uint32_t _id);

    /** @brief Ready なスロットを返す。未Readyなら既定テクスチャ、それも無ければnullptr（ロックなし） */
    const TextureSlot* FindReadySlot(uint32_t _id) const;

    /** @brief ファイルを読み込みCPU側でデコードする（GPUには触らない） */
    DirectX::ScratchImage DecodeImage(const std::string& _fileName) const;

    void UploadTextureData(DX12Resource* _texture, const DirectX::ScratchImage& _mipImages) const;

    static DirectX::ScratchImage LoadDDS(const std::wstring& _path);
};

#endif // TEXTUREMANAGER_HPP_
