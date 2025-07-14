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
};

struct NodeAnimation {
    AnimationCurve<Vector3>    translate;
    AnimationCurve<Quaternion> rotation;
    AnimationCurve<Vector3>    scale;
};

namespace AnimationCurveFunction {
    inline Vector3 Calculate(const AnimationCurve<Vector3>& _ac, float _time) {
        if (_ac.keyframes.empty()) Utils::Alert("AnimationCurve::Calculate: No _ac.keyframes available");

        if (_ac.keyframes.size() == 1 || _time <= _ac.keyframes[0].time){
            return _ac.keyframes.front().value; // Single keyframe, no interpolation needed
        }
        for (size_t index = 0; index < _ac.keyframes.size() - 1; ++index){
            size_t next = index + 1;

            if (_ac.keyframes[index].time <= _time && _time <= _ac.keyframes[next].time){
                float t = (_time - _ac.keyframes[index].time) / (_ac.keyframes[next].time - _ac.keyframes[index].time);
                return MathUtils::Lerp(_ac.keyframes[index].value, _ac.keyframes[next].value, t);
            }
        }
        return (_ac.keyframes.back()).value;
    }

    inline Quaternion Calculate(AnimationCurve<Quaternion>& _ac, float _time) {
        if (_ac.keyframes.empty()) Utils::Alert("AnimationCurve::Calculate: No _ac.keyframes available");
        if (_ac.keyframes.size() == 1 || _time <= _ac.keyframes[0].time){
            return _ac.keyframes.front().value; // Single keyframe, no interpolation needed
        }
        for (size_t index = 0; index < _ac.keyframes.size() - 1; ++index){
            size_t next = index + 1;
            if (_ac.keyframes[index].time <= _time && _time <= _ac.keyframes[next].time){
                float t = (_time - _ac.keyframes[index].time) / (_ac.keyframes[next].time - _ac.keyframes[index].time);
                return MathUtils::Slerp(_ac.keyframes[index].value, _ac.keyframes[next].value, t);
            }
        }
        return (_ac.keyframes.back()).value;
    }
}

#endif // NodeAnimation_HPP_
