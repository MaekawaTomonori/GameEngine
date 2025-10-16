#pragma once
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "DebugUI.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

#include "LightType.hpp"
#include "DirectionalLight/DirectionalLight.h"
#include "PointLight/PointLight.h"
#include "SpotLight/SpotLight.h"

/// <summary>
/// ライト管理クラス
/// 指向性ライト、点光源、スポットライトを統合管理
/// </summary>
class LightManager final{
	/// <summary>
	/// ライト数カウント
	/// </summary>
	struct LightCount{
        uint32_t dlCount;
        uint32_t plCount;
        uint32_t slCount;
	};
    DirectXAdapter* adapter_ = nullptr;
    DebugUI* debug_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    std::unique_ptr<DX12Resource> directionalResource_;
    DirectionalLight* mdDirectional_ = nullptr;

    std::unique_ptr<DX12Resource> pointResource_;
    PointLight* mdPointLight_ = nullptr;

    std::unique_ptr<DX12Resource> spotResource_;
    SpotLight* mdSpotLight_ = nullptr;

    std::unique_ptr<DX12Resource> countResource_;
    LightCount* lightCount_ = nullptr;

    const LightCount MAX_COUNT {20, 20, 20};

    std::vector<std::unique_ptr<RawDirectionalLight>> rawDirectionalLights_;
    std::vector<std::unique_ptr<RawPointLight>> rawPointLights_;
    std::vector<std::unique_ptr<RawSpotLight>> rawSpotLights_;

    std::string path = "Light";

public:
    ~LightManager();

    /// <summary>
    /// ライトマネージャーを初期化
    /// </summary>
    /// <param name="_adapter">DirectXアダプター</param>
    /// <param name="_debug">デバッグUI</param>
	void Initialize(DirectXAdapter* _adapter, DebugUI* _debug);

    /// <summary>
    /// ライトの更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// ライトを描画
    /// </summary>
    void Draw() const;

    /// <summary>
    /// ライトを追加
    /// </summary>
    /// <param name="type">ライトタイプ</param>
    void Add(LightType type);

private:
    void Debug();
    void CheckState();

    void Load();
    void Save() const;
};

