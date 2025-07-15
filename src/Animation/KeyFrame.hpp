#ifndef KeyFrame_HPP_
#define KeyFrame_HPP_
#include "Math/Vector3.hpp"
#include "Math/Quaternion.hpp"

template<typename T >
struct Keyframe {
    T value;
    float time;
};

using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

#endif // KeyFrame_HPP_
