#ifndef ModelCommon_HPP_
#define ModelCommon_HPP_

#include "src/Common/Common.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/ResourceRepository/ResourceRepository.hpp"

class ModelCommon : public Common{
    GESTD::WeakPtr<ResourceRepository> resource_ = nullptr;
    SRVManager* srv_ = nullptr;

    std::unique_ptr<PipelineStateObject> staticPipeline_;

    std::vector<RenderingCommand> staticDrawCommands_;
    std::vector<RenderingCommand> skinningDrawCommands_;

    void Initialize(const GESTD::WeakPtr<DirectXAdapter>& _adapter, const GESTD::WeakPtr<DebugUI>& _debugUi) override;
    void CreateSkinningPipeline() const;
    void CreateStaticPipeline() const;

public:
    void Initialize(const GESTD::WeakPtr<DirectXAdapter>& _adapter, const GESTD::WeakPtr<DebugUI>& _debugUi, GESTD::WeakPtr<ResourceRepository> _resource, SRVManager* _srv);

    void RegisterStaticDraw(const std::function<void()>& _command, bool _isApplyPostEffect = true);
    void RegisterSkinningDraw(const std::function<void()>& _command, bool _isApplyPostEffect = true);

    void Draw(Renderer* _renderer) override;

    void DrawSkinning() const;
    void DrawStatic() const;

    GESTD::WeakPtr<ResourceRepository> GetResourceRepository() const {
        return resource_;
    }
    SRVManager* GetSRVManager() const {
        return srv_;
    }
}; // class ModelCommon

#endif // ModelCommon_HPP_
