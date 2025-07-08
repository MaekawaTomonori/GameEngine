#ifndef ModelCommon_HPP_
#define ModelCommon_HPP_

#include "src/Common/Common.hpp"
#include "src/Mesh/Repository/MeshRepository.hpp"
#include "src/ResourceRepository/ResourceRepository.hpp"

class ModelCommon : public Common{
    ResourceRepository* resource_ = nullptr;
public:
    void Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) override;
    ModelRepository* GetModelRepository() const {
        return resource_->GetModelRepository();
    }
    MeshRepository* GetMeshRepository() const {
        return resource_->GetMeshRepository();
    }
}; // class ModelCommon

#endif // ModelCommon_HPP_
