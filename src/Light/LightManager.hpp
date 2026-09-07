#ifndef LIGHTMANAGER_HPP_
#define LIGHTMANAGER_HPP_

#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "DebugUI.hpp"
#include "ReferencePtr.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"

#include "LightType.hpp"
#include "DirectionalLight/DirectionalLight.h"
#include "PointLight/PointLight.h"
#include "SpotLight/SpotLight.h"

/** @brief ライト管理クラス
 * 指向性ライト、点光源、スポットライトを統合管理
 */
class LightManager final{
	/** @brief ライト数カウント
	 */
	struct LightCount{
        uint32_t dlCount;
        uint32_t plCount;
        uint32_t slCount;
	};
    GESTD::ReferencePtr<DirectXAdapter> adapter_;
    GESTD::ReferencePtr<DebugUI> debug_;
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

    std::optional<Vector3> ref_;
    bool refEnabled_ = true;

public:
    ~LightManager();

    /** @brief ライトマネージャーを初期化
     * @param _adapter DirectXアダプター
     * @param _debug デバッグUI
     */
	void Initialize(GESTD::ReferencePtr<DirectXAdapter> _adapter, GESTD::ReferencePtr<DebugUI> _debug);


    /** @brief ライトの更新処理
     */
    void Update();

    /** @brief ライトを描画
     */
    void Draw() const;

    /** @brief ライトを追加
     * @param _type ライトタイプ
     */
    void Add(LightType _type);

    /** @brief 参照追従が有効なライトの座標を設定
     * @param _pos 参照座標
     * @note SetFollowsGlobalRef(true) を設定したライトのみが対象。
     *       他のライトはそれぞれの個別設定を維持する。
     */
    void SetPosition(const Vector3& _pos);

    /** @brief 参照追従が有効な点光源の強さを設定
     * @param _intensity 強さ
     */
    void SetIntensity(float _intensity);

    /** @brief 参照追従が有効な点光源の半径を設定
     * @param _radius 半径
     */
    void SetRadius(float _radius);

    void ClearRef();

    const PointLight* GetShadowCastingPointLight();

    void Debug();

private:
    void CheckState();
    void UpdateLights();

    void Load();
    void Save() const;
};

#endif // LIGHTMANAGER_HPP_
