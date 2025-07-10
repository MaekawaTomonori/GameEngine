#include "GraphicsPipeline.hpp"

#include <cassert>
#include <format>

#include "Log.hpp"
#include "Utils.hpp"
#include "src/DirectX/DirectXAdapter.hpp"

void GraphicsPipeline::Create(DirectXAdapter* _adapter, const Type _type) {
    adapter_ = _adapter;
    type_ = _type;

    CreateRootSignature();
    CreateInputLayout();
    CreateBlendState();
    CreateShader();
    CreateRasterizerState();
    CreateDepthStencil();

    CreatePSO();
}

void GraphicsPipeline::DrawCall() const {
    adapter_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
    adapter_->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());
}

void GraphicsPipeline::SetBlendMode(BlendMode mode) {
    blendMode_ = mode;
    blendDesc_.RenderTarget[0].BlendEnable = true;
    switch (blendMode_){
        case BlendMode::ALPHA:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            break;
        case BlendMode::ADD:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
            blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
            blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            break;
        case BlendMode::SUB:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            break;
        case BlendMode::MULTI:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_DEST_COLOR;
            break;
        case BlendMode::SCREEN:
            blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
            blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
            break;
        case BlendMode::NONE:
            blendDesc_.RenderTarget[0].BlendEnable = false;
    }
    //graphicsPipelineStateDesc.BlendState = blendDesc_;
}

void GraphicsPipeline::DescriptorRange() {
    descriptorRange_[0].BaseShaderRegister = 0;
    descriptorRange_[0].NumDescriptors = 1;
    descriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

void GraphicsPipeline::CreateRootSignature() {
    HRESULT hr = S_FALSE;

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    DescriptorRange();

    D3D12_ROOT_PARAMETER rp{};
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

    if (type_ == Type::MODEL || type_ == Type::SKINNING_MODEL){
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

        //Light Count
        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rp.Descriptor.ShaderRegister = 5;
        rootParameters_.push_back(rp);
    }

    if (type_ == Type::SKINNING_MODEL) {
        D3D12_DESCRIPTOR_RANGE srvRange{};
        srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors = 1;
        srvRange.BaseShaderRegister = 0; // Base register for the descriptor table
        srvRange.RegisterSpace = 0;
        srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rp.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rp.DescriptorTable.NumDescriptorRanges = 1;
        rp.DescriptorTable.pDescriptorRanges = &srvRange;
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
        Log::Send(Log::Level::WARNING, static_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    hr = adapter_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void GraphicsPipeline::CreateInputLayout() {
    D3D12_INPUT_ELEMENT_DESC ied{};

    ied.SemanticName = "POSITION";
    ied.SemanticIndex = 0;
    ied.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    ied.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    ied.InputSlot = 0;
    inputElementDescs_.push_back(ied);

    ied.SemanticName = "TEXCOORD";
    ied.SemanticIndex = 0;
    ied.Format = DXGI_FORMAT_R32G32_FLOAT;
    ied.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs_.push_back(ied);

    if (type_ != Type::SPRITE && type_ != Type::PARTICLE2D){
        ied.SemanticName = "NORMAL";
        ied.SemanticIndex = 0;
        ied.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        ied.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        inputElementDescs_.push_back(ied);
    }

    if (type_ == Type::SKINNING_MODEL){
        ied.SemanticName = "WEIGHT";
        ied.SemanticIndex = 0;
        ied.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        ied.InputSlot = 1;
        ied.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        inputElementDescs_.push_back(ied);

        ied.SemanticName = "INDEX";
        ied.SemanticIndex = 0;
        ied.Format = DXGI_FORMAT_R32G32B32A32_SINT;
        ied.InputSlot = 1;
        ied.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        inputElementDescs_.push_back(ied);
    }
    //System::Debug::Log(std::format(L"InputElementSlot : {}\n", inputElementDescs_[0].InputSlot));

    inputLayoutDesc_.pInputElementDescs = inputElementDescs_.data();
    inputLayoutDesc_.NumElements = static_cast<UINT>(inputElementDescs_.size());
}

void GraphicsPipeline::CreateBlendState() {
    blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    if (type_ == Type::PARTICLE){
        SetBlendMode(BlendMode::ADD);
    }
}

void GraphicsPipeline::CreateShader() {
    shader_ = std::make_unique<Shader>();

    std::wstring name;
    switch (type_){
        case Type::MODEL:
            name = L"Model";
            break;
        case Type::SKINNING_MODEL:
            name = L"Skinning";
            break;
        case Type::SPRITE:
            name = L"Sprite";
            break;
        case Type::PARTICLE:
        case Type::PARTICLE2D:
            name = L"Particle";
            break;
    }

    shader_->Create(name);
}

void GraphicsPipeline::CreateRasterizerState() {
    switch (type_){
        case Type::MODEL:
        case Type::SKINNING_MODEL:
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

void GraphicsPipeline::CreatePSO() {
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc_;
    graphicsPipelineStateDesc.BlendState = blendDesc_;
    graphicsPipelineStateDesc.VS = { shader_->GetVertexShader()->GetBufferPointer(), shader_->GetVertexShader()->GetBufferSize() };
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc_;
    graphicsPipelineStateDesc.PS = { shader_->GetPixelShader()->GetBufferPointer(), shader_->GetPixelShader()->GetBufferSize() };
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc_;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    HRESULT hr = adapter_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
    assert(SUCCEEDED(hr));
    if (FAILED(hr)){
        Log::Send(Log::Level::ERR, "Failed to create graphics pipeline state");
        Utils::Alert("Failed to create graphics pipeline state");
        assert(false);
    }
}
