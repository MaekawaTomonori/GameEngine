#include "Ui/UserInterface.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "imgui_internal.h"
#include "Log.hpp"
#include "Pattern/Singleton.hpp"
#include "SpriteElement.hpp"
#include "TextElement.hpp"
#include "Ui/UiManager.hpp"
#include "src/Json/JsonParams.hpp"

#undef min
#undef max

namespace Ui {
    static std::unique_ptr<Element> CreateElement(std::string_view _type) {
        if (_type == "Text") return std::make_unique<TextElement>();
        return std::make_unique<SpriteElement>();
    }

    void Canvas::Setup(const std::string& _name) {
        if (_name.empty()) return;

        constexpr std::string_view ext = ".json";

        name_ = _name;

        auto len = std::min(name_.size(), buffName_.size() - 1);
        std::copy_n(name_.begin(), len, buffName_.begin());
        buffName_[len] = '\0';

        Load(_name + ext.data());

        Singleton<Manager>::GetInstance()->Register(name_, GESTD::ReferencePtr(this, lifetime_));
    }

    void Canvas::Update(float dt) {
        if (phase_ == Phase::Inactive) return;

        if (reload_) {
            reload_ = false;
            elements_.clear();
            Load(name_ + ".json");
        }

        if (phase_ == Phase::Showing) {
            const bool allDone = std::ranges::all_of(elements_,
                [](const auto& e) { return e->IsShowDone(); });
            if (allDone) {
                phase_ = Phase::Idle;
                for (auto& e : elements_) e->PlayIdle();
            }
        } else if (phase_ == Phase::Hiding) {
            const bool allDone = std::ranges::all_of(elements_,
                [](const auto& e) { return e->IsHideDone(); });
            if (allDone) {
                phase_ = Phase::Inactive;
                return;
            }
        }

        for (auto& element : elements_) {
            element->SetParent(position_);
            element->Update(dt);
        }
    }

    void Canvas::Draw() const {
        if (phase_ == Phase::Inactive) return;

        for (const auto& element : elements_) {
            if (!element->IsVisible()) continue;
            element->Draw();
        }
    }

    GESTD::ReferencePtr<Canvas> Canvas::Find(const std::string& _name) {
        return Singleton<Ui::Manager>::GetInstance()->GetCanvas(_name);
    }

    void Canvas::Editor(bool* _open) {
        const std::string title = name_.empty() ? "Editor" : name_ + " Editor";
        ImGui::Begin(title.c_str(), _open);

        HeaderUiParams();
        ElementCommonParams();
        ElementsParam();

        if (customDebug_) {
            ImGui::Separator();
            customDebug_();
        }

        ImGui::End();
    }

    void Canvas::SetCustomDebug(std::function<void()> _func) {
        customDebug_ = std::move(_func);
    }

    void Canvas::Reload() { reload_ = true; }

    void Canvas::RegisterAction(const std::string& _actionKey, const std::function<void()>& _action) {
        eventSystem_.Register(_actionKey, _action);
    }

    void Canvas::RegisterAnimFunc(const std::string& _key, AnimFunc _func) {
        animFuncs_[_key] = std::move(_func);
    }

    void Canvas::SetActive(bool _active) {
        if (_active) {
            if (phase_ == Phase::Inactive || phase_ == Phase::Hiding) {
                ResolveAnimFuncs();
                phase_ = Phase::Showing;
                for (auto& e : elements_) e->PlayShow();
                // Show アニメーションが全要素で即完了（未設定）なら即座に Idle へ
                const bool allDone = std::ranges::all_of(elements_,
                    [](const auto& e) { return e->IsShowDone(); });
                if (allDone) {
                    phase_ = Phase::Idle;
                    for (auto& e : elements_) e->PlayIdle();
                }
            }
        } else {
            if (phase_ == Phase::Idle || phase_ == Phase::Showing) {
                phase_ = Phase::Hiding;
                for (auto& e : elements_) e->PlayHide();
                // Hide アニメーションが全要素で即完了（未設定）なら即座に Inactive へ
                const bool allDone = std::ranges::all_of(elements_,
                    [](const auto& e) { return e->IsHideDone(); });
                if (allDone) phase_ = Phase::Inactive;
            }
        }
    }

    bool          Canvas::IsActive()  const { return phase_ != Phase::Inactive; }
    bool          Canvas::IsDirty()   const { return dirty_; }
    Canvas::Phase Canvas::GetPhase()  const { return phase_; }

    void Canvas::ResolveAnimFuncs() {
        for (auto& element : elements_) {
            if (!element->GetShowAnimKey().empty()) {
                auto it = animFuncs_.find(element->GetShowAnimKey());
                if (it != animFuncs_.end()) element->SetShowFunc(it->second);
            }
            if (!element->GetIdleAnimKey().empty()) {
                auto it = animFuncs_.find(element->GetIdleAnimKey());
                if (it != animFuncs_.end()) element->SetIdleFunc(it->second);
            }
            if (!element->GetHideAnimKey().empty()) {
                auto it = animFuncs_.find(element->GetHideAnimKey());
                if (it != animFuncs_.end()) element->SetHideFunc(it->second);
            }
        }
    }

    std::vector<size_t> Canvas::GetIndicesWithEvent(EventKey _event) const {
        std::vector<size_t> indices;
        for (size_t i = 0; i < elements_.size(); ++i) {
            if (elements_[i]->HasEvent(_event)) indices.push_back(i);
        }
        return indices;
    }

    void Canvas::ExecuteAction(const std::string& _actionKey) {
        eventSystem_.Execute(_actionKey);
    }

    void Canvas::ExecuteActionAt(size_t _elementIndex, EventKey _event) {
        if (_elementIndex >= elements_.size()) return;
        const auto& actionKey = elements_[_elementIndex]->GetActionKey(_event);
        if (!actionKey.empty()) eventSystem_.Execute(actionKey);
    }

    Canvas::ElementRect Canvas::GetElementRect(size_t _elementIndex) const {
        if (_elementIndex >= elements_.size()) return {};
        const auto& elem = elements_[_elementIndex];
        return { elem->GetPosition(), elem->GetSize() };
    }

    std::vector<std::string> Canvas::GetActionKeys() const {
        return eventSystem_.GetActionKeys();
    }

    std::vector<std::string> Canvas::GetAnimFuncKeys() const {
        std::vector<std::string> keys;
        keys.reserve(animFuncs_.size());
        for (const auto& k : animFuncs_ | std::views::keys) keys.push_back(k);
        return keys;
    }

    void Canvas::SetElementEvent(const std::string& _name, EventKey _event, const std::string& _actionKey) {
        auto* elem = FindElementByName(_name);
        if (!elem) return;
        elem->SetEvent(_event, _actionKey);
    }

    Element* Canvas::FindElementByName(const std::string& _name) const {
        for (auto& element : elements_) {
            if (element->GetName() == _name) return element.get();
        }
        return nullptr;
    }

    Element* Canvas::GetElement(size_t _index) const {
        if (_index >= elements_.size()) return nullptr;
        return elements_[_index].get();
    }

    size_t Canvas::GetElementCount() const { return elements_.size(); }

    void Canvas::HeaderUiParams() {
        if (changingName_) {
            ImGui::SetNextItemWidth(160.f);
            ImGui::InputText("##name", buffName_.data(), buffName_.size());
            ImGui::SameLine();
            if (ImGui::Button("Apply")) {
                name_ = buffName_.data();
                changingName_ = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                auto len = std::min(name_.size(), buffName_.size() - 1);
                std::copy_n(name_.begin(), len, buffName_.begin());
                buffName_[len] = '\0';
                changingName_ = false;
            }
        } else {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.f, 1.f), "%s", name_.c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Rename")) changingName_ = true;
        }

        if (ImGui::Button("Save",   ImVec2(80.f, 0.f))) Save();
        ImGui::SameLine();
        if (ImGui::Button("Reload", ImVec2(80.f, 0.f))) reload_ = true;

        ImGui::Text("Position"); ImGui::SameLine();
        ImGui::SetNextItemWidth(100.f);
        ImGui::DragFloat2("##pos", &position_.x);
    }

    void Canvas::ElementCommonParams() {
        ImGui::Text("Editing UI Element");

        static int typeIndex = 0;
        const char* typeNames[] = { "Sprite", "Text" };
        ImGui::SetNextItemWidth(70.f);
        ImGui::Combo("##addType", &typeIndex, typeNames, 2);
        ImGui::SameLine();
        if (ImGui::Button("Add Element")) {
            auto element = CreateElement(typeNames[typeIndex]);
            element->Initialize();
            element->IsOpen() = defaultOpen_;
            elements_.emplace_back(std::move(element));
        }

        bool exe  = false;
        bool open = true;
        if (ImGui::Button("Open"))  { exe = true; open = true; }
        ImGui::SameLine();
        if (ImGui::Button("Close")) { exe = true; open = false; }
        ImGui::SameLine();
        ImGui::Checkbox("Default Open", &defaultOpen_);

        if (exe) {
            for (auto& element : elements_) element->IsOpen() = open;
        }
    }

    void Canvas::ElementsParam() {
        ImGui::BeginChild("Elements", ImVec2(0.f, 0.f), true, ImGuiWindowFlags_NoMove);

        std::vector<std::unique_ptr<Element>> duplicated;
        std::vector<std::string> toDelete;

        for (auto itr = elements_.begin(); itr != elements_.end(); ++itr) {
            bool& isOpen = (*itr)->IsOpen();

            ImGui::PushID((*itr)->GetUUID().c_str());
            ImGui::BeginGroup();

            if (ImGui::SmallButton(isOpen ? "-" : "+")) isOpen = !isOpen;
            ImGui::SameLine();
            ImGui::Text("%s", (*itr)->GetName().c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("[%s]", std::string((*itr)->GetType()).c_str());

            ImGui::SameLine(ImGui::GetWindowWidth() - 150.f);

            ImGui::BeginDisabled(itr == elements_.begin());
            if (ImGui::SmallButton("^")) { std::iter_swap(itr, std::prev(itr)); dirty_ = true; }
            ImGui::EndDisabled();
            ImGui::SameLine();

            ImGui::BeginDisabled(itr == std::prev(elements_.end()));
            if (ImGui::SmallButton("v")) { std::iter_swap(itr, std::next(itr)); dirty_ = true; }
            ImGui::EndDisabled();
            ImGui::SameLine();

            if (ImGui::SmallButton("[+]")) duplicated.push_back((*itr)->Clone());
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Duplicate");
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.6f, 0.1f, 0.1f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.5f, 0.0f, 0.0f, 1.f));
            if (ImGui::SmallButton("[x]")) toDelete.push_back((*itr)->GetUUID());
            ImGui::PopStyleColor(3);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Delete");

            ImGui::EndGroup();

            (*itr)->Debug(GetActionKeys(), GetAnimFuncKeys());
            ImGui::PopID();
        }

        ImGui::Separator();
        ImGui::EndChild();

        for (const auto& uuid : toDelete) {
            std::erase_if(elements_, [&uuid](const std::unique_ptr<Element>& e) {
                return e->GetUUID() == uuid;
            });
            dirty_ = true;
        }

        for (auto& dup : duplicated) {
            dup->IsOpen() = defaultOpen_;
            elements_.push_back(std::move(dup));
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
        } catch (const nlohmann::json::parse_error& e) {
            Log::Send(Log::Level::ERR, "Canvas: JSON parse error - " + std::string(e.what()));
            file.close();
            return;
        }

        if (root.contains("Position") && root["Position"].is_array() && root["Position"].size() >= 2) {
            position_ = { root["Position"][0].get<float>(), root["Position"][1].get<float>() };
        } else {
            position_ = {};
        }

        elements_.clear();

        if (!root.contains("Elements") || !root["Elements"].is_array()) {
            ResolveAnimFuncs();
            return;
        }

        for (const auto& j : root["Elements"]) {
            const std::string type = j.value("Type", "Sprite");
            auto element = CreateElement(type);
            element->Initialize();

            element->SetName(j.value("Name", "NoName"));
            element->SetVisible(j.value("Visible", true));
            element->SetShowAnimKey(j.value("ShowAnim", ""));
            element->SetIdleAnimKey(j.value("IdleAnim", ""));
            element->SetHideAnimKey(j.value("HideAnim", ""));

            if (j.contains("Position") && j["Position"].is_array() && j["Position"].size() >= 2) {
                element->SetPosition({ j["Position"][0].get<float>(), j["Position"][1].get<float>() });
            }
            if (j.contains("Color") && j["Color"].is_array() && j["Color"].size() >= 4) {
                element->SetColor({
                    j["Color"][0].get<float>(), j["Color"][1].get<float>(),
                    j["Color"][2].get<float>(), j["Color"][3].get<float>() });
            }

            if (j.contains("Events") && j["Events"].is_object()) {
                for (auto& [keyStr, val] : j["Events"].items()) {
                    element->SetEvent(StringToEventKey(keyStr), val.get<std::string>());
                }
            }

            if (type == "Text") {
                if (auto* te = dynamic_cast<TextElement*>(element.get())) {
                    TextElement::Data td;
                    td.text     = j.value("Text",     "");
                    td.fontSize = j.value("FontSize",  32.f);
                    te->SetTextData(td);
                }
            } else {
                if (auto* se = dynamic_cast<SpriteElement*>(element.get())) {
                    SpriteElement::Data sd;
                    sd.texture = j.value("Texture", "white_x16.png");
                    if (j.contains("Size") && j["Size"].is_array() && j["Size"].size() >= 2) {
                        sd.size = { j["Size"][0].get<float>(), j["Size"][1].get<float>() };
                    }
                    if (j.contains("TextureLeftTop") && j["TextureLeftTop"].is_array() && j["TextureLeftTop"].size() >= 2) {
                        sd.textureLeftTop = { j["TextureLeftTop"][0].get<float>(), j["TextureLeftTop"][1].get<float>() };
                    }
                    if (j.contains("TextureSize") && j["TextureSize"].is_array() && j["TextureSize"].size() >= 2) {
                        sd.textureSize = { j["TextureSize"][0].get<float>(), j["TextureSize"][1].get<float>() };
                    }
                    se->SetSpriteData(sd);
                }
            }

            element->IsOpen() = defaultOpen_;
            elements_.push_back(std::move(element));
        }

        ResolveAnimFuncs();
        if (phase_ == Phase::Idle) {
            for (auto& e : elements_) e->PlayIdle();
        }

        Log::Send(Log::Level::INFO,
            "Canvas: Loaded " + _name + " (" + std::to_string(elements_.size()) + " elements)");
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
            nlohmann::json j;

            j["Type"]     = std::string(element->GetType());
            j["Name"]     = element->GetName();
            j["Position"] = { element->GetPosition().x, element->GetPosition().y };
            j["Color"]    = { element->GetColor().x, element->GetColor().y,
                              element->GetColor().z, element->GetColor().w };

            if (!element->IsVisible())              j["Visible"]  = false;
            if (!element->GetShowAnimKey().empty()) j["ShowAnim"] = element->GetShowAnimKey();
            if (!element->GetIdleAnimKey().empty()) j["IdleAnim"] = element->GetIdleAnimKey();
            if (!element->GetHideAnimKey().empty()) j["HideAnim"] = element->GetHideAnimKey();

            const auto& events = element->GetEvents();
            if (!events.empty()) {
                nlohmann::json eventsJson;
                for (const auto& [eventKey, actionKey] : events) {
                    eventsJson[EventKeyToString(eventKey)] = actionKey;
                }
                j["Events"] = eventsJson;
            }

            if (auto* te = dynamic_cast<const TextElement*>(element.get())) {
                const auto td = te->GetTextData();
                j["Text"]     = td.text;
                j["FontSize"] = td.fontSize;
            } else if (auto* se = dynamic_cast<const SpriteElement*>(element.get())) {
                const auto sd = se->GetSpriteData();
                j["Texture"] = sd.texture;
                j["Size"]    = { sd.size.x, sd.size.y };
                if (sd.textureSize.x > 0.f && sd.textureSize.y > 0.f) {
                    j["TextureLeftTop"] = { sd.textureLeftTop.x, sd.textureLeftTop.y };
                    j["TextureSize"]    = { sd.textureSize.x, sd.textureSize.y };
                }
            }

            elementsArray.push_back(j);
        }

        root["Elements"] = elementsArray;

        const std::string filePath = std::string(JSON_PATH) + name_ + ".json";
        std::filesystem::create_directories(JSON_PATH);
        std::ofstream file(filePath);
        if (!file.is_open()) {
            Log::Send(Log::Level::ERR, "Canvas: Failed to open file for saving - " + filePath);
            return;
        }

        file << root.dump(4);
        file.close();

        Log::Send(Log::Level::INFO,
            "Canvas: Saved " + name_ + ".json (" + std::to_string(elements_.size()) + " elements)");
    }

} // namespace Ui
