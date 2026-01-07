#ifndef LevelData_HPP_
#define LevelData_HPP_
#include <string>
#include <vector>
#include "Math/Vector3.hpp"

/** @brief レベルデータ
 ** ステージ内のオブジェクト配置情報を保持
 **/
struct LevelData {
    /** @brief オブジェクトデータ
     ** 個別オブジェクトの配置情報
     **/
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
