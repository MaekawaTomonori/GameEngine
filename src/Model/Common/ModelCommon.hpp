#ifndef ModelCommon_HPP_
#define ModelCommon_HPP_

#include "src/Common/Common.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/ResourceRepository/ResourceRepository.hpp"

class ModelCommon : public Common{
    GESTD::ReferencePtr<ResourceRepository> resource_ = nullptr;
    SRVManager* srv_ = nullptr;

    std::unique_ptr<PipelineStateObject> staticPipeline_;

    std::vector<RenderingCommand> staticDrawCommands_;
    std::vector<RenderingCommand> skinningDrawCommands_;

    std::unordered_map<std::string, std::function<void()>> shadowCommands_;

    uint32_t shadowSrvIndex_ = UINT_MAX;
    D3D12_GPU_VIRTUAL_ADDRESS shadowCbvAddress_ = 0;

    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi) override;
    void CreateSkinningPipeline() const;
    void CreateStaticPipeline() const;

public:
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi, GESTD::ReferencePtr<ResourceRepository> _resource, SRVManager* _srv);

    void RegisterStaticDraw(const std::function<void()>& _command, bool _isApplyPostEffect = true);
    void RegisterSkinningDraw(const std::function<void()>& _command, bool _isApplyPostEffect = true);
    void RegisterShadowDraw(const std::string& _id, const std::function<void()>& _func);
    void UnregisterShadowDraw(const std::string& _id);

    void ExecuteShadowDraw() const;
    void SetShadowBinding(uint32_t _srvIndex, D3D12_GPU_VIRTUAL_ADDRESS _cbvAddress);

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
