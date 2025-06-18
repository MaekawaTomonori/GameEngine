#ifndef SpriteCommon_HPP_
#define SpriteCommon_HPP_
#include <memory>

#include "src/DirectX/GraphicsPipeline/GraphicsPipeline.hpp"


class SpriteCommon {
    DirectXAdapter* adapter_ = nullptr;

    std::unique_ptr<GraphicsPipeline> pipeline_;
public:
	void Initialize(DirectXAdapter* _adapter);
    void Draw() const;

    DirectXAdapter* GetAdapter() const {
        return adapter_;
    }
private:
}; // class SpriteCommon

#endif // SpriteCommon_HPP_
