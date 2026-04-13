#define NOMINMAX
#include "Ui/Element.hpp"
#include "SpriteElement.hpp"
#include "TextElement.hpp"

#include "imgui.h"
#include "Utils.hpp"

namespace Ui {

    void Element::Initialize() {
        uuid_     = Utils::GenerateUniqueId();
        name_     = "NoName";
        visible_  = true;
        position_ = {};
        color_    = {1.f, 1.f, 1.f, 1.f};

        auto len = std::min(name_.size(), nameBuff_.size() - 1);
        std::copy_n(name_.begin(), len, nameBuff_.begin());
        nameBuff_[len] = '\0';

        OnInitialize();
    }

    void Element::Update(float dt) {
        if (!visible_) return;

        posOffset_ = {};
        animColor_ = color_;

        if (showSlot_.playing) {
            showSlot_.Update(dt, posOffset_, animColor_);
        } else if (idleSlot_.playing) {
            idleSlot_.Update(dt, posOffset_, animColor_);
        } else if (hideSlot_.playing) {
            hideSlot_.Update(dt, posOffset_, animColor_);
        }

        OnUpdate(dt);
    }

    void Element::Draw() const {
        if (!visible_) return;
        OnDraw();
    }

    void Element::Debug(const std::vector<std::string>& _availableActions, const std::vector<std::string>& _availableAnimKeys) {
        if (!isOpen_) return;

        ImGui::BeginChild("Body", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoMove);

        if (changeName_) {
            ImGui::SetNextItemWidth(80.f);
            ImGui::InputText("##name", nameBuff_.data(), nameBuff_.size());
            ImGui::SameLine();
            if (nameBuff_[0] == '\0') ImGui::BeginDisabled();
            if (ImGui::Button("Apply")) {
                name_ = nameBuff_.data();
                changeName_ = false;
            }
            if (nameBuff_[0] == '\0') ImGui::EndDisabled();
        } else {
            ImGui::TextColored(ImVec4(0.f, 1.f, 0.6f, 1.f), "%s", name_.c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("[%s]", std::string(GetType()).c_str());
            ImGui::SameLine();
            if (ImGui::Button("Change")) {
                auto len = std::min(name_.size(), nameBuff_.size() - 1);
                std::copy_n(name_.begin(), len, nameBuff_.begin());
                nameBuff_[len] = '\0';
                changeName_ = true;
            }
        }

        ImGui::Separator();

        ImGui::Text("Visible : "); ImGui::SameLine();
        ImGui::Checkbox("##visible", &visible_);

        ImGui::Separator();

        ImGui::Text("Events");
        for (auto eventKey : ALL_EVENT_KEYS) {
            const char* label = EventKeyToString(eventKey);
            auto it = events_.find(eventKey);
            const std::string current = (it != events_.end()) ? it->second : "";

            ImGui::Text("%s", label); ImGui::SameLine();
            ImGui::SetNextItemWidth(150.f);
            const std::string comboId = std::string("##ev_") + label;
            if (ImGui::BeginCombo(comboId.c_str(), current.empty() ? "(None)" : current.c_str())) {
                if (ImGui::Selectable("(None)", current.empty())) events_.erase(eventKey);
                for (const auto& key : _availableActions) {
                    bool selected = (current == key);
                    if (ImGui::Selectable(key.c_str(), selected)) events_[eventKey] = key;
                }
                ImGui::EndCombo();
            }
        }

        ImGui::Separator();

        ImGui::Text("Position"); ImGui::SetNextItemWidth(130.f);
        ImGui::DragFloat2("##pos", &position_.x);

        ImGui::Text("Color"); ImGui::SetNextItemWidth(200.f);
        ImGui::ColorEdit4("##color", &color_.x);

        ImGui::Separator();

        DebugParams();

        ImGui::Separator();

        ImGui::Text("Animation Keys");
        {
            auto animKeyCombo = [&](const char* _label, const char* _id, std::string& _key) {
                ImGui::Text("%s", _label); ImGui::SameLine(); ImGui::SetNextItemWidth(120.f);
                if (ImGui::BeginCombo(_id, _key.empty() ? "(None)" : _key.c_str())) {
                    if (ImGui::Selectable("(None)", _key.empty())) _key.clear();
                    for (const auto& k : _availableAnimKeys) {
                        const bool selected = (_key == k);
                        if (ImGui::Selectable(k.c_str(), selected)) _key = k;
                    }
                    ImGui::EndCombo();
                }
            };

            animKeyCombo("Show", "##showAnim", showAnimKey_);
            animKeyCombo("Idle", "##idleAnim", idleAnimKey_);
            animKeyCombo("Hide", "##hideAnim", hideAnimKey_);
        }

        ImGui::EndChild();
    }

    bool                    Element::IsVisible()   const { return visible_; }
    std::string Element::GetName() const { return name_; }
    std::string Element::GetUUID() const { return uuid_; }
    bool&                   Element::IsOpen()            { return isOpen_; }
    const std::string&      Element::GetShowAnimKey() const { return showAnimKey_; }
    const std::string&      Element::GetIdleAnimKey() const { return idleAnimKey_; }
    const std::string&      Element::GetHideAnimKey() const { return hideAnimKey_; }
    Vector2     Element::GetPosition() const { return position_; }
    const Vector4& Element::GetColor() const { return color_; }

    void Element::SetPosition(const Vector2& _pos) { position_ = _pos; }
    void Element::SetColor(const Vector4& _color)  { color_ = _color; }
    void Element::SetVisible(bool _visible)        { visible_ = _visible; }
    void Element::SetParent(const Vector2& _pos)   { parent_ = _pos; }

    void Element::SetName(const std::string& _name) {
        name_ = _name;
        auto len = std::min(name_.size(), nameBuff_.size() - 1);
        std::copy_n(name_.begin(), len, nameBuff_.begin());
        nameBuff_[len] = '\0';
    }

    void Element::SetShowFunc(const AnimFunc& _func) { showSlot_.func = _func; }
    void Element::SetIdleFunc(const AnimFunc& _func) { idleSlot_.func = _func; }
    void Element::SetHideFunc(const AnimFunc& _func) { hideSlot_.func = _func; }

    void Element::PlayShow() { StopAnim(); showSlot_.Play(); }
    void Element::PlayIdle() { StopAnim(); idleSlot_.Play(); }
    void Element::PlayHide() { StopAnim(); hideSlot_.Play(); }

    void Element::StopAnim() {
        showSlot_.Stop();
        idleSlot_.Stop();
        hideSlot_.Stop();
    }

    bool Element::IsShowDone() const { return showSlot_.IsDone(); }
    bool Element::IsHideDone() const { return hideSlot_.IsDone(); }

    void Element::SetEvent(EventKey _event, const std::string& _actionKey) {
        if (_actionKey.empty()) events_.erase(_event);
        else events_[_event] = _actionKey;
    }

    static const std::string EMPTY_STRING;

    const std::string& Element::GetActionKey(EventKey _event) const {
        auto it = events_.find(_event);
        return (it != events_.end()) ? it->second : EMPTY_STRING;
    }

    bool Element::HasEvent(EventKey _event)   const { return events_.contains(_event); }
    bool Element::HasAnyEvent()               const { return !events_.empty(); }
    const std::unordered_map<EventKey, std::string>& Element::GetEvents() const { return events_; }

    void Element::CopyCommonTo(Element& _dst) const {
        _dst.SetName(name_);
        _dst.SetPosition(position_);
        _dst.SetColor(color_);
        _dst.SetVisible(visible_);
        _dst.SetShowAnimKey(showAnimKey_);
        _dst.SetIdleAnimKey(idleAnimKey_);
        _dst.SetHideAnimKey(hideAnimKey_);
        for (const auto& [k, v] : events_) _dst.SetEvent(k, v);
    }

} // namespace Ui
