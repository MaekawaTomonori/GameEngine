#ifndef PostEffectChain_HPP_
#define PostEffectChain_HPP_
#include <memory>
#include <string>
#include <vector>
#include <d3d12.h>

#include "ReferencePtr.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/DirectX/Heap/SRVHandle.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/PostProcess/IPostEffect.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

class DirectXAdapter;
class Heap;
class PostEffectFactory;

/** @brief 常時構成のPostEffectチェーンを1枚のRTに適用するクラス
 * Canvas1つにつき1つ持つ想定。ワンショット演出（一時エフェクト）は持たない。
 */
class PostEffectChain {
public:
    /** @brief 常時構成エフェクト。型ごとに最大1個 */
    struct EffectData {
        std::unique_ptr<IPostEffect> effect;
        std::string type;
        uint32_t rtvSlot = 0;
        bool enabled = true;
    };

private:
    GESTD::ReferencePtr<DirectXAdapter> adapter_;
    GESTD::ReferencePtr<SRVManager> srv_ = nullptr;
    GESTD::ReferencePtr<PostEffectFactory> factory_ = nullptr;

    std::vector<EffectData> effects_;
    std::vector<std::string> pendingRemovals_;

    /** シーン描画用 RenderTexture */
    std::unique_ptr<DX12Resource> renderTexture_;
    std::unique_ptr<Heap> rtvHeap_;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle_{};

    std::unique_ptr<PipelineStateObject> pso_;

    Vector4 clearColor_{ 0.0f, 0.0f, 0.0f, 0.0f };
    SRVHandle srvIndex_;

    /** 名前。プリセットパスの一意化に使う */
    std::string name_;

public:
    /** @brief 初期化
     * @param _adapter DirectX adapter
     * @param _srv SRV manager
     * @param _name 名前。プリセットパスの一意化に使う
     */
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv, const std::string& _name);

    /** @brief PostEffect factory を設定 */
    void SetFactory(GESTD::ReferencePtr<PostEffectFactory> _factory);

    /** @brief PostEffect factory を取得 */
    GESTD::ReferencePtr<PostEffectFactory> GetFactory() const { return factory_; }

    /** @brief 名前を取得 */
    const std::string& GetName() const { return name_; }

    /** @brief クリア色を変更する。RenderTexture生成後に呼んでも次回以降のクリアから反映される */
    void SetClearColor(const Vector4& _color) { clearColor_ = _color; }

    /** @brief 常時構成（型＋baseParameters）を"./Assets/Data/PostEffect/Canvases/<名前>.json"から読み込む */
    void LoadPermanentConfig();

    /** @brief 常時構成を"./Assets/Data/PostEffect/Canvases/<名前>.json"へ保存する */
    void SavePermanentConfig() const;

    /** @brief エフェクトを追加
     * @param _effect 追加するエフェクト
     */
    void Add(std::unique_ptr<IPostEffect> _effect);

    /** @brief フレーム開始時の描画準備
     * 前フレームまでに要求された削除もここで反映する
     */
    void BeginFrame();

    /** @brief フレーム終了時のクリーンアップ */
    void EndFrame() const;

    /** @brief エフェクトを実行し、最終的なSRVハンドルを返す */
    D3D12_GPU_DESCRIPTOR_HANDLE Execute();

    /** @brief 直前のExecute()結果を描画 */
    void Draw() const;

    /** @brief Draw()で使う最終SRVハンドルを上書きする
     * Execute()後にさらに別のエフェクトを適用した結果を描画したい場合に使う
     */
    void SetSrvHandle(D3D12_GPU_DESCRIPTOR_HANDLE _handle) { srvHandle_ = _handle; }

    /** @brief エフェクトの有効/無効を切り替える
     * @param _type エフェクトタイプ名
     * @param _enable 有効フラグ
     */
    void SetActive(const std::string& _type, bool _enable);

    /** @brief 常時構成のエフェクトを取得する。無ければ生成する
     * @param _type effect type
     * @return effect instance。失敗時は nullptr
     */
    IPostEffect* Create(const std::string& _type);

    /** @brief 常時構成エフェクト一覧を取得（Debug UI等から参照） */
    std::vector<EffectData>& GetEffects() { return effects_; }

    /** @brief 常時構成のエフェクトを削除する
     * GPUが今フレームの描画で参照中の可能性があるため、実際の破棄は次のBeginFrame()まで遅延する
     * @param _type effect type
     */
    void RemoveEffect(const std::string& _type);

    /** @brief エフェクトの適用順を1つ上げる（インデックスを1つ前に） */
    void MoveEffectUp(const std::string& _type);

    /** @brief エフェクトの適用順を1つ下げる（インデックスを1つ後ろに） */
    void MoveEffectDown(const std::string& _type);

    /** @brief render texture をリサイズする */
    void ResizeRenderTextures();

    /** @brief シーン描画用テクスチャのリソースを取得（DebugUIへのテクスチャ登録用） */
    ID3D12Resource* GetRenderTextureResource() const;

    /** @brief プレビュー表示用のリソースを取得する
     * 有効なエフェクトのうち最後に適用されるものの出力を返す。エフェクトが無ければ生シーンを返す。
     * @return プレビュー表示すべきリソース
     */
    ID3D12Resource* GetPreviewResource() const;

    /** @brief シーン描画用テクスチャのアスペクト比（幅/高さ）を取得する */
    float GetAspectRatio() const;

    /** @brief RTVヒープからスロットを確保する（ワンショット一時エフェクト用） */
    uint32_t AllocateRtvSlot() const;

    /** @brief RTVヒープのスロットを解放する（ワンショット一時エフェクト用） */
    void FreeRtvSlot(uint32_t _slot) const;

    /** @brief RTVヒープの指定スロットのCPUハンドルを取得する（ワンショット一時エフェクト用） */
    D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCpuHandle(uint32_t _slot) const;

private:
    /** @brief シーン描画用 render texture を作成 */
    void CreateSceneRenderTexture();

    /** @brief RTV/SRV を作成 */
    void CreateRenderTextureViews();

    /** @brief pendingRemovals_に溜まった削除要求を実際に反映する */
    void ApplyPendingRemovals();

    /** @brief "./Assets/Data/PostEffect/Canvases/<名前>.json"のパスを取得 */
    std::string PermanentConfigPath() const { return "./Assets/Data/PostEffect/Canvases/" + name_ + ".json"; }
}; // class PostEffectChain

#endif // PostEffectChain_HPP_
