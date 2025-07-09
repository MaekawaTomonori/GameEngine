#ifndef ModelCommon_HPP_
#define ModelCommon_HPP_

#include "src/Common/Common.hpp"
#include "src/ResourceRepository/ResourceRepository.hpp"

class ModelCommon : public Common{
    ResourceRepository* resource_ = nullptr;
    void Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) override;
public:
    void Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi, ResourceRepository* _resource);
   
    ResourceRepository* GetResourceRepository() const {
        return resource_;
    }
}; // class ModelCommon

#endif // ModelCommon_HPP_
