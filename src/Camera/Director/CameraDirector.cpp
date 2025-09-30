#define NOMINMAX
#include "CameraDirector.hpp"

#include <filesystem>

#include "Utils.hpp"
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

void CameraDirector::Load(const std::string& _key) {
    LoadWork(_key);
}

void CameraDirector::Run(const std::string& _key) {
    if (isProgress_) return;

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
        Work work;
        
        // Extract duration
        if (workData.contains("duration")) {
            work.duration = std::get<float>(workData.at("duration"));
        }
        
        // Extract arrays
        std::vector<Vector3> positions;
        std::vector<Vector2> rotations;
        
        if (workData.contains("positions")) {
            positions = std::get<std::vector<Vector3>>(workData.at("positions"));
        }
        
        if (workData.contains("rotations")) {
            rotations = std::get<std::vector<Vector2>>(workData.at("rotations"));
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
