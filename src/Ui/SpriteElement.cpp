#define NOMINMAX
#include "SpriteElement.hpp"

#include <filesystem>

#include "imgui.h"
#include "Utils.hpp"

namespace Ui {

    void SpriteElement::SetTexture(const std::string& _tex) {
        data_.texture = _tex;
    }

    void SpriteElement::SetTextureRegion(const Vector2& _leftTop, const Vector2& _size) {
        data_.textureLeftTop = _leftTop;
        data_.textureSize    = _size;
    }

    void SpriteElement::SetSize(const Vector2& _size) {
        data_.size = _size;
    }

    SpriteElement::Data SpriteElement::GetSpriteData() const { return data_; }

    void SpriteElement::SetSpriteData(const Data& _data) {
        data_ = _data;
        if (sprite_) {
            sprite_->SetSize(data_.size);
            if (data_.textureSize.x > 0.f && data_.textureSize.y > 0.f) {
                sprite_->SetTextureLeftTop(data_.textureLeftTop);
                sprite_->SetTextureSize(data_.textureSize);
            }
        }
    }

    void SpriteElement::OnInitialize() {
        position_ = {640.f, 360.f};
        color_    = {1.f, 0.5859f, 0.7421f, 1.f};

        sprite_ = std::make_unique<Sprite>();
        sprite_->Initialize(data_.texture);
        sprite_->SetPosition(position_);
        sprite_->SetSize(data_.size);
        sprite_->SetColor(color_);
    }

    void SpriteElement::OnUpdate(float /*dt*/) {
        if (textureBuff_[0] != '\0') {
            data_.texture = textureBuff_.data();
            textureBuff_.fill('\0');
        }

        sprite_->SetTexture(data_.texture);
        sprite_->SetPosition(position_ + parent_ + posOffset_);
        sprite_->SetSize(data_.size);
        sprite_->SetColor(animColor_);

        if (data_.textureSize.x > 0.f && data_.textureSize.y > 0.f) {
            sprite_->SetTextureLeftTop(data_.textureLeftTop);
            sprite_->SetTextureSize(data_.textureSize);
        }

        sprite_->Update();
    }

    void SpriteElement::OnDraw() const {
        sprite_->Draw();
    }

    void SpriteElement::DebugParams() {
        ImGui::Text("Texture :");
        ImGui::SameLine();
        if (!data_.texture.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.f), "%s", data_.texture.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("...##BrowseTexture")) {
            const std::string selected = Utils::OpenFileDialog("PNG");
            if (!selected.empty()) {
                const std::string filename = std::filesystem::path(selected).filename().string();
                strcpy_s(textureBuff_.data(), textureBuff_.size(), filename.c_str());
            }
        }

        ImGui::Text("Size : AspectLock"); ImGui::SameLine();
        ImGui::Checkbox("##asplock", &aspectLock_);
        ImGui::SetNextItemWidth(130.f);
        if (aspectLock_) {
            if (ImGui::DragFloat("##size", &data_.size.x)) data_.size.y = data_.size.x;
        } else {
            ImGui::DragFloat2("##size", &data_.size.x);
        }
    }

    std::unique_ptr<Element> SpriteElement::Clone() const {
        auto clone = std::make_unique<SpriteElement>();
        clone->Initialize();
        CopyCommonTo(*clone);
        clone->SetSpriteData(data_);
        return clone;
    }

} // namespace Ui
