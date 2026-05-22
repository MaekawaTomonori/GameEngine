#include "Ui/Module/MouseModule.hpp"
#include "Ui/UserInterface.hpp"

#include <filesystem>

#include "imgui.h"
#include "Input.hpp"
#include "Sprite.hpp"
#include "Utils.hpp"

namespace Ui {
    void MouseModule::ApplyPendingTexture() {
        if (!sprite_) {
            sprite_ = std::make_unique<Sprite>();
            sprite_->Initialize(textureName_);
        }
        
        if (pendingTexture_.empty()) return;
        textureName_ = pendingTexture_;
        pendingTexture_ = {};
        sprite_->SetTexture(textureName_);
    }

    void MouseModule::SetTexture(const std::string& _name) {
        pendingTexture_ = _name;
    }

    void MouseModule::Update(Canvas& canvas, const Input& input) {
        if (canvas.GetPhase() == Canvas::Phase::Inactive) return;

        ApplyPendingTexture();

        mousePos_ = input.GetMousePosition();

        const Vector2 canvasPos = canvas.GetPosition();
        int hovered = -1;

        for (size_t i = 0; i < canvas.GetElementCount(); ++i) {
            const auto* elem = canvas.GetElement(i);
            if (!elem || !elem->IsVisible() || !elem->IsSelectable()) continue;

            const auto    rect      = canvas.GetElementRect(i);
            const Vector2 screenPos = canvasPos + rect.position;

            const float hw = rect.size.x / 2.f + hitPadding_;
            const float hh = rect.size.y / 2.f + hitPadding_;
            if (mousePos_.x >= screenPos.x - hw && mousePos_.x < screenPos.x + hw &&
                mousePos_.y >= screenPos.y - hh && mousePos_.y < screenPos.y + hh) {
                hovered = static_cast<int>(i);
                break;
            }
        }

        if (hovered >= 0) {
            canvas.MoveCursorTo(hovered);
            if (input.IsMouseTrigger(0)) canvas.ExecuteCursor();
        }

        sprite_->SetPosition(mousePos_);
        sprite_->SetSize(mouseSize_);
        sprite_->Update();
    }

    void MouseModule::Draw() const {
        if (sprite_) sprite_->Draw();
    }

    void MouseModule::Debug() {
        ImGui::Text("Cursor:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 0.7f, 1.f), "%s", textureName_.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("...##mouseTex")) {
            const std::string selected = Utils::OpenFileDialog("PNG");
            if (!selected.empty())
                pendingTexture_ = std::filesystem::path(selected).filename().string();
        }
        ImGui::Text("Size");
        if (ImGui::DragFloat("##CursorSize", &mouseSize_.x)) {
            mouseSize_.y = mouseSize_.x;
        }
        ImGui::Text("HitPadding");
        ImGui::DragFloat("##HitPadding", &hitPadding_, 1.f, 0.f, 64.f);
    }

    void MouseModule::LoadFromJson(const nlohmann::json& j) {
        if (j.contains("Texture") && j["Texture"].is_string()) {
            textureName_ = j["Texture"].get<std::string>();
        }
        if (j.contains("Size") && j["Size"].is_number()) {
            mouseSize_.x = mouseSize_.y = j["Size"].get<float>();
        }
        if (j.contains("HitPadding") && j["HitPadding"].is_number()) {
            hitPadding_ = j["HitPadding"].get<float>();
        }
    }

    void MouseModule::SaveToJson(nlohmann::json& j) const {
        j["Texture"]    = textureName_;
        j["Size"]       = mouseSize_.x;
        j["HitPadding"] = hitPadding_;
    }

} // namespace Ui
