#ifndef SpriteCommon_HPP_
#define SpriteCommon_HPP_
#include <memory>
#include <mutex>

#include "DebugUI.hpp"
#include "src/DirectX/GraphicsPipeline/GraphicsPipeline.hpp"

class SpriteCommon {
    DirectXAdapter* adapter_ = nullptr;
    DebugUI* debugUI_ = nullptr;

	std::mutex mutex_; 

    std::unique_ptr<GraphicsPipeline> pipeline_;

public:
	void Initialize(DirectXAdapter* _adapter, DebugUI* _debugUi);
    void Draw();

    void RegisterCommand(const std::string& _id, const std::function<void()> &_command);

    DirectXAdapter* GetAdapter() const {
        return adapter_;
    }
private:
}; // class SpriteCommon

#endif // SpriteCommon_HPP_
