#include "Line.hpp"

#include "Pattern/Singleton.hpp"
#include "src/Line/Common/LineCommon.hpp"
#include "src/Line/LineInstance.hpp"

Line::Line() = default;

Line::~Line() {
    if (instance_) {
        Singleton<LineCommon>::GetInstance()->DestroyInstance(instance_);
    }
}

Line::Line(Line&& _other) noexcept : instance_(_other.instance_) {
    _other.instance_.Reset();
}

Line& Line::operator=(Line&& _other) noexcept {
    if (this != &_other) {
        if (instance_) {
            Singleton<LineCommon>::GetInstance()->DestroyInstance(instance_);
        }
        instance_ = _other.instance_;
        _other.instance_.Reset();
    }
    return *this;
}

void Line::Initialize() {
    if (instance_) {
        Singleton<LineCommon>::GetInstance()->DestroyInstance(instance_);
    }
    instance_ = Singleton<LineCommon>::GetInstance()->CreateInstance();
}

void Line::Update() {
    if (instance_) instance_->Update();
}

void Line::Draw() const {
    if (instance_) instance_->Draw();
}

void Line::AddLine(const Vector3& _start, const Vector3& _end) {
    if (instance_) instance_->AddLine(_start, _end);
}

void Line::Clear() {
    if (instance_) instance_->Clear();
}

void Line::SetColor(const Vector4& _color) const {
    if (instance_) instance_->SetColor(_color);
}

void Line::SetName(const std::string& _name) {
    if (instance_) instance_->SetName(_name);
}
