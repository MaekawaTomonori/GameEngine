#ifndef FrameDebugger_HPP_
#define FrameDebugger_HPP_

#include <cstdint>

#include "WeakPtr.hpp"

class DebugUI;

/** @brief フレームステップ制御（一時停止・ステップ実行）
 */
class FrameDebugger {
    GESTD::WeakPtr<DebugUI> debugUI_;
    bool paused_          = false;
    bool stepRequested_   = false;
    uint64_t frameCount_  = 0;

    // >| 長押し加速用
    int maxSpeedInterval_ = 20;
    int holdFrames_       = 0;
    int lastStepFrame_    = -1;

public:
    void Initialize(const GESTD::WeakPtr<DebugUI>& _debugUi);

    /** @brief 一時停止する
     */
    void Pause();

    /** @brief 一時停止を解除する
     */
    void Resume();

    /** @brief このフレームで Update を実行すべきか（Release では常に true）
     */
    bool ShouldUpdate();

    /** @brief 一時停止中かどうかを返す
     */
    bool IsPaused() const { return paused_; }

    /** @brief >| ステップボタンのみ描画（長押し加速あり）
     * 一時停止中に LocalDebugger の Launch スロットからインライン呼び出し用
     */
    void RenderStepButton();

}; // class FrameDebugger

#endif // FrameDebugger_HPP_
