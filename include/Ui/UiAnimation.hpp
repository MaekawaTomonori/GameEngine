#ifndef UiAnimation_HPP_
#define UiAnimation_HPP_
#include <functional>

#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"

namespace Ui {

    /** @brief 1フレームごとのアニメーション処理
     * @param elapsed   経過時間（秒）
     * @param posOffset ベース位置への加算値（毎フレーム {0,0} で初期化済み）
     * @param color     ベースカラーで初期化済み、上書き可能
     * @return 再生終了なら true
     */
    using AnimFunc = std::function<bool(float elapsed, Vector2& posOffset, Vector4& color)>;

    /** @brief アニメーションスロットのランタイム状態 */
    struct AnimSlot {
        AnimFunc func;
        float elapsed = 0.f;
        bool  playing = false;
        bool  done    = false;

        /** 再生開始。func が空なら即完了扱い */
        void Play() noexcept {
            elapsed = 0.f;
            if (!func) {
                playing = false;
                done    = true;
            } else {
                playing = true;
                done    = false;
            }
        }

        void Stop() noexcept {
            playing = false;
        }

        /** func が未設定 or 再生完了なら true */
        bool IsDone() const noexcept {
            return !func || done;
        }

        void Update(float dt, Vector2& posOffset, Vector4& color) {
            if (!func || done || !playing) return;
            elapsed += dt;
            done = func(elapsed, posOffset, color);
            if (done) playing = false;
        }
    };

} // namespace Ui

#endif // UiAnimation_HPP_
