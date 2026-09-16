#ifndef SampleScene_HPP_
#define SampleScene_HPP_
#include <memory>
#include <optional>

#include "IScene.hpp"
#include "Model.hpp"
#include "Sprite.hpp"
#include "Handle.hpp"
#include "Math/Vector3.hpp"

class SampleScene final : public IScene{
    std::unique_ptr<Model> model_;
    std::unique_ptr<Model> plane;

    /** @brief マウスカーソルを追従するデバッグ用スプライト（フルスクリーン時の座標変換確認用） */
    std::unique_ptr<Sprite> cursorSprite_;

    Vector3 cubePosition_ = {0.f, .5f, 0.f};

    std::optional<Audio::Handle> kick_;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;
private:
    /** @brief マウスがCubeのスクリーン投影位置に重なっているかを判定し、色を変える（デバッグ用） */
    void UpdateCubeHover();

}; // class SampleScene

#endif // SampleScene_HPP_
