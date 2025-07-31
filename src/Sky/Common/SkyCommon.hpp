#ifndef SkyCommon_HPP_
#define SkyCommon_HPP_
#include "src/Common/Common.hpp"


class SkyCommon : public Common{
public:
    void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) override;
}; // class SkyCommon

#endif // SkyCommon_HPP_
