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

    struct Item {
        std::string key;
        bool loop;
    };

    std::unordered_map<std::string, Work> works_;
    std::vector<Item> availableWorks_;

    DebugUI* debug_ = nullptr;

    bool isProgress_ = false;
    bool isLoop_ = false;

    float timer_ = 0;

    Camera* active_ = nullptr;
    Transform originalTransform_{};
    std::string currentWorkKey_;

    // Editor state
    bool showEditor_ = false;
    std::string newWorkName_;
    Work editingWork_;
    std::string editingWorkKey_;
    int selectedPointIndex_ = -1;
    bool isEditingWork_ = false;
    bool isPreviewingPoint_ = false;

public:
    void Initialize(DebugUI* _debug);
    void Update();
    void Load(const std::string& _key);
    void Run(const std::string& _key);

private:
    void Debug();
    void LoadWorkList();
    void LoadWork(const std::string& _key);
    void OnComplete();
    Point InterpolatePoint(const Point& start, const Point& end, float t);

    // Editor functions
    void ShowEditor();
    void RenderGizmos();
    void SaveWork(const std::string& _key, const Work& _work);
    void StartEditingWork(const std::string& _key);
    void StopEditingWork();
    void DeleteWork(const std::string& _key);
    void AddPoint();
    void RemovePoint(int _index);
    void CaptureCurrentCameraAsPoint();
    void PreviewPoint(int _index);
    void StopPreview();
}; // class CameraDirector

#endif // CameraDirector_HPP_
