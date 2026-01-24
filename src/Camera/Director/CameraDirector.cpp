#define NOMINMAX
#include "CameraDirector.hpp"

#include <filesystem>
#include <fstream>
#include <utility>
#include <algorithm>

#include "Utils.hpp"
#include "imgui.h"
#include "Math/MathUtils.hpp"
#include "Math/Easing.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "externals/json/json.hpp"

void CameraDirector::Initialize(DebugUI* _debug) {
    debug_ = _debug;
    LoadWorkList();
}

void CameraDirector::Update() {
    if (showEditor_) {
        ShowEditor();

        // If editor window was closed by user, stop editing
        if (!showEditor_ && isEditingWork_) {
            StopEditingWork();
        }
    }

    Debug();

    // Update camera to point position only in preview mode
    if (isEditingWork_ && isPreviewingPoint_ && active_ && selectedPointIndex_ >= 0 && selectedPointIndex_ < static_cast<int>(editingWork_.points.size())) {
        Point& point = editingWork_.points[selectedPointIndex_];
        active_->transform_.translate = point.position;
        active_->transform_.rotate = Vector3(point.rotation.y, point.rotation.x, 0.0f);
    }

    if (!isProgress_ || !active_) return;

    Work& currentWork = works_[currentWorkKey_];
    timer_ += 1.0f / 60.0f; // Assuming 60 FPS

    float progress = timer_ / currentWork.duration;

    if (progress >= 1.0f) {
        if (isLoop_) {
            timer_ = 0.0f; // Reset timer for loop
        } else {
            OnComplete();
            return;
        }
    }

    // Migrate legacy order array to segments if needed
    MigrateOrderToSegments(currentWork);

    // Use segments for interpolation
    if (currentWork.segments.size() < 1) return;

    // Find points by name from segments array
    float segmentDuration = currentWork.duration / currentWork.segments.size();
    int segmentIndex = static_cast<int>(timer_ / segmentDuration);
    segmentIndex = std::min(segmentIndex, static_cast<int>(currentWork.segments.size() - 1));

    float segmentProgress = (timer_ - segmentIndex * segmentDuration) / segmentDuration;
    segmentProgress = std::clamp(segmentProgress, 0.0f, 1.0f);

    // Get current segment
    const Segment& currentSegment = currentWork.segments[segmentIndex];

    // Find start and end points by name
    auto startIt = std::find_if(currentWork.points.begin(), currentWork.points.end(),
        [&currentSegment](const Point& _p) { return _p.name == currentSegment.startPoint; });
    auto endIt = std::find_if(currentWork.points.begin(), currentWork.points.end(),
        [&currentSegment](const Point& _p) { return _p.name == currentSegment.endPoint; });

    if (startIt == currentWork.points.end() || endIt == currentWork.points.end()) return;

    // Interpolate between current and next point with separate interpolation types for position and rotation
    Point interpolatedPoint = InterpolatePointWithSeparateTypes(*startIt, *endIt, segmentProgress,
        currentSegment.positionInterpolationType, currentSegment.rotationInterpolationType);

    // Apply to camera
    active_->transform_.translate = interpolatedPoint.position;
    active_->transform_.rotate = Vector3(interpolatedPoint.rotation.y, interpolatedPoint.rotation.x, 0.0f);
}

void CameraDirector::Debug() {
    if (!debug_) return;

    debug_->RegisterCommand("CameraDirector", [this]() {
        ImGui::Begin("CameraDirector");

        // Available works section
        if (ImGui::CollapsingHeader("Available Works", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Count: %zu", availableWorks_.size());

            ImGui::SameLine();
            if (ImGui::Button("Refresh")) {
                LoadWorkList();
            }

            ImGui::Separator();

            for (auto& item : availableWorks_) {
                ImGui::PushID(item.key.c_str());

                bool isPlaying = (isProgress_ && currentWorkKey_ == item.key);
                bool isCurrentlyEditing = (isEditingWork_ && editingWorkKey_ == item.key);

                // Status indicator (fixed width to prevent layout shift)
                if (isPlaying) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[Play]");
                } else if (isCurrentlyEditing) {
                    ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "[Edit]");
                } else {
                    ImGui::Text("      "); // Empty space to maintain layout
                }

                ImGui::SameLine();

                // Work name
                ImGui::Text("%s", item.key.c_str());

                ImGui::SameLine();

                // Play/Stop button (toggles based on playing state)
                if (isPlaying) {
                    // Stop button (red text)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                    if (ImGui::Button("Stop")) {
                        Stop();
                    }
                    ImGui::PopStyleColor();
                } else {
                    // Play button
                    if (ImGui::Button("Play")) {
                        Run(item.key, item.loop);
                    }
                }

                ImGui::SameLine();

                // Loop checkbox (always in same position)
                if (ImGui::Checkbox("Loop", &item.loop)) {
                    // If currently playing this work, update the active loop state
                    if (isPlaying) {
                        isLoop_ = item.loop;
                    }
                }

                ImGui::SameLine();

                // Edit button
                if (ImGui::Button("Edit")) {
                    if (!isEditingWork_) {
                        LoadWork(item.key);
                        if (works_.contains(item.key)) {
                            StartEditingWork(item.key);
                            showEditor_ = true;
                        }
                    }
                }

                ImGui::SameLine();

                // Delete button
                if (ImGui::Button("Delete")) {
                    if (!isCurrentlyEditing) {
                        DeleteWork(item.key);
                    }
                }

                ImGui::PopID();
            }
        }

        ImGui::Separator();

        // Create new work section
        static char nameBuffer[256] = "";
        ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));

        ImGui::SameLine();

        if (ImGui::Button("Create")) {
            if (strlen(nameBuffer) > 0 && !isEditingWork_) {
                std::string newName = nameBuffer;
                StartEditingWork(newName);
                showEditor_ = true;
                nameBuffer[0] = '\0';
            }
        }

        ImGui::Separator();

        // Status section at the bottom
        ImGui::Text("Status: %s", isProgress_ ? "Playing" : "Idle");

        // Details section (collapsible)
        static bool showDetails = false;
        if (ImGui::Button(showDetails ? "Hide Details" : "Show Details")) {
            showDetails = !showDetails;
        }

        if (showDetails && isProgress_) {
            ImGui::Indent();
            ImGui::Text("Current Work: %s", currentWorkKey_.c_str());
            ImGui::Text("Timer: %.2f / %.2f", timer_, works_[currentWorkKey_].duration);
            ImGui::Text("Loop: %s", isLoop_ ? "ON" : "OFF");
            ImGui::ProgressBar(timer_ / works_[currentWorkKey_].duration);

            // Stop button in details
            if (ImGui::Button("Stop Playback")) {
                Stop();
            }
            ImGui::Unindent();
        }

        ImGui::End();
    });
}

void CameraDirector::Load(const std::string& _key) {
    LoadWork(_key);
}

void CameraDirector::Run(const std::string& _key, bool _loop) {
    if (isProgress_ || isEditingWork_) return;

    // Load work if not already loaded
    if (!works_.contains(_key)) {
        // Try to load directly from file first
        LoadWork(_key);
    }

    if (!works_.contains(_key)) {
        // If still not found, return silently
        return;
    }

    // Set loop flag
    isLoop_ = _loop;

    // Update availableWorks_ item loop state to sync with UI
    for (auto& item : availableWorks_) {
        if (Utils::EqualsIgnoreCase(_key, item.key)){
            item.loop = _loop;
            break;
        }
    }

    isProgress_ = true;
    timer_ = 0;
    currentWorkKey_ = _key;
    active_ = Singleton<CameraController>::GetInstance()->GetActive();

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

    const std::string cameraworkDir = "Assets/Data/Camerawork/";

    if (!std::filesystem::exists(cameraworkDir)) {
        return;
    }

    // Scan directory for JSON files
    for (const auto& entry : std::filesystem::directory_iterator(cameraworkDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::string filename = entry.path().stem().string();
            Item item;
            item.key = filename;
            item.loop = false;
            availableWorks_.push_back(item);
        }
    }
}

void CameraDirector::LoadWork(const std::string& _key) {
    std::string path = "Assets/Data/Camerawork/" + _key + ".json";
    std::ifstream file(path);
    if (!file.is_open()) return;

    nlohmann::json root;
    file >> root;
    file.close();

    if (!root.contains("Camerawork")) return;
    auto& cameraworkData = root["Camerawork"];
    if (!cameraworkData.contains(_key)) return;

    auto& workData = cameraworkData[_key];
    Work work{};

    // Extract duration
    if (workData.contains("duration")) {
        work.duration = workData["duration"].get<float>();
    }

    // Extract order array (legacy format)
    if (workData.contains("order")) {
        work.order = workData["order"].get<std::vector<std::string>>();
    }

    // Extract segments array
    if (workData.contains("segments")) {
        for (const auto& segmentData : workData["segments"]) {
            Segment segment;

            // Load start and end points (default to empty string if missing)
            segment.startPoint = segmentData.value("start", "");
            segment.endPoint = segmentData.value("end", "");

            // Load separate interpolation types (default to Linear if missing)
            if (segmentData.contains("positionType")) {
                segment.positionInterpolationType = StringToInterpolationType(segmentData["positionType"].get<std::string>());
            } else {
                segment.positionInterpolationType = InterpolationType::Linear;
            }

            if (segmentData.contains("rotationType")) {
                segment.rotationInterpolationType = StringToInterpolationType(segmentData["rotationType"].get<std::string>());
            } else {
                segment.rotationInterpolationType = InterpolationType::Linear;
            }

            work.segments.push_back(segment);
        }
    }

    // Extract points array
    if (workData.contains("points")) {
        for (const auto& pointData : workData["points"]) {
            Point point;

            // Load name (default to empty string)
            point.name = pointData.value("name", "");

            // Load position (default to zero vector)
            if (pointData.contains("position") && pointData["position"].is_array() && pointData["position"].size() == 3) {
                point.position = Vector3(
                    pointData["position"][0].get<float>(),
                    pointData["position"][1].get<float>(),
                    pointData["position"][2].get<float>()
                );
            } else {
                point.position = Vector3(0.0f, 0.0f, 0.0f);
            }

            // Load rotation (default to zero)
            if (pointData.contains("rotation") && pointData["rotation"].is_array() && pointData["rotation"].size() >= 2) {
                point.rotation = Vector2(
                    pointData["rotation"][0].get<float>(),
                    pointData["rotation"][1].get<float>()
                );
            } else {
                point.rotation = Vector2(0.0f, 0.0f);
            }

            // Load LookAt data (default to false and zero vector)
            point.useLookAt = pointData.value("useLookAt", false);

            if (pointData.contains("lookAtTarget") && pointData["lookAtTarget"].is_array() && pointData["lookAtTarget"].size() == 3) {
                point.lookAtTarget = Vector3(
                    pointData["lookAtTarget"][0].get<float>(),
                    pointData["lookAtTarget"][1].get<float>(),
                    pointData["lookAtTarget"][2].get<float>()
                );
            } else {
                point.lookAtTarget = Vector3(0.0f, 0.0f, 0.0f);
            }

            work.points.push_back(point);
        }
    }

    works_[_key] = work;
}

void CameraDirector::OnComplete() {
    if (active_) {
        active_->transform_ = originalTransform_;
    }

    isProgress_ = false;
    isLoop_ = false;
    timer_ = 0;
    currentWorkKey_.clear();
    active_ = nullptr;
}

CameraDirector::Point CameraDirector::InterpolatePoint(const Point& _start, const Point& _end, float _t) {
    Point result;
    result.position = MathUtils::Lerp(_start.position, _end.position, _t);
    result.rotation = MathUtils::Lerp(_start.rotation, _end.rotation, _t);
    return result;
}

CameraDirector::Point CameraDirector::InterpolatePointWithType(const Point& _start, const Point& _end, float _t, InterpolationType _type) {
    // Legacy function: use same interpolation for both position and rotation
    return InterpolatePointWithSeparateTypes(_start, _end, _t, _type, _type);
}

CameraDirector::Point CameraDirector::InterpolatePointWithSeparateTypes(const Point& _start, const Point& _end, float _t, InterpolationType _posType, InterpolationType _rotType) {
    Point result;
    result.name = _start.name;

    // Lambda to select interpolation function based on type
    auto interpolate = [_t](const auto& _a, const auto& _b, InterpolationType _type) {
        switch (_type) {
            case InterpolationType::EaseInQuad: return Ease::In::Quad(_a, _b, _t);
            case InterpolationType::EaseOutQuad: return Ease::Out::Quad(_a, _b, _t);
            case InterpolationType::EaseInOutQuad: return Ease::InOut::Quad(_a, _b, _t);
            case InterpolationType::EaseInCubic: return Ease::In::Cubic(_a, _b, _t);
            case InterpolationType::EaseOutCubic: return Ease::Out::Cubic(_a, _b, _t);
            case InterpolationType::EaseInOutCubic: return Ease::InOut::Cubic(_a, _b, _t);
            default: return MathUtils::Lerp(_a, _b, _t);
        }
    };

    // Interpolate position with position interpolation type
    result.position = interpolate(_start.position, _end.position, _posType);

    // Handle LookAt or rotation with rotation interpolation type
    if (_start.useLookAt && _end.useLookAt) {
        result.useLookAt = true;
        result.lookAtTarget = interpolate(_start.lookAtTarget, _end.lookAtTarget, _posType);
        result.rotation = CalculateLookAtRotation(result.position, result.lookAtTarget);
    } else {
        result.useLookAt = false;
        result.rotation = interpolate(_start.rotation, _end.rotation, _rotType);
    }

    return result;
}

void CameraDirector::MigrateOrderToSegments(Work& _work) {
    // If segments already exist, no need to migrate
    if (!_work.segments.empty()) return;

    // If order array doesn't exist or has less than 2 elements, nothing to migrate
    if (_work.order.size() < 2) return;

    // Convert order array to segments with Linear interpolation
    for (size_t i = 0; i < _work.order.size() - 1; ++i) {
        Segment segment;
        segment.startPoint = _work.order[i];
        segment.endPoint = _work.order[i + 1];
        segment.positionInterpolationType = InterpolationType::Linear;
        segment.rotationInterpolationType = InterpolationType::Linear;
        _work.segments.push_back(segment);
    }
}

CameraDirector::InterpolationType CameraDirector::StringToInterpolationType(const std::string& _typeStr) {
    if (Utils::EqualsIgnoreCase(_typeStr, "Linear")) return InterpolationType::Linear;
    if (Utils::EqualsIgnoreCase(_typeStr, "EaseInQuad")) return InterpolationType::EaseInQuad;
    if (Utils::EqualsIgnoreCase(_typeStr, "EaseOutQuad")) return InterpolationType::EaseOutQuad;
    if (Utils::EqualsIgnoreCase(_typeStr, "EaseInOutQuad")) return InterpolationType::EaseInOutQuad;
    if (Utils::EqualsIgnoreCase(_typeStr, "EaseInCubic")) return InterpolationType::EaseInCubic;
    if (Utils::EqualsIgnoreCase(_typeStr, "EaseOutCubic")) return InterpolationType::EaseOutCubic;
    if (Utils::EqualsIgnoreCase(_typeStr, "EaseInOutCubic")) return InterpolationType::EaseInOutCubic;
    return InterpolationType::Linear; // Default
}

std::string CameraDirector::InterpolationTypeToString(InterpolationType _type) {
    switch (_type) {
        case InterpolationType::Linear: return "Linear";
        case InterpolationType::EaseInQuad: return "EaseInQuad";
        case InterpolationType::EaseOutQuad: return "EaseOutQuad";
        case InterpolationType::EaseInOutQuad: return "EaseInOutQuad";
        case InterpolationType::EaseInCubic: return "EaseInCubic";
        case InterpolationType::EaseOutCubic: return "EaseOutCubic";
        case InterpolationType::EaseInOutCubic: return "EaseInOutCubic";
        default: return "Linear";
    }
}

Vector2 CameraDirector::CalculateLookAtRotation(const Vector3& _position, const Vector3& _target) {
    // Calculate direction vector from camera to target
    Vector3 direction = _target - _position;

    // Calculate horizontal distance
    float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);

    // Safety check: if camera and target are too close horizontally, return neutral rotation
    const float epsilon = 1e-4f;
    if (horizontalDistance < epsilon) {
        // Camera is directly above or below target - return neutral yaw and appropriate pitch
        return Vector2(0.0f, 0.0f);
    }

    // Calculate yaw (horizontal rotation around Y axis)
    float yaw = std::atan2(direction.x, direction.z);

    // Calculate pitch (vertical rotation)
    float pitch = std::atan2(direction.y, horizontalDistance);

    // Return as Vector2 (yaw, pitch)
    return Vector2(yaw, pitch);
}

void CameraDirector::ShowEditor() {
    if (!debug_) return;

    debug_->RegisterCommand("Camerawork Editor", [this]() {
        bool isEditorOpen = showEditor_;
        ImGui::Begin("Camerawork Editor", &showEditor_);

        // If window was closed by X button, stop editing
        if (isEditorOpen && !showEditor_ && isEditingWork_) {
            StopEditingWork();
        }

        // Only show editor if a work is being edited
        if (!editingWorkKey_.empty() && isEditingWork_) {
            ImGui::Text("Editing: %s", editingWorkKey_.c_str());

            ImGui::SameLine();
            if (ImGui::Button("Save")) {
                SaveWork(editingWorkKey_, editingWork_);
            }
            ImGui::SameLine();

            if (ImGui::Button("Cancel")) {
                StopEditingWork();
            }


            ImGui::Separator();

            ImGui::DragFloat("Duration", &editingWork_.duration, 0.1f, 0.1f, 60.0f);

            // Points list
            ImGui::Text("Points (%zu)", editingWork_.points.size());
            ImGui::SameLine();
            if (ImGui::Button("Add Empty Point")) {
                AddPoint();
            }
            ImGui::SameLine();
            if (ImGui::Button("Capture Current Camera")) {
                CaptureCurrentCameraAsPoint();
            }

            for (size_t i = 0; i < editingWork_.points.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));

                bool isSelected = (std::cmp_equal(selectedPointIndex_, i));
                if (ImGui::Selectable(editingWork_.points[i].name.c_str(), isSelected)) {
                    selectedPointIndex_ = static_cast<int>(i);
                }

                if (isSelected) {
                    ImGui::Indent();

                    Point& point = editingWork_.points[i];

                    // Point name input
                    char nameBuffer[256];
                    strncpy_s(nameBuffer, point.name.c_str(), sizeof(nameBuffer) - 1);
                    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
                        point.name = nameBuffer;
                    }

                    if (!isPreviewingPoint_) {
                        if (ImGui::Button("Preview")) {
                            PreviewPoint(static_cast<int>(i));
                        }
                    } else {
                        if (ImGui::Button("Stop Preview")) {
                            StopPreview();
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Update from Camera")) {
                        if (Camera* camera = Singleton<CameraController>::GetInstance()->GetActive()) {
                            point.position = camera->transform_.translate;
                            point.rotation = Vector2(std::get<Vector3>(camera->transform_.rotate).y, std::get<Vector3>(camera->transform_.rotate).x);
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Remove")) {
                        RemovePoint(static_cast<int>(i));
                    }

                    ImGui::DragFloat3("Position", &point.position.x, 0.1f);

                    // LookAt settings
                    ImGui::Checkbox("Use LookAt", &point.useLookAt);
                    if (point.useLookAt) {
                        ImGui::Indent();
                        ImGui::DragFloat3("LookAt Target", &point.lookAtTarget.x, 0.1f);
                        ImGui::Unindent();
                    } else {
                        ImGui::DragFloat2("Rotation (Yaw, Pitch)", &point.rotation.x, 0.01f);
                    }

                    ImGui::Unindent();
                }

                ImGui::PopID();
            }

            ImGui::Separator();

            // Segment Editor
            if (ImGui::CollapsingHeader("Segment Editor", ImGuiTreeNodeFlags_DefaultOpen)) {
                // Migrate from order to segments if needed
                if (!editingWork_.order.empty() && editingWork_.segments.empty()) {
                    if (ImGui::Button("Migrate Order to Segments")) {
                        MigrateOrderToSegments(editingWork_);
                        editingWork_.order.clear();
                    }
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "Legacy order format detected");
                }

                ImGui::Text("Points: %zu, Segments: %zu", editingWork_.points.size(), editingWork_.segments.size());

                // Add new segment
                static int startPointIndex = 0;
                static int endPointIndex = 0;
                if (editingWork_.points.size() < 2) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Add at least 2 points first!");
                } else {
                    ImGui::Text("Add Segment:");

                    // Start point combo
                    ImGui::SetNextItemWidth(150);
                    if (ImGui::BeginCombo("##StartPoint", editingWork_.points[startPointIndex].name.c_str())) {
                        for (size_t i = 0; i < editingWork_.points.size(); ++i) {
                            bool isSelected = (startPointIndex == static_cast<int>(i));
                            if (ImGui::Selectable(editingWork_.points[i].name.c_str(), isSelected)) {
                                startPointIndex = static_cast<int>(i);
                            }
                            if (isSelected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    ImGui::SameLine();
                    ImGui::Text("->");
                    ImGui::SameLine();

                    // End point combo
                    ImGui::SetNextItemWidth(150);
                    if (ImGui::BeginCombo("##EndPoint", editingWork_.points[endPointIndex].name.c_str())) {
                        for (size_t i = 0; i < editingWork_.points.size(); ++i) {
                            bool isSelected = (endPointIndex == static_cast<int>(i));
                            if (ImGui::Selectable(editingWork_.points[i].name.c_str(), isSelected)) {
                                endPointIndex = static_cast<int>(i);
                            }
                            if (isSelected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Add Segment")) {
                        Segment newSegment;
                        newSegment.startPoint = editingWork_.points[startPointIndex].name;
                        newSegment.endPoint = editingWork_.points[endPointIndex].name;
                        newSegment.positionInterpolationType = InterpolationType::Linear;
                        newSegment.rotationInterpolationType = InterpolationType::Linear;
                        editingWork_.segments.push_back(newSegment);
                    }
                }

                ImGui::Separator();

                // Segment List
                if (ImGui::BeginChild("SegmentList", ImVec2(0, 250), true)) {
                    ImGui::Text("Camera Path Segments:");
                    ImGui::Separator();

                    if (editingWork_.segments.empty()) {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "No segments. Add segments above.");
                    } else {
                        for (size_t i = 0; i < editingWork_.segments.size(); ++i) {
                            ImGui::PushID(static_cast<int>(i));

                            Segment& segment = editingWork_.segments[i];

                            // Segment display
                            ImGui::Text("%zu: %s -> %s", i + 1, segment.startPoint.c_str(), segment.endPoint.c_str());

                            // Position interpolation type combo
                            ImGui::Text("  Position:");
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(140);
                            std::string posTypeStr = InterpolationTypeToString(segment.positionInterpolationType);
                            if (ImGui::BeginCombo("##PosInterpType", posTypeStr.c_str())) {
                                const char* types[] = {"Linear", "EaseInQuad", "EaseOutQuad", "EaseInOutQuad", "EaseInCubic", "EaseOutCubic", "EaseInOutCubic"};
                                for (int t = 0; t < 7; ++t) {
                                    bool isSelected = (posTypeStr == types[t]);
                                    if (ImGui::Selectable(types[t], isSelected)) {
                                        segment.positionInterpolationType = StringToInterpolationType(types[t]);
                                    }
                                    if (isSelected) ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }

                            // Rotation interpolation type combo
                            ImGui::SameLine();
                            ImGui::Text("Rotation:");
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(140);
                            std::string rotTypeStr = InterpolationTypeToString(segment.rotationInterpolationType);
                            if (ImGui::BeginCombo("##RotInterpType", rotTypeStr.c_str())) {
                                const char* types[] = {"Linear", "EaseInQuad", "EaseOutQuad", "EaseInOutQuad", "EaseInCubic", "EaseOutCubic", "EaseInOutCubic"};
                                for (int t = 0; t < 7; ++t) {
                                    bool isSelected = (rotTypeStr == types[t]);
                                    if (ImGui::Selectable(types[t], isSelected)) {
                                        segment.rotationInterpolationType = StringToInterpolationType(types[t]);
                                    }
                                    if (isSelected) ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }

                            // Delete button
                            ImGui::SameLine();
                            if (ImGui::Button("Delete")) {
                                editingWork_.segments.erase(editingWork_.segments.begin() + i);
                                ImGui::PopID();
                                break;
                            }

                            ImGui::Separator();

                            ImGui::PopID();
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::Separator();

                if (ImGui::Button("Clear All Segments")) {
                    editingWork_.segments.clear();
                }
            }
        } else {
            // If no work is being edited, close the editor window
            showEditor_ = false;
        }
        ImGui::End();
    });
}

void CameraDirector::SaveWork(const std::string& _key, const Work& _work) {
    if (_key.empty()) return;

    nlohmann::json root;

    // Read existing file if it exists to preserve other works
    std::string path = "Assets/Data/Camerawork/" + _key + ".json";
    std::ifstream inFile(path);
    if (inFile.is_open()) {
        inFile >> root;
        inFile.close();
    }

    // Create Camerawork root if it doesn't exist
    if (!root.contains("Camerawork")) {
        root["Camerawork"] = nlohmann::json::object();
    }

    // Build work data
    nlohmann::json workData;
    workData["duration"] = _work.duration;

    // Save segments (new format) or order (legacy format)
    if (!_work.segments.empty()) {
        nlohmann::json segmentsArray = nlohmann::json::array();
        for (const auto& segment : _work.segments) {
            nlohmann::json segmentObj;
            segmentObj["start"] = segment.startPoint;
            segmentObj["end"] = segment.endPoint;
            segmentObj["positionType"] = InterpolationTypeToString(segment.positionInterpolationType);
            segmentObj["rotationType"] = InterpolationTypeToString(segment.rotationInterpolationType);
            segmentsArray.push_back(segmentObj);
        }
        workData["segments"] = segmentsArray;
    } else {
        workData["order"] = _work.order;
    }

    // Build points array
    nlohmann::json pointsArray = nlohmann::json::array();
    for (const auto& point : _work.points) {
        nlohmann::json pointObj;
        pointObj["name"] = point.name;
        pointObj["position"] = {point.position.x, point.position.y, point.position.z};
        pointObj["rotation"] = {point.rotation.x, point.rotation.y, 0.0f}; // Add roll as 0

        // Save LookAt data
        pointObj["useLookAt"] = point.useLookAt;
        if (point.useLookAt) {
            pointObj["lookAtTarget"] = {point.lookAtTarget.x, point.lookAtTarget.y, point.lookAtTarget.z};
        }

        pointsArray.push_back(pointObj);
    }
    workData["points"] = pointsArray;

    // Set work data
    root["Camerawork"][_key] = workData;

    // Ensure directory exists
    std::filesystem::path dirPath = "Assets/Data/Camerawork/";
    if (!std::filesystem::exists(dirPath)) {
        std::filesystem::create_directories(dirPath);
    }

    // Write to file
    std::ofstream outFile(path, std::ios::trunc);
    if (!outFile.is_open()) {
        return;
    }

    outFile << root.dump(4) << '\n';
    outFile.close();

    // Update cache
    works_[_key] = _work;

    // Refresh work list
    LoadWorkList();

    // Stop editing after save
    StopEditingWork();
}

void CameraDirector::StartEditingWork(const std::string& _key) {
    if (_key.empty() || isEditingWork_) return;

    editingWorkKey_ = _key;

    // Load existing work or create new
    if (works_.contains(_key)) {
        editingWork_ = works_[_key];
    } else {
        editingWork_ = Work{};
        editingWork_.duration = 5.0f;
    }

    selectedPointIndex_ = -1;
    isEditingWork_ = true;

    // Capture active camera
    active_ = Singleton<CameraController>::GetInstance()->GetActive();
    if (active_) {
        originalTransform_ = active_->transform_;
    }
}

void CameraDirector::StopEditingWork() {
    if (!isEditingWork_) return;

    // Stop preview if active
    if (isPreviewingPoint_) {
        StopPreview();
    }

    // Restore original camera transform
    if (active_) {
        active_->transform_ = originalTransform_;
    }

    editingWorkKey_.clear();
    editingWork_ = Work{};
    selectedPointIndex_ = -1;
    isEditingWork_ = false;
    active_ = nullptr;
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

void CameraDirector::AddPoint() {
    Point newPoint{};
    newPoint.name = "Point" + std::to_string(editingWork_.points.size());
    newPoint.position = Vector3(0.0f, 0.0f, 0.0f);
    newPoint.rotation = Vector2(0.0f, 0.0f);
    editingWork_.points.push_back(newPoint);
    selectedPointIndex_ = static_cast<int>(editingWork_.points.size() - 1);
}

void CameraDirector::RemovePoint(int _index) {
    if (_index >= 0 && _index < static_cast<int>(editingWork_.points.size())) {
        // Get the name of the point being removed
        std::string removedPointName = editingWork_.points[_index].name;

        // Remove the point
        editingWork_.points.erase(editingWork_.points.begin() + _index);

        // Remove all segments that reference this point
        auto it = editingWork_.segments.begin();
        while (it != editingWork_.segments.end()) {
            if (it->startPoint == removedPointName || it->endPoint == removedPointName) {
                it = editingWork_.segments.erase(it);
            } else {
                ++it;
            }
        }

        if (isPreviewingPoint_) {
            StopPreview();
        }

        selectedPointIndex_ = -1;
    }
}

void CameraDirector::CaptureCurrentCameraAsPoint() {
    if (!isEditingWork_) return;

    Camera* camera = Singleton<CameraController>::GetInstance()->GetActive();
    if (!camera) return;

    Point newPoint{};
    newPoint.name = "Point" + std::to_string(editingWork_.points.size());
    newPoint.position = camera->transform_.translate;
    newPoint.rotation = Vector2(std::get<Vector3>(camera->transform_.rotate).y, std::get<Vector3>(camera->transform_.rotate).x);

    editingWork_.points.push_back(newPoint);
    selectedPointIndex_ = static_cast<int>(editingWork_.points.size() - 1);
}

void CameraDirector::PreviewPoint(int _index) {
    if (!isEditingWork_ || _index < 0 || _index >= static_cast<int>(editingWork_.points.size())) return;

    isPreviewingPoint_ = true;
    selectedPointIndex_ = _index;

    const Point& point = editingWork_.points[_index];
    if (active_) {
        active_->transform_.translate = point.position;
        active_->transform_.rotate = Vector3(point.rotation.y, point.rotation.x, 0.0f);
    }
}

void CameraDirector::StopPreview() {
    if (!isEditingWork_ || !active_) return;

    isPreviewingPoint_ = false;
    active_->transform_ = originalTransform_;
}
