#ifndef CanvasLayer_HPP_
#define CanvasLayer_HPP_
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ReferencePtr.hpp"
#include "src/Canvas/Canvas.hpp"

class DirectXAdapter;
class SRVManager;
class PostEffectFactory;

/** @brief 複数Canvasの前後関係（Z順）だけを管理するクラス
 * エフェクトの中身には一切関与しない。
 */
class CanvasLayer {
public:
    using ZOrder = uint32_t;

private:
    static constexpr ZOrder kAutoZBegin_ = 1;
    static constexpr ZOrder kAutoZEnd_   = 0x7FFFFFFF;
    static constexpr ZOrder kTopZBegin_  = kAutoZEnd_ + 1;

    GESTD::ReferencePtr<DirectXAdapter> adapter_;
    GESTD::ReferencePtr<SRVManager> srv_;
    GESTD::ReferencePtr<PostEffectFactory> factory_;

    /** <Z順, Canvas> */
    std::map<ZOrder, std::unique_ptr<Canvas>> canvases_;

public:
    /** @brief 初期化 */
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<SRVManager>& _srv);

    /** @brief PostEffect factory を設定（保持中の全Canvas＋以後追加するCanvasに適用） */
    void SetFactory(GESTD::ReferencePtr<PostEffectFactory> _factory);

    /** @brief Canvasを追加する（Z順は自動採番） */
    Canvas* AddCanvas(const std::string& _name);

    /** @brief Canvasを最前面グループに追加する */
    Canvas* AddCanvasTop(const std::string& _name);

    /** @brief Canvasを削除する */
    void RemoveCanvas(const std::string& _name);

    /** @brief 名前からCanvasを取得する */
    Canvas* GetCanvas(const std::string& _name) const;

    /** @brief 全Canvasを Z順（昇順）で取得する（Editor等の一覧表示用） */
    std::vector<std::pair<ZOrder, Canvas*>> GetCanvases() const;

    /** @brief 各Canvasが自分のタスクキューを消化し、自分のRTへ描画する */
    void DrawObjects();

    /** @brief 各Canvasの常時構成PostEffectを適用する */
    void ApplyPostEffects();

    /** @brief Z昇順に各Canvasを合成描画する（現在バインドされている描画先へ） */
    void DrawCanvases() const;

    /** @brief 全Canvasのrender textureをリサイズする */
    void ResizeRenderTextures() const;
}; // class CanvasLayer

#endif // CanvasLayer_HPP_
