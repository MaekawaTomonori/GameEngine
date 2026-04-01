#ifndef Common_HPP_
#define Common_HPP_
#include <mutex>

#include "DebugUI.hpp"
#include "ReferencePtr.hpp"
#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/GraphicsPipeline/Object/PipelineStateObject.hpp"
#include "src/Renderer/Renderer.hpp"

class Common {
protected:
    struct RenderingCommand {
        std::function<void()> func;
        bool applyPostEffects;
        /** Add PostEffect Mask later
         */
    };

    GESTD::ReferencePtr<DirectXAdapter> adapter_;
    GESTD::ReferencePtr<DebugUI> debugUI_;
    std::string windowName_;

    std::mutex mutex_;

    std::unique_ptr<PipelineStateObject> pipeline_;

    std::unordered_map<std::string, std::function<void()>> debugCommands_;
    std::unordered_map<std::string, std::function<void()>> updateCommands_;
    std::vector<RenderingCommand> drawFunctions_;


public:
    virtual ~Common() = default;
    virtual void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi) = 0;
    virtual void Update();
    virtual void Debug();
    virtual void Draw(Renderer* _renderer);

    void RegisterDebug(const std::string& _id, const std::function<void()>& _func);
    void RegisterUpdate(const std::string& _id, const std::function<void()>& _func);
    void RegisterDraw(const std::function<void()>& _command, bool _isApplyPostEffect = true);

    void Unregister(const std::string& _uuid);

    GESTD::ReferencePtr<DirectXAdapter> GetAdapter() const {
        return adapter_;
    }
protected:
    void Setup(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi, const std::string& _windowName);
}; // class Common

#endif // Common_HPP_
