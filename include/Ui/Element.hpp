#ifndef UiElement_HPP_
#define UiElement_HPP_
#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Math/Vector2.hpp"
#include "Math/Vector4.hpp"
#include "Ui/EventSystem.hpp"
#include "Ui/UiAnimation.hpp"

namespace Ui {

    /** @brief UI 要素の抽象基底クラス
     *
     * SpriteElement / TextElement など全 UI 要素の共通インタフェース。
     * Game は Canvas::FindElementByName() で取得した Element* を通じて操作する。
     */
    class Element {
        std::string uuid_;
        std::string name_;
        bool        visible_    = true;
        bool        isOpen_     = true;
        bool        changeName_ = false;
        std::array<char, 256> nameBuff_ = {};

        std::string showAnimKey_;
        std::string idleAnimKey_;
        std::string hideAnimKey_;

        std::unordered_map<EventKey, std::string> events_;

    protected:
        Vector2 position_ {};
        Vector4 color_    {1.f, 1.f, 1.f, 1.f};
        Vector2 parent_    {};
        Vector2 posOffset_ {};
        Vector4 animColor_ {1.f, 1.f, 1.f, 1.f};

        AnimSlot showSlot_;
        AnimSlot idleSlot_;
        AnimSlot hideSlot_;

        virtual void OnInitialize() = 0;
        virtual void OnUpdate(float _dt) = 0;
        virtual void OnDraw() const = 0;
        virtual void DebugParams() = 0;

        /** @brief Clone 実装用：基底の共通フィールドを dst へコピーする */
        void CopyCommonTo(Element& _dst) const;

    public:
        virtual ~Element() = default;

        virtual std::string_view GetType() const = 0;
        virtual std::unique_ptr<Element> Clone() const = 0;

        void Initialize();
        void Update(float dt = 1.f / 60.f);
        void Draw() const;
        void Debug(const std::vector<std::string>& _availableActions = {},
                   const std::vector<std::string>& _availableAnimKeys = {});

        bool           IsVisible()   const;
        std::string    GetName()     const;
        std::string    GetUUID()     const;
        Vector2        GetPosition() const;
        const Vector4& GetColor()    const;
        virtual Vector2 GetSize()    const { return {}; }

        void SetVisible(bool _visible);
        void SetName(const std::string& _name);
        void SetPosition(const Vector2& _pos);
        void SetColor(const Vector4& _color);
        void SetParent(const Vector2& _pos);

        /** @brief スクリーン上のサイズを設定する（SpriteElement で有効）*/
        virtual void SetSize(const Vector2&) {}

        virtual void SetTexture(const std::string&) {}

        /** @brief UV クリップ領域を設定する（SpriteElement で有効）
         * param テクスチャ左上座標（ピクセル）
         * param クリップサイズ（ピクセル）。{0,0} でテクスチャ全体
         */
        virtual void SetTextureRegion(const Vector2& /*_leftTop*/, const Vector2& /*_size*/) {}

        virtual void SetText(const std::string&) {}
        virtual void SetFontSize(float) {}

        /** @brief 表示文字列を返す（TextElement 以外では空文字列）*/
        virtual const std::string& GetText() const {
            static const std::string empty;
            return empty;
        }

        void PlayShow();
        void PlayIdle();
        void PlayHide();
        void StopAnim();
        bool IsShowDone() const;
        bool IsHideDone() const;

        bool& IsOpen();
        const std::string& GetShowAnimKey() const;
        const std::string& GetIdleAnimKey() const;
        const std::string& GetHideAnimKey() const;
        void SetShowAnimKey(const std::string& _k) { showAnimKey_ = _k; }
        void SetIdleAnimKey(const std::string& _k) { idleAnimKey_ = _k; }
        void SetHideAnimKey(const std::string& _k) { hideAnimKey_ = _k; }

        void SetShowFunc(const AnimFunc& _func);
        void SetIdleFunc(const AnimFunc& _func);
        void SetHideFunc(const AnimFunc& _func);

        void SetEvent(EventKey _event, const std::string& _actionKey);
        const std::string& GetActionKey(EventKey _event) const;
        bool HasEvent(EventKey _event) const;
        bool HasAnyEvent() const;
        const std::unordered_map<EventKey, std::string>& GetEvents() const;
    };

} // namespace Ui

#endif // UiElement_HPP_
