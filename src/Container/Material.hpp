#ifndef Material_HPP_
#define Material_HPP_
#include <cstdint>

#include "Math/Vector4.hpp"

struct Material {
	Vector4 color;
    uint32_t light;
    float shininess;
};

#endif // Material_HPP_
