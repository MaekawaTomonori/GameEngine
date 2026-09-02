#ifndef ParticleGradientKey_HPP_
#define ParticleGradientKey_HPP_
#include <algorithm>
#include <vector>

#include "Math/MathUtils.hpp"

/** @brief 時間(0～1)と値の組。パーティクルの色・サイズなどを寿命に沿って補間するための制御点
 * Animation/KeyFrame.hpp の Keyframe とは無関係（用途が異なるため別定義）
 */
template <typename T>
struct GradientKey {
    float time = 0.f;
    T value{};
};

/** @brief 制御点列を時間で線形補間して評価する
 * @param _keys 時間昇順にソート済みの制御点列
 * @param _t 評価する時間（0～1）
 * @param _fallback キーが1つも無い場合に返す値
 */
template <typename T>
T EvaluateGradient(const std::vector<GradientKey<T>>& _keys, float _t, const T& _fallback) {
    if (_keys.empty()) return _fallback;
    if (_keys.size() == 1) return _keys.front().value;
    if (_t <= _keys.front().time) return _keys.front().value;
    if (_t >= _keys.back().time) return _keys.back().value;

    for (size_t i = 0; i + 1 < _keys.size(); ++i) {
        if (_t >= _keys[i].time && _t <= _keys[i + 1].time) {
            const float span = _keys[i + 1].time - _keys[i].time;
            const float localT = span > 0.f ? (_t - _keys[i].time) / span : 0.f;
            return MathUtils::Lerp(_keys[i].value, _keys[i + 1].value, localT);
        }
    }
    return _keys.back().value;
}

/** @brief 制御点列を時間昇順にソートする */
template <typename T>
void SortGradient(std::vector<GradientKey<T>>& _keys) {
    std::ranges::sort(_keys, [](const GradientKey<T>& _a, const GradientKey<T>& _b) {
        return _a.time < _b.time;
    });
}

#endif // ParticleGradientKey_HPP_
