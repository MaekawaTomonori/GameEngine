#include "Model.hpp"

#include "Pattern/Singleton.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/Model/Loader/ModelLoaderFactory.hpp"
#include "src/Model/ModelInstance.hpp"

Model::Model() = default;

Model::~Model() {
    if (instance_) {
        Singleton<ModelCommon>::GetInstance()->DestroyModelInstance(instance_);
    }
}

Model::Model(Model&& _other) noexcept : instance_(_other.instance_) {
    _other.instance_.Reset();
}

Model& Model::operator=(Model&& _other) noexcept {
    if (this != &_other) {
        if (instance_) {
            Singleton<ModelCommon>::GetInstance()->DestroyModelInstance(instance_);
        }
        instance_ = _other.instance_;
        _other.instance_.Reset();
    }
    return *this;
}

void Model::Initialize(const std::string& _name) {
    if (instance_) {
        Singleton<ModelCommon>::GetInstance()->DestroyModelInstance(instance_);
    }
    instance_ = Singleton<ModelCommon>::GetInstance()->CreateModelInstance(_name);
}

void Model::Update() {
    if (instance_) instance_->Update();
}

void Model::UpdateMapData() const {
    if (instance_) instance_->UpdateMapData();
}

void Model::Draw() const {
    if (instance_) instance_->Draw();
}

Model& Model::SetName(const std::string& _name) {
    if (instance_) instance_->SetName(_name);
    return *this;
}

Model& Model::SetTranslate(const Vector3& _translate) {
    if (instance_) instance_->SetTranslate(_translate);
    return *this;
}

Model& Model::SetRotate(const Vector3& _rotate) {
    if (instance_) instance_->SetRotate(_rotate);
    return *this;
}

Model& Model::SetScale(const Vector3& _scale) {
    if (instance_) instance_->SetScale(_scale);
    return *this;
}

Model& Model::SetEnvironmentTexture(const std::string& _texture) {
    if (instance_) instance_->SetEnvironmentTexture(_texture);
    return *this;
}

Model& Model::SetTexture(const std::string& _texture) {
    if (instance_) instance_->SetTexture(_texture);
    return *this;
}

Model& Model::SetTilingMultiply(const Vector2 _mul) {
    if (instance_) instance_->SetTilingMultiply(_mul);
    return *this;
}

Model& Model::SetColor(const Vector4& _color) {
    if (instance_) instance_->SetColor(_color);
    return *this;
}

Model& Model::SetCanvasName(const std::string& _canvasName) {
    if (instance_) instance_->SetCanvasName(_canvasName);
    return *this;
}

const std::string& Model::GetCanvasName() const {
    static const std::string EMPTY;
    return instance_ ? instance_->GetCanvasName() : EMPTY;
}

const std::string& Model::GetName() const {
    static const std::string EMPTY;
    return instance_ ? instance_->GetName() : EMPTY;
}

void Model::Load(const std::string& _name) {
    ModelLoaderFactory::Load(_name, Singleton<ModelCommon>::GetInstance()->GetResourceRepository());
}
