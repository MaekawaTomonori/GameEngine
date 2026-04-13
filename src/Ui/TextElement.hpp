#ifndef TextElement_HPP_
#define TextElement_HPP_

#include "Ui/Element.hpp"

namespace Ui {

    /** @brief テキスト表示 UI 要素
     *
     * SDF フォント描画で 1 行テキストを Canvas に配置する。
     * Canvas が内部で生成・管理する。Game は Element* を通じてのみ操作する。
     */
    class TextElement final : public Element {
    public:
        struct Data {
            std::string text;
            float       fontSize = 32.f;
        };

    private:
        Data data_ {};

    public:
        std::string_view GetType() const override { return "Text"; }
        std::unique_ptr<Element> Clone() const override;

        const std::string& GetText() const override { return data_.text; }
        void SetText(const std::string& _t) override { data_.text = _t; }
        void SetFontSize(float _sz) override          { data_.fontSize = _sz; }
        float GetFontSize() const                     { return data_.fontSize; }

        Data GetTextData() const;
        void SetTextData(const Data& _data);

    protected:
        void OnInitialize() override;
        void OnUpdate(float dt) override;
        void OnDraw() const override;
        void DebugParams() override;
    };

} // namespace Ui

#endif // TextElement_HPP_
