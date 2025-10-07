#define NOMINMAX
#include "CameraDirector.hpp"

#include <filesystem>
#include <fstream>
#include <utility>
#include <algorithm>

#include "Utils.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include "imnodes.h"
#include "Math/MathUtils.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Camera/Camera.hpp"
#include "src/Camera/Controller/CameraController.hpp"
#include "vendor/json/json.hpp"

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
        if (isLoop_) {
            timer_ = 0.0f; // Reset timer for loop
        } else {
            OnComplete();
            return;
        }
    }

    // Check if order array is valid
    if (currentWork.order.size() < 2) return;

    // Find points by name from order array
    float segmentDuration = currentWork.duration / (currentWork.order.size() - 1);
    int segmentIndex = static_cast<int>(timer_ / segmentDuration);
    segmentIndex = std::min(segmentIndex, static_cast<int>(currentWork.order.size() - 2));

    float segmentProgress = (timer_ - segmentIndex * segmentDuration) / segmentDuration;
    segmentProgress = std::clamp(segmentProgress, 0.0f, 1.0f);

    // Find start and end points by name
    const std::string& startName = currentWork.order[segmentIndex];
    const std::string& endName = currentWork.order[segmentIndex + 1];

    auto startIt = std::find_if(currentWork.points.begin(), currentWork.points.end(),
        [&startName](const Point& p) { return p.name == startName; });
    auto endIt = std::find_if(currentWork.points.begin(), currentWork.points.end(),
        [&endName](const Point& p) { return p.name == endName; });

    if (startIt == currentWork.points.end() || endIt == currentWork.points.end()) return;

    // Interpolate between current and next point
    Point interpolatedPoint = InterpolatePoint(*startIt, *endIt, segmentProgress);

    // Apply to camera
    active_->transform_.translate = interpolatedPoint.position;
    active_->transform_.rotate = Vector3(interpolatedPoint.rotation.y, interpolatedPoint.rotation.x, 0.0f);
}

void CameraDirector::Debug() {
    if (!debug_) return;

    debug_->RegisterCommand("CameraDirector", [this]() {
        ImGui::Begin("CameraDirector");

        // Editor toggle at the top
        if (ImGui::Button(showEditor_ ? "Close Editor" : "Open Editor")) {
            showEditor_ = !showEditor_;

            // Stop playback when opening editor
            if (showEditor_ && isProgress_) {
                OnComplete();
            }
        }

        ImGui::Separator();

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

                // Work name - color changes during playback
                if (isPlaying) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", item.key.c_str());
                } else {
                    ImGui::Text("%s", item.key.c_str());
                }

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

                ImGui::PopID();
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
        for (const auto& item : availableWorks_) {
            if (Utils::EqualsIgnoreCase(_key, item.key)){
                LoadWork(item.key);
                break;
            }
        }
    }

    if (!works_.contains(_key)) return;

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

    // Extract order array
    if (workData.contains("order")) {
        work.order = workData["order"].get<std::vector<std::string>>();
    }

    // Extract points array
    if (workData.contains("points")) {
        for (const auto& pointData : workData["points"]) {
            Point point;

            if (pointData.contains("name")) {
                point.name = pointData["name"].get<std::string>();
            }

            if (pointData.contains("position") && pointData["position"].is_array() && pointData["position"].size() == 3) {
                point.position = Vector3(
                    pointData["position"][0].get<float>(),
                    pointData["position"][1].get<float>(),
                    pointData["position"][2].get<float>()
                );
            }

            if (pointData.contains("rotation") && pointData["rotation"].is_array() && pointData["rotation"].size() == 3) {
                // Rotation format: [yaw, pitch, roll] - we use x=yaw, y=pitch
                point.rotation = Vector2(
                    pointData["rotation"][0].get<float>(),
                    pointData["rotation"][1].get<float>()
                );
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
            for (const auto& item : availableWorks_) {
                ImGui::PushID(item.key.c_str());

                bool isCurrentlyEditing = (isEditingWork_ && editingWorkKey_ == item.key);

                if (ImGui::Button("Edit")) {
                    if (!isEditingWork_) {
                        LoadWork(item.key);
                        if (works_.contains(item.key)) {
                            StartEditingWork(item.key);
                        }
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Delete")) {
                    if (!isCurrentlyEditing) {
                        DeleteWork(item.key);
                    }
                }

                ImGui::SameLine();
                if (isCurrentlyEditing) {
                    ImGui::TextColored(ImVec4(0.2f, 0.7f, 0.2f, 1.0f), "%s [Editing]", item.key.c_str());
                } else {
                    ImGui::Text("%s", item.key.c_str());
                }

                ImGui::PopID();
            }
        }

        ImGui::Separator();

        // Create new work section
        if (ImGui::CollapsingHeader("Create New Work")) {
            static char nameBuffer[256] = "";
            ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer));

            ImGui::SameLine();

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
                    ImGui::DragFloat2("Rotation (Yaw, Pitch)", &point.rotation.x, 0.01f);

                    ImGui::Unindent();
                }

                ImGui::PopID();
            }

            ImGui::Separator();

            // Order Node Editor
            if (ImGui::CollapsingHeader("Order Node Editor", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Connect points to define order:");
                ImGui::Text("Points: %zu, Order: %zu", editingWork_.points.size(), editingWork_.order.size());

                // Safety check
                if (editingWork_.points.empty()) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Add points first!");
                } else {
                    ImNodes::BeginNodeEditor();

                // Draw nodes for each point
                for (size_t i = 0; i < editingWork_.points.size(); ++i) {
                    int nodeId = static_cast<int>(i);
                    ImNodes::BeginNode(nodeId);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::TextUnformatted(editingWork_.points[i].name.c_str());
                    ImNodes::EndNodeTitleBar();

                    // Output attribute (connects to next point)
                    ImNodes::BeginOutputAttribute(nodeId * 2);
                    ImGui::Text("Out");
                    ImNodes::EndOutputAttribute();

                    // Input attribute (receives connection from previous point)
                    ImNodes::BeginInputAttribute(nodeId * 2 + 1);
                    ImGui::Text("In");
                    ImNodes::EndInputAttribute();

                    ImNodes::EndNode();
                }

                // Draw links
                if (editingWork_.order.size() > 1) {
                    for (size_t i = 0; i < editingWork_.order.size() - 1; ++i) {
                        // Find indices
                        auto currentIt = std::find_if(editingWork_.points.begin(), editingWork_.points.end(),
                            [&](const Point& p) { return p.name == editingWork_.order[i]; });
                        auto nextIt = std::find_if(editingWork_.points.begin(), editingWork_.points.end(),
                            [&](const Point& p) { return p.name == editingWork_.order[i + 1]; });

                        if (currentIt != editingWork_.points.end() && nextIt != editingWork_.points.end()) {
                            int currentIdx = static_cast<int>(std::distance(editingWork_.points.begin(), currentIt));
                            int nextIdx = static_cast<int>(std::distance(editingWork_.points.begin(), nextIt));

                            int linkId = static_cast<int>(i);
                            ImNodes::Link(linkId, currentIdx * 2, nextIdx * 2 + 1);
                        }
                    }
                }

                ImNodes::EndNodeEditor();

                // Handle new link creation
                int startAttr, endAttr;
                if (ImNodes::IsLinkCreated(&startAttr, &endAttr)) {
                    int startNode = startAttr / 2;
                    int endNode = (endAttr - 1) / 2;

                    if (startNode >= 0 && startNode < static_cast<int>(editingWork_.points.size()) &&
                        endNode >= 0 && endNode < static_cast<int>(editingWork_.points.size()) &&
                        startNode != endNode) {
                        std::string startName = editingWork_.points[startNode].name;
                        std::string endName = editingWork_.points[endNode].name;

                        // Rebuild order array from all links
                        // This is a simplified approach - proper topological sort would be better
                        if (editingWork_.order.empty()) {
                            editingWork_.order.push_back(startName);
                        }

                        // Check if endName already exists
                        auto endIt = std::find(editingWork_.order.begin(), editingWork_.order.end(), endName);
                        if (endIt == editingWork_.order.end()) {
                            // Find position of startName and insert after it
                            auto startIt = std::find(editingWork_.order.begin(), editingWork_.order.end(), startName);
                            if (startIt != editingWork_.order.end()) {
                                editingWork_.order.insert(startIt + 1, endName);
                            } else {
                                // startName not in order, add both
                                editingWork_.order.push_back(startName);
                                editingWork_.order.push_back(endName);
                            }
                        }
                    }
                }

                // Handle link deletion - just clear order and let user rebuild
                int linkId;
                if (ImNodes::IsLinkDestroyed(&linkId)) {
                    // For now, just clear the order when a link is deleted
                    // User needs to reconnect nodes to rebuild order
                    editingWork_.order.clear();
                }

                ImGui::Separator();
                ImGui::Text("Current Order (%zu points):", editingWork_.order.size());
                for (size_t i = 0; i < editingWork_.order.size(); ++i) {
                    if (i < editingWork_.order.size()) {
                        ImGui::Text("%zu: %s", i + 1, editingWork_.order[i].c_str());
                    }
                }

                if (ImGui::Button("Clear Order")) {
                    editingWork_.order.clear();
                }
                }
            }
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
    workData["order"] = _work.order;

    // Build points array
    nlohmann::json pointsArray = nlohmann::json::array();
    for (const auto& point : _work.points) {
        nlohmann::json pointObj;
        pointObj["name"] = point.name;
        pointObj["position"] = {point.position.x, point.position.y, point.position.z};
        pointObj["rotation"] = {point.rotation.x, point.rotation.y, 0.0f}; // Add roll as 0
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
