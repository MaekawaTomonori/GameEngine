#include "ParticleSystem.hpp"

#include <filesystem>
#include <fstream>
#include <ranges>

#include "DebugUI.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "imgui_internal.h"
#include "json.hpp"

ParticleSystem::TemplateEditor::TemplateEditor(Template* _template)
    : template_(_template) {
}

ParticleSystem::TemplateEditor& ParticleSystem::TemplateEditor::AddEmitter(const EmitterConfig& _config) {
    template_->emitters.push_back(_config);
    return *this;
}

ParticleSystem::ParticleSystem(GESTD::ReferencePtr<DirectXAdapter> _adapter, SRVManager* _srv, GESTD::ReferencePtr<MeshRepository> _mesh, GESTD::ReferencePtr<DebugUI> _debugUI)
    : adapter_(_adapter), srv_(_srv), mesh_(_mesh), debugUI_(_debugUI) {
}

void ParticleSystem::Initialize() {
    SetupPSO();
    LoadTemplates();
    debugUI_->RegisterMenuButton("Particle");
}

void ParticleSystem::Update() {
    for (auto* emitter : active_) {
        emitter->Update();
    }

    std::erase_if(active_, [this](Emitter* _emitter) {
        if (_emitter->IsFinished()) {
            _emitter->Reset();
            available_.push_back(_emitter);
            return true;
        }
        return false;
    });
}

void ParticleSystem::Draw(Renderer* _renderer) {
    if (active_.empty()) return;

    _renderer->Register([&] {
        pso_->DrawCall();
        for (auto* emitter : active_) {
            emitter->Draw();
        }
    }, true);
}

void ParticleSystem::RegisterUpdateFunc(const std::string& _key, UpdateFunc _func) {
    if (updateFuncs_.contains(_key)) {
        Log::Send(Log::Level::WARNING, "UpdateFunc key already exists, overwriting: " + _key);
    }
    updateFuncs_[_key] = std::move(_func);
}

ParticleSystem::TemplateEditor ParticleSystem::Register(const std::string& _name) {
    if (templates_.contains(_name)) {
        Log::Send(Log::Level::WARNING, "Template already exists: " + _name);
        return TemplateEditor(&templates_[_name]);
    }
    templates_[_name] = Template{};
    return TemplateEditor(&templates_[_name]);
}

ParticleSystem::TemplateEditor ParticleSystem::Register(const std::string& _name, const Template& _template, bool _overwrite) {
    if (!_overwrite && templates_.contains(_name)) {
        Log::Send(Log::Level::WARNING, "Template already exists. About : " + _name);
        return TemplateEditor(&templates_[_name]);
    }
    templates_[_name] = _template;
    return TemplateEditor(&templates_[_name]);
}

void ParticleSystem::LoadTemplate(const std::string& _name) {
    std::string path = PATH + _name + ".json";
    std::ifstream file(path);
    if (!file.is_open()) {
        Log::Send(Log::Level::WARNING, "Failed to open particle template: " + path);
        return;
    }

    nlohmann::json root;
    file >> root;

    Template tmpl;
    if (root.contains("Emitters") && root["Emitters"].is_array()) {
        for (const auto& emitterData : root["Emitters"]) {
            EmitterConfig config;
            config.texture = emitterData.value("Texture", "white_x16.png");
            config.frequency = emitterData.value("Frequency", 0.f);
            config.duration = emitterData.value("Duration", 1.f);
            config.spawnCount = emitterData.value("SpawnCount", static_cast<uint16_t>(1));

            if (emitterData.contains("Size") && emitterData["Size"].is_array() && emitterData["Size"].size() >= 3) {
                config.size = {emitterData["Size"][0].get<float>(), emitterData["Size"][1].get<float>(), emitterData["Size"][2].get<float>()};
            }
            if (emitterData.contains("Velocity") && emitterData["Velocity"].is_array() && emitterData["Velocity"].size() >= 3) {
                config.velocity = {emitterData["Velocity"][0].get<float>(), emitterData["Velocity"][1].get<float>(), emitterData["Velocity"][2].get<float>()};
            }
            if (emitterData.contains("Color") && emitterData["Color"].is_array() && emitterData["Color"].size() >= 4) {
                config.color = {emitterData["Color"][0].get<float>(), emitterData["Color"][1].get<float>(), emitterData["Color"][2].get<float>(), emitterData["Color"][3].get<float>()};
            }

            config.updateFuncKey = emitterData.value("UpdateFunc", "");
            config.spawnFuncKey  = emitterData.value("SpawnFunc",  "");
            tmpl.emitters.push_back(config);
        }
    }

    templates_[_name] = std::move(tmpl);
    Log::Send(Log::Level::INFO, "Loaded particle template: " + _name);
}

void ParticleSystem::SaveTemplate(const std::string& _name) const {
    if (!templates_.contains(_name)) {
        Log::Send(Log::Level::WARNING, "Template not found for saving: " + _name);
        return;
    }

    const auto& tmpl = templates_.at(_name);
    nlohmann::json root;
    nlohmann::json emittersJson = nlohmann::json::array();

    for (const auto& config : tmpl.emitters) {
        nlohmann::json emitterData;
        emitterData["Texture"] = config.texture;
        emitterData["Frequency"] = config.frequency;
        emitterData["Duration"] = config.duration;
        emitterData["SpawnCount"] = config.spawnCount;
        emitterData["Size"] = {config.size.x, config.size.y, config.size.z};
        emitterData["Velocity"] = {config.velocity.x, config.velocity.y, config.velocity.z};
        emitterData["Color"] = {config.color.x, config.color.y, config.color.z, config.color.w};
        if (!config.updateFuncKey.empty()) {
            emitterData["UpdateFunc"] = config.updateFuncKey;
        }
        if (!config.spawnFuncKey.empty()) {
            emitterData["SpawnFunc"] = config.spawnFuncKey;
        }
        emittersJson.push_back(emitterData);
    }

    root["Emitters"] = emittersJson;

    std::filesystem::create_directories("Assets/Data/Particle/");
    std::string path = PATH + _name + ".json";
    std::ofstream file(path);
    if (!file.is_open()) {
        Log::Send(Log::Level::WARNING, "Failed to save particle template: " + path);
        return;
    }

    file << root.dump(4);
    Log::Send(Log::Level::INFO, "Saved particle template: " + _name);
}

void ParticleSystem::Emit(const std::string& _templateName, const Vector3& _position) {
    if (!poolInitialized_) {
        InitializePool();
    }

    if (!templates_.contains(_templateName)) {
        Log::Send(Log::Level::WARNING, "Template not found: " + _templateName);
        return;
    }

    const auto& tmpl = templates_[_templateName];

    for (const auto& config : tmpl.emitters) {
        if (available_.empty()) {
            Log::Send(Log::Level::WARNING, "Emitter pool exhausted, skipping emit");
            return;
        }

        Emitter* emitter = available_.back();
        available_.pop_back();

        emitter->SetTexture(config.texture)
            .SetFrequency(config.frequency)
            .SetDuration(config.duration)
            .SetSpawnCount(config.spawnCount)
            .SetSize(config.size)
            .SetVelocity(config.velocity)
            .SetColor(config.color)
            .SetPosition(_position);

        UpdateFunc updateFunc = ResolveUpdateFunc(config);
        if (updateFunc) {
            emitter->SetUpdateFunction(updateFunc);
        }

        SpawnFunc spawnFunc = ResolveSpawnFunc(config);
        if (spawnFunc) {
            emitter->SetSpawnFunction(spawnFunc);
        }

        emitter->Emit();
        active_.push_back(emitter);
    }
}

void ParticleSystem::DeleteTemplate(const std::string& _name) {
    templates_.erase(_name);
}

void ParticleSystem::ClearActive() {
    for (auto* emitter : active_) {
        emitter->Reset();
        available_.push_back(emitter);
    }
    active_.clear();
}


ParticleSystem::UpdateFunc ParticleSystem::ResolveUpdateFunc(const EmitterConfig& _config) const {
    if (_config.updateFunc) {
        return _config.updateFunc;
    }
    if (!_config.updateFuncKey.empty() && updateFuncs_.contains(_config.updateFuncKey)) {
        return updateFuncs_.at(_config.updateFuncKey);
    }
    return nullptr;
}

void ParticleSystem::RegisterSpawnFunc(const std::string& _key, SpawnFunc _func) {
    if (spawnFuncs_.contains(_key)) {
        Log::Send(Log::Level::WARNING, "SpawnFunc key already exists, overwriting: " + _key);
    }
    spawnFuncs_[_key] = std::move(_func);
}

ParticleSystem::SpawnFunc ParticleSystem::ResolveSpawnFunc(const EmitterConfig& _config) const {
    if (_config.spawnFunc) {
        return _config.spawnFunc;
    }
    if (!_config.spawnFuncKey.empty() && spawnFuncs_.contains(_config.spawnFuncKey)) {
        return spawnFuncs_.at(_config.spawnFuncKey);
    }
    return nullptr;
}

void ParticleSystem::InitializePool() {
    const auto& planeMesh = mesh_->Get("plane");
    pool_.reserve(POOL_SIZE);
    available_.reserve(POOL_SIZE);
    for (uint16_t i = 0; i < POOL_SIZE; ++i) {
        auto emitter = std::make_unique<Emitter>(adapter_, srv_);
        emitter->Initialize(planeMesh);
        available_.push_back(emitter.get());
        pool_.push_back(std::move(emitter));
    }
    poolInitialized_ = true;
}

void ParticleSystem::LoadTemplates() {
    std::vector<std::string> files = Utils::GetFileNames(PATH, ".json");

    for (const auto& file : files) {
        LoadTemplate(file);
    }
}

void ParticleSystem::Debug() {
    debugUI_->RegisterCommand("Particle", [&] {
        ImGui::Begin("Particle Editor", &debugUI_->IsVisible("Particle"));

        // 登録済みテンプレート一覧
        std::string toDelete;
        for (auto& [name, tmpl] : templates_) {
            ImGui::PushID(name.c_str());
            if (ImGui::CollapsingHeader(name.c_str())) {
                int emitterIndex = 0;
                int deleteEmitterIndex = -1;
                for (auto& config : tmpl.emitters) {
                    ImGui::PushID(emitterIndex);
                    if (ImGui::TreeNode(("Emitter " + std::to_string(emitterIndex)).c_str())) {
                        ImGui::Text("Texture: %s", config.texture.c_str());
                        ImGui::DragFloat("Frequency", &config.frequency, 0.01f, 0.f, 100.f);
                        ImGui::DragFloat("Duration", &config.duration, 0.01f, 0.01f, 100.f);
                        int spawnCount = static_cast<int>(config.spawnCount);
                        if (ImGui::DragInt("SpawnCount", &spawnCount, 1.f, 1, 1000)) {
                            config.spawnCount = static_cast<uint16_t>(std::clamp(spawnCount, 1, 1000));
                        }
                        ImGui::DragFloat3("Size", &config.size.x, 0.01f, 0.01f, 100.f);
                        ImGui::DragFloat3("Velocity", &config.velocity.x, 0.01f, -100.f, 100.f);
                        ImGui::ColorEdit4("Color", &config.color.x);

                        if (!config.updateFuncKey.empty()) {
                            ImGui::Text("UpdateFunc: %s", config.updateFuncKey.c_str());
                        }
                        if (!config.spawnFuncKey.empty()) {
                            ImGui::Text("SpawnFunc:  %s", config.spawnFuncKey.c_str());
                        }

                        ImGui::Spacing();
                        if (ImGui::SmallButton("Test")) {
                            if (!poolInitialized_) InitializePool();
                            if (!available_.empty()) {
                                Emitter* emitter = available_.back();
                                available_.pop_back();
                                emitter->SetTexture(config.texture)
                                    .SetFrequency(config.frequency)
                                    .SetDuration(config.duration)
                                    .SetSpawnCount(config.spawnCount)
                                    .SetSize(config.size)
                                    .SetVelocity(config.velocity)
                                    .SetColor(config.color)
                                    .SetPosition({0.f, 1.f, 0.f});
                                UpdateFunc uf = ResolveUpdateFunc(config);
                                if (uf) emitter->SetUpdateFunction(uf);
                                SpawnFunc sf = ResolveSpawnFunc(config);
                                if (sf) emitter->SetSpawnFunction(sf);
                                emitter->Emit();
                                active_.push_back(emitter);
                            }
                        }
                        ImGui::SameLine();
                        if (ImGui::SmallButton("Delete")) {
                            deleteEmitterIndex = emitterIndex;
                        }

                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                    emitterIndex++;
                }

                if (deleteEmitterIndex >= 0) {
                    tmpl.emitters.erase(tmpl.emitters.begin() + deleteEmitterIndex);
                }

                ImGui::Spacing();
                if (ImGui::Button("Add Emitter")) {
                    tmpl.emitters.push_back(EmitterConfig{});
                }
                ImGui::SameLine();
                if (ImGui::Button("Save")) {
                    SaveTemplate(name);
                }
                ImGui::SameLine();
                if (ImGui::Button("Test Emit")) {
                    Emit(name, {0.f, 1.f, 0.f});
                }
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.5f, 0.1f, 0.1f, 1.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.1f, 0.1f, 1.f));
                if (ImGui::Button("Delete Template")) {
                    toDelete = name;
                }
                ImGui::PopStyleColor(2);
            }
            ImGui::PopID();
        }

        if (!toDelete.empty()) {
            DeleteTemplate(toDelete);
        }

        // 新規テンプレート作成
        ImGui::Separator();
        ImGui::Text("New Template");
        static char newName[64] = {};
        ImGui::SetNextItemWidth(180.f);
        ImGui::InputText("##newname", newName, sizeof(newName));
        ImGui::SameLine();
        const bool canCreate = newName[0] != '\0' && !templates_.contains(newName);
        if (!canCreate) ImGui::BeginDisabled();
        if (ImGui::Button("Create")) {
            Register(newName);
            newName[0] = '\0';
        }
        if (!canCreate) ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::Text("Pool: %d / %d (Available: %d, Active: %d)",
            static_cast<int>(available_.size() + active_.size()),
            static_cast<int>(POOL_SIZE),
            static_cast<int>(available_.size()),
            static_cast<int>(active_.size()));

        ImGui::End();
    });
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
    });

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
            .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO,
            .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
        })
        .SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT)
        .Create();
}
