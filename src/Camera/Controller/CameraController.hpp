#ifndef CameraController_HPP_
#define CameraController_HPP_
#include <memory>
#include <string>

#include "DebugUI.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Camera/Repository/CameraRepository.hpp"

class CameraController {
    DebugUI* debug_ = nullptr;
    std::unique_ptr<CameraRepository> repository_;
    Camera* activeCamera_ = nullptr;

public:
    void Initialize(float _ratio, DebugUI* _debug);
    void Update();

    Camera* GetActive() const;
    Camera* Add(const std::string& _name = "");
    Camera* SetActive(const std::string& _name);

private:
    void Debug();
    void Load();
    void Save();
}; // class CameraController

#endif // CameraController_HPP_
