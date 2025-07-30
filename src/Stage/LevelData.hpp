#ifndef LevelData_HPP_
#define LevelData_HPP_
#include <string>
#include <vector>
#include "Math/Vector3.hpp"

struct LevelData {
    struct ObjectData {
        std::string name;
        std::string file;
        Vector3 translate;
        Vector3 rotate;
        Vector3 scale;

        //std::vector<ObjectData> children;
    };
    std::vector<ObjectData> objects;
}; // class LevelData

#endif // LevelData_HPP_
