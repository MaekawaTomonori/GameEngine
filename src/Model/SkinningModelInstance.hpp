#ifndef SkinningModelInstance_HPP_
#define SkinningModelInstance_HPP_
#include "src/Model/ModelInstance.hpp"
#include "src/Model/Skinning/SkinningState.hpp"

/** @brief スキニング（骨格アニメーション）を持つモデルの実体
 */
class SkinningModelInstance : public ModelInstance {
    std::unique_ptr<SkinningState> skinning_;

public:
    void Initialize(const std::string& _name);

    void Update() override;
    void Draw() override;
    void Debug() override;

    /** @brief 描画本体（ModelTypeRendererから直接・非virtualに呼ばれる） */
    void ExecuteDraw() const;
}; // class SkinningModelInstance

#endif // SkinningModelInstance_HPP_
