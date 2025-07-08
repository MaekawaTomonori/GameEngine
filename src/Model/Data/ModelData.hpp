#ifndef ModelData_HPP_
#define ModelData_HPP_
#include "src/Animation/Animation.hpp"
#include "src/Model/Node/Node.hpp"
#include "src/Model/Skeleton/Skeleton.hpp"

struct ObjData {
    std::string name;
    std::string mesh;
}; // class ModelData

struct GltfData {
    std::string name;
    std::string mesh;

    Node root;
    Animation animation;
    Skeleton skeleton;
};
#endif // ModelData_HPP_
