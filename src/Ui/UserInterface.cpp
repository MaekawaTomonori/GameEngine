#include "Ui/UserInterface.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "imgui_internal.h"
#include "Input.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "Pattern/Singleton.hpp"
#include "SpriteElement.hpp"
#include "TextElement.hpp"
#include "Ui/Module/KeyboardModule.hpp"
#include "Ui/Module/MouseModule.hpp"
#include "Ui/UiManager.hpp"
#include "src/Json/JsonParams.hpp"

#undef min
#undef max

namespace Ui {

    // -----------------------------------------------------------------------
    //  Helpers
    // -----------------------------------------------------------------------
    static std::unique_ptr<Element> CreateElement(std::string_view _type) {
        if (_type == "Text") return std::make_unique<TextElement>();
        return std::make_unique<SpriteElement>();
    }

    static const char* DikToName(uint8_t code) {
        for (const auto& k : KeyCodeMap) {
            if (k.code == code) return k.name;
        }
        return "?";
    }

    // -----------------------------------------------------------------------
    //  Setup / lifecycle
    // -----------------------------------------------------------------------
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

        if (input_) {
            for (const auto& module : input_->modules) {
                module->Draw();
            }
        }
    }

    void Canvas::ProcessInput(const Input& _input) {
        if (!input_) return;
        for (auto& module : input_->modules) {
            module->Update(*this, _input);
        }
    }

    // -----------------------------------------------------------------------
    //  SetActive
    // -----------------------------------------------------------------------
    void Canvas::SetActive(bool _active) {
        if (_active) {
            if (phase_ == Phase::Inactive || phase_ == Phase::Hiding) {
                ResolveAnimFuncs();
                phase_ = Phase::Showing;
                for (auto& e : elements_) e->PlayShow();
                const bool allDone = std::ranges::all_of(elements_,
                    [](const auto& e) { return e->IsShowDone(); });
                if (allDone) {
                    phase_ = Phase::Idle;
                    for (auto& e : elements_) e->PlayIdle();
                }

                if (input_) {
                    input_->navDirty = true;
                    InitCursor();
                    Singleton<Manager>::GetInstance()->SetFocusedCanvas(this);
                }
            }
        } else {
            if (phase_ == Phase::Idle || phase_ == Phase::Showing) {
                phase_ = Phase::Hiding;
                for (auto& e : elements_) e->PlayHide();
                const bool allDone = std::ranges::all_of(elements_,
                    [](const auto& e) { return e->IsHideDone(); });
                if (allDone) phase_ = Phase::Inactive;

                if (input_) input_->hasFocus = false;
            }
        }
    }

    // -----------------------------------------------------------------------
    //  Cursor
    // -----------------------------------------------------------------------
    void Canvas::MoveCursorTo(int idx) {
        if (!input_) return;
        if (idx < 0 || idx >= static_cast<int>(elements_.size())) return;
        if (idx == input_->cursorIndex) return;
        input_->cursorIndex = idx;
        if (input_->hasFocus)
            ExecuteActionAt(static_cast<size_t>(idx), EventKey::Hover);
    }

    void Canvas::MoveCursorDir(CursorDir dir) {
        if (!input_) return;
        if (input_->navDirty) RebuildNavGraph();
        if (input_->navSelectables.empty()) return;

        if (input_->cursorIndex < 0) {
            MoveCursorTo(ResolveDefaultFocus());
            return;
        }

        int curSi = -1;
        for (int si = 0; si < static_cast<int>(input_->navSelectables.size()); ++si) {
            if (input_->navSelectables[si] == input_->cursorIndex) { curSi = si; break; }
        }
        if (curSi < 0) { MoveCursorTo(input_->navSelectables[0]); return; }

        const auto& nb = input_->navGraph[curSi];
        int next = -1;
        switch (dir) {
            case CursorDir::Up:    next = nb.up;    break;
            case CursorDir::Down:  next = nb.down;  break;
            case CursorDir::Left:  next = nb.left;  break;
            case CursorDir::Right: next = nb.right; break;
        }
        if (next >= 0) MoveCursorTo(next);
    }

    void Canvas::ExecuteCursor() {
        if (!input_ || input_->cursorIndex < 0) return;
        if (!input_->hasFocus) return;
        ExecuteActionAt(static_cast<size_t>(input_->cursorIndex), EventKey::Execute);
    }

    int Canvas::GetCursorIndex() const {
        return input_ ? input_->cursorIndex : -1;
    }

    void Canvas::InitCursor() {
        if (!input_) return;
        input_->cursorIndex = -1;
        if (input_->navDirty) RebuildNavGraph();

        const int target = ResolveDefaultFocus();
        if (target >= 0) {
            input_->cursorIndex = target;
            ExecuteActionAt(static_cast<size_t>(target), EventKey::Hover);
        }
    }

    int Canvas::ResolveDefaultFocus() const {
        if (!input_) return -1;

        if (!input_->defaultFocusName.empty()) {
            for (int i = 0; i < static_cast<int>(elements_.size()); ++i) {
                if (elements_[i]->GetName() == input_->defaultFocusName &&
                    elements_[i]->IsSelectable())
                    return i;
            }
        }
        if (!input_->navSelectables.empty()) return input_->navSelectables[0];
        return -1;
    }

    // -----------------------------------------------------------------------
    //  Navigation graph
    // -----------------------------------------------------------------------
    void Canvas::RebuildNavGraph() {
        if (!input_) return;
        auto& ctx = *input_;

        ctx.navSelectables.clear();
        for (int i = 0; i < static_cast<int>(elements_.size()); ++i) {
            if (elements_[i]->IsSelectable() && elements_[i]->IsVisible())
                ctx.navSelectables.push_back(i);
        }

        ctx.navGraph.resize(ctx.navSelectables.size());
        for (int si = 0; si < static_cast<int>(ctx.navSelectables.size()); ++si) {
            int idx = ctx.navSelectables[si];
            auto& nb = ctx.navGraph[si];
            nb.up    = FindNavNeighbor(idx,  0.f, -1.f);
            nb.down  = FindNavNeighbor(idx,  0.f,  1.f);
            nb.left  = FindNavNeighbor(idx, -1.f,  0.f);
            nb.right = FindNavNeighbor(idx,  1.f,  0.f);
        }
        ctx.navDirty = false;
    }

    int Canvas::FindNavNeighbor(int fromElemIdx, float dx, float dy) const {
        if (!input_) return -1;
        const Vector2 from = GetElementCenter(fromElemIdx);

        int   best     = -1;
        float bestDist = FLT_MAX;

        for (int idx : input_->navSelectables) {
            if (idx == fromElemIdx) continue;

            const Vector2 to      = GetElementCenter(idx);
            const float   deltaX  = to.x - from.x;
            const float   deltaY  = to.y - from.y;
            const float   forward = deltaX * dx + deltaY * dy;
            if (forward <= 0.f) continue;

            const float lateral = std::abs(deltaX * dy - deltaY * dx);
            if (lateral > forward) continue; // 45° コーン外

            const float dist = std::sqrt(deltaX * deltaX + deltaY * deltaY);
            if (dist < bestDist) { bestDist = dist; best = idx; }
        }
        return best;
    }

    Vector2 Canvas::GetElementCenter(int elemIdx) const {
        const auto& elem = elements_[elemIdx];
        const Vector2 pos  = elem->GetPosition();
        const Vector2 size = elem->GetSize();
        return { pos.x + size.x * 0.5f, pos.y + size.y * 0.5f };
    }

    // -----------------------------------------------------------------------
    //  Module management
    // -----------------------------------------------------------------------
    void Canvas::AddModule(std::unique_ptr<IModule> _module) {
        if (!input_) input_ = std::make_unique<InputContext>();
        input_->modules.push_back(std::move(_module));
        input_->navDirty = true;
    }

    IModule* Canvas::FindModule(std::string_view _type) const {
        if (!input_) return nullptr;
        for (const auto& m : input_->modules) {
            if (m->GetType() == _type) return m.get();
        }
        return nullptr;
    }

    // -----------------------------------------------------------------------
    //  Focus
    // -----------------------------------------------------------------------
    void Canvas::SetFocused(bool _focused) {
        if (_focused) {
            Singleton<Manager>::GetInstance()->SetFocusedCanvas(this);
        } else {
            if (input_) input_->hasFocus = false;
        }
    }

    bool Canvas::HasFocus() const {
        return input_ && input_->hasFocus;
    }

    // -----------------------------------------------------------------------
    //  DefaultFocus
    // -----------------------------------------------------------------------
    void Canvas::SetDefaultFocusName(const std::string& _name) {
        if (!input_) input_ = std::make_unique<InputContext>();
        input_->defaultFocusName = _name;
    }

    const std::string& Canvas::GetDefaultFocusName() const {
        static const std::string empty;
        return input_ ? input_->defaultFocusName : empty;
    }

    // -----------------------------------------------------------------------
    //  JSON Load / Save (module section)
    // -----------------------------------------------------------------------
    void Canvas::LoadModulesFromJson(const nlohmann::json& root) {
        if (!root.contains("Modules") || !root["Modules"].is_object()) return;

        const auto& mods = root["Modules"];

        if (mods.contains("Mouse")) {
            AddModule(std::make_unique<MouseModule>());
        }
        if (mods.contains("Keyboard")) {
            auto km = std::make_unique<KeyboardModule>();
            km->LoadFromJson(mods["Keyboard"]);
            AddModule(std::move(km));
        }
        if (root.contains("DefaultFocus") && root["DefaultFocus"].is_string()) {
            input_->defaultFocusName = root["DefaultFocus"].get<std::string>();
        }
    }

    void Canvas::SaveModulesToJson(nlohmann::json& root) const {
        if (!input_ || input_->modules.empty()) return;

        nlohmann::json mods = nlohmann::json::object();
        for (const auto& module : input_->modules) {
            nlohmann::json mj = nlohmann::json::object();
            module->SaveToJson(mj);
            mods[std::string(module->GetType())] = mj;
        }
        root["Modules"] = mods;

        if (!input_->defaultFocusName.empty())
            root["DefaultFocus"] = input_->defaultFocusName;
    }

    // -----------------------------------------------------------------------
    //  Load / Save
    // -----------------------------------------------------------------------
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
        input_.reset();

        if (!root.contains("Elements") || !root["Elements"].is_array()) {
            LoadModulesFromJson(root);
            ResolveAnimFuncs();
            return;
        }

        for (const auto& j : root["Elements"]) {
            const std::string type = j.value("Type", "Sprite");
            auto element = CreateElement(type);
            element->Initialize();

            element->SetName(j.value("Name", "NoName"));
            element->SetVisible(j.value("Visible", true));
            element->SetSelectable(j.value("Selectable", false));
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

        LoadModulesFromJson(root);
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

            if (!element->IsVisible())              j["Visible"]    = false;
            if (element->IsSelectable())            j["Selectable"] = true;
            if (!element->GetShowAnimKey().empty()) j["ShowAnim"]   = element->GetShowAnimKey();
            if (!element->GetIdleAnimKey().empty()) j["IdleAnim"]   = element->GetIdleAnimKey();
            if (!element->GetHideAnimKey().empty()) j["HideAnim"]   = element->GetHideAnimKey();

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
                    j["TextureSize"]    = { sd.textureSize.x,    sd.textureSize.y };
                }
            }

            elementsArray.push_back(j);
        }

        root["Elements"] = elementsArray;
        SaveModulesToJson(root);

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

    // -----------------------------------------------------------------------
    //  Editor
    // -----------------------------------------------------------------------
    GESTD::ReferencePtr<Canvas> Canvas::Find(const std::string& _name) {
        return Singleton<Ui::Manager>::GetInstance()->GetCanvas(_name);
    }

    void Canvas::Editor(bool* _open) {
        const std::string title = name_.empty() ? "Editor" : name_ + " Editor";
        ImGui::Begin(title.c_str(), _open);

        HeaderUiParams();
        ModulesEditor();
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

    void Canvas::ModulesEditor() {
        ImGui::Separator();
        if (!ImGui::CollapsingHeader("Modules")) return;

        // --- Mouse ---
        auto* mm = static_cast<MouseModule*>(FindModule("Mouse"));
        if (!mm) {
            if (ImGui::Button("+ Mouse")) AddModule(std::make_unique<MouseModule>());
        } else {
            ImGui::TextColored(ImVec4(0.4f, 1.f, 0.4f, 1.f), "[Mouse]");
            ImGui::SameLine();
            if (ImGui::SmallButton("x##rmMouse")) {
                if (input_) std::erase_if(input_->modules,
                    [](const auto& m) { return m->GetType() == "Mouse"; });
                mm = nullptr;
            }
            if (mm) {
                mm->Debug();
            }
        }

        // --- Keyboard ---
        auto* km = static_cast<KeyboardModule*>(FindModule("Keyboard"));
        if (!km) {
            if (ImGui::Button("+ Keyboard")) AddModule(std::make_unique<KeyboardModule>());
        } else {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.f, 1.f), "[Keyboard]");
            ImGui::SameLine();
            if (ImGui::SmallButton("x##rmKb")) {
                if (input_) std::erase_if(input_->modules,
                    [](const auto& m) { return m->GetType() == "Keyboard"; });
                km = nullptr;
            }
            if (km) {
                KeyboardModule::KeyBindings bindings = km->GetKeyBindings();
                bool changed = false;

                auto keyCombo = [&](const char* label, std::vector<uint8_t>& keys) {
                    ImGui::Text("%s", label);
                    for (int i = 0; i < static_cast<int>(keys.size()); ++i) {
                        ImGui::PushID(i);
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(90.f);
                        if (ImGui::BeginCombo("##k", DikToName(keys[i]))) {
                            for (const auto& entry : KeyCodeMap) {
                                bool sel = (entry.code == keys[i]);
                                if (ImGui::Selectable(entry.name, sel)) {
                                    keys[i] = static_cast<uint8_t>(entry.code);
                                    changed = true;
                                }
                            }
                            ImGui::EndCombo();
                        }
                        ImGui::SameLine();
                        if (keys.size() > 1 && ImGui::SmallButton("-##rk")) {
                            keys.erase(keys.begin() + i);
                            changed = true;
                        }
                        ImGui::PopID();
                    }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("+##ak")) {
                        keys.push_back(keys.empty() ? 0x1Cu : keys.back());
                        changed = true;
                    }
                };

                keyCombo("Up   ", bindings.up);
                keyCombo("Down ", bindings.down);
                keyCombo("Left ", bindings.left);
                keyCombo("Right", bindings.right);
                keyCombo("Exec ", bindings.execute);

                if (changed) km->SetKeyBindings(bindings);
            }
        }

        // --- DefaultFocus ---
        ImGui::Separator();
        const std::string& dfName = GetDefaultFocusName();
        ImGui::Text("Default Focus: %s",
            dfName.empty() ? "(auto: first selectable)" : dfName.c_str());
        if (!dfName.empty()) {
            ImGui::SameLine();
            if (ImGui::SmallButton("Clear##df")) {
                if (input_) input_->defaultFocusName.clear();
            }
        }
    }

    void Canvas::ElementCommonParams() {
        ImGui::Separator();
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
            if (ImGui::SmallButton("^")) { std::iter_swap(itr, std::prev(itr)); dirty_ = true; if (input_) input_->navDirty = true; }
            ImGui::EndDisabled();
            ImGui::SameLine();

            ImGui::BeginDisabled(itr == std::prev(elements_.end()));
            if (ImGui::SmallButton("v")) { std::iter_swap(itr, std::next(itr)); dirty_ = true; if (input_) input_->navDirty = true; }
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

            // Selectable 時に "Set Default" ボタンを注入
            (*itr)->Debug(GetActionKeys(), GetAnimFuncKeys(), [&]() {
                if (!input_) input_ = std::make_unique<InputContext>();
                const bool isDefault = (input_->defaultFocusName == (*itr)->GetName());
                if (isDefault) {
                    ImGui::SameLine();
                    ImGui::TextDisabled("[Default]");
                } else {
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Set Default"))
                        input_->defaultFocusName = (*itr)->GetName();
                }
            });

            ImGui::PopID();
        }

        ImGui::Separator();
        ImGui::EndChild();

        for (const auto& uuid : toDelete) {
            std::erase_if(elements_, [&uuid](const std::unique_ptr<Element>& e) {
                return e->GetUUID() == uuid;
            });
            dirty_ = true;
            if (input_) input_->navDirty = true;
        }

        for (auto& dup : duplicated) {
            dup->IsOpen() = defaultOpen_;
            elements_.push_back(std::move(dup));
        }
    }

    // -----------------------------------------------------------------------
    //  Accessors
    // -----------------------------------------------------------------------
    void Canvas::RegisterAction(const std::string& _actionKey, const std::function<void()>& _action) {
        eventSystem_.Register(_actionKey, _action);
    }

    void Canvas::RegisterAnimFunc(const std::string& _key, AnimFunc _func) {
        animFuncs_[_key] = std::move(_func);
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

} // namespace Ui
