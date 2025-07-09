#ifndef ModelData_HPP_
#define ModelData_HPP_
#include "src/Animation/Animation.hpp"
#include "src/Model/Node/Node.hpp"
#include "src/Model/Skeleton/Skeleton.hpp"

struct VertexWeightData {
    float weight;
    uint32_t index;
};

struct JointWeightData {
    Matrix4x4 inverseBindPose;
    std::vector<VertexWeightData> weights;
};

struct ModelData {
    std::map<std::string, JointWeightData> skinCluster;
    std::string name;
    std::string mesh;
    Node root;
    Animation animation;
    Skeleton skeleton;
};

#endif // ModelData_HPP_
