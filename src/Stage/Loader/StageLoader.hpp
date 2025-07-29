#ifndef StageLoader_HPP_
#define StageLoader_HPP_
#include <string>

#include "src/Stage/StageRepository.hpp"
#include "vendor/json/json.hpp"


class StageLoader {
    const std::string DIR = "Assets/Data/Stage/";

    StageRepository* repository_ = nullptr;

public:
    void Initialize(StageRepository* _repository);
    bool Load(const std::string& _path) const;
private:
    static std::unique_ptr<LevelData> Recursive(nlohmann::json _base);
}; // class StageLoader

#endif // StageLoader_HPP_
