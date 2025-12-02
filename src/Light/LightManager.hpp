#ifndef LIGHTMANAGER_HPP_
#define LIGHTMANAGER_HPP_

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

/** @brief ライト管理クラス
 ** 指向性ライト、点光源、スポットライトを統合管理
 **/
class LightManager final{
	/** @brief ライト数カウント
	 **/
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

    std::optional<std::reference_wrapper<Vector3>> ref_;

public:
    ~LightManager();

    /** @brief ライトマネージャーを初期化
     ** @param _adapter DirectXアダプター
     ** @param _debug デバッグUI
     **/
	void Initialize(DirectXAdapter* _adapter, DebugUI* _debug);

    /** @brief ライトの更新処理
     **/
    void Update();

    /** @brief ライトを描画
     **/
    void Draw() const;

    /** @brief ライトを追加
     ** @param type ライトタイプ
     **/
    void Add(LightType type);

    /** @brief ライトの参照座標を設定
     ** @param _ref 参照座標
     **/
    void SetReference(Vector3& _ref);

private:
    void Debug();
    void CheckState();

    void Load();
    void Save() const;
};

#endif // LIGHTMANAGER_HPP_

