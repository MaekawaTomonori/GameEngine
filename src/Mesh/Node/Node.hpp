#ifndef Node_HPP_
#define Node_HPP_
#include <string>
#include <vector>

#include "Math/Matrix.hpp"
#include "Math/Transform.hpp"

struct Node {
    Transform transform;
    Matrix4x4 local;
    std::string name;
    std::vector<Node> children;
};

#endif // Node_HPP_
