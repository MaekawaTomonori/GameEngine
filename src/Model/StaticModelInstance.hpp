#ifndef StaticModelInstance_HPP_
#define StaticModelInstance_HPP_
#include "src/Model/ModelInstance.hpp"

/** @brief スキニングを持たないモデルの実体
 */
class StaticModelInstance : public ModelInstance {
public:
    void Initialize(const std::string& _name);

    void Update() override;
    void Draw() override;
    void Debug() override;

    /** @brief 描画本体（ModelTypeRendererから直接・非virtualに呼ばれる） */
    void ExecuteDraw() const;
}; // class StaticModelInstance

#endif // StaticModelInstance_HPP_
