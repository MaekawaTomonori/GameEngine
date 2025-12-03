#include "CameraRepository.hpp"

#include "Pattern/Singleton.hpp"
#include "Utils.hpp"
#include "src/Json/Json.hpp"

void CameraRepository::Initialize(float _ratio) {
    ratio_ = ratio;
}

Camera* CameraRepository::Add(const std::string& _name) {
    std::string actualName = name.empty() ? GenerateUniqueName() : name;

    if (cameras_.contains(actualName)) {
        return cameras_[actualName].get();
    }

    cameras_[actualName] = std::make_unique<Camera>();
    cameras_[actualName]->Initialize(ratio_);
    return cameras_[actualName].get();
}

Camera* CameraRepository::Get(const std::string& _name) {
    if (!cameras_.contains(name)) {
        return nullptr;
    }
    return cameras_[name].get();
}

void CameraRepository::Remove(const std::string& _name) {
    cameras_.erase(name);
}

bool CameraRepository::Contains(const std::string& _name) const {
    return cameras_.contains(name);
}

bool CameraRepository::IsEmpty() const {
    return cameras_.empty();
}

std::vector<std::string> CameraRepository::GetNames() const {
    std::vector<std::string> names;
    names.reserve(cameras_.size());
    for (const auto& name : cameras_ | std::views::keys) {
        names.push_back(name);
    }
    return names;
}

std::string CameraRepository::GetFirstName() const {
    if (cameras_.empty()) {
        return "";
    }
    return cameras_.begin()->first;
}

void CameraRepository::LoadFromFile() {
    Clear();

    Json* json = Singleton<Json>::GetInstance();
    if (!json->Load("Camera")) return;

    auto group = json->GetGroups("Camera");
    for (auto& [groupId, object] : group) {
        Camera* camera = Add(groupId);
        camera->transform_ = {
            {1,1,1},
            std::get<Vector3>(object["Rotate"]),
            std::get<Vector3>(object["Position"])
        };
    }
}

void CameraRepository::SaveToFile() {
    Json* json = Singleton<Json>::GetInstance();
    for (auto& [name, camera] : cameras_) {
        json->SetValue("Camera", name, "Position", camera->transform_.translate);
        json->SetValue("Camera", name, "Rotate", std::get<Vector3>(camera->transform_.rotate));
    }
    json->Save("Camera");
}

void CameraRepository::Clear() {
    cameras_.clear();
    nonameCount_ = 0;
}

std::string CameraRepository::GenerateUniqueName() {
    return "noname" + std::to_string(nonameCount_++);
}
