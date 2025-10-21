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

    enum class InterpolationType {
        Linear,
        EaseInQuad,
        EaseOutQuad,
        EaseInOutQuad,
        EaseInCubic,
        EaseOutCubic,
        EaseInOutCubic
    };

    struct Point {
        std::string name;
        Vector3 position;
        Vector2 rotation; // x: yaw, y: pitch
        bool useLookAt = false; // LookAt機能を使用するか
        Vector3 lookAtTarget = Vector3(0.0f, 0.0f, 0.0f); // 注視点座標
    };

    struct Segment {
        std::string startPoint;
        std::string endPoint;
        InterpolationType positionInterpolationType = InterpolationType::Linear;
        InterpolationType rotationInterpolationType = InterpolationType::Linear;
    };

    struct Work {
        std::vector<Point> points;
        std::vector<std::string> order; // Point name sequence for camerawork progression (legacy)
        std::vector<Segment> segments; // New segment-based approach with interpolation types
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
    void Run(const std::string& _key, bool _loop = false);
    void Stop();
    bool IsPlaying() const { return isProgress_; }
    const std::string& GetCurrentWork() const { return currentWorkKey_; }

private:
    void Debug();
    void LoadWorkList();
    void LoadWork(const std::string& _key);
    void OnComplete();
    Point InterpolatePoint(const Point& start, const Point& end, float t);
    Point InterpolatePointWithType(const Point& start, const Point& end, float t, InterpolationType type);
    Point InterpolatePointWithSeparateTypes(const Point& start, const Point& end, float t, InterpolationType posType, InterpolationType rotType);
    void MigrateOrderToSegments(Work& work);
    InterpolationType StringToInterpolationType(const std::string& typeStr);
    std::string InterpolationTypeToString(InterpolationType type);
    Vector2 CalculateLookAtRotation(const Vector3& position, const Vector3& target);

    // Editor functions
    void ShowEditor();
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
