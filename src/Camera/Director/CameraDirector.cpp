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

void CameraDirector::Initialize(DebugUI* _debug) {
    debug_ = _debug;

    controlPointModel_ = std::make_unique<Model>();
    controlPointModel_->Initialize("AnimatedCube");
    controlPointModel_->SetScale(Vector3(0.15f, 0.15f, 0.15f));
    controlPointModel_->SetColor(Vector4{1.0f, 0.84f, 0.0f, 1.0f}); // gold

    LoadWorkList();

    if (debug_) {
        debug_->RegisterMenuButton("CameraDirector");
    }
}

void CameraDirector::Update() {
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
    if (isEditingWork_ && isPreviewingKeyframe_ &&
        selectedKeyframeIndex_ >= 0 &&
        selectedKeyframeIndex_ < static_cast<int>(editingWork_.keyframes.size())) {
        // active_ が null の場合は再取得を試みる
        if (!active_) {
            active_ = Singleton<CameraController>::GetInstance()->GetActive();
            if (active_) {
                originalTransform_ = active_->transform_;
            }
        }
        if (active_) {
            const Keyframe& kf      = editingWork_.keyframes[selectedKeyframeIndex_];
            Vector3         worldPos = ToWorld(kf.position);
            Vector2         rot      = kf.useLookAt
                                     ? CalculateLookAtRotation(worldPos, ToWorld(kf.lookAtTarget))
                                     : kf.rotation;
            active_->transform_.translate = worldPos;
            active_->transform_.rotate    = Vector3(rot.y, rot.x, 0.0f);
        }
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

void CameraDirector::Run(const std::string& _key, bool _loop, bool _overwriteOnComplete) {
    if (isProgress_ || isEditingWork_) return;

    if (!works_.contains(_key)) {
        LoadWork(_key);
    }
    if (!works_.contains(_key)) return;

    isLoop_              = _loop;
    overwriteOnComplete_ = _overwriteOnComplete;
    isProgress_          = true;
    timer_               = 0.0f;
    currentWorkKey_      = _key;
    active_              = Singleton<CameraController>::GetInstance()->GetActive();

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
    std::ifstream file("Assets/Data/Camerawork/" + _key + ".json");
    if (!file.is_open()) return;

    nlohmann::json root;
    file >> root;

    // 3要素配列からVector3、2要素配列からVector2を読む。存在しない場合はデフォルト値
    auto readVec3 = [](const nlohmann::json& _j, const char* _key) -> Vector3 {
        if (!_j.contains(_key)) return {};
        const auto& a = _j[_key];
        if (!a.is_array() || a.size() < 3) return {};
        return Vector3(a[0].get<float>(), a[1].get<float>(), a[2].get<float>());
    };
    auto readVec2 = [](const nlohmann::json& _j, const char* _key) -> Vector2 {
        if (!_j.contains(_key)) return {};
        const auto& a = _j[_key];
        if (!a.is_array() || a.size() < 2) return {};
        return Vector2(a[0].get<float>(), a[1].get<float>());
    };

    Work work{};
    work.duration = root.value("duration", 5.0f);

    for (const auto& kfData : root.value("keyframes", nlohmann::json::array())) {
        Keyframe kf;
        kf.name         = kfData.value("name",      std::string{});
        kf.position     = readVec3(kfData, "position");
        kf.rotation     = readVec2(kfData, "rotation");
        kf.useLookAt    = kfData.value("useLookAt",  false);
        kf.lookAtTarget = readVec3(kfData, "lookAtTarget");
        kf.pathType     = StringToPathType(kfData.value("pathType",    std::string{"Linear"}));
        kf.timeEasing   = StringToTimeEasing(kfData.value("timeEasing", std::string{"Linear"}));
        kf.controlPoint = readVec3(kfData, "controlPoint");
        work.keyframes.push_back(kf);
    }

    works_[_key] = work;
}

void CameraDirector::SaveWork(const std::string& _key, const Work& _work) {
    if (_key.empty()) return;

    nlohmann::json root;
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
        if (overwriteOnComplete_ && works_.contains(currentWorkKey_)) {
            const Work& w = works_[currentWorkKey_];
            if (!w.keyframes.empty()) {
                const Keyframe& lastKf   = w.keyframes.back();
                Vector3         worldPos = ToWorld(lastKf.position);
                Vector2         rot      = lastKf.useLookAt
                                         ? CalculateLookAtRotation(worldPos, ToWorld(lastKf.lookAtTarget))
                                         : lastKf.rotation;
                active_->transform_.translate = worldPos;
                active_->transform_.rotate    = Vector3(rot.y, rot.x, 0.0f);
            } else {
                active_->transform_ = originalTransform_;
            }
        } else {
            active_->transform_ = originalTransform_;
        }
    }
    isProgress_          = false;
    isLoop_              = false;
    overwriteOnComplete_ = false;
    timer_               = 0.0f;
    currentWorkKey_.clear();
    active_              = nullptr;
}

Vector3 CameraDirector::ToWorld(const Vector3& _local) const {
    return anchor_ + _local;
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
        // 弧長再パラメータ化で均一速度を実現
        Vector3 cp   = ToWorld(_a.controlPoint);
        float   arcT = MathUtils::QuadBezierArcLengthT(worldA, cp, worldB, easedT);
        return MathUtils::QuadBezier(worldA, cp, worldB, arcT);
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

    float yaw = std::atan2(dir.x, dir.z);
    // MakeRotateX(pitch) ではローカル +Z の Y 成分が -sin(pitch) になるため
    // ターゲットが上方 (dir.y > 0) のときは負の pitch が必要
    float pitch = -std::atan2(dir.y, hDist);
    return Vector2(yaw, pitch);
}

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

void CameraDirector::Debug() {
    if (!debug_) return;
    
    if (showEditor_) {
        if (!debug_->IsVisible("CameraDirector")) { StopEditingWork(); }
        ShowEditor();
    }
    
    debug_->RegisterCommand("CameraDirector", [this]() {
        ImGui::Begin("CameraDirector", &debug_->IsVisible("CameraDirector"));

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

        ImGui::Text("Anchor:");
        ImGui::SameLine();
        if (ImGui::Button("Reset##Anchor")) { anchor_ = Vector3{}; }
        ImGui::DragFloat3("##Anchor", &anchor_.x, 0.1f);
        ImGui::TextDisabled("Keyframe positions/LookAt are relative to this point.");

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

        ImGui::Text("Editing: %s", editingWorkKey_.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Save"))   { SaveWork(editingWorkKey_, editingWork_); }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) { StopEditingWork(); }

        ImGui::Separator();
        ImGui::DragFloat("Duration (s)", &editingWork_.duration, 0.1f, 0.1f, 300.0f);
        ImGui::Separator();

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
                if (isPreviewingKeyframe_) {
                    // プレビュー中は停止せずカメラを新しいキーフレームに即時移動
                    PreviewKeyframe(i);
                } else {
                    selectedKeyframeIndex_ = i;
                }
            }

            ImGui::PopID();
        }

        ImGui::Separator();

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
                kf.position      = worldPos - anchor_;
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

    // プレイ中は Run() が設定した active_/originalTransform_ をそのまま使う
    if (!isProgress_) {
        active_ = Singleton<CameraController>::GetInstance()->GetActive();
        if (active_) {
            originalTransform_ = active_->transform_;
        }
    }
}

void CameraDirector::StopEditingWork() {
    if (!isEditingWork_) return;

    if (isPreviewingKeyframe_) StopPreview();

    // プレイ中でない場合のみカメラ状態を復元・active_ をクリア
    // プレイ中は playback ループがカメラを引き続き制御する
    if (!isProgress_) {
        if (active_) {
            active_->transform_ = originalTransform_;
        }
        active_ = nullptr;
    }

    editingWorkKey_.clear();
    editingWork_           = Work{};
    selectedKeyframeIndex_ = -1;
    isEditingWork_         = false;
    showEditor_            = false;
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
    kf.position      = worldPos - anchor_;
    kf.rotation      = Vector2(
        std::get<Vector3>(camera->transform_.rotate).y,
        std::get<Vector3>(camera->transform_.rotate).x);

    editingWork_.keyframes.push_back(kf);
    selectedKeyframeIndex_ = static_cast<int>(editingWork_.keyframes.size() - 1);
}

void CameraDirector::PreviewKeyframe(int _index) {
    if (!isEditingWork_ || _index < 0 ||
        _index >= static_cast<int>(editingWork_.keyframes.size())) return;

    // active_ が未取得の場合は再試行（エディタ開始後にカメラが切り替わった場合等）
    if (!active_) {
        active_ = Singleton<CameraController>::GetInstance()->GetActive();
        if (active_) {
            originalTransform_ = active_->transform_;
        }
    }

    selectedKeyframeIndex_ = _index;
    isPreviewingKeyframe_  = true;

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
