#ifndef CameraDirector_HPP_
#define CameraDirector_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "DebugUI.hpp"
#include "WeakPtr.hpp"
#include "Model.hpp"
#include "Math/Transform.hpp"
#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"

class Camera;

class CameraDirector {
    enum class PathType { Linear, Bezier };

    enum class TimeEasing {
        Linear, EaseInQuad, EaseOutQuad, EaseInOutQuad,
        EaseInCubic, EaseOutCubic, EaseInOutCubic
    };

    struct Keyframe {
        std::string name;
        Vector3     position{};        // anchor-relative (or world if no anchor)
        Vector2     rotation{};         // (yaw, pitch)
        bool        useLookAt = false;
        Vector3     lookAtTarget{};
        /** path settings to NEXT keyframe (ignored for last keyframe)
         */
        PathType    pathType   = PathType::Linear;
        Vector3     controlPoint{};     // Bezier control point (anchor-relative)
        TimeEasing  timeEasing = TimeEasing::Linear;
    };

    struct Work {
        std::vector<Keyframe> keyframes;
        float duration = 5.0f;
    };

    struct Item {
        std::string key;
        bool loop = false;
    };

    std::unordered_map<std::string, Work> works_;
    std::vector<Item> availableWorks_;

    GESTD::WeakPtr<DebugUI> debug_;

    bool  isProgress_          = false;
    bool  isLoop_              = false;
    bool  overwriteOnComplete_ = false;
    float timer_               = 0.0f;

    GESTD::WeakPtr<Camera>    active_            = nullptr;
    Transform  originalTransform_{};
    std::string currentWorkKey_;

    /** Anchor: world-space reference point. Position/LookAt in keyframes are relative to this.
     * Not saved to JSON. Editable from the main debug UI.
     */
    Vector3 anchor_{};

    /** Editor state
     */
    bool        showEditor_            = false;
    Work        editingWork_;
    std::string editingWorkKey_;
    int         selectedKeyframeIndex_ = -1;
    bool        isEditingWork_         = false;
    bool        isPreviewingKeyframe_  = false;

    /** Control point visualizer shown when editing a Bezier keyframe
     */
    std::unique_ptr<Model> controlPointModel_;

public:
    void Initialize(GESTD::WeakPtr<DebugUI> _debug);
    void Update();
    void Draw();
    void Load(const std::string& _key);
    void Run(const std::string& _key, bool _loop = false, bool _overwriteOnComplete = false);
    void Stop();

    bool               IsPlaying()      const { return isProgress_; }
    const std::string& GetCurrentWork() const { return currentWorkKey_; }

    /** @brief アンカー（相対座標の基準点）を設定する
     */
    void SetAnchor(const Vector3& _anchor) { anchor_ = _anchor; }
    void ClearAnchor()                     { anchor_ = Vector3{}; }

    void Debug();

private:
    Vector3 ToWorld(const Vector3& _local) const;

    void LoadWorkList();
    void LoadWork(const std::string& _key);
    void OnComplete();

    float   ApplyEasing(float _t, TimeEasing _easing) const;
    Vector3 InterpolatePosition(const Keyframe& _a, const Keyframe& _b, float _t) const;
    Vector2 InterpolateRotation(const Keyframe& _a, const Keyframe& _b, float _t, const Vector3& _worldPos) const;
    Vector2 CalculateLookAtRotation(const Vector3& _position, const Vector3& _target) const;

    PathType    StringToPathType(const std::string& _str)   const;
    std::string PathTypeToString(PathType _type)             const;
    TimeEasing  StringToTimeEasing(const std::string& _str) const;
    std::string TimeEasingToString(TimeEasing _type)         const;

    /** Editor
     */
    void ShowEditor();
    void SaveWork(const std::string& _key, const Work& _work);
    void StartEditingWork(const std::string& _key);
    void StopEditingWork();
    void DeleteWork(const std::string& _key);
    void AddKeyframe();
    void RemoveKeyframe(int _index);
    void CaptureCurrentCameraAsKeyframe();
    void PreviewKeyframe(int _index);
    void StopPreview();

}; // class CameraDirector

#endif // CameraDirector_HPP_
