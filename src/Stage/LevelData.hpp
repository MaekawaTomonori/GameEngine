#ifndef LevelData_HPP_
#define LevelData_HPP_
#include <string>
#include "Math/Vector3.hpp"

struct LevelData {
    struct ObjectData {
        std::string name;
        Vector3 translate;
        Vector3 rotate;
        Vector3 scale;

        std::vector<ObjectData> children;
    };
    std::vector<ObjectData> objects;
}; // class LevelData

#endif // LevelData_HPP_
