#ifndef ModelData_HPP_
#define ModelData_HPP_
#include "src/Animation/Animation.hpp"
#include "src/Model/Node/Node.hpp"
#include "src/Model/Skeleton/Skeleton.hpp"

/** @brief 頂点ウェイトデータ
 **/
struct VertexWeightData {
    float weight;
    uint32_t index;
};

/** @brief ジョイントウェイトデータ
 **/
struct JointWeightData {
    Matrix4x4 inverseBindPose;
    std::vector<VertexWeightData> weights;
};

/** @brief モデルデータ
 ** メッシュ、アニメーション、スケルトン情報を保持
 **/
struct ModelData {
    std::map<std::string, JointWeightData> skinCluster;
    std::string name;
    std::string mesh;
    Node root;
    std::optional<Animation> animation;
    std::optional<Skeleton> skeleton;
};

#endif // ModelData_HPP_
