#ifndef CameraController_HPP_
#define CameraController_HPP_
#include <memory>
#include <string>

#include "DebugUI.hpp"
#include "WeakPtr.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Camera/Repository/CameraRepository.hpp"

class CameraController {
    GESTD::WeakPtr<DebugUI> debug_;
    std::unique_ptr<CameraRepository> repository_;
    GESTD::WeakPtr<Camera> activeCamera_ = nullptr;

public:
    void Initialize(float _ratio, GESTD::WeakPtr<DebugUI> _debug);
    void Update() const;
    void Debug();

    GESTD::WeakPtr<Camera> GetActive() const;
    GESTD::WeakPtr<Camera> Add(const std::string& _name = "") const;
    GESTD::WeakPtr<Camera> SetActive(const std::string& _name);

private:
    void Load();
    void Save() const;
}; // class CameraController

#endif // CameraController_HPP_
