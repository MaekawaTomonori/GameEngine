#ifndef CameraDirector_HPP_
#define CameraDirector_HPP_
#include <string>
#include <unordered_map>

#include "DebugUI.hpp"
#include "Math/Transform.hpp"
#include "Math/Vector3.hpp"

class Camera;

class CameraDirector {
    const std::string FilePath = "Resources/Data/Camerawork/";

    struct Point {
        Vector3 position;
        Vector2 rotation; // x: yaw, y: pitch
    };
    struct Work {
        std::vector<Point> points;
        float duration;
    };

    std::unordered_map<std::string, Work> works_;
    std::vector<std::string> availableWorks_;

    DebugUI* debug_ = nullptr;

    bool isProgress_ = false;

    float timer_ = 0;

    Camera* active_ = nullptr;
    Transform originalTransform_{};
    std::string currentWorkKey_;

public:
    void Initialize(DebugUI* _debug);
    void Update();
    void Load(const std::string& _key);
    void Run(const std::string& _key);

private:
    const std::vector<std::string>& GetAvailableWorks() const { return availableWorks_; }
    void LoadWorkList();
    void LoadWork(const std::string& _key);
    void OnComplete();
    Point InterpolatePoint(const Point& start, const Point& end, float t);
}; // class CameraDirector

#endif // CameraDirector_HPP_
