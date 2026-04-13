#ifndef FrameDebugger_HPP_
#define FrameDebugger_HPP_

#include <cstdint>
#include <functional>

#include "ReferencePtr.hpp"

class DebugUI;

/** @brief フレームステップ制御（再生・一時停止・停止・ステップ実行）
 */
class FrameDebugger {
public:
    enum class State { Running, Paused, Stopped };

private:
    GESTD::ReferencePtr<DebugUI> debugUI_;
    State state_           = State::Running;
    bool stepRequested_    = false;
    uint64_t frameCount_   = 0;
    int stopFramesLeft_    = 0;          // Stop 後に1フレームだけ通す
    std::function<void()> onStop_;

    // >| 長押し加速用
    int maxSpeedInterval_ = 20;
    int holdFrames_       = 0;
    int lastStepFrame_    = -1;

public:
    void Initialize(const GESTD::ReferencePtr<DebugUI>& _debugUi);

    /** @brief 再生（Stopped / Paused → Running）
     */
    void Play();

    /** @brief 一時停止（Running → Paused）
     */
    void Pause();

    /** @brief 停止してシーンをリセット（Running / Paused → Stopped + onStop_ 呼び出し）
     */
    void Stop();

    /** @brief Play() の別名（後方互換）
     */
    void Resume() { Play(); }

    /** @brief Stop 時に呼ぶコールバックを登録（シーン再起動など）
     */
    void SetStopCallback(const std::function<void()>& _cb);

    /** @brief このフレームで Update を実行すべきか（Release では常に true）
     */
    bool ShouldUpdate();

    State GetState()  const { return state_; }
    bool  IsRunning() const { return state_ == State::Running; }
    bool  IsPaused()  const { return state_ == State::Paused;  }
    bool  IsStopped() const { return state_ == State::Stopped; }

    /** @brief メニューバー用ツールバーを描画（Stop / Play / Pause / Step）
     * SetToolbarContent 経由で DebugUI::RenderMainMenuBar から呼ばれる
     */
    //void RenderToolbar();

    /** @brief >| ステップボタンのみ描画（長押し加速あり）
     * 一時停止中に LocalDebugger の Launch スロットからインライン呼び出し用
     */
    void RenderStepButton();

}; // class FrameDebugger

#endif // FrameDebugger_HPP_
