#ifndef LevelEditor_HPP_
#define LevelEditor_HPP_
#include <memory>

#include "LevelData.hpp"
#include "Model.hpp"
#include "Loader/StageLoader.hpp"

class LevelEditor {
    struct StageObject {
        LevelData data;
        std::unique_ptr<Model> model;
    };

    DebugUI* debug_;

    std::unique_ptr<StageLoader> loader_;
    std::unique_ptr<StageRepository> repository_;

    std::vector<StageObject> objects_;

    std::string current_;
    uint64_t tick_ = 0;
    bool reload_ = false;

public:
    LevelEditor(DebugUI* _debug);
    void Initialize(const std::string& _name);
    void Update();
    void Draw() const;
private:
    void Spawn();
    void Debug();
}; // class LevelEditor

#endif // LevelEditor_HPP_
