#include "CameraRepository.hpp"

#include "Pattern/Singleton.hpp"
#include "Utils.hpp"
#include "src/Json/JsonParams.hpp"

void CameraRepository::Initialize(float _ratio) {
    ratio_ = _ratio;
}

GESTD::ReferencePtr<Camera> CameraRepository::Add(const std::string& _name) {
    std::string actualName = _name.empty() ? GenerateUniqueName() : _name;

    if (!cameras_.contains(actualName)) {
        cameras_[actualName] = std::make_unique<Camera>();
        cameras_[actualName]->Initialize(ratio_);
    }

    return GESTD::ReferencePtr(cameras_[actualName]);
}

GESTD::ReferencePtr<Camera> CameraRepository::Get(const std::string& _name) {
    if (!cameras_.contains(_name)) {
        return nullptr;
    }
    return GESTD::ReferencePtr(cameras_[_name]);
}

void CameraRepository::Remove(const std::string& _name) {
    cameras_.erase(_name);
}

bool CameraRepository::Contains(const std::string& _name) const {
    return cameras_.contains(_name);
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

    auto json = Singleton<JsonParams>::GetInstance();
    if (!json->Load("Camera")) return;

    auto group = json->GetGroups("Camera");
    for (auto& [groupId, object] : group) {
        auto camera = Add(groupId);
        camera->transform_ = {
            {1,1,1},
            std::get<Vector3>(object["Rotate"]),
            std::get<Vector3>(object["Position"])
        };
    }
}

void CameraRepository::SaveToFile() {
    auto json = Singleton<JsonParams>::GetInstance();
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
