#ifndef CameraManager_HPP_
#define CameraManager_HPP_
#include <memory>
#include <string>
#include <unordered_map>

#include "DebugUI.hpp"
#include "src/Camera/Camera.hpp"

class CameraManager {
    DebugUI* debug_ = nullptr;

    // name 
    std::unordered_map<std::string, std::unique_ptr<Camera>> cameras_;
    // current camera
    Camera* active_ = nullptr;

    uint16_t nonameCount_ = 0;
    bool initialize = false;
    int currentIndex_ = 0;
    std::vector<std::string>names_;

    float ratio_ = 0;

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
}; // class CameraManager

#endif // CameraManager_HPP_
