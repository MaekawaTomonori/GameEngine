#ifndef ParticleTestScene_HPP_
#define ParticleTestScene_HPP_
#include <memory>

#include "IScene.hpp"
#include "Model.hpp"

/** @brief パーティクルシステム単体の動作確認用シーン
 * SceneSwitcherデバッグパネルから切り替えて使う。
 */
class ParticleTestScene final : public IScene {
    std::unique_ptr<Model> plane_;
    float timer_ = 0.f;

public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;
}; // class ParticleTestScene

#endif // ParticleTestScene_HPP_
