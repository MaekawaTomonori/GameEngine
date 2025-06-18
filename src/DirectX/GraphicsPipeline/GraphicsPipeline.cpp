#include "GraphicsPipeline.hpp"

#include <cassert>
#include <format>
#include <iostream>

#include "Log.hpp"

GraphicsPipeline::~GraphicsPipeline() {
    WaitForAsyncCreation();
}

void GraphicsPipeline::Create(DirectXAdapter* adapter, Type type) {
    adapter_ = adapter;
    type_ = type;

    Create();
}

void GraphicsPipeline::DrawCall(BlendMode mode) const {
    adapter_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());

    {
        std::lock_guard<std::mutex> lock(psoMutex_);

        auto it = pipelineStatesByBlendMode_.find(mode);
        if (it != pipelineStatesByBlendMode_.end()){
            adapter_->GetCommandList()->SetPipelineState(it->second.Get());
        } else{
            auto defaultIt = pipelineStatesByBlendMode_.find(BlendMode::NONE);
            if (defaultIt != pipelineStatesByBlendMode_.end()){
                adapter_->GetCommandList()->SetPipelineState(defaultIt->second.Get());
            } else{
                if (!pipelineStatesByBlendMode_.empty()){
                    adapter_->GetCommandList()->SetPipelineState(pipelineStatesByBlendMode_.begin()->second.Get());
                }
                Log::Send(Log::Level::WARNING, "Warning: No pipeline state found for blend mode " + static_cast<int>(mode));
            }
        }
    }
}

void GraphicsPipeline::Create() {
    CreateShader();

    if (shader_ && useReflection_){
        if (shader_->GetVertexShader()){
            if (ReflectShader(
                shader_->GetVertexShader()->GetBufferPointer(),
                shader_->GetVertexShader()->GetBufferSize(),
                vsReflectionData_)){
                Log::Send(Log::Level::INFO, "Vertex shader reflection succeeded");
            } else{
                Log::Send(Log::Level::WARNING, "Vertex shader reflection failed");
            }
        }

        // ピクセルシェーダーのリフレクション
        if (shader_->GetPixelShader()){
            if (ReflectShader(
                shader_->GetPixelShader()->GetBufferPointer(),
                shader_->GetPixelShader()->GetBufferSize(),
                psReflectionData_)){
                Log::Send(Log::Level::INFO, "Pixel shader reflection succeeded");
            } else{
                Log::Send(Log::Level::WARNING, "Pixel shader reflection failed");
            }
        }

        CreateRootSignatureFromReflection();
    } else{
        CreateRootSignature();
    }

    CreateInputLayout();
    CreateRasterizerState();
    CreateDepthStencil();
    CreateSampler();

    CreateDefaultPSO();

    AsyncCreatePipelineStates();
}

void GraphicsPipeline::CreateDefaultPSO() {
    CreatePSOWithBlendMode(BlendMode::NONE);
}

void GraphicsPipeline::AsyncCreatePipelineStates() {
    if (isAsyncCreationActive_.exchange(true)){
        return;
    }

    asyncCreationFuture_ = std::async(std::launch::async, [this](){
        for (int i = 0; i < static_cast<int>(BlendMode::BLEND_MODE_COUNT); ++i){
            BlendMode mode = static_cast<BlendMode>(i);
            if (mode != BlendMode::NONE){
                CreatePSOWithBlendMode(mode);
            }
        }

        isAsyncCreationActive_ = false;
    });
}

void GraphicsPipeline::WaitForAsyncCreation() {
    if (isAsyncCreationActive_ && asyncCreationFuture_.valid()){
        asyncCreationFuture_.wait();
    }
}

bool GraphicsPipeline::IsPipelineStateReady(BlendMode mode) const {
    std::lock_guard<std::mutex> lock(psoMutex_);
    return pipelineStatesByBlendMode_.find(mode) != pipelineStatesByBlendMode_.end();
}

void GraphicsPipeline::DescriptorRange() {
    descriptorRange_[0].BaseShaderRegister = 0;
    descriptorRange_[0].NumDescriptors = 1;
    descriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

void GraphicsPipeline::CreateRootSignature() {
    HRESULT hr = S_OK;

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature {};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    DescriptorRange();

    D3D12_ROOT_PARAMETER rp {};
    if (type_ != Type::PARTICLE2D){
        //PixelShader Material
        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rp.Descriptor.ShaderRegister = 0;
        rootParameters_.push_back(rp);
    }

    //VertexShader WVP
    if (type_ == Type::PARTICLE || type_ == Type::PARTICLE2D){
        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rp.DescriptorTable.pDescriptorRanges = descriptorRange_;
        rp.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange_);
    } else{
        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rp.Descriptor.ShaderRegister = 0;
    }
    rootParameters_.push_back(rp);

    //DescriptorTable Texture
    rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rp.DescriptorTable.pDescriptorRanges = descriptorRange_;
    rp.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange_);
    rootParameters_.push_back(rp);

    if (type_ == Type::MODEL){
        //DirectionalLight
        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rp.Descriptor.ShaderRegister = 1;
        rootParameters_.push_back(rp);

        //Camera For GPU
        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rp.Descriptor.ShaderRegister = 2;
        rootParameters_.push_back(rp);


        //PointLight
        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rp.Descriptor.ShaderRegister = 3;
        rootParameters_.push_back(rp);

        //SpotLight
        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rp.Descriptor.ShaderRegister = 4;
        rootParameters_.push_back(rp);

        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rp.Descriptor.ShaderRegister = 5;
        rootParameters_.push_back(rp);
    }

    //set
    descriptionRootSignature.pParameters = rootParameters_.data();
    descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters_.size());

    //StaticSampler
    CreateSampler();
    descriptionRootSignature.pStaticSamplers = staticSamplers_;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers_);

    //Serialize
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

    if (FAILED(hr)){
        if (errorBlob){
            std::cerr << "Root signature serialization failed: "
                << static_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
        }
        assert(false);
    }

    hr = adapter_->GetDevice()->CreateRootSignature(
        0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_));

    assert(SUCCEEDED(hr));
}

void GraphicsPipeline::CreateInputLayout() {
    inputElementDescs_.resize(3);

    inputElementDescs_[0].SemanticName = "POSITION";
    inputElementDescs_[0].SemanticIndex = 0;
    inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs_[0].InputSlot = 0;

    inputElementDescs_[1].SemanticName = "TEXCOORD";
    inputElementDescs_[1].SemanticIndex = 0;
    inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    if (type_ != Type::PARTICLE2D){
        inputElementDescs_[2].SemanticName = "NORMAL";
        inputElementDescs_[2].SemanticIndex = 0;
        inputElementDescs_[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        inputElementDescs_[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    } else{
        inputElementDescs_.resize(2);
    }

    inputLayoutDesc_.pInputElementDescs = inputElementDescs_.data();
    inputLayoutDesc_.NumElements = static_cast<UINT>(inputElementDescs_.size());
}

void GraphicsPipeline::CreateBlendState(BlendMode mode, D3D12_BLEND_DESC& blendDesc) {
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    switch (mode){
        case BlendMode::ALPHA:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            break;
        case BlendMode::ADD:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            break;
        case BlendMode::SUB:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            break;
        case BlendMode::MULTI:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_DEST_COLOR;
            break;
        case BlendMode::SCREEN:
            blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
            blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            break;
        case BlendMode::NONE:
        default:
            blendDesc.RenderTarget[0].BlendEnable = false;
            break;
    }

    if (type_ == Type::PARTICLE2D){
        D3D12_RENDER_TARGET_BLEND_DESC defaultDesc {};
        defaultDesc.BlendEnable = false;
        defaultDesc.LogicOpEnable = false;
        defaultDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        blendDesc.RenderTarget[1] = defaultDesc;
        blendDesc.RenderTarget[2] = defaultDesc;
    }
}

void GraphicsPipeline::CreateShader() {
    shader_ = std::make_unique<Shader>();

    std::wstring name;
    switch (type_){
        case Type::MODEL:
            name = L"Model";
            break;
        case Type::SPRITE:
            name = L"Sprite";
            break;
        case Type::PARTICLE:
            name = L"Particle";
            break;
        case Type::PARTICLE2D:
            name = L"Particle2d";
            break;
    }

    shader_->Create(name);
}

void GraphicsPipeline::CreateRasterizerState() {
    switch (type_){
        case Type::MODEL:
            rasterizerDesc_.CullMode = D3D12_CULL_MODE_BACK;
            rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
            break;
        case Type::SPRITE:
            rasterizerDesc_.CullMode = D3D12_CULL_MODE_NONE;
            rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
            break;
        case Type::PARTICLE:
        case Type::PARTICLE2D:
            rasterizerDesc_.CullMode = D3D12_CULL_MODE_NONE;
            rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
            break;
        default:;
    }
}

void GraphicsPipeline::CreateSampler() {
    staticSamplers_[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers_[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers_[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers_[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers_[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers_[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers_[0].ShaderRegister = 0;
    staticSamplers_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void GraphicsPipeline::CreateDepthStencil() {
    depthStencilDesc_.DepthEnable = true;
    depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    if (type_ == Type::PARTICLE){
        //depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    }
}

void GraphicsPipeline::CreatePSOWithBlendMode(BlendMode mode) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    D3D12_BLEND_DESC blendDesc = {};
    CreateBlendState(mode, blendDesc);

    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.InputLayout = inputLayoutDesc_;
    psoDesc.BlendState = blendDesc;
    psoDesc.VS = {shader_->GetVertexShader()->GetBufferPointer(), shader_->GetVertexShader()->GetBufferSize()};
    psoDesc.RasterizerState = rasterizerDesc_;
    psoDesc.PS = {shader_->GetPixelShader()->GetBufferPointer(), shader_->GetPixelShader()->GetBufferSize()};
    psoDesc.DepthStencilState = depthStencilDesc_;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    if (type_ == Type::PARTICLE2D){
        psoDesc.NumRenderTargets = 3;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        psoDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    } else{
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    }
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    HRESULT hr = adapter_->GetDevice()->CreateGraphicsPipelineState(
        &psoDesc, IID_PPV_ARGS(&pipelineState));

    if (SUCCEEDED(hr)){
        {
            std::lock_guard<std::mutex> lock(psoMutex_);
            pipelineStatesByBlendMode_[mode] = pipelineState;
        }

        std::cout << "Created pipeline state for blend mode " << static_cast<int>(mode) << std::endl;
    } else{
        std::cerr << "Failed to create pipeline state for blend mode "
            << static_cast<int>(mode) << ", hr: 0x" << std::hex << hr << std::endl;
        assert(false);
    }
}

bool GraphicsPipeline::ReflectShader(const void* shader_bytecode, size_t bytecode_length, ShaderReflectionData& out_data) {
    Microsoft::WRL::ComPtr<ID3D12ShaderReflection> reflection;
    HRESULT hr = D3DReflect(shader_bytecode, bytecode_length, IID_PPV_ARGS(&reflection));

    if (FAILED(hr)){
        std::cerr << "Failed to create shader reflection: 0x" << std::hex << hr << std::endl;
        return false;
    }

    D3D12_SHADER_DESC shaderDesc;
    reflection->GetDesc(&shaderDesc);
    out_data.shaderDesc = shaderDesc;

    for (UINT i = 0; i < shaderDesc.BoundResources; i++){
        D3D12_SHADER_INPUT_BIND_DESC bindDesc;
        reflection->GetResourceBindingDesc(i, &bindDesc);

        ShaderResourceInfo resource;
        resource.name = bindDesc.Name;
        resource.bind_desc = bindDesc;
        resource.space = bindDesc.Space;
        resource.binding_point = bindDesc.BindPoint;

        switch (bindDesc.Type){
            case D3D_SIT_CBUFFER:
                out_data.constantBuffers.push_back(resource);

                {
                    ID3D12ShaderReflectionConstantBuffer* cbReflection =
                        reflection->GetConstantBufferByName(bindDesc.Name);

                    ConstantBufferLayout cbLayout;
                    cbLayout.name = bindDesc.Name;
                    ExtractConstantBufferLayout(cbReflection, cbLayout);
                    out_data.cbufferLayouts[bindDesc.Name] = cbLayout;
                }
                break;

            case D3D_SIT_TEXTURE:
            case D3D_SIT_STRUCTURED:
            case D3D_SIT_BYTEADDRESS:
                out_data.textures.push_back(resource);
                break;

            case D3D_SIT_SAMPLER:
                out_data.samplers.push_back(resource);
                break;

            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SIT_UAV_APPEND_STRUCTURED:
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                out_data.unorderedAccessViews.push_back(resource);
                break;
        }
    }

    return true;
}

void GraphicsPipeline::ExtractConstantBufferLayout(
    ID3D12ShaderReflectionConstantBuffer* cb_reflection,
    ConstantBufferLayout& out_layout)
{
    D3D12_SHADER_BUFFER_DESC bufferDesc;
    cb_reflection->GetDesc(&bufferDesc);

    out_layout.size = bufferDesc.Size;

    for (UINT i = 0; i < bufferDesc.Variables; i++){
        ID3D12ShaderReflectionVariable* var = cb_reflection->GetVariableByIndex(i);

        D3D12_SHADER_VARIABLE_DESC varDesc;
        var->GetDesc(&varDesc);

        out_layout.variables.push_back(varDesc);
    }
}

void GraphicsPipeline::CreateRootSignatureFromReflection() {
}

std::optional<ConstantBufferLayout> GraphicsPipeline::GetConstantBufferLayout(
    const std::string& name) const
{
    auto vs_it = vsReflectionData_.cbufferLayouts.find(name);
    if (vs_it != vsReflectionData_.cbufferLayouts.end()){
        return vs_it->second;
    }

    auto ps_it = psReflectionData_.cbufferLayouts.find(name);
    if (ps_it != psReflectionData_.cbufferLayouts.end()){
        return ps_it->second;
    }

    return std::nullopt;
}

uint32_t GraphicsPipeline::GetCBVBindPoint(const std::string& name) const {
    auto it = cbvNameToIndex_.find(name);
    if (it != cbvNameToIndex_.end()){
        return it->second;
    }
    return UINT32_MAX;
}

uint32_t GraphicsPipeline::GetSRVBindPoint(const std::string& name) const {
    auto it = srvNameToIndex_.find(name);
    if (it != srvNameToIndex_.end()){
        return it->second;
    }
    return UINT32_MAX;
}

uint32_t GraphicsPipeline::GetSamplerBindPoint(const std::string& name) const {
    auto it = samplerNameToIndex_.find(name);
    if (it != samplerNameToIndex_.end()){
        return it->second;
    }
    return UINT32_MAX;
}

uint32_t GraphicsPipeline::GetUAVBindPoint(const std::string& name) const {
    auto it = uavNameToIndex_.find(name);
    if (it != uavNameToIndex_.end()){
        return it->second;
    }
    return UINT32_MAX;
}
