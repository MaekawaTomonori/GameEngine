#ifndef CameraController_HPP_
#define CameraController_HPP_
#include <memory>
#include <string>

#include "DebugUI.hpp"
#include "ReferencePtr.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Camera/Repository/CameraRepository.hpp"

class CameraController {
    GESTD::ReferencePtr<DebugUI> debug_;
    std::unique_ptr<CameraRepository> repository_;
    GESTD::ReferencePtr<Camera> activeCamera_ = nullptr;

public:
    void Initialize(float _ratio, GESTD::ReferencePtr<DebugUI> _debug);
    void Update() const;
    void Debug();

    GESTD::ReferencePtr<Camera> GetActive() const;
    GESTD::ReferencePtr<Camera> Add(const std::string& _name = "") const;
    GESTD::ReferencePtr<Camera> SetActive(const std::string& _name);

private:
    void Load();
    void Save() const;
}; // class CameraController

#endif // CameraController_HPP_
