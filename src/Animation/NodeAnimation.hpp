#ifndef NodeAnimation_HPP_
#define NodeAnimation_HPP_
#include <vector>

#include "KeyFrame.hpp"
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

#endif // NodeAnimation_HPP_
