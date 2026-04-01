#ifndef SkyCommon_HPP_
#define SkyCommon_HPP_
#include "src/Common/Common.hpp"


class SkyCommon : public Common{
public:
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi) override;
}; // class SkyCommon

#endif // SkyCommon_HPP_
