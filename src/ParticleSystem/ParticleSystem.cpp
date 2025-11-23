#include "ParticleSystem.hpp"

#include <ranges>
#include <stdexcept>

#include "DebugUI.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "imgui_internal.h"

ParticleSystem::ParticleSystem(DirectXAdapter* _adapter, SRVManager* _srv, MeshRepository* _mesh, DebugUI* _debugUI)
    : adapter_(_adapter), srv_(_srv), mesh_(_mesh), debugUI_(_debugUI) {
}

ParticleSystem::GroupEditor::GroupEditor(std::string _name, Group* _group, DirectXAdapter* _adapter, SRVManager* _srv, MeshRepository* _mesh)
    : name_(std::move(_name)), group_(_group), adapter_(_adapter), srv_(_srv), mesh_(_mesh) {
}

ParticleSystem::GroupEditor& ParticleSystem::GroupEditor::AddEmitter(const EmitterConfig& _config) {
    std::unique_ptr<Emitter> emitter = std::make_unique<Emitter>(adapter_, srv_);
    emitter->Initialize(mesh_->Get("plane"));
    emitter->SetTexture(_config.texture);
    emitter->SetFrequency(_config.frequency);
    emitter->SetSpawnCount(_config.spawnCount);
    emitter->SetColor(_config.color);
    group_->emitters.push_back(std::move(emitter));
    return *this;
}

ParticleSystem::GroupEditor& ParticleSystem::GroupEditor::SetPosition(const Vector3& _position) {
    group_->position = _position;
    return *this;
}

ParticleSystem::GroupEditor& ParticleSystem::GroupEditor::ClearEmitters() {
    group_->emitters.clear();
    return *this;
}

ParticleSystem::GroupEditor& ParticleSystem::GroupEditor::Emit() {
    for (const auto& emitter : group_->emitters) {
        emitter->Emit();
    }
    return *this;
}


void ParticleSystem::Initialize() {
    SetupPSO();
}

void ParticleSystem::Update() {
    if (groups_.empty())return;

    Debug();

    for (const auto& group : groups_ | std::views::values) {
        for (const auto& emitter : group->emitters) {
            emitter->Update();
        }
    }
}

void ParticleSystem::Draw(Renderer* _renderer) {
    if (groups_.empty())return;

    _renderer->Register([&] {
        pso_->DrawCall();
        for (auto& group : groups_ | std::views::values) {
            for (auto& emitter : group->emitters) {
                emitter->Draw();
            }
        }
        }, true);
}

void ParticleSystem::Debug() {
    debugUI_->RegisterCommand("ParticleSystemDebug", [&] { 
        ImGui::Begin("Particle System Debug");
        for (const auto& [name, group] : groups_) {
            if (ImGui::CollapsingHeader(name.c_str())) {
                ImGui::Text("Position: %f, %f, %f", group->position.x, group->position.y, group->position.z);
                ImGui::Text("Emitters: %d", group->emitters.size());
                if (ImGui::Button("Emit")) {
                    for (const auto& emitter : group->emitters) {
                        emitter->Emit();
                    }
                }

                for (const auto& emitter : group->emitters) {
                    emitter->Debug();
                }
            }
        }
        ImGui::End();
    });
}

ParticleSystem::GroupEditor ParticleSystem::Register(const std::string& _name, Vector3 _position) {
    if (groups_.contains(_name)) {
        Log::Send(Log::Level::WARNING, "Group already exists.");
        return { _name, groups_.at(_name).get(), adapter_, srv_, mesh_ };
    }

    groups_.emplace(_name, std::make_unique<Group>(_position));

    return { _name, groups_.at(_name).get(), adapter_, srv_, mesh_ };
}

ParticleSystem::GroupEditor ParticleSystem::Edit(const std::string& _name) const {
    if (groups_.contains(_name)) {
        return { _name, groups_.at(_name).get(), adapter_, srv_, mesh_ };
    }

    Log::Send(Log::Level::WARNING, "Group not found.");
    Utils::Alert("Group not found.");
    throw std::runtime_error("Group not found.");
}

void ParticleSystem::Delete(const std::string& _name) {
    if (!groups_.contains(_name)) return;

    groups_.erase(_name);
}

void ParticleSystem::SetupPSO() {
    D3D12_DESCRIPTOR_RANGE rangeForVS[1]{};
    rangeForVS[0].BaseShaderRegister = 0;
    rangeForVS[0].NumDescriptors = 1;
    rangeForVS[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    rangeForVS[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE rangeForPS[1]{};
    rangeForPS[0].BaseShaderRegister = 0;
    rangeForPS[0].NumDescriptors = 1;
    rangeForPS[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    rangeForPS[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    RootSignature rs = RootSignature();
    rs.AddParameter({
        .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        .DescriptorTable = {
            .NumDescriptorRanges = _countof(rangeForVS),
            .pDescriptorRanges = rangeForVS,
        },
        .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
    }).AddParameter({
        .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        .DescriptorTable = {
            .NumDescriptorRanges = _countof(rangeForPS),
            .pDescriptorRanges = rangeForPS,
        },
        .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
    }).SetSampler({
        .Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
        .MaxLOD = D3D12_FLOAT32_MAX,
        .ShaderRegister = 0,
        .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
    }).Create(adapter_->GetDevice());

    InputLayout il = InputLayout();
    il.SetElement({
        .SemanticName = "POSITION",
        .SemanticIndex = 0,
        .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
        .InputSlot = 0,
        .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
    }).SetElement({
        .SemanticName = "TEXCOORD",
        .SemanticIndex = 0,
        .Format = DXGI_FORMAT_R32G32_FLOAT,
        .InputSlot = 0,
        .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT
    }).SetElement({
        .SemanticName = "NORMAL",
        .SemanticIndex = 0,
        .Format = DXGI_FORMAT_R32G32B32_FLOAT,
        .InputSlot = 0,
        .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT
    });

    pso_ = std::make_unique<PipelineStateObject>(adapter_);
    pso_->SetRootSignature(rs)
        .SetInputLayout(il)
        .SetBlend(BlendMode::ADD)
        .SetShader(std::make_unique<Shader>(L"Particle"))
        .SetRasterizer({
            .FillMode = D3D12_FILL_MODE_SOLID,
            .CullMode = D3D12_CULL_MODE_NONE
        })
        .SetDepthStencil({
            .DepthEnable = true,
            .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
            .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
        })
        .SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT)
        .Create();
}
