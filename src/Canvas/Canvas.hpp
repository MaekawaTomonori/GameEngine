#ifndef Canvas_HPP_
#define Canvas_HPP_
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ReferencePtr.hpp"
#include "src/PostProcess/Chain/PostEffectChain.hpp"

class DirectXAdapter;
class SRVManager;
class PostEffectFactory;

/** @brief 描画先1枚（RTとタスクキュー）を表すクラス
 * 常時構成のPostEffectは内部の PostEffectChain が担う。
 * ワンショット演出は持たない（合成後の画面にのみかける）。
 */
class Canvas {
    std::string name_;
    bool enabled_ = true;
    std::vector<std::function<void()>> tasks_;
    std::unique_ptr<PostEffectChain> chain_;

public:
    /** @brief 初期化
     * @param _adapter DirectX adapter
     * @param _srv SRV manager
     * @param _name Canvas名
     */
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv, const std::string& _name);

    /** @brief PostEffect factory を設定 */
    void SetFactory(GESTD::ReferencePtr<PostEffectFactory> _factory);

    /** @brief 描画タスクを登録する（次のDrawObjects()で消化される） */
    void RegisterTask(const std::function<void()>& _task);

    /** @brief 登録されたタスクを自分のRTへ描画する */
    void DrawObjects();

    /** @brief 常時構成のPostEffectを適用する */
    void ApplyPostEffects();

    /** @brief 適用結果を、現在バインドされている描画先へ合成描画する */
    void Draw() const;

    /** @brief 名前を取得 */
    const std::string& GetName() const { return name_; }

    /** @brief 有効かどうか */
    bool IsEnabled() const { return enabled_; }

    /** @brief 有効/無効を切り替える */
    void SetEnabled(bool _enabled) { enabled_ = _enabled; }

    /** @brief 内部のPostEffectChainを取得（Editor等から常時構成を編集する用） */
    PostEffectChain* GetChain() const { return chain_.get(); }

    /** @brief render texture をリサイズする */
    void ResizeRenderTextures() { chain_->ResizeRenderTextures(); }
}; // class Canvas

#endif // Canvas_HPP_
