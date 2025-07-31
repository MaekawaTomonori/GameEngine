#include "Line.hpp"
#include "Pattern/Singleton.hpp"
#include "Utils.hpp"
#include "imgui_internal.h"
#include "src/Line/Common/LineCommon.hpp"
#include "Math/MathUtils.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
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
    CreateMaterialBuffer();
    CreateTransformationBuffer();
}

void Line::Update() const {
    common_->RegisterCommand(uuid_, [&] {
        ImGui::Begin("Line");
        ImGui::ColorEdit4("color", &materialData_->color.x);
        ImGui::End();
    });

    // Update vertex data with positions
    for (size_t i = 0; i < positions_.size(); ++i){
        vertexData_[i].position = positions_[i];
    }
    Matrix4x4 world = MathUtils::Matrix::MakeIdentity();
    transformationData_->WVP = world * cameraManager_->GetActive()->GetViewProjection();
}

void Line::Draw() const {
    common_->Draw();
    
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
    
    commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->Get()->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootConstantBufferView(1, transformationResource_->Get()->GetGPUVirtualAddress());
    
    commandList_->DrawInstanced(static_cast<UINT>(positions_.size()), 1, 0, 0);
}

void Line::AddLine(const Vector3& start, const Vector3& end) {
    if (positions_.size() >= MAX_LINES) {
        Utils::Alert("Line::AddLine: Maximum number of lines exceeded. Cannot add more lines.");
        return;
    }

    positions_.push_back({start.x, start.y, start.z, 1.f});
    positions_.push_back({end.x, end.y, end.z, 1.f});
}

void Line::Clear() {
    positions_.clear();
}

void Line::SetColor(Vector4 color) const {
    if (!materialData_){
        Utils::Alert("Line::SetColor: Material data is not initialized.");
        return;
    }
    materialData_->color = color;
}

void Line::CreateVertexBuffer() {
    vertexResource_ = adapter_->CreateBufferResource(sizeof(VertexData) * 2 * MAX_LINES);
    vertexBufferView_.BufferLocation = vertexResource_->Get()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 2 * MAX_LINES;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    vertexResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    vertexData_[0].position = {0.0f, -0.5f, 0.0f, 1.0f};
    vertexData_[1].position = {0.0f, 0.5f, 0.0f, 1.0f};
}

void Line::CreateMaterialBuffer() {
    materialResource_ = adapter_->CreateBufferResource(sizeof(Material));
    materialResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
  }

void Line::CreateTransformationBuffer() {
    transformationResource_ = adapter_->CreateBufferResource(sizeof(Transformation));
    transformationResource_->Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationData_));
    transformationData_->WVP = MathUtils::Matrix::MakeIdentity();
}
