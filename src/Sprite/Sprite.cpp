#include "Sprite.hpp"

#include "Pattern/Singleton.hpp"
#include "Utils.hpp"
#include "imgui.h"
#include "Math/MathUtils.hpp"
#include "Common/SpriteCommon.hpp"

#include "src/Texture/TextureManager.hpp"

#include "vendor/DirectXTex/DirectXTex.h"

Sprite::Sprite() {
    common_ = Singleton<SpriteCommon>::GetInstance();
    adapter_ = common_->GetAdapter();
    commandList_ = adapter_->GetCommandList();

    uuid_ = Utils::GenerateUniqueId();
}

void Sprite::Initialize(const std::string&_texture) {
    texturePath_ = _texture;
    Singleton<TextureManager>::GetInstance()->Load(texturePath_);

    vr_ = adapter_->CreateBufferResource(sizeof(VertexData) * 4);
    vbv_.BufferLocation = vr_->Get()->GetGPUVirtualAddress();
    vbv_.SizeInBytes = sizeof(VertexData) * 4;
    vbv_.StrideInBytes = sizeof(VertexData);

    vr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&vd_));

    vd_[0].position = {0, 1.f, 0, 1};
    vd_[1].position = {0, 0, 0, 1};
    vd_[2].position = {1.f, 1.f, 0, 1};
    vd_[3].position = {1.f, 0, 0, 1};

    vd_[0].uv = {0, 1};
    vd_[1].uv = {0, 0};
    vd_[2].uv = {1, 1};
    vd_[3].uv = {1, 0};

    //IndexData
    ir_ = adapter_->CreateBufferResource(sizeof(uint32_t) * 6);

    ibv_.BufferLocation = ir_->Get()->GetGPUVirtualAddress();
    ibv_.SizeInBytes = sizeof(uint32_t) * 6;
    ibv_.Format = DXGI_FORMAT_R32_UINT;

    ir_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&index_));

    index_[0] = 0;
    index_[1] = 1;
    index_[2] = 2;
    index_[3] = 1;
    index_[4] = 3;
    index_[5] = 2;

    //MaterialData
    mr_ = adapter_->CreateBufferResource(sizeof(Material));
    mr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&material_));

    material_->color = {1, 1, 1, 1};

    wr_ = adapter_->CreateBufferResource(sizeof(Transformation));
    wr_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&wd_));
    wd_->wvp = MathUtils::Matrix::MakeIdentity();
    wd_->world = MathUtils::Matrix::MakeIdentity();

    size_ = {100, 100};

    AdjustTextureSize();
}

void Sprite::Update() {
    Debug();
#pragma region Vertex position
    float left = 0.f - anchorPoint_.x;
    float right = 1.f - anchorPoint_.x;
    float top = 0 - anchorPoint_.y;
    float bottom = 1.f - anchorPoint_.y;

    if (flipX_){
        left = -left;
        right = -right;
    }

    if (flipY_){
        top = -top;
        bottom = -bottom;
    }

    vd_[0].position = {left, bottom, 0, 1};
    vd_[1].position = {left, top, 0, 1};
    vd_[2].position = {right, bottom, 0, 1};
    vd_[3].position = {right, top, 0, 1};

#pragma endregion

#pragma region Vertex texcoord
    const DirectX::TexMetadata& metadata = Singleton<TextureManager>::GetInstance()->GetTextureMetadata(texturePath_);
    float texLeft = leftTop_.x / static_cast<float>(metadata.width);
    float texRight = (leftTop_.x + texSize_.x) / static_cast<float>(metadata.width);
    float texTop = leftTop_.y / static_cast<float>(metadata.height);
    float texBottom = (leftTop_.y + texSize_.y) / static_cast<float>(metadata.height);

    vd_[0].uv= {texLeft, texBottom};
    vd_[1].uv= {texLeft, texTop};
    vd_[2].uv= {texRight, texBottom};
    vd_[3].uv= {texRight, texTop};
#pragma endregion

    wd_->world = MathUtils::Matrix::MakeAffineMatrix({size_.x, size_.y, 1}, Vector3{0, 0, rotation_}, {position_.x, position_.y, 0});
    Matrix4x4 viewProjection = MathUtils::Matrix::MakeIdentity() * MathUtils::Matrix::MakeOrthogonalMatrix(0, static_cast<float>(adapter_->GetWidth()), 0, static_cast<float>(adapter_->GetHeight()), 0, 100.f);

    wd_->wvp = (wd_->world * viewProjection);
}

void Sprite::Draw() {
    common_->RegisterDraw([&]{
        commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList_->IASetVertexBuffers(0, 1, &vbv_);
        commandList_->IASetIndexBuffer(&ibv_);
        commandList_->SetGraphicsRootConstantBufferView(0, mr_->Get()->GetGPUVirtualAddress());
        commandList_->SetGraphicsRootConstantBufferView(1, wr_->Get()->GetGPUVirtualAddress());
        commandList_->SetGraphicsRootDescriptorTable(2, Singleton<TextureManager>::GetInstance()->GetGPUHandle(texturePath_));

        commandList_->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }, posteffect_);
}

void Sprite::AdjustTextureSize() {
    const DirectX::TexMetadata& metadata = Singleton<TextureManager>::GetInstance()->GetTextureMetadata(texturePath_);
    texSize_ = {static_cast<float>(metadata.width), static_cast<float>(metadata.height)};
    size_ = texSize_;
}

void Sprite::Debug() {
	common_->RegisterDebug(
		uuid_,
        [this](){
	        ImGui::Begin("Sprite");
	        if (ImGui::CollapsingHeader(uuid_.c_str())){
                ImGui::PushID(uuid_.c_str());
		        ImGui::Text("Texture Path: %s", texturePath_.c_str());
		        ImGui::DragFloat2("Position", &position_.x, 1.f);
		        ImGui::DragFloat2("Size", &size_.x, 1);
		        ImGui::DragFloat("Rotation", &rotation_, 0.01f);
		        ImGui::DragFloat2("Anchor Point", &anchorPoint_.x);
		        ImGui::Checkbox("Flip X", &flipX_);
		        ImGui::Checkbox("Flip Y", &flipY_);
                ImGui::ColorEdit4("Color", &material_->color.x);
                ImGui::PopID();
            }
	        ImGui::End();
		}
    );
}

const Vector2& Sprite::GetPosition() const {
    return position_;
}

void Sprite::SetPosition(const Vector2& p) {
    position_ = p;
}

const Vector2& Sprite::GetSize() const {
    return size_;
}

void Sprite::SetSize(const Vector2& s) {
    size_ = s;
}

float Sprite::GetRotation() const {
    return rotation_;
}

void Sprite::SetRotation(float r) {
    rotation_ = r;
}

const Vector4& Sprite::GetColor() const {
    return material_->color;
}

void Sprite::SetColor(const Vector4& color) const {
    material_->color = color;
}

const Vector2& Sprite::GetAnchorPoint() const {
    return anchorPoint_;
}

void Sprite::SetAnchorPoint(const Vector2& a) {
    anchorPoint_ = a;
}

bool Sprite::IsFlipX() const {
    return flipX_;
}

void Sprite::SetFlipX(bool f) {
    flipX_ = f;
}

bool Sprite::IsFlipY() const {
    return flipY_;
}

void Sprite::SetFlipY(bool f) {
    flipY_ = f;
}

const Vector2& Sprite::GetTextureLeftTop() const {
    return leftTop_;
}

void Sprite::SetTextureLeftTop(const Vector2& _textureLeftTop) {
    leftTop_ = _textureLeftTop;
}

const Vector2& Sprite::GetTextureSize() const {
    return texSize_;
}

void Sprite::SetTextureSize(const Vector2& _textureSize) {
    texSize_ = _textureSize;
}

void Sprite::SetActivePostEffect(const bool _active) {
    posteffect_ = _active;
}
