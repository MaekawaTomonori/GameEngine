#ifndef SpriteCommon_HPP_
#define SpriteCommon_HPP_
#include <vector>

#include "src/Common/Common.hpp"
#include "src/Sprite/SpriteInstance.hpp"

class SpriteCommon : public Common{
    std::vector<std::unique_ptr<SpriteInstance>> instances_;
    std::vector<SpriteInstance*> drawQueue_;

public:
	void Initialize(const GESTD::ReferencePtr<DirectXAdapter>& _adapter, const GESTD::ReferencePtr<DebugUI>& _debugUi) override;

    /** @brief スプライト実体を生成し登録する
     * @param _texture テクスチャパス
     * @return 実体への安全な参照
     */
    GESTD::ReferencePtr<SpriteInstance> CreateInstance(const std::string& _texture);

    /** @brief スプライト実体を破棄する
     * @param _instance 破棄する実体への参照
     */
    void DestroyInstance(const GESTD::ReferencePtr<SpriteInstance>& _instance);

    /** @brief このフレームの描画キューに登録する（呼ばれた順序を保つ） */
    void RegisterDraw(SpriteInstance* _instance);

    void Draw(Renderer* _renderer) override;
}; // class SpriteCommon

#endif // SpriteCommon_HPP_
