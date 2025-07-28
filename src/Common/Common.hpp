#ifndef Common_HPP_
#define Common_HPP_
#include <mutex>

#include "DebugUI.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/GraphicsPipeline/GraphicsPipeline.hpp"

class Common {
protected:
    DirectXAdapter* adapter_ = nullptr;
    DebugUI* debugUI_ = nullptr;

    std::mutex mutex_;

    std::unique_ptr<GraphicsPipeline> pipeline_;

public:
    virtual ~Common() = default;
    virtual void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) = 0;
    void Draw();

    void RegisterCommand(const std::string& _id, const std::function<void()>& _command);

    DirectXAdapter* GetAdapter() const {
        return adapter_;
    }
protected:
    void Setup(DirectXAdapter* _adapter, DebugUI* _debugUi,GraphicsPipeline::Type _type);
    void Setup(DirectXAdapter* _adapter, DebugUI* _debugUi);
}; // class Common

#endif // Common_HPP_
