#define NOMINMAX
#include "CameraDirector.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "Utils.hpp"
#include "imgui.h"
#include "Math/MathUtils.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "externals/json/json.hpp"

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void CameraDirector::Initialize(DebugUI* _debug) {
    debug_ = _debug;

    controlPointModel_ = std::make_unique<Model>();
    controlPointModel_->Initialize("AnimatedCube");
    controlPointModel_->SetScale(Vector3(0.15f, 0.15f, 0.15f));
    controlPointModel_->SetColor(Vector4{1.0f, 0.84f, 0.0f, 1.0f}); // gold

    LoadWorkList();
}

void CameraDirector::Update() {
    if (showEditor_) {
        ShowEditor();
    }

    Debug();

    // Control point visualizer: update position when a Bezier keyframe is selected
    if (controlPointModel_ && isEditingWork_ &&
        selectedKeyframeIndex_ >= 0 &&
        selectedKeyframeIndex_ < static_cast<int>(editingWork_.keyframes.size())) {
        const Keyframe& kf = editingWork_.keyframes[selectedKeyframeIndex_];
        if (kf.pathType == PathType::Bezier) {
            controlPointModel_->SetTranslate(ToWorld(kf.controlPoint));
            controlPointModel_->Update();
        }
    }

    // Preview: lock the active camera to the selected keyframe position
    if (isEditingWork_ && isPreviewingKeyframe_ && active_ &&
        selectedKeyframeIndex_ >= 0 &&
        selectedKeyframeIndex_ < static_cast<int>(editingWork_.keyframes.size())) {
        const Keyframe& kf      = editingWork_.keyframes[selectedKeyframeIndex_];
        Vector3         worldPos = ToWorld(kf.position);
        Vector2         rot      = kf.useLookAt
                                 ? CalculateLookAtRotation(worldPos, ToWorld(kf.lookAtTarget))
                                 : kf.rotation;
        active_->transform_.translate = worldPos;
        active_->transform_.rotate    = Vector3(rot.y, rot.x, 0.0f);
    }

    if (!isProgress_ || !active_) return;

    Work& current = works_[currentWorkKey_];
    timer_ += 1.0f / 60.0f;

    if (timer_ / current.duration >= 1.0f) {
        if (isLoop_) {
            timer_ = 0.0f;
        } else {
            OnComplete();
            return;
        }
    }

    int numKeyframes = static_cast<int>(current.keyframes.size());
    if (numKeyframes < 2) return;

    int   numSegments     = numKeyframes - 1;
    float segmentDuration = current.duration / static_cast<float>(numSegments);
    int   segIdx          = static_cast<int>(timer_ / segmentDuration);
    segIdx = std::min(segIdx, numSegments - 1);

    float segProgress = (timer_ - segIdx * segmentDuration) / segmentDuration;
    segProgress = std::clamp(segProgress, 0.0f, 1.0f);

    const Keyframe& kfA = current.keyframes[segIdx];
    const Keyframe& kfB = current.keyframes[segIdx + 1];

    Vector3 worldPos = InterpolatePosition(kfA, kfB, segProgress);
    Vector2 rot      = InterpolateRotation(kfA, kfB, segProgress, worldPos);

    active_->transform_.translate = worldPos;
    active_->transform_.rotate    = Vector3(rot.y, rot.x, 0.0f);
}

void CameraDirector::Draw() {
    if (!controlPointModel_ || !isEditingWork_) return;
    if (selectedKeyframeIndex_ < 0 ||
        selectedKeyframeIndex_ >= static_cast<int>(editingWork_.keyframes.size())) return;

    const Keyframe& kf = editingWork_.keyframes[selectedKeyframeIndex_];
    if (kf.pathType != PathType::Bezier) return;

    controlPointModel_->Draw();
}

void CameraDirector::Load(const std::string& _key) {
    LoadWork(_key);
}

void CameraDirector::Run(const std::string& _key, bool _loop) {
    if (isProgress_ || isEditingWork_) return;

    if (!works_.contains(_key)) {
        LoadWork(_key);
    }
    if (!works_.contains(_key)) return;

    isLoop_         = _loop;
    isProgress_     = true;
    timer_          = 0.0f;
    currentWorkKey_ = _key;
    active_         = Singleton<CameraController>::GetInstance()->GetActive();

    for (auto& item : availableWorks_) {
        if (Utils::EqualsIgnoreCase(_key, item.key)) {
            item.loop = _loop;
            break;
        }
    }

    if (active_) {
        originalTransform_ = active_->transform_;
    }
}

void CameraDirector::Stop() {
    if (!isProgress_) return;
    OnComplete();
}

// ---------------------------------------------------------------------------
// Private: loading / saving
// ---------------------------------------------------------------------------

void CameraDirector::LoadWorkList() {
    availableWorks_.clear();
    works_.clear();

    const std::string dir = "Assets/Data/Camerawork/";
    if (!std::filesystem::exists(dir)) return;

    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            Item item;
            item.key  = entry.path().stem().string();
            item.loop = false;
            availableWorks_.push_back(item);
        }
    }
}

void CameraDirector::LoadWork(const std::string& _key) {
    std::string   path = "Assets/Data/Camerawork/" + _key + ".json";
    std::ifstream file(path);
    if (!file.is_open()) return;

    nlohmann::json root;
    file >> root;
    file.close();

    Work work{};

    // ---- v2 format -------------------------------------------------------
    if (root.contains("version") && root["version"].get<int>() == 2) {
        work.duration = root.value("duration", 5.0f);

        if (root.contains("keyframes") && root["keyframes"].is_array()) {
            for (const auto& kfData : root["keyframes"]) {
                Keyframe kf;
                kf.name = kfData.value("name", "");

                if (kfData.contains("position") &&
                    kfData["position"].is_array() && kfData["position"].size() >= 3) {
                    kf.position = Vector3(
                        kfData["position"][0].get<float>(),
                        kfData["position"][1].get<float>(),
                        kfData["position"][2].get<float>());
                }

                if (kfData.contains("rotation") &&
                    kfData["rotation"].is_array() && kfData["rotation"].size() >= 2) {
                    kf.rotation = Vector2(
                        kfData["rotation"][0].get<float>(),
                        kfData["rotation"][1].get<float>());
                }

                kf.useLookAt = kfData.value("useLookAt", false);

                if (kfData.contains("lookAtTarget") &&
                    kfData["lookAtTarget"].is_array() && kfData["lookAtTarget"].size() >= 3) {
                    kf.lookAtTarget = Vector3(
                        kfData["lookAtTarget"][0].get<float>(),
                        kfData["lookAtTarget"][1].get<float>(),
                        kfData["lookAtTarget"][2].get<float>());
                }

                kf.pathType   = StringToPathType(kfData.value("pathType",   "Linear"));
                kf.timeEasing = StringToTimeEasing(kfData.value("timeEasing", "Linear"));

                if (kfData.contains("controlPoint") &&
                    kfData["controlPoint"].is_array() && kfData["controlPoint"].size() >= 3) {
                    kf.controlPoint = Vector3(
                        kfData["controlPoint"][0].get<float>(),
                        kfData["controlPoint"][1].get<float>(),
                        kfData["controlPoint"][2].get<float>());
                }

                work.keyframes.push_back(kf);
            }
        }

        works_[_key] = work;
        return;
    }

    // ---- v1 format (legacy migration) ------------------------------------
    if (!root.contains("Camerawork")) return;
    auto& cwData = root["Camerawork"];
    if (!cwData.contains(_key)) return;
    auto& workData = cwData[_key];

    work.duration = workData.value("duration", 5.0f);

    struct V1Point {
        std::string name;
        Vector3 position{};
        Vector2 rotation{};
        bool useLookAt = false;
        Vector3 lookAtTarget{};
    };
    std::vector<V1Point> pointPool;

    if (workData.contains("points")) {
        for (const auto& pData : workData["points"]) {
            V1Point p;
            p.name = pData.value("name", "");

            if (pData.contains("position") &&
                pData["position"].is_array() && pData["position"].size() >= 3) {
                p.position = Vector3(
                    pData["position"][0].get<float>(),
                    pData["position"][1].get<float>(),
                    pData["position"][2].get<float>());
            }

            if (pData.contains("rotation") &&
                pData["rotation"].is_array() && pData["rotation"].size() >= 2) {
                p.rotation = Vector2(
                    pData["rotation"][0].get<float>(),
                    pData["rotation"][1].get<float>());
            }

            p.useLookAt = pData.value("useLookAt", false);

            if (pData.contains("lookAtTarget") &&
                pData["lookAtTarget"].is_array() && pData["lookAtTarget"].size() >= 3) {
                p.lookAtTarget = Vector3(
                    pData["lookAtTarget"][0].get<float>(),
                    pData["lookAtTarget"][1].get<float>(),
                    pData["lookAtTarget"][2].get<float>());
            }

            pointPool.push_back(p);
        }
    }

    auto findV1Point = [&](const std::string& _name) -> const V1Point* {
        for (const auto& p : pointPool) {
            if (p.name == _name) return &p;
        }
        return nullptr;
    };

    auto toKeyframe = [](const V1Point& _p, TimeEasing _easing = TimeEasing::Linear) -> Keyframe {
        Keyframe kf;
        kf.name         = _p.name;
        kf.position     = _p.position;
        kf.rotation     = _p.rotation;
        kf.useLookAt    = _p.useLookAt;
        kf.lookAtTarget = _p.lookAtTarget;
        kf.pathType     = PathType::Linear;
        kf.timeEasing   = _easing;
        return kf;
    };

    if (workData.contains("segments") &&
        workData["segments"].is_array() && !workData["segments"].empty()) {
        // Build ordered sequence from segment chain
        bool firstAdded = false;
        for (const auto& seg : workData["segments"]) {
            std::string startName = seg.value("start", "");
            std::string endName   = seg.value("end",   "");
            TimeEasing  easing    = StringToTimeEasing(seg.value("positionType", "Linear"));

            if (!firstAdded) {
                if (const V1Point* p = findV1Point(startName)) {
                    work.keyframes.push_back(toKeyframe(*p, easing));
                }
                firstAdded = true;
            }

            if (const V1Point* p = findV1Point(endName)) {
                // End keyframe easing comes from the next segment (simplified: Linear)
                work.keyframes.push_back(toKeyframe(*p, TimeEasing::Linear));
            }
        }
    } else if (workData.contains("order") && workData["order"].is_array()) {
        for (const auto& nameVal : workData["order"]) {
            std::string name = nameVal.get<std::string>();
            if (const V1Point* p = findV1Point(name)) {
                work.keyframes.push_back(toKeyframe(*p));
            }
        }
    } else {
        for (const auto& p : pointPool) {
            work.keyframes.push_back(toKeyframe(p));
        }
    }

    works_[_key] = work;
}

void CameraDirector::SaveWork(const std::string& _key, const Work& _work) {
    if (_key.empty()) return;

    nlohmann::json root;
    root["version"]  = 2;
    root["duration"] = _work.duration;

    nlohmann::json kfArray = nlohmann::json::array();
    for (const auto& kf : _work.keyframes) {
        nlohmann::json kfObj;
        kfObj["name"]         = kf.name;
        kfObj["position"]     = {kf.position.x,     kf.position.y,     kf.position.z};
        kfObj["rotation"]     = {kf.rotation.x,     kf.rotation.y};
        kfObj["useLookAt"]    = kf.useLookAt;
        kfObj["lookAtTarget"] = {kf.lookAtTarget.x, kf.lookAtTarget.y, kf.lookAtTarget.z};
        kfObj["pathType"]     = PathTypeToString(kf.pathType);
        kfObj["controlPoint"] = {kf.controlPoint.x, kf.controlPoint.y, kf.controlPoint.z};
        kfObj["timeEasing"]   = TimeEasingToString(kf.timeEasing);
        kfArray.push_back(kfObj);
    }
    root["keyframes"] = kfArray;

    std::filesystem::path dirPath = "Assets/Data/Camerawork/";
    if (!std::filesystem::exists(dirPath)) {
        std::filesystem::create_directories(dirPath);
    }

    std::string   path = "Assets/Data/Camerawork/" + _key + ".json";
    std::ofstream outFile(path, std::ios::trunc);
    if (!outFile.is_open()) return;

    outFile << root.dump(4) << '\n';
    outFile.close();

    works_[_key] = _work;
    LoadWorkList();
    StopEditingWork();
}

// ---------------------------------------------------------------------------
// Private: playback helpers
// ---------------------------------------------------------------------------

void CameraDirector::OnComplete() {
    if (active_) {
        active_->transform_ = originalTransform_;
    }
    isProgress_    = false;
    isLoop_        = false;
    timer_         = 0.0f;
    currentWorkKey_.clear();
    active_        = nullptr;
}

Vector3 CameraDirector::ToWorld(const Vector3& _local) const {
    return anchor_ ? *anchor_ + _local : _local;
}

float CameraDirector::ApplyEasing(float _t, TimeEasing _easing) const {
    switch (_easing) {
    case TimeEasing::EaseInQuad:     return _t * _t;
    case TimeEasing::EaseOutQuad:    return _t * (2.0f - _t);
    case TimeEasing::EaseInOutQuad:  return _t < 0.5f ? 2.0f * _t * _t
                                                       : -1.0f + (4.0f - 2.0f * _t) * _t;
    case TimeEasing::EaseInCubic:    return _t * _t * _t;
    case TimeEasing::EaseOutCubic:   { float f = _t - 1.0f; return f * f * f + 1.0f; }
    case TimeEasing::EaseInOutCubic: return _t < 0.5f ? 4.0f * _t * _t * _t
                                                       : (_t - 1.0f) * (2.0f * _t - 2.0f) * (2.0f * _t - 2.0f) + 1.0f;
    default:                         return _t;
    }
}

Vector3 CameraDirector::InterpolatePosition(const Keyframe& _a, const Keyframe& _b, float _t) const {
    float   easedT = ApplyEasing(_t, _a.timeEasing);
    Vector3 worldA = ToWorld(_a.position);
    Vector3 worldB = ToWorld(_b.position);

    if (_a.pathType == PathType::Bezier) {
        // Quadratic Bezier: B(t) = (1-t)²·P₀ + 2(1-t)t·CP + t²·P₁
        Vector3 cp = ToWorld(_a.controlPoint);
        float   u  = 1.0f - easedT;
        return worldA * (u * u) + cp * (2.0f * u * easedT) + worldB * (easedT * easedT);
    }

    return MathUtils::Lerp(worldA, worldB, easedT);
}

Vector2 CameraDirector::InterpolateRotation(const Keyframe& _a, const Keyframe& _b,
                                            float _t, const Vector3& _worldPos) const {
    float easedT = ApplyEasing(_t, _a.timeEasing);

    if (_a.useLookAt && _b.useLookAt) {
        Vector3 lerpedTarget = MathUtils::Lerp(
            ToWorld(_a.lookAtTarget), ToWorld(_b.lookAtTarget), easedT);
        return CalculateLookAtRotation(_worldPos, lerpedTarget);
    }

    Vector2 rotA = _a.useLookAt
                 ? CalculateLookAtRotation(_worldPos, ToWorld(_a.lookAtTarget))
                 : _a.rotation;
    Vector2 rotB = _b.useLookAt
                 ? CalculateLookAtRotation(_worldPos, ToWorld(_b.lookAtTarget))
                 : _b.rotation;
    return MathUtils::Lerp(rotA, rotB, easedT);
}

Vector2 CameraDirector::CalculateLookAtRotation(const Vector3& _position,
                                                 const Vector3& _target) const {
    Vector3 dir   = _target - _position;
    float   hDist = std::sqrt(dir.x * dir.x + dir.z * dir.z);
    if (hDist < 1e-4f) return Vector2(0.0f, 0.0f);

    float yaw   = std::atan2(dir.x, dir.z);
    float pitch = std::atan2(dir.y, hDist);
    return Vector2(yaw, pitch);
}

// ---------------------------------------------------------------------------
// Private: enum <-> string converters
// ---------------------------------------------------------------------------

CameraDirector::PathType CameraDirector::StringToPathType(const std::string& _str) const {
    if (Utils::EqualsIgnoreCase(_str, "Bezier")) return PathType::Bezier;
    return PathType::Linear;
}

std::string CameraDirector::PathTypeToString(PathType _type) const {
    return (_type == PathType::Bezier) ? "Bezier" : "Linear";
}

CameraDirector::TimeEasing CameraDirector::StringToTimeEasing(const std::string& _str) const {
    if (Utils::EqualsIgnoreCase(_str, "EaseInQuad"))     return TimeEasing::EaseInQuad;
    if (Utils::EqualsIgnoreCase(_str, "EaseOutQuad"))    return TimeEasing::EaseOutQuad;
    if (Utils::EqualsIgnoreCase(_str, "EaseInOutQuad"))  return TimeEasing::EaseInOutQuad;
    if (Utils::EqualsIgnoreCase(_str, "EaseInCubic"))    return TimeEasing::EaseInCubic;
    if (Utils::EqualsIgnoreCase(_str, "EaseOutCubic"))   return TimeEasing::EaseOutCubic;
    if (Utils::EqualsIgnoreCase(_str, "EaseInOutCubic")) return TimeEasing::EaseInOutCubic;
    return TimeEasing::Linear;
}

std::string CameraDirector::TimeEasingToString(TimeEasing _type) const {
    switch (_type) {
    case TimeEasing::EaseInQuad:     return "EaseInQuad";
    case TimeEasing::EaseOutQuad:    return "EaseOutQuad";
    case TimeEasing::EaseInOutQuad:  return "EaseInOutQuad";
    case TimeEasing::EaseInCubic:    return "EaseInCubic";
    case TimeEasing::EaseOutCubic:   return "EaseOutCubic";
    case TimeEasing::EaseInOutCubic: return "EaseInOutCubic";
    default:                         return "Linear";
    }
}

// ---------------------------------------------------------------------------
// Private: Debug UI (main window)
// ---------------------------------------------------------------------------

void CameraDirector::Debug() {
    if (!debug_) return;

    debug_->RegisterCommand("CameraDirector", [this]() {
        ImGui::Begin("CameraDirector");

        // ---- Playback status bar ----
        if (isProgress_ && works_.contains(currentWorkKey_)) {
            const Work& w    = works_[currentWorkKey_];
            float       frac = (w.duration > 0.0f) ? timer_ / w.duration : 0.0f;

            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f),
                               "● PLAYING: %s", currentWorkKey_.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Stop")) { Stop(); }

            char progStr[64];
            snprintf(progStr, sizeof(progStr), "%.1f / %.1f s", timer_, w.duration);
            ImGui::ProgressBar(frac, ImVec2(-1.0f, 0.0f), progStr);
        } else {
            ImGui::TextDisabled("Idle");
        }

        ImGui::Separator();

        // ---- Works list ----
        ImGui::Text("Works (%zu)", availableWorks_.size());
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) { LoadWorkList(); }

        ImGui::Separator();

        for (auto& item : availableWorks_) {
            ImGui::PushID(item.key.c_str());

            bool isPlaying  = (isProgress_     && currentWorkKey_  == item.key);
            bool isEditing  = (isEditingWork_  && editingWorkKey_  == item.key);

            if (isPlaying)      ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[>]");
            else if (isEditing) ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "[E]");
            else                ImGui::TextDisabled("   ");

            ImGui::SameLine();
            ImGui::Text("%s", item.key.c_str());
            ImGui::SameLine();

            if (isPlaying) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                if (ImGui::Button("Stop")) { Stop(); }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button("Play")) { Run(item.key, item.loop); }
            }

            ImGui::SameLine();
            if (ImGui::Checkbox("Loop", &item.loop) && isPlaying) {
                isLoop_ = item.loop;
            }

            ImGui::SameLine();
            if (!isEditing) {
                if (ImGui::Button("Edit")) {
                    if (!isEditingWork_) {
                        LoadWork(item.key);
                        if (works_.contains(item.key)) {
                            StartEditingWork(item.key);
                            showEditor_ = true;
                        }
                    }
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("X")) {
                if (!isEditing) { DeleteWork(item.key); }
            }

            ImGui::PopID();
        }

        ImGui::Separator();

        // ---- Create new work ----
        static char nameBuffer[256] = "";
        ImGui::SetNextItemWidth(150.0f);
        ImGui::InputText("##NewWorkName", nameBuffer, sizeof(nameBuffer));
        ImGui::SameLine();
        if (ImGui::Button("+ Create")) {
            if (strlen(nameBuffer) > 0 && !isEditingWork_) {
                StartEditingWork(nameBuffer);
                showEditor_ = true;
                nameBuffer[0] = '\0';
            }
        }

        ImGui::End();
    });
}

// ---------------------------------------------------------------------------
// Private: Editor UI (editor window)
// ---------------------------------------------------------------------------

void CameraDirector::ShowEditor() {
    if (!debug_) return;

    debug_->RegisterCommand("Camerawork Editor", [this]() {
        bool isOpen = showEditor_;
        ImGui::Begin("Camerawork Editor", &showEditor_);

        // Window closed via X button
        if (isOpen && !showEditor_ && isEditingWork_) {
            StopEditingWork();
        }

        if (!isEditingWork_ || editingWorkKey_.empty()) {
            showEditor_ = false;
            ImGui::End();
            return;
        }

        // ---- Header ----
        ImGui::Text("Editing: %s", editingWorkKey_.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Save"))   { SaveWork(editingWorkKey_, editingWork_); }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) { StopEditingWork(); }

        ImGui::Separator();
        ImGui::DragFloat("Duration (s)", &editingWork_.duration, 0.1f, 0.1f, 300.0f);
        ImGui::Separator();

        // ---- Keyframe list ----
        ImGui::Text("Keyframes (%zu)", editingWork_.keyframes.size());
        ImGui::SameLine();
        if (ImGui::Button("+ Add"))   { AddKeyframe(); }
        ImGui::SameLine();
        if (ImGui::Button("Capture")) { CaptureCurrentCameraAsKeyframe(); }

        for (int i = 0; i < static_cast<int>(editingWork_.keyframes.size()); ++i) {
            ImGui::PushID(i);

            const Keyframe& kf         = editingWork_.keyframes[i];
            bool            isSelected = (i == selectedKeyframeIndex_);

            char label[256];
            if (kf.pathType == PathType::Bezier) {
                snprintf(label, sizeof(label), "%d: %s  (%.1f, %.1f, %.1f) [Bezier]",
                         i, kf.name.c_str(),
                         kf.position.x, kf.position.y, kf.position.z);
            } else {
                snprintf(label, sizeof(label), "%d: %s  (%.1f, %.1f, %.1f)",
                         i, kf.name.c_str(),
                         kf.position.x, kf.position.y, kf.position.z);
            }

            if (ImGui::Selectable(label, isSelected)) {
                if (isPreviewingKeyframe_) StopPreview();
                selectedKeyframeIndex_ = i;
            }

            ImGui::PopID();
        }

        ImGui::Separator();

        // ---- Selected keyframe detail ----
        if (selectedKeyframeIndex_ < 0 ||
            selectedKeyframeIndex_ >= static_cast<int>(editingWork_.keyframes.size())) {
            ImGui::End();
            return;
        }

        Keyframe& kf = editingWork_.keyframes[selectedKeyframeIndex_];

        ImGui::Text("Keyframe: \"%s\"", kf.name.c_str());

        // Name
        char nameBuffer[256];
        strncpy_s(nameBuffer, kf.name.c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
            kf.name = nameBuffer;
        }

        // Position
        ImGui::DragFloat3("Position", &kf.position.x, 0.1f);

        // Rotation mode toggle
        ImGui::Text("Rotation:");
        ImGui::SameLine();
        if (ImGui::Button(kf.useLookAt ? "[LookAt]" : "[Manual]")) {
            kf.useLookAt = !kf.useLookAt;
        }

        if (kf.useLookAt) {
            ImGui::Indent();
            ImGui::DragFloat3("LookAt Target", &kf.lookAtTarget.x, 0.1f);
            ImGui::Unindent();
        } else {
            ImGui::Indent();
            ImGui::DragFloat("Yaw",   &kf.rotation.x, 0.01f);
            ImGui::DragFloat("Pitch", &kf.rotation.y, 0.01f);
            ImGui::Unindent();
        }

        // Path to next (irrelevant for last keyframe)
        if (selectedKeyframeIndex_ < static_cast<int>(editingWork_.keyframes.size()) - 1) {
            ImGui::Text("Path to next:");
            ImGui::SameLine();

            bool isLinear = (kf.pathType == PathType::Linear);
            bool isBezier = (kf.pathType == PathType::Bezier);

            if (isLinear) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.6f, 0.35f, 1.0f));
            if (ImGui::Button("Linear")) { kf.pathType = PathType::Linear; }
            if (isLinear) ImGui::PopStyleColor();

            ImGui::SameLine();

            if (isBezier) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.35f, 0.75f, 1.0f));
            if (ImGui::Button("Bezier")) { kf.pathType = PathType::Bezier; }
            if (isBezier) ImGui::PopStyleColor();

            if (kf.pathType == PathType::Bezier) {
                ImGui::Indent();
                ImGui::DragFloat3("Control Point", &kf.controlPoint.x, 0.1f);
                ImGui::Unindent();
            }

            // Easing combo
            const char* easingNames[] = {
                "Linear", "EaseInQuad", "EaseOutQuad", "EaseInOutQuad",
                "EaseInCubic", "EaseOutCubic", "EaseInOutCubic"
            };
            int currentEasing = static_cast<int>(kf.timeEasing);
            ImGui::SetNextItemWidth(180.0f);
            if (ImGui::Combo("Easing", &currentEasing, easingNames, 7)) {
                kf.timeEasing = static_cast<TimeEasing>(currentEasing);
            }
        }

        // Action buttons
        ImGui::Separator();

        if (!isPreviewingKeyframe_) {
            if (ImGui::Button("Preview")) { PreviewKeyframe(selectedKeyframeIndex_); }
        } else {
            if (ImGui::Button("Stop Preview")) { StopPreview(); }
        }

        ImGui::SameLine();

        if (ImGui::Button("From Camera")) {
            if (Camera* camera = Singleton<CameraController>::GetInstance()->GetActive()) {
                Vector3 worldPos = camera->transform_.translate;
                kf.position      = anchor_ ? worldPos - *anchor_ : worldPos;
                kf.rotation      = Vector2(
                    std::get<Vector3>(camera->transform_.rotate).y,
                    std::get<Vector3>(camera->transform_.rotate).x);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            RemoveKeyframe(selectedKeyframeIndex_);
        }

        ImGui::End();
    });
}

// ---------------------------------------------------------------------------
// Private: editor operations
// ---------------------------------------------------------------------------

void CameraDirector::StartEditingWork(const std::string& _key) {
    if (_key.empty() || isEditingWork_) return;

    editingWorkKey_ = _key;

    if (works_.contains(_key)) {
        editingWork_ = works_[_key];
    } else {
        editingWork_          = Work{};
        editingWork_.duration = 5.0f;
    }

    selectedKeyframeIndex_ = -1;
    isEditingWork_         = true;

    active_ = Singleton<CameraController>::GetInstance()->GetActive();
    if (active_) {
        originalTransform_ = active_->transform_;
    }
}

void CameraDirector::StopEditingWork() {
    if (!isEditingWork_) return;

    if (isPreviewingKeyframe_) StopPreview();

    if (active_) {
        active_->transform_ = originalTransform_;
    }

    editingWorkKey_.clear();
    editingWork_           = Work{};
    selectedKeyframeIndex_ = -1;
    isEditingWork_         = false;
    showEditor_            = false;
    active_                = nullptr;
}

void CameraDirector::DeleteWork(const std::string& _key) {
    std::string filePath = "Assets/Data/Camerawork/" + _key + ".json";
    if (std::filesystem::exists(filePath)) {
        std::filesystem::remove(filePath);
    }

    works_.erase(_key);

    if (editingWorkKey_ == _key && isEditingWork_) {
        StopEditingWork();
    }

    LoadWorkList();
}

void CameraDirector::AddKeyframe() {
    Keyframe kf;
    kf.name = "Keyframe" + std::to_string(editingWork_.keyframes.size());
    editingWork_.keyframes.push_back(kf);
    selectedKeyframeIndex_ = static_cast<int>(editingWork_.keyframes.size() - 1);
}

void CameraDirector::RemoveKeyframe(int _index) {
    if (_index < 0 || _index >= static_cast<int>(editingWork_.keyframes.size())) return;

    if (isPreviewingKeyframe_) StopPreview();

    editingWork_.keyframes.erase(editingWork_.keyframes.begin() + _index);
    selectedKeyframeIndex_ = -1;
}

void CameraDirector::CaptureCurrentCameraAsKeyframe() {
    if (!isEditingWork_) return;

    Camera* camera = Singleton<CameraController>::GetInstance()->GetActive();
    if (!camera) return;

    Keyframe kf;
    kf.name          = "Keyframe" + std::to_string(editingWork_.keyframes.size());
    Vector3 worldPos = camera->transform_.translate;
    kf.position      = anchor_ ? worldPos - *anchor_ : worldPos;
    kf.rotation      = Vector2(
        std::get<Vector3>(camera->transform_.rotate).y,
        std::get<Vector3>(camera->transform_.rotate).x);

    editingWork_.keyframes.push_back(kf);
    selectedKeyframeIndex_ = static_cast<int>(editingWork_.keyframes.size() - 1);
}

void CameraDirector::PreviewKeyframe(int _index) {
    if (!isEditingWork_ || _index < 0 ||
        _index >= static_cast<int>(editingWork_.keyframes.size())) return;

    isPreviewingKeyframe_  = true;
    selectedKeyframeIndex_ = _index;

    if (!active_) return;

    const Keyframe& kf      = editingWork_.keyframes[_index];
    Vector3         worldPos = ToWorld(kf.position);
    Vector2         rot      = kf.useLookAt
                             ? CalculateLookAtRotation(worldPos, ToWorld(kf.lookAtTarget))
                             : kf.rotation;
    active_->transform_.translate = worldPos;
    active_->transform_.rotate    = Vector3(rot.y, rot.x, 0.0f);
}

void CameraDirector::StopPreview() {
    if (!isEditingWork_ || !active_) return;

    isPreviewingKeyframe_ = false;
    active_->transform_   = originalTransform_;
}
