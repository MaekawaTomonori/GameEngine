#ifndef ModelData_HPP_
#define ModelData_HPP_
#include "src/Animation/Animation.hpp"
#include "src/Model/Node/Node.hpp"
#include "src/Model/Skeleton/Skeleton.hpp"

/// <summary>
/// 頂点ウェイトデータ
/// </summary>
struct VertexWeightData {
    float weight;
    uint32_t index;
};

/// <summary>
/// ジョイントウェイトデータ
/// </summary>
struct JointWeightData {
    Matrix4x4 inverseBindPose;
    std::vector<VertexWeightData> weights;
};

/// <summary>
/// モデルデータ
/// メッシュ、アニメーション、スケルトン情報を保持
/// </summary>
struct ModelData {
    std::map<std::string, JointWeightData> skinCluster;
    std::string name;
    std::string mesh;
    Node root;
    std::optional<Animation> animation;
    std::optional<Skeleton> skeleton;
};

#endif // ModelData_HPP_
