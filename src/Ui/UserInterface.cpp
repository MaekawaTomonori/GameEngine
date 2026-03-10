#include "Ui/UserInterface.hpp"

#include <filesystem>
#include <fstream>

#include "imgui_internal.h"
#include "Log.hpp"
#include "src/Json/JsonParams.hpp"

#undef min
#undef max

namespace Ui {
    void Canvas::Setup(const std::string& _name) {
        if (_name.empty()) return;

        constexpr std::string_view ext = ".json";

        name_ = _name;

        auto len = std::min(name_.size(), buffName_.size() - 1);
        std::copy_n(name_.begin(), len, buffName_.begin());
        buffName_[len] = '\0';

        Load(_name + ext.data());
    }

    void Canvas::Update() {
        //if (!active_) return;

        if (reload_) {
            reload_ = false;

            elements_.clear();
            Load(name_ + ".json");
        }

        for (auto& element : elements_) {
            element->SetParent(position_);
            element->Update();
        }
    }

    void Canvas::Draw() const {
        if (!active_) return;

        for (auto& element : elements_) {
            if (!element->IsVisible()) continue;
            element->Draw();
        }
    }

    void Canvas::StandaloneDebug() {
        ImGui::Begin("UI");

        Debug();

        ImGui::End();

        if (editing_) {
            Editor();
        }
    }

    void Canvas::Debug() {
        HeaderUiParams();
        ElementCommonParams();
        ElementsParam();
    }

    void Canvas::RegisterAction(const std::string& _actionKey, const std::function<void()>& _action) {
        eventSystem_.Register(_actionKey, _action);
    }

    void Canvas::SetActive(bool _active) {
        active_ = _active;
    }

    bool Canvas::IsActive() const {
        return active_;
    }

    bool Canvas::IsDirty() const {
        return dirty_;
    }

    std::vector<size_t> Canvas::GetIndicesWithEvent(EventKey _event) const {
        std::vector<size_t> indices;
        for (size_t i = 0; i < elements_.size(); ++i) {
            if (elements_[i]->HasEvent(_event)) {
                indices.push_back(i);
            }
        }
        return indices;
    }

    void Canvas::ExecuteAction(const std::string& _actionKey) {
        eventSystem_.Execute(_actionKey);
    }

    void Canvas::ExecuteActionAt(size_t _elementIndex, EventKey _event) {
        if (_elementIndex >= elements_.size()) return;
        const auto& actionKey = elements_[_elementIndex]->GetActionKey(_event);
        if (!actionKey.empty()) {
            eventSystem_.Execute(actionKey);
        }
    }

    Canvas::ElementRect Canvas::GetElementRect(size_t _elementIndex) const {
        if (_elementIndex >= elements_.size()) {
            return {};
        }
        auto data = elements_[_elementIndex]->GetData();
        return {data.position, data.size};
    }

    std::vector<std::string> Canvas::GetActionKeys() const {
        return eventSystem_.GetActionKeys();
    }

    Element* Canvas::FindElementByName(const std::string& _name) {
        for (auto& element : elements_) {
            if (element->GetName() == _name) {
                return element.get();
            }
        }
        return nullptr;
    }

    Element* Canvas::GetElement(size_t _index) const {
        if (_index >= elements_.size()) return nullptr;
        return elements_[_index].get();
    }

    size_t Canvas::GetElementCount() const {
        return elements_.size();
    }

    void Canvas::Editor() {
        std::string title = "Editor";
        if (!name_.empty()) {
            title = name_ + " Editor";
        }

        ImGui::Begin(title.c_str(), &editing_);

        HeaderUiParams();
        ElementCommonParams();

        ElementsParam();
        ImGui::End();
    }

    void Canvas::HeaderUiParams() {
        ImGui::InputText("##name", buffName_.data(), buffName_.size());
        name_ = buffName_.data();

        const char* active = active_ ? "Active" : "Inactive";
        if (ImGui::Button(active, ImVec2(100.f, 0.f))) {
            active_ = !active_;
        }
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(100.f, 0.f))) {
            Save();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Re")) {
            reload_ = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Reload Json");
        }
        ImGui::Text("Position");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.f);
        ImGui::DragFloat2("##pos", &position_.x);

    }

    void Canvas::ElementCommonParams() {
        ImGui::Text("Editing UI Element");

        if (ImGui::Button("Add Element", ImVec2(100.f, 0.f))) {
            auto element = std::make_unique<Element>();
            element->Initialize();
            element->IsOpen() = defaultOpen_;
            elements_.emplace_back(std::move(element));
        }

        bool exe = false;
        bool open = true;
        if (ImGui::Button("Open")) {
            exe = true;
            open = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            exe = true;
            open = false;
        }
        ImGui::SameLine();
        ImGui::Checkbox("Default Open", &defaultOpen_);

        if (exe) {
            for (auto& element : elements_) {
                element->IsOpen() = open;
            }
        }
    }

    void Canvas::ElementsParam() {
        ImGui::BeginChild("Elements", ImVec2(0.f, 0.f), true, ImGuiWindowFlags_NoMove);

        std::vector<Element::Data> duplicated;

        for (auto itr = elements_.begin(); itr != elements_.end(); ++itr) {
            bool& isOpen = (*itr)->IsOpen();

            ImGui::PushID((*itr)->GetUUID().c_str());

            ImGui::BeginGroup();

            if (ImGui::SmallButton(isOpen ? "-" : "+")) {
                isOpen = !isOpen;
            }

            ImGui::SameLine();

            ImGui::Text((*itr)->GetName().c_str());

            ImGui::SameLine(ImGui::GetWindowWidth() - 120.f);

            ImGui::BeginDisabled(itr == elements_.begin());
            if (ImGui::SmallButton("^")) {
                std::iter_swap(itr, std::prev(itr));
                dirty_ = true;
            }
            ImGui::EndDisabled();
            ImGui::SameLine();

            ImGui::BeginDisabled(itr == std::prev(elements_.end()));
            if (ImGui::SmallButton("v")) {
                std::iter_swap(itr, std::next(itr));
                dirty_ = true;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::SmallButton("[+]")) {
                duplicated.push_back((*itr)->GetData());
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Duplicate");
            }

            ImGui::EndGroup();

            (*itr)->Debug(GetActionKeys());

            ImGui::PopID();
        }

        ImGui::Separator();

        ImGui::EndChild();

        for (auto& data : duplicated) {
            auto element = std::make_unique<Element>();
            element->Initialize();
            element->SetData(data);
            element->IsOpen() = defaultOpen_;
            elements_.push_back(std::move(element));
        }
    }

    void Canvas::Load(const std::string& _name) {
        std::ifstream file(std::string(JSON_PATH) + _name);
        if (!file.is_open()) {
            Log::Send(Log::Level::WARNING, "Canvas: JSON file not found - " + _name);
            return;
        }

        nlohmann::json root;
        try {
            file >> root;
            file.close();
        }
        catch (const nlohmann::json::parse_error& e) {
            Log::Send(Log::Level::ERR, "Canvas: JSON parse error - " + std::string(e.what()));
            file.close();
            return;
        }

        if (root.contains("Position") && root["Position"].is_array() && root["Position"].size() >= 2) {
            position_ = {
                root["Position"][0].get<float>(),
                root["Position"][1].get<float>()
            };
        } else {
            position_ = {0.f, 0.f};
        }

        if (root.contains("Elements") && root["Elements"].is_array()) {
            elements_.clear();

            for (const auto& elementData : root["Elements"]) {
                Element::Data data;

                data.texture = elementData.value("Texture", "white_x16.png");

                if (elementData.contains("Position") && elementData["Position"].is_array() && elementData["Position"].size() >= 2) {
                    data.position = {
                        elementData["Position"][0].get<float>(),
                        elementData["Position"][1].get<float>()
                    };
                }

                if (elementData.contains("Size") && elementData["Size"].is_array() && elementData["Size"].size() >= 2) {
                    data.size = {
                        elementData["Size"][0].get<float>(),
                        elementData["Size"][1].get<float>()
                    };
                }

                if (elementData.contains("Color") && elementData["Color"].is_array() && elementData["Color"].size() >= 4) {
                    data.color = {
                        elementData["Color"][0].get<float>(),
                        elementData["Color"][1].get<float>(),
                        elementData["Color"][2].get<float>(),
                        elementData["Color"][3].get<float>()
                    };
                }

                if (elementData.contains("Visible") && elementData["Visible"].is_boolean()) {
                    data.visible = elementData["Visible"].get<bool>();
                }

                if (elementData.contains("TextureLeftTop") && elementData["TextureLeftTop"].is_array() && elementData["TextureLeftTop"].size() >= 2) {
                    data.textureLeftTop = {
                        elementData["TextureLeftTop"][0].get<float>(),
                        elementData["TextureLeftTop"][1].get<float>()
                    };
                }

                if (elementData.contains("TextureSize") && elementData["TextureSize"].is_array() && elementData["TextureSize"].size() >= 2) {
                    data.textureSize = {
                        elementData["TextureSize"][0].get<float>(),
                        elementData["TextureSize"][1].get<float>()
                    };
                }

                std::string name = elementData.value("Name", "NoName");

                auto element = std::make_unique<Element>();
                element->Initialize();
                element->SetName(name);
                element->SetData(data);
                element->IsOpen() = defaultOpen_;

                if (elementData.contains("Events") && elementData["Events"].is_object()) {
                    for (auto& [eventKeyStr, actionKeyVal] : elementData["Events"].items()) {
                        EventKey eventKey = StringToEventKey(eventKeyStr);
                        element->SetEvent(eventKey, actionKeyVal.get<std::string>());
                    }
                }

                elements_.push_back(std::move(element));
            }
        }

        Log::Send(Log::Level::INFO, "Canvas: Loaded " + _name + " (" + std::to_string(elements_.size()) + " elements)");
    }

    void Canvas::Save() {
        if (name_.empty()) {
            Log::Send(Log::Level::WARNING, "Canvas: Cannot save - no name specified");
            return;
        }

        nlohmann::json root;
        root["Position"] = { position_.x, position_.y };

        nlohmann::json elementsArray = nlohmann::json::array();

        for (const auto& element : elements_) {
            nlohmann::json elementJson;
            auto data = element->GetData();

            elementJson["Name"] = element->GetName();
            elementJson["Texture"] = data.texture;
            elementJson["Position"] = { data.position.x, data.position.y };
            elementJson["Size"] = { data.size.x, data.size.y };
            elementJson["Color"] = { data.color.x, data.color.y, data.color.z, data.color.w };

            if (!data.visible) {
                elementJson["Visible"] = false;
            }

            if (data.textureSize.x > 0.f && data.textureSize.y > 0.f) {
                elementJson["TextureLeftTop"] = { data.textureLeftTop.x, data.textureLeftTop.y };
                elementJson["TextureSize"] = { data.textureSize.x, data.textureSize.y };
            }

            const auto& events = element->GetEvents();
            if (!events.empty()) {
                nlohmann::json eventsJson;
                for (const auto& [eventKey, actionKey] : events) {
                    eventsJson[EventKeyToString(eventKey)] = actionKey;
                }
                elementJson["Events"] = eventsJson;
            }

            elementsArray.push_back(elementJson);
        }

        root["Elements"] = elementsArray;

        std::string filePath = std::string(JSON_PATH) + name_ + ".json";
        std::ofstream file(filePath);
        if (!file.is_open()) {
            Log::Send(Log::Level::ERR, "Canvas: Failed to open file for saving - " + filePath);
            return;
        }

        file << root.dump(4);
        file.close();

        Log::Send(Log::Level::INFO, "Canvas: Saved " + name_ + ".json (" + std::to_string(elements_.size()) + " elements)");
    }
}
