#include "Sprite.hpp"

#include "Pattern/Singleton.hpp"
#include "src/Sprite/Common/SpriteCommon.hpp"
#include "src/Sprite/SpriteInstance.hpp"

Sprite::Sprite() = default;

Sprite::~Sprite() {
    if (instance_) {
        Singleton<SpriteCommon>::GetInstance()->DestroyInstance(instance_);
    }
}

Sprite::Sprite(Sprite&& _other) noexcept : instance_(_other.instance_) {
    _other.instance_.Reset();
}

Sprite& Sprite::operator=(Sprite&& _other) noexcept {
    if (this != &_other) {
        if (instance_) {
            Singleton<SpriteCommon>::GetInstance()->DestroyInstance(instance_);
        }
        instance_ = _other.instance_;
        _other.instance_.Reset();
    }
    return *this;
}

void Sprite::Initialize(const std::string& _texture) {
    if (instance_) {
        Singleton<SpriteCommon>::GetInstance()->DestroyInstance(instance_);
    }
    instance_ = Singleton<SpriteCommon>::GetInstance()->CreateInstance(_texture);
}

void Sprite::Update() {
    if (instance_) instance_->Update();
}

void Sprite::Draw() {
    if (instance_) instance_->Draw();
}

const Vector2& Sprite::GetPosition() const {
    static const Vector2 EMPTY{};
    return instance_ ? instance_->GetPosition() : EMPTY;
}

void Sprite::SetPosition(const Vector2& _p) {
    if (instance_) instance_->SetPosition(_p);
}

const Vector2& Sprite::GetSize() const {
    static const Vector2 EMPTY{};
    return instance_ ? instance_->GetSize() : EMPTY;
}

void Sprite::SetSize(const Vector2& _s) {
    if (instance_) instance_->SetSize(_s);
}

float Sprite::GetRotation() const {
    return instance_ ? instance_->GetRotation() : 0.f;
}

void Sprite::SetRotation(float _r) {
    if (instance_) instance_->SetRotation(_r);
}

const Vector4& Sprite::GetColor() const {
    static const Vector4 EMPTY{};
    return instance_ ? instance_->GetColor() : EMPTY;
}

void Sprite::SetColor(const Vector4& _color) const {
    if (instance_) instance_->SetColor(_color);
}

const Vector2& Sprite::GetAnchorPoint() const {
    static const Vector2 EMPTY{};
    return instance_ ? instance_->GetAnchorPoint() : EMPTY;
}

void Sprite::SetAnchorPoint(const Vector2& _a) {
    if (instance_) instance_->SetAnchorPoint(_a);
}

bool Sprite::IsFlipX() const {
    return instance_ && instance_->IsFlipX();
}

void Sprite::SetFlipX(bool _f) {
    if (instance_) instance_->SetFlipX(_f);
}

bool Sprite::IsFlipY() const {
    return instance_ && instance_->IsFlipY();
}

void Sprite::SetFlipY(bool _f) {
    if (instance_) instance_->SetFlipY(_f);
}

const Vector2& Sprite::GetTextureLeftTop() const {
    static const Vector2 EMPTY{};
    return instance_ ? instance_->GetTextureLeftTop() : EMPTY;
}

void Sprite::SetTextureLeftTop(const Vector2& _textureLeftTop) {
    if (instance_) instance_->SetTextureLeftTop(_textureLeftTop);
}

const Vector2& Sprite::GetTextureSize() const {
    static const Vector2 EMPTY{};
    return instance_ ? instance_->GetTextureSize() : EMPTY;
}

void Sprite::SetTextureSize(const Vector2& _textureSize) {
    if (instance_) instance_->SetTextureSize(_textureSize);
}

void Sprite::SetActivePostEffect(bool _active) {
    if (instance_) instance_->SetActivePostEffect(_active);
}

void Sprite::SetTexture(const std::string& _texture) {
    if (instance_) instance_->SetTexture(_texture);
}
