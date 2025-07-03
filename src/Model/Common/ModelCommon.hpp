#ifndef ModelCommon_HPP_
#define ModelCommon_HPP_

#include "src/Common/Common.hpp"

class ModelCommon : public Common{
public:
    void Initialize(DirectXAdapter *_adapter, DebugUI *_debugUi) override;
}; // class ModelCommon

#endif // ModelCommon_HPP_
