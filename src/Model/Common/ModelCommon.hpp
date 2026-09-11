#ifndef ModelCommon_HPP_
#define ModelCommon_HPP_

#include <unordered_map>

#include "src/Common/Common.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/DirectX/Resource/DX12Resource.hpp"
#include "src/Model/ModelInstance.hpp"
#include "src/ResourceRepository/ResourceRepository.hpp"

struct CameraForGpu;

class ModelCommon : public Common{
    GESTD::ReferencePtr<ResourceRepository> resource_ = nullptr;
    SRVManager* srv_ = nullptr;

    std::unique_ptr<DX12Resource> cameraResource_;
    CameraForGpu* cameraData_ = nullptr;

    std::unique_ptr<PipelineStateObject> staticPipeline_;
    std::unique_ptr<PipelineStateObject> staticTransparentPipeline_;
    std::unique_ptr<PipelineStateObject> skinningTransparentPipeline_;

    std::vector<RenderingCommand> staticDrawCommands_;
    std::vector<RenderingCommand> skinningDrawCommands_;
    std::vector<RenderingCommand> staticTransparentCommands_;
    std::vector<RenderingCommand> skinningTransparentCommands_;

    std::unordered_map<std::string, std::function<void()>> shadowCommands_;

    uint32_t shadowSrvIndex_ = UINT_MAX;
    D3D12_GPU_VIRTUAL_ADDRESS shadowCbvAddress_ = 0;

    std::vector<std::unique_ptr<ModelInstance>> instances_;

    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi) override;
    void CreateSkinningPipeline() const;
    void CreateStaticPipeline() const;
    void CreateStaticTransparentPipeline();
    void CreateSkinningTransparentPipeline();

public:
    ~ModelCommon() override;

    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi, GESTD::ReferencePtr<ResourceRepository> _resource, SRVManager* _srv);

    void RegisterStaticDraw(const std::function<void()>& _command, const std::string& _canvasName = "Main");
    void RegisterSkinningDraw(const std::function<void()>& _command, const std::string& _canvasName = "Main");
    void RegisterStaticTransparentDraw(const std::function<void()>& _command, const std::string& _canvasName = "Main");
    void RegisterSkinningTransparentDraw(const std::function<void()>& _command, const std::string& _canvasName = "Main");
    void RegisterShadowDraw(const std::string& _id, const std::function<void()>& _func);
    void UnregisterShadowDraw(const std::string& _id);

    void ExecuteShadowDraw() const;
    void SetShadowBinding(uint32_t _srvIndex, D3D12_GPU_VIRTUAL_ADDRESS _cbvAddress);

    D3D12_GPU_VIRTUAL_ADDRESS GetCameraCBVAddress() const;

    /** @brief モデル実体を生成しプールに登録する
     * @param _name モデル名
     * @return 実体への安全な参照
     */
    GESTD::ReferencePtr<ModelInstance> CreateModelInstance(const std::string& _name);

    /** @brief モデル実体をプールから破棄する
     * @param _instance 破棄する実体への参照
     */
    void DestroyModelInstance(const GESTD::ReferencePtr<ModelInstance>& _instance);

    void Draw(Renderer* _renderer) override;

    void DrawSkinning() const;
    void DrawStatic() const;

    GESTD::ReferencePtr<ResourceRepository> GetResourceRepository() const {
        return resource_;
    }
    SRVManager* GetSRVManager() const {
        return srv_;
    }
}; // class ModelCommon

#endif // ModelCommon_HPP_
