#ifndef LineCommon_HPP_
#define LineCommon_HPP_
#include "src/Common/Common.hpp"
#include "src/DirectX/Heap/SRVManager.h"

class LineCommon : public Common {
    SRVManager* srv_ = nullptr;
    void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) override;
public:
    void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, SRVManager* _srv);
    
    SRVManager* GetSRVManager() const {
        return srv_;
    }
};

#endif // LineCommon_HPP_