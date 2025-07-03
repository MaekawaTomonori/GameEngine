#ifndef Animation_HPP_
#define Animation_HPP_
#include <map>
#include <string>

#include "NodeAnimation.hpp"

struct Animation {
    float duration;
    std::map<std::string, NodeAnimation> nodeAnimations;
};

#endif // Animation_HPP_
