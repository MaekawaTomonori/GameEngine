#ifndef SpriteElement_HPP_
#define SpriteElement_HPP_
#include <array>
#include <memory>

#include "Sprite.hpp"
#include "Ui/Element.hpp"

namespace Ui {

    /** @brief スプライト表示 UI 要素
     *
     * テクスチャ・サイズ・UV クリップを持つ標準的な UI 要素。
     * Canvas が内部で生成・管理する。Game は Element* を通じてのみ操作する。
     */
    class SpriteElement final : public Element {
    public:
        struct Data {
            std::string texture        = "white_x16.png";
            Vector2     size           {64.f, 64.f};
            Vector2     textureLeftTop {};
            Vector2     textureSize    {};
        };

    private:
        Data data_ {};
        std::unique_ptr<Sprite> sprite_ = nullptr;
        bool aspectLock_ = false;
        std::array<char, 256> textureBuff_ = {};

    public:
        std::string_view GetType() const override { return "Sprite"; }
        std::unique_ptr<Element> Clone() const override;
        Vector2 GetSize() const override { return data_.size; }

        void SetTexture(const std::string& _tex) override;

        /** @brief UV クリップ領域を設定する
         * @param _leftTop テクスチャ左上座標（ピクセル）
         * @param _size    クリップサイズ（ピクセル）。{0,0} でテクスチャ全体
         */
        void SetTextureRegion(const Vector2& _leftTop, const Vector2& _size) override;

        void SetSize(const Vector2& _size) override;

        Data GetSpriteData() const;
        void SetSpriteData(const Data& _data);

    protected:
        void OnInitialize() override;
        void OnUpdate(float dt) override;
        void OnDraw() const override;
        void DebugParams() override;
    };

} // namespace Ui

#endif // SpriteElement_HPP_
