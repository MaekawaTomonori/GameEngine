#ifndef LineCommon_HPP_
#define LineCommon_HPP_
#include <vector>

#include "src/Common/Common.hpp"
#include "src/DirectX/Heap/SRVManager.h"
#include "src/Line/LineInstance.hpp"

class LineCommon : public Common {
    SRVManager* srv_ = nullptr;
    uint32_t count_ = 0;

    std::vector<std::unique_ptr<LineInstance>> instances_;
    std::vector<LineInstance*> drawQueue_;

public:
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi, SRVManager* _srv);

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

    /** @brief ライン実体を生成し登録する
     * @return 実体への安全な参照
     */
    GESTD::ReferencePtr<LineInstance> CreateInstance();

    /** @brief ライン実体を破棄する
     * @param _instance 破棄する実体への参照
     */
    void DestroyInstance(const GESTD::ReferencePtr<LineInstance>& _instance);

    /** @brief このフレームの描画キューに登録する（呼ばれた順序を保つ） */
    void RegisterDraw(LineInstance* _instance);

    void Draw(Renderer* _renderer) override;

private:
    void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi) override;
};

#endif // LineCommon_HPP_
