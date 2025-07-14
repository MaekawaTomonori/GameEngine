#include "Line.hpp"
#include "Pattern/Singleton.hpp"
#include "Utils.hpp"
#include "src/Line/Common/LineCommon.hpp"
#include "Math/MathUtils.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/Camera/Manager/CameraManager.hpp"

Line::Line() {
    common_ = Singleton<LineCommon>::GetInstance();
    adapter_ = common_->GetAdapter();
    commandList_ = adapter_->GetCommandList();
    cameraManager_ = Singleton<CameraManager>::GetInstance();
    uuid_ = Utils::GenerateUniqueId();
}

Line::~Line() = default;

void Line::Initialize() {
    CreateVertexBuffer();
    CreateInstanceBuffer();
    CreateTransformBuffer();
}

void Line::Update() {
    UpdateInstanceBuffer();
}

void Line::Draw() const {
    if (currentInstanceCount_ == 0) return;

    common_->Draw();
    
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
    
    commandList_->SetGraphicsRootConstantBufferView(0, transformResource_->GetGPUVirtualAddress());
    common_->GetSRVManager()->SetGraphicsRootDescriptorTable(1, instanceSrvIndex_);
    
    commandList_->DrawInstanced(4, currentInstanceCount_, 0, 0);
}

void Line::AddLine(const Vector3& start, const Vector3& end, const Vector4& color, float thickness) {
    if (lines_.size() >= maxInstances_) return;

    LineInstance instance;
    instance.startPos = {start.x, start.y, start.z, thickness};
    instance.endPos = {end.x, end.y, end.z, 0.0f};
    instance.color = color;
    
    lines_.push_back(instance);
}

void Line::Clear() {
    lines_.clear();
    currentInstanceCount_ = 0;
}

void Line::SetViewProjectionMatrix(const Matrix4x4& viewProjection) {
    if (transformData_) {
        transformData_->viewProjection = viewProjection;
    }
}

void Line::CreateVertexBuffer() {
    vertexResource_.Attach(adapter_->CreateBufferResource(sizeof(VertexData) * 4));
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    vertexData_[0].position = {0.0f, -0.5f, 0.0f, 1.0f};
    vertexData_[1].position = {0.0f, 0.5f, 0.0f, 1.0f};
    vertexData_[2].position = {1.0f, -0.5f, 0.0f, 1.0f};
    vertexData_[3].position = {1.0f, 0.5f, 0.0f, 1.0f};
}

void Line::CreateInstanceBuffer() {
    instanceResource_.Attach(adapter_->CreateBufferResource(sizeof(LineInstance) * maxInstances_));
    instanceResource_->Map(0, nullptr, reinterpret_cast<void**>(&instanceData_));
    
    instanceSrvIndex_ = common_->GetSRVManager()->Allocate();
    common_->GetSRVManager()->CreateSRVforStructuredBuffer(instanceSrvIndex_, instanceResource_.Get(), maxInstances_, sizeof(LineInstance));
}

void Line::CreateTransformBuffer() {
    transformResource_.Attach(adapter_->CreateBufferResource(sizeof(TransformMatrix)));
    transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));
    
    transformData_->viewProjection = MathUtils::Matrix::MakeIdentity();
}

void Line::UpdateInstanceBuffer() {
    currentInstanceCount_ = static_cast<uint32_t>(lines_.size());
    if (currentInstanceCount_ > 0 && instanceData_) {
        std::memcpy(instanceData_, lines_.data(), sizeof(LineInstance) * currentInstanceCount_);
    }
}
