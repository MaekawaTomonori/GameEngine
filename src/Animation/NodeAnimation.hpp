#ifndef NodeAnimation_HPP_
#define NodeAnimation_HPP_
#include <vector>

#include "KeyFrame.hpp"
#include "Utils.hpp"
#include "Math/MathUtils.hpp"
#include "Math/Vector3.hpp"
#include "Math/Quaternion.hpp"

template<typename T>
struct AnimationCurve {
    std::vector<Keyframe<T>> keyframes;

    T Calculate(float _time);
};

template <typename T>
T AnimationCurve<T>::Calculate(float _time) {
    if (keyframes.empty()) Utils::Alert("AnimationCurve::Calculate: No keyframes available");
    
    if (keyframes.size() == 1 || _time <= keyframes[0].time){
        return keyframes[0].value; // Single keyframe, no interpolation needed
    }
    for (size_t index = 0; index < keyframes.size(); ++index) {
        size_t next = index + 1;

        if (keyframes[index].time <= _time && _time <- keyframes[next].time) {
            float t = (_time - keyframes[index].time) / (keyframes[next].time - keyframes[index].time);
            return MathUtils::Lerp(keyframes[index].value, keyframes[next].value, t);
        }
    }
    return (*keyframes.rbegin()).value;
}

struct NodeAnimation {
    AnimationCurve<Vector3>    translate;
    AnimationCurve<Quaternion> rotation;
    AnimationCurve<Vector3>    scale;
};

#endif // NodeAnimation_HPP_
