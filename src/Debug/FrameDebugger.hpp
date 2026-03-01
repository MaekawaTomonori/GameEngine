#ifndef FrameDebugger_HPP_
#define FrameDebugger_HPP_

#include <cstdint>

class DebugUI;

/** @brief フレーム単位デバッガー
 ** デバッグビルド時のみSceneのUpdateをフレーム単位で制御する
 **/
class FrameDebugger {
    DebugUI* debugUI_ = nullptr;
    bool paused_ = false;
    bool stepRequested_ = false;
    uint64_t frameCount_ = 0;

public:

    void Initialize(DebugUI* _debugUi);

    /** @brief このフレームでSceneのUpdateを実行すべきか
     ** @return 実行すべき場合true。非デバッグビルドでは常にtrue
     **/
    bool ShouldUpdate();

    /** @brief ImGuiウィンドウを登録
     **/
    void Debug();
}; // class FrameDebugger

#endif // FrameDebugger_HPP_
