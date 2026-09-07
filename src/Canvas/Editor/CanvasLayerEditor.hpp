#ifndef CanvasLayerEditor_HPP_
#define CanvasLayerEditor_HPP_
#include <cstdint>
#include <string>
#include <unordered_map>

#include "ReferencePtr.hpp"

class DebugUI;
class PostProcessExecutor;
class Canvas;

/** @brief Canvas一覧・Z順・各Canvasの常時構成を編集するEditor
 * Layer概要は1つのウィンドウ、各Canvasの詳細はCanvasごとの独立ウィンドウで表示する
 */
class CanvasLayerEditor {
    GESTD::ReferencePtr<DebugUI> debug_ = nullptr;
    GESTD::ReferencePtr<PostProcessExecutor> executor_ = nullptr;

    bool showEditor_ = false;

    char newCanvasNameBuf_[128] = "";

    /** @brief 現在詳細編集中のCanvas名。空なら編集ウィンドウは非表示
     * Canvasの詳細編集は原則1つずつしか開けない
     */
    std::string editingCanvasName_;

    /** @brief Overlay（Executor自身の常時構成チェーン）を編集中かどうか
     * Overlayは通常のCanvasではないため editingCanvasName_ とは別に管理する。両者は排他。
     */
    bool editingOverlay_ = false;

    std::unordered_map<std::string, int> selectedEffectTypeByCanvas_;
    std::unordered_map<std::string, std::string> selectedEffectByCanvas_;

    struct PreviewCacheEntry {
        void* resource = nullptr;
        uint64_t textureId = 0;
    };
    std::unordered_map<std::string, PreviewCacheEntry> previewCache_;

public:
    CanvasLayerEditor() = default;
    ~CanvasLayerEditor() = default;

    void Initialize(const GESTD::ReferencePtr<DebugUI>& _debug, const GESTD::ReferencePtr<PostProcessExecutor>& _executor);

    void ShowEditor();
    void OpenEditor();
    void CloseEditor();
    bool IsOpen() const { return showEditor_; }

private:
    void RenderCanvasList();
    void RenderAddCanvasSection();

    /** @brief 現在編集中のCanvas/Overlay詳細ウィンドウ（1つだけ）を描画する */
    void RenderEditingCanvasWindow();
    void RenderCanvasDetail(Canvas* _canvas);

    uint64_t ResolvePreviewTextureId(Canvas* _canvas);
}; // class CanvasLayerEditor

#endif // CanvasLayerEditor_HPP_
