#ifndef Node_HPP_
#define Node_HPP_
#include <string>
#include <vector>

#include "Math/Matrix.hpp"

struct Node {
    Matrix4x4 local;
    std::string name;
    std::vector<Node> children;
};

#endif // Node_HPP_
