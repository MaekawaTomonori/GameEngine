#define NOMINMAX
#include "CameraDirector.hpp"

#include <filesystem>

#include "Utils.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Math/MathUtils.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "src/Json/Json.hpp"

void CameraDirector::Initialize(DebugUI* _debug) {
    debug_ = _debug;
    LoadWorkList();
}

void CameraDirector::Update() {
    if (showEditor_) {
        ShowEditor();
    }

    Debug();

    // Update camera to point position only in preview mode
    if (isEditingWork_ && isPreviewingPoint_ && active_ && selectedPointIndex_ >= 0 && selectedPointIndex_ < static_cast<int>(editingWork_.points.size())) {
        Point& point = editingWork_.points[selectedPointIndex_];
        active_->transform_.translate = point.position;
        active_->transform_.rotate = Vector3(point.rotation.y, point.rotation.x, 0.0f);
    }

    if (!isProgress_ || !active_) return;


    const Work& currentWork = works_[currentWorkKey_];
    timer_ += 1.0f / 60.0f; // Assuming 60 FPS
    
    float progress = timer_ / currentWork.duration;
    
    if (progress >= 1.0f) {
        OnComplete();
        return;
    }
    
    // Calculate which segment we're in
    if (currentWork.points.size() < 2) return;
    
    float segmentDuration = currentWork.duration / (currentWork.points.size() - 1);
    int segmentIndex = static_cast<int>(timer_ / segmentDuration);
    segmentIndex = std::min(segmentIndex, static_cast<int>(currentWork.points.size() - 2));
    
    float segmentProgress = (timer_ - segmentIndex * segmentDuration) / segmentDuration;
    segmentProgress = std::clamp(segmentProgress, 0.0f, 1.0f);
    
    // Interpolate between current and next point
    const Point& startPoint = currentWork.points[segmentIndex];
    const Point& endPoint = currentWork.points[segmentIndex + 1];
    
    Point interpolatedPoint = InterpolatePoint(startPoint, endPoint, segmentProgress);
    
    // Apply to camera
    active_->transform_.translate = interpolatedPoint.position;
    active_->transform_.rotate = Vector3(interpolatedPoint.rotation.y, interpolatedPoint.rotation.x, 0.0f);
}

void CameraDirector::Debug() {
    if (!debug_) return;

    debug_->RegisterCommand("CameraDirector", [this]() {
        ImGui::Begin("CameraDirector");

        ImGui::Checkbox("Show Editor", &showEditor_);

        ImGui::Separator();

        ImGui::Text("Status: %s", isProgress_ ? "Playing" : "Idle");
        if (isProgress_) {
            ImGui::Text("Current Work: %s", currentWorkKey_.c_str());
            ImGui::Text("Timer: %.2f", timer_);
        }

        ImGui::Separator();

        ImGui::Text("Available Works: %zu", availableWorks_.size());
        if (ImGui::Button("Refresh Work List")) {
            LoadWorkList();
        }
        ImGui::End();
    });
}

void CameraDirector::Load(const std::string& _key) {
    LoadWork(_key);
}

void CameraDirector::Run(const std::string& _key) {
    if (isProgress_ || isEditingWork_) return;

    for (const auto& work : availableWorks_) {
        if (Utils::EqualsIgnoreCase(_key, work)){
            LoadWork(work);
        }
    }
    
    if (!works_.contains(_key)) return;

    isProgress_ = true;
    timer_ = 0;
    currentWorkKey_ = _key;
    active_ = Singleton<CameraController>::GetInstance()->GetActive();
    
    if (active_) {
        originalTransform_ = active_->transform_;
    }
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
            availableWorks_.push_back(filename);
        }
    }
}

void CameraDirector::LoadWork(const std::string& _key) {
    Json* json = Singleton<Json>::GetInstance();
    if (!json->Load("Camerawork", _key)) return;
    
    auto groups = json->GetGroups("Camerawork");
    if (!groups.empty()) {
        const auto& workData = groups.begin()->second;
        Work work{};
        
        // Extract duration
        if (workData.contains("duration")) {
            work.duration = std::get<float>(workData.at("duration"));
        }
        
        // Extract arrays
        std::vector<Vector3> positions;
        std::vector<Vector2> rotations;

        std::string key = "positions";
        if (workData.contains(key)) {
            positions = std::get<std::vector<Vector3>>(workData.at(key));
        }

        key = "rotations";
        if (workData.contains(key)){
            rotations = std::get<std::vector<Vector2>>(workData.at(key));
        }
        
        // Create points from arrays
        for (size_t i = 0; i < positions.size(); ++i) {
            Point point;
            point.position = positions[i];
            point.rotation = (i < rotations.size()) ? rotations[i] : Vector2{0.0f, 0.0f};
            work.points.push_back(point);
        }
        
        works_[_key] = work;
    }
}

void CameraDirector::OnComplete() {
    if (active_) {
        active_->transform_ = originalTransform_;
    }
    
    isProgress_ = false;
    timer_ = 0;
    currentWorkKey_.clear();
    active_ = nullptr;
}

CameraDirector::Point CameraDirector::InterpolatePoint(const Point& start, const Point& end, float t) {
    Point result;
    result.position = MathUtils::Lerp(start.position, end.position, t);
    result.rotation = MathUtils::Lerp(start.rotation, end.rotation, t);
    return result;
}

void CameraDirector::ShowEditor() {
    if (!debug_) return;

    debug_->RegisterCommand("Camerawork Editor", [this]() {
        ImGui::Begin("Camerawork Editor");
        // Work list section
        if (ImGui::CollapsingHeader("Available Works", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (const auto& workName : availableWorks_) {
                ImGui::PushID(workName.c_str());

                if (ImGui::Button("Edit")) {
                    if (!isEditingWork_) {
                        LoadWork(workName);
                        if (works_.contains(workName)) {
                            StartEditingWork(workName);
                        }
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Play")) {
                    Run(workName);
                }

                ImGui::SameLine();
                if (ImGui::Button("Delete")) {
                    if (!isEditingWork_ || editingWorkKey_ != workName) {
                        DeleteWork(workName);
                    }
                }

                ImGui::SameLine();
                ImGui::Text("%s", workName.c_str());

                ImGui::PopID();
            }
        }

        ImGui::Separator();

        // Create new work section
        if (ImGui::CollapsingHeader("Create New Work")) {
            static char nameBuffer[256] = "";
            ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));

            if (ImGui::Button("Create")) {
                if (strlen(nameBuffer) > 0 && !isEditingWork_) {
                    std::string newName = nameBuffer;
                    StartEditingWork(newName);
                    nameBuffer[0] = '\0';
                }
            }
        }

        ImGui::Separator();

        // Edit current work
        if (!editingWorkKey_.empty() && isEditingWork_) {
            ImGui::Text("Editing: %s", editingWorkKey_.c_str());

            ImGui::DragFloat("Duration", &editingWork_.duration, 0.1f, 0.1f, 60.0f);

            if (ImGui::Button("Capture Current Camera")) {
                CaptureCurrentCameraAsPoint();
            }

            ImGui::SameLine();
            if (ImGui::Button("Add Empty Point")) {
                AddPoint();
            }

            ImGui::SameLine();
            if (ImGui::Button("Save")) {
                SaveWork(editingWorkKey_, editingWork_);
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                StopEditingWork();
            }

            ImGui::Separator();

            // Points list
            ImGui::Text("Points (%zu)", editingWork_.points.size());

            for (size_t i = 0; i < editingWork_.points.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));

                bool isSelected = (selectedPointIndex_ == static_cast<int>(i));
                if (ImGui::Selectable(("Point " + std::to_string(i)).c_str(), isSelected)) {
                    selectedPointIndex_ = static_cast<int>(i);
                }

                if (isSelected) {
                    ImGui::Indent();

                    Point& point = editingWork_.points[i];

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
                        Camera* camera = Singleton<CameraController>::GetInstance()->GetActive();
                        if (camera) {
                            point.position = camera->transform_.translate;
                            point.rotation = Vector2(std::get<Vector3>(camera->transform_.rotate).y, std::get<Vector3>(camera->transform_.rotate).x);
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Remove")) {
                        RemovePoint(static_cast<int>(i));
                    }

                    ImGui::DragFloat3("Position", &point.position.x, 0.1f);
                    ImGui::DragFloat2("Rotation (Yaw, Pitch)", &point.rotation.x, 0.01f);

                    ImGui::Unindent();
                }

                ImGui::PopID();
            }
        }
        ImGui::End();

        // Render gizmos after ImGui windows
        RenderGizmos();
    });
}

void CameraDirector::RenderGizmos() {
    if (!isEditingWork_ || !active_) return;
    if (selectedPointIndex_ < 0 || selectedPointIndex_ >= static_cast<int>(editingWork_.points.size())) return;

    Point& selectedPoint = editingWork_.points[selectedPointIndex_];

    // Setup ImGuizmo
    ImGuizmo::SetOrthographic(false);

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    // Get camera matrices
    Matrix4x4 view = active_->GetView();
    Matrix4x4 projection = active_->GetProjection();

    // Create transform matrix for the point
    Matrix4x4 translation = MathUtils::Matrix::MakeTranslateMatrix(selectedPoint.position);
    Matrix4x4 rotationY = MathUtils::Matrix::MakeRotateY(selectedPoint.rotation.x); // yaw
    Matrix4x4 rotationX = MathUtils::Matrix::MakeRotateX(selectedPoint.rotation.y); // pitch
    Matrix4x4 transform = translation * rotationY * rotationX;

    // Manipulate with ImGuizmo
    static ImGuizmo::OPERATION currentOperation = ImGuizmo::TRANSLATE;
    static ImGuizmo::MODE currentMode = ImGuizmo::WORLD;

    if (ImGui::IsKeyPressed(ImGuiKey_T)) currentOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R)) currentOperation = ImGuizmo::ROTATE;

    ImGuizmo::Manipulate(
        reinterpret_cast<float*>(&view),
        reinterpret_cast<float*>(&projection),
        currentOperation,
        currentMode,
        reinterpret_cast<float*>(&transform)
    );

    // Extract position and rotation from modified transform
    if (ImGuizmo::IsUsing()) {
        selectedPoint.position = Vector3(transform.matrix[3][0], transform.matrix[3][1], transform.matrix[3][2]);

        // Extract rotation (simplified - proper extraction would use decomposition)
        // For now, update rotation based on transform changes
        float yaw = atan2f(transform.matrix[2][0], transform.matrix[2][2]);
        float pitch = asinf(-transform.matrix[2][1]);
        selectedPoint.rotation = Vector2(yaw, pitch);
    }
}

void CameraDirector::SaveWork(const std::string& _key, const Work& _work) {
    if (_key.empty()) return;

    Json* json = Singleton<Json>::GetInstance();

    // Prepare data for saving
    std::vector<Vector3> positions;
    std::vector<Vector2> rotations;

    for (const auto& point : _work.points) {
        positions.push_back(point.position);
        rotations.push_back(point.rotation);
    }

    // Set values
    json->SetValue("Camerawork", _key, "duration", _work.duration);
    json->SetValue("Camerawork", _key, "positions", positions);
    json->SetValue("Camerawork", _key, "rotations", rotations);

    // Save to file
    json->Save("Camerawork", _key);

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
    newPoint.position = Vector3(0.0f, 0.0f, 0.0f);
    newPoint.rotation = Vector2(0.0f, 0.0f);
    editingWork_.points.push_back(newPoint);
    selectedPointIndex_ = static_cast<int>(editingWork_.points.size() - 1);
}

void CameraDirector::RemovePoint(int _index) {
    if (_index >= 0 && _index < static_cast<int>(editingWork_.points.size())) {
        editingWork_.points.erase(editingWork_.points.begin() + _index);

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
