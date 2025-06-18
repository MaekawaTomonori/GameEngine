#ifndef GraphicsPipeline_HPP_
#define GraphicsPipeline_HPP_

#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>
#include <unordered_map>
#include <string>
#include <optional>
#include <d3dcompiler.h>
#include <format>
#include <future>
#include <mutex>
#include <atomic>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Shader/Shader.h"

class Heap;

enum class BlendMode{
    ALPHA,
    ADD,
    SUB,
    MULTI,
    SCREEN,
    NONE,
    BLEND_MODE_COUNT
};

struct ShaderResourceInfo{
    std::string name;
    D3D12_SHADER_INPUT_BIND_DESC bind_desc;
    uint32_t space;
    uint32_t binding_point;
};

struct ConstantBufferLayout{
    std::string name;
    uint32_t size;
    std::vector<D3D12_SHADER_VARIABLE_DESC> variables;
};

struct ShaderReflectionData{
    std::vector<ShaderResourceInfo> constantBuffers;
    std::vector<ShaderResourceInfo> textures;
    std::vector<ShaderResourceInfo> samplers;
    std::vector<ShaderResourceInfo> unorderedAccessViews;
    std::unordered_map<std::string, ConstantBufferLayout> cbufferLayouts;
    D3D12_SHADER_DESC shaderDesc;
};

class GraphicsPipeline{
    public:
    enum class Type{
        MODEL,
        SPRITE,
        PARTICLE,
        PARTICLE2D,
    };

    GraphicsPipeline() = default;
    ~GraphicsPipeline();

    void Create(DirectXAdapter* adapter, Type type);

    void DrawCall(BlendMode mode = BlendMode::ALPHA) const;

    const ShaderReflectionData& GetVertexShaderReflection() const {
        return vsReflectionData_;
    }
    const ShaderReflectionData& GetPixelShaderReflection() const {
        return psReflectionData_;
    }

    std::optional<ConstantBufferLayout> GetConstantBufferLayout(const std::string& name) const;

    uint32_t GetCBVBindPoint(const std::string& name) const;
    uint32_t GetSRVBindPoint(const std::string& name) const;
    uint32_t GetSamplerBindPoint(const std::string& name) const;
    uint32_t GetUAVBindPoint(const std::string& name) const;

    void WaitForAsyncCreation();

    bool IsPipelineStateReady(BlendMode mode) const;

private://Methods
    void Create();
    void CreateRootSignature();
    void DescriptorRange();
    void CreateInputLayout();
    void CreateBlendState(BlendMode mode, D3D12_BLEND_DESC& blendDesc);
    void CreateShader();
    void CreateRasterizerState();
    void CreateSampler();
    void CreateDepthStencil();

    void CreateDefaultPSO();

    void CreatePSOWithBlendMode(BlendMode mode);

    void AsyncCreatePipelineStates();

    bool ReflectShader(const void* shader_bytecode, size_t bytecode_length, ShaderReflectionData& out_data);
    void ExtractConstantBufferLayout(ID3D12ShaderReflectionConstantBuffer* cb_reflection, ConstantBufferLayout& out_layout);
    void CreateRootSignatureFromReflection();

private://Variables
    DirectXAdapter* adapter_ = nullptr;

    Type type_ = Type::MODEL;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
    D3D12_DESCRIPTOR_RANGE descriptorRange_[1] {};
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_ {};
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs_ {};

    mutable std::mutex psoMutex_;
    std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> pipelineStatesByBlendMode_;
    std::atomic<bool> isAsyncCreationActive_ = false;
    std::future<void> asyncCreationFuture_;

    D3D12_RASTERIZER_DESC rasterizerDesc_ {};

    D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};

    std::unique_ptr<Shader> shader_;

    //DepthStencil
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_ {};

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc {};

    ShaderReflectionData vsReflectionData_ {};
    ShaderReflectionData psReflectionData_ {};

    std::unordered_map<std::string, uint32_t> cbvNameToIndex_;
    std::unordered_map<std::string, uint32_t> srvNameToIndex_;
    std::unordered_map<std::string, uint32_t> samplerNameToIndex_;
    std::unordered_map<std::string, uint32_t> uavNameToIndex_;

    bool useReflection_ = false;
};
#endif // GraphicsPipeline_HPP_