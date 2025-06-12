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
#include <thread>
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
    BLEND_MODE_COUNT // カウント用
};

/**
 * シェーダーリソース情報を格納する構造体。
 */
struct ShaderResourceInfo{
    std::string name;
    D3D12_SHADER_INPUT_BIND_DESC bind_desc;
    uint32_t space;
    uint32_t binding_point;
};

/**
 * 定数バッファのレイアウト情報。
 */
struct ConstantBufferLayout{
    std::string name;
    uint32_t size;
    std::vector<D3D12_SHADER_VARIABLE_DESC> variables;
};

/**
 * シェーダーリフレクション情報を格納する構造体。
 */
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

    // BlendModeを指定してDrawCallを行う
    void DrawCall(BlendMode mode = BlendMode::ALPHA) const;

    // シェーダーリフレクション情報を取得
    const ShaderReflectionData& GetVertexShaderReflection() const {
        return vsReflectionData_;
    }
    const ShaderReflectionData& GetPixelShaderReflection() const {
        return psReflectionData_;
    }

    // 定数バッファのレイアウト情報を取得
    std::optional<ConstantBufferLayout> GetConstantBufferLayout(const std::string& name) const;

    // インデックスからバインドポイント情報を取得
    uint32_t GetCBVBindPoint(const std::string& name) const;
    uint32_t GetSRVBindPoint(const std::string& name) const;
    uint32_t GetSamplerBindPoint(const std::string& name) const;
    uint32_t GetUAVBindPoint(const std::string& name) const;

    // 非同期生成の完了を待機
    void WaitForAsyncCreation();

    // 特定のBlendModeのPSOが生成済みかどうかチェック
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

    // NONEモード用のPSO作成
    void CreateDefaultPSO();

    // 特定のBlendMode用のPSO作成
    void CreatePSOWithBlendMode(BlendMode mode);

    // 非同期でPSOを生成する関数
    void AsyncCreatePipelineStates();

    // シェーダーリフレクション関連のメソッド
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

    // 各ブレンドモード用のPSO
    mutable std::mutex psoMutex_; // PSOマップへのアクセス用ミューテックス
    std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> pipelineStatesByBlendMode_;
    std::atomic<bool> isAsyncCreationActive_ = false; // 非同期処理が実行中かどうか
    std::future<void> asyncCreationFuture_; // 非同期処理用Future

    D3D12_RASTERIZER_DESC rasterizerDesc_ {};

    D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};

    std::unique_ptr<Shader> shader_;

    //DepthStencil
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_ {};

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc {}; // 基本PSO設定

    // シェーダーリフレクション情報
    ShaderReflectionData vsReflectionData_ {};
    ShaderReflectionData psReflectionData_ {};

    // リソース名とバインドポイントのマッピング
    std::unordered_map<std::string, uint32_t> cbvNameToIndex_;
    std::unordered_map<std::string, uint32_t> srvNameToIndex_;
    std::unordered_map<std::string, uint32_t> samplerNameToIndex_;
    std::unordered_map<std::string, uint32_t> uavNameToIndex_;

    // リフレクションを使用するフラグ
    bool useReflection_ = false;
};
#endif // GraphicsPipeline_HPP_