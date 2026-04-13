#include "TextElement.hpp"

#include "imgui.h"
#include "Pattern/Singleton.hpp"
#include "src/Text/TextCommon.hpp"

namespace Ui {

    TextElement::Data TextElement::GetTextData() const { return data_; }

    void TextElement::SetTextData(const Data& _data) {
        data_ = _data;
    }

    void TextElement::OnInitialize() {
        position_ = {20.f, 20.f};
        color_    = {1.f, 1.f, 1.f, 1.f};
        data_.text     = "Text";
        data_.fontSize = 32.f;
    }

    void TextElement::OnUpdate(float /*dt*/) {
        // テキストは Draw 時に TextCommon へ直接送るため Update では何もしない
    }

    void TextElement::OnDraw() const {
        if (data_.text.empty()) return;

        auto common = Singleton<TextCommon>::GetInstance();
        if (!common) return;

        const Vector2 pos = position_ + parent_ + posOffset_;
        common->SubmitEntry(
            data_.text,
            pos.x, pos.y,
            data_.fontSize,
            animColor_.x, animColor_.y, animColor_.z, animColor_.w);
    }

    void TextElement::DebugParams() {
        static char textBuf[256]{};
        strncpy_s(textBuf, data_.text.c_str(), _TRUNCATE);

        ImGui::Text("Text");
        ImGui::SetNextItemWidth(180.f);
        if (ImGui::InputText("##text", textBuf, sizeof(textBuf))) {
            data_.text = textBuf;
        }

        ImGui::Text("FontSize");
        ImGui::SetNextItemWidth(100.f);
        ImGui::DragFloat("##fontSize", &data_.fontSize, 0.5f, 4.f, 256.f);
    }

    std::unique_ptr<Element> TextElement::Clone() const {
        auto clone = std::make_unique<TextElement>();
        clone->Initialize();
        CopyCommonTo(*clone);
        clone->SetTextData(data_);
        return clone;
    }

} // namespace Ui
