#ifndef SpriteCommon_HPP_
#define SpriteCommon_HPP_
#include <memory>

#include "src/DirectX/GraphicsPipeline/GraphicsPipeline.hpp"


class SpriteCommon {
    DirectXAdapter* adapter_ = nullptr;
    std::mutex mutex_; 

    std::unique_ptr<GraphicsPipeline> pipeline_;
public:
	void Initialize(DirectXAdapter* _adapter);
    void Draw();

    DirectXAdapter* GetAdapter() const {
        return adapter_;
    }
private:
}; // class SpriteCommon

#endif // SpriteCommon_HPP_
