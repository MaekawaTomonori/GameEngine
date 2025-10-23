#ifndef Common_HPP_
#define Common_HPP_
#include <mutex>

#include "DebugUI.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/Renderer/Renderer.hpp"

class Common {
protected:
    struct RenderingCommand {
        std::function<void()> func;
        bool applyPostEffects;
        // Add PostEffect Mask later
    };


    DirectXAdapter* adapter_ = nullptr;
    DebugUI* debugUI_ = nullptr;

    std::mutex mutex_;

    std::unique_ptr<PipelineStateObject> pipeline_;

    std::vector<RenderingCommand> drawFunctions_;

public:
    virtual ~Common() = default;
    virtual void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi) = 0;
    void Draw(Renderer* _renderer);

    void RegisterDebug(const std::string& _id, const std::function<void()>& _command);
    void RegisterDraw(const std::function<void()>& _command, bool _isApplyPostEffect = true);

    DirectXAdapter* GetAdapter() const {
        return adapter_;
    }
protected:
    void Setup(DirectXAdapter* _adapter, DebugUI* _debugUi);
}; // class Common

#endif // Common_HPP_
