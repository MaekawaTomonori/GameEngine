#ifndef Node_HPP_
#define Node_HPP_
#include <string>
#include <vector>

#include "Math/Matrix.hpp"
#include "Math/Transform.hpp"

/** @brief ノードデータ
 * モデルの階層構造を表現
 */
struct Node {
    std::string name;
    Transform transform;
    Matrix4x4 local;
    std::vector<Node> children;
};

#endif // Node_HPP_
