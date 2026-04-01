#include "StageRepository.hpp"

GESTD::WeakPtr<LevelData> StageRepository::Get(const std::string& _name) {
    auto it = data_.find(_name);
    if (it != data_.end()){
        return GESTD::WeakPtr<LevelData>(it->second);
    }
    return nullptr;
}

void StageRepository::Add(const std::string& _name, std::unique_ptr<LevelData> _data) {
    data_[_name] = std::move(_data);
}

void StageRepository::Remove(const std::string& _name) {
    data_.erase(_name);
}
