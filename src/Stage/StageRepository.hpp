#ifndef StageRepository_HPP_
#define StageRepository_HPP_

#include <memory>
#include <string>
#include <unordered_map>

#include "LevelData.hpp"


class StageRepository {
    std::unordered_map<std::string, std::unique_ptr<LevelData>> data_;
public:
    LevelData* Get(const std::string& _name);
    void Add(const std::string& _name, std::unique_ptr<LevelData> _data);
}; // class StageRepository

#endif // StageRepository_HPP_
