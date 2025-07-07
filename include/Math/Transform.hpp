#ifndef Transform_HPP_
#define Transform_HPP_
#include <variant>

#include "Quaternion.hpp"
#include "Vector3.hpp"

struct Transform {
	Vector3 scale;
	std::variant<Vector3, Quaternion> rotate;
	Vector3 translate;
}; // class Transform

#endif // Transform_HPP_
