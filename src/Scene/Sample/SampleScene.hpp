#ifndef SampleScene_HPP_
#define SampleScene_HPP_
#include <array>
#include <memory>
#include <optional>

#include "GEUI.hpp"
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

    // GEUI のハンドル確認用
    GEUI::Canvas pause_;
    GEUI::Canvas hud_;
    GEUI::Widget resume_;
    std::array<GEUI::Widget, 3> buttons_;
    std::array<bool, 3> hovered_{};
    bool quitEnabled_ = true;

    // KillCounter と同じ使い方の確認用
    GEUI::Canvas counter_;
    GEUI::Widget counterRoot_;
    std::array<GEUI::Widget, 3> digits_;
    uint32_t kills_ = 0;
    float introTimer_ = 0.f;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;
    void Debug() override;
private:
    /** @brief マウスがCubeのスクリーン投影位置に重なっているかを判定し、色を変える（デバッグ用） */
    void UpdateCubeHover();

    /** @brief 上下キーで選択を動かし、Enter で押す（Navigate / Submit の確認用） */
    void UpdateButtonSelection();

    /** @brief カーソルが乗った・外れた瞬間を Log に出す（IsHovered の確認用） */
    void UpdateButtonHover();

    /** @brief K で数を増やし、T で登場の演出をやり直す（切り出し・位置・透明度の確認用） */
    void UpdateCounter();

}; // class SampleScene

#endif // SampleScene_HPP_
