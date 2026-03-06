#define NOMINMAX
#include "Ui/Element.hpp"

#include <filesystem>

#include "imgui_internal.h"
#include "Utils.hpp"

namespace Ui {
    void Element::Initialize() {
        uuid_ = Utils::GenerateUniqueId();
        name_ = "NoName";
        data_.visible = true;

        data_.texture = "white_x16.png";
        data_.position = {640.f, 360.f};
        data_.size = {64.f, 64.f};
        data_.color = {1.f, 0.5859f, 0.7421f, 1.f};

        sprite_ = std::make_unique<Sprite>();
        sprite_->Initialize(data_.texture);
        sprite_->SetPosition(data_.position);
        sprite_->SetSize(data_.size);
        sprite_->SetColor(data_.color);

        auto len = std::min(name_.size(), nameBuff_.size() - 1);
        std::copy_n(name_.begin(), len, nameBuff_.begin());
        nameBuff_[len] = '\0';
    }

    void Element::Update() {
        if (!data_.visible)return;

        if (textureBuff_[0] != '\0') {
            data_.texture = textureBuff_.data();
            textureBuff_.fill('\0');
        }

        sprite_->SetTexture(data_.texture);
        sprite_->SetPosition(data_.position + parent_);
        sprite_->SetSize(data_.size);
        sprite_->SetColor(data_.color);
        if (data_.textureSize.x > 0.f && data_.textureSize.y > 0.f) {
            sprite_->SetTextureLeftTop(data_.textureLeftTop);
            sprite_->SetTextureSize(data_.textureSize);
        }
        sprite_->Update();
    }

    void Element::Draw() const {
        if (!data_.visible) return;

        sprite_->Draw();
    }

    void Element::Debug(const std::vector<std::string>& _availableActions) {
        if (isOpen_){
            ImGui::BeginChild("Body", ImVec2(0.f, 280.f), true, ImGuiWindowFlags_NoMove);

            ImGui::Text("Name : ");
            ImGui::SameLine();
            if (changeName_){
                ImGui::SetNextItemWidth(80.f);
                ImGui::InputText("##name", nameBuff_.data(), nameBuff_.size());
                ImGui::SameLine();
                if (nameBuff_[0] == '\0') { ImGui::BeginDisabled(); }
                if (ImGui::Button("Apply")) {
                    name_ = nameBuff_.data();
                    changeName_ = false;
                }
                if (nameBuff_[0] == '\0') { ImGui::EndDisabled(); }
            }else {
                ImGui::SetNextItemWidth(80.f);
                ImGui::TextColored(ImVec4(0.f, 1.f, 0.6f, 1.f), name_.c_str());
                ImGui::SameLine();
                if (ImGui::Button("Change")){
                    auto len = std::min(name_.size(), nameBuff_.size() - 1);
                    std::copy_n(name_.begin(), len, nameBuff_.begin());
                    nameBuff_[len] = '\0';
                    changeName_ = true;
                }
            }

            ImGui::Separator();

            ImGui::Text("Visible : ");
            ImGui::SameLine();
            ImGui::Checkbox("##visible", &data_.visible);

            ImGui::Separator();

            ImGui::Text("Events");
            for (auto eventKey : ALL_EVENT_KEYS) {
                const char* label = EventKeyToString(eventKey);
                auto it = events_.find(eventKey);
                std::string current = (it != events_.end()) ? it->second : "";

                ImGui::Text("%s", label);
                ImGui::SameLine();
                ImGui::SetNextItemWidth(150.f);

                std::string comboId = std::string("##event_") + label;
                if (ImGui::BeginCombo(comboId.c_str(), current.empty() ? "(None)" : current.c_str())) {
                    if (ImGui::Selectable("(None)", current.empty())) {
                        events_.erase(eventKey);
                    }
                    for (const auto& key : _availableActions) {
                        bool selected = (current == key);
                        if (ImGui::Selectable(key.c_str(), selected)) {
                            events_[eventKey] = key;
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            ImGui::Separator();

            ImGui::Text("Params");

            ImGui::Text("Texture: ");
            ImGui::SameLine();
            if (!data_.texture.empty()) {
                ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), data_.texture.c_str());
            }
            ImGui::SameLine();
            if (ImGui::Button("...##BrowseTexture")) {
                std::string selectedFile = Utils::OpenFileDialog("PNG");
                if (!selectedFile.empty()) {
                    std::filesystem::path fullPath(selectedFile);
                    std::string filename = fullPath.filename().string();
                    strcpy_s(textureBuff_.data(), textureBuff_.size(), filename.c_str());
                }
            }

            ImGui::Text("Position");
            ImGui::SetNextItemWidth(120.f);
            ImGui::DragFloat2("##pos", &data_.position.x);

            ImGui::Text("Size : AspectLock");
            ImGui::SameLine();
            ImGui::Checkbox("##asplock", &aspectLock_);
            ImGui::SetNextItemWidth(120.f);
            if (aspectLock_) {
                if (ImGui::DragFloat("##size", &data_.size.x)){
                    data_.size.y = data_.size.x;
                }
            } else {
                ImGui::DragFloat2("##size", &data_.size.x);
            }

            ImGui::Text("Color");
            ImGui::ColorEdit4("##color", &data_.color.x);

            ImGui::EndChild();
        }
    }

    bool Element::IsVisible() const {
        return data_.visible;
    }

    std::string Element::GetName() const {
        return name_;
    }

    void Element::SetName(const std::string& _name) {
        name_ = _name;
        auto len = std::min(name_.size(), nameBuff_.size() - 1);
        std::copy_n(name_.begin(), len, nameBuff_.begin());
        nameBuff_[len] = '\0';
    }

    std::string Element::GetUUID() const {
        return uuid_;
    }

    bool& Element::IsOpen() {
        return isOpen_;
    }

    Element::Data Element::GetData() const {
        return data_;
    }

    void Element::SetData(const Data& _data) {
        data_ = _data;
    }

    void Element::SetEvent(EventKey _event, const std::string& _actionKey) {
        if (_actionKey.empty()) {
            events_.erase(_event);
        } else {
            events_[_event] = _actionKey;
        }
    }

    static const std::string EMPTY_STRING;

    const std::string& Element::GetActionKey(EventKey _event) const {
        auto it = events_.find(_event);
        if (it != events_.end()) {
            return it->second;
        }
        return EMPTY_STRING;
    }

    bool Element::HasEvent(EventKey _event) const {
        return events_.contains(_event);
    }

    bool Element::HasAnyEvent() const {
        return !events_.empty();
    }

    const std::unordered_map<EventKey, std::string>& Element::GetEvents() const {
        return events_;
    }

    void Element::SetParent(const Vector2& _pos) {
        parent_ = _pos;
    }
}
