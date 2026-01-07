#ifndef LineCommon_HPP_
#define LineCommon_HPP_
#include "src/Common/Common.hpp"
#include "src/DirectX/Heap/SRVManager.h"

class LineCommon : public Common {
    SRVManager* srv_ = nullptr;
    uint32_t count_ = 0;

public:
    void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi, SRVManager* _srv);

    SRVManager* GetSRVManager() const {
        return srv_;
    }

    uint32_t AddCount(){
        return ++count_;
    }

    void Remove(){
        if (count_ > 0){
            --count_;
        }
    }

private:
    void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) override;
};

#endif // LineCommon_HPP_
