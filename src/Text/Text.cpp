#include "Text/Text.hpp"

#include "Pattern/Singleton.hpp"
#include "src/Text/TextCommon.hpp"

Text::Text() {
    common_ = Singleton<TextCommon>::GetInstance();
    if (common_) common_->RegisterText(this);
}

Text::~Text() {
    if (common_) common_->UnregisterText(this);
}

void Text::Initialize(const std::string& _text, float _x, float _y, float _fontSize) {
    text_     = _text;
    position_ = {_x, _y};
    fontSize_ = _fontSize;
}

void Text::Draw() {
    if (!visible_ || text_.empty() || !common_) return;
    common_->SubmitEntry(
        text_,
        position_.x, position_.y,
        fontSize_,
        color_.x, color_.y, color_.z, color_.w);
}

void Text::SetText(const std::string& _text)   { text_ = _text; }
void Text::SetPosition(float _x, float _y)     { position_ = {_x, _y}; }
void Text::SetFontSize(float _fontSize)        { fontSize_ = _fontSize; }
void Text::SetColor(const Vector4& _color)     { color_ = _color; }
void Text::SetVisible(bool _visible)           { visible_ = _visible; }
