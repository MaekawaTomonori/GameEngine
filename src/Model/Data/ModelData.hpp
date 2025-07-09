#ifndef ModelData_HPP_
#define ModelData_HPP_
#include "src/Animation/Animation.hpp"
#include "src/Model/Node/Node.hpp"
#include "src/Model/Skeleton/Skeleton.hpp"

struct ModelData {
    std::string name;
    std::string mesh;
    Node root;
    Animation animation;
    Skeleton skeleton;
};

#endif // ModelData_HPP_
