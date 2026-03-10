#ifndef Skeleton_HPP_
#define Skeleton_HPP_
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "Math/Matrix.hpp"
#include "Math/Transform.hpp"

/** @brief ジョイント（ボーン）データ
 * スケルトンアニメーション用の変換情報を保持
 */
struct Joint {
    Transform transform;
    Matrix4x4 local;
    Matrix4x4 space;
    std::string name;
    std::vector<int32_t> children;
    int32_t index;
    std::optional<int32_t> parent;
};

/** @brief スケルトンデータ
 * ジョイント階層とマッピング情報を保持
 */
struct Skeleton{
    int32_t root;
    std::map<std::string, int32_t> map;
    std::vector<Joint> joints;
};

#endif // Skeleton_HPP_
