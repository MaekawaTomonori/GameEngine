#ifndef Animation_HPP_
#define Animation_HPP_
#include <map>
#include <string>

#include "NodeAnimation.hpp"

/// <summary>
/// アニメーションデータ
/// ノードアニメーションのコレクションと再生時間を保持
/// </summary>
struct Animation {
    float duration;
    std::map<std::string, NodeAnimation> nodeAnimations;
};

#endif // Animation_HPP_
