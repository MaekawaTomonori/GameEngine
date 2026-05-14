#pragma once
#include <array>
#include <d3d12.h>
#include <memory>

#include "Math/Matrix.hpp"
#include "Math/Vector3.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/DirectX/Resource/DX12Resource.hpp"
#include "src/Light/LightManager.hpp"
#include "src/Model/Common/ModelCommon.hpp"
#include "src/Shadow/ShadowCubeMap.hpp"

class ShadowPass {
    struct ShadowCamera {
        Matrix4x4 faceVP;
        Vector3   lightPos;
        float     farPlane;
    };

    struct ShadowData {
        Vector3 lightPos;
        float   farPlane;
    };

    DirectXAdapter* adapter_ = nullptr;

    std::unique_ptr<PipelineStateObject> pso_;

    std::array<std::unique_ptr<DX12Resource>, ShadowCubeMap::FACE_COUNT> shadowCameraResources_;
    std::array<ShadowCamera*, ShadowCubeMap::FACE_COUNT>                  shadowCameras_{};

    std::unique_ptr<DX12Resource> shadowDataResource_;
    ShadowData* shadowData_ = nullptr;

public:
    void Initialize(DirectXAdapter* _adapter);
    void Execute(ShadowCubeMap& _cubeMap, ModelCommon& _model, LightManager& _light);
    D3D12_GPU_VIRTUAL_ADDRESS GetShadowDataAddress() const;
};
