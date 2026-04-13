#ifndef UserInterface_HPP_
#define UserInterface_HPP_
#include <array>
#include <functional>
#include <unordered_map>

#include "ReferencePtr.hpp"
#include "Ui/Element.hpp"
#include "Ui/EventSystem.hpp"
#include "Ui/UiAnimation.hpp"

namespace Ui {

    class Canvas {
        static constexpr std::string_view JSON_PATH = "Assets/Data/UI/";

        friend class Manager;
    public:
        enum class Phase { Inactive, Showing, Idle, Hiding };
        struct ElementRect { Vector2 position; Vector2 size; };

    private:
        std::string name_;
        Vector2 position_ {};
        EventSystem eventSystem_;
        std::vector<std::unique_ptr<Element>> elements_;
        GESTD::LifetimeSentinel lifetime_;
        std::unordered_map<std::string, AnimFunc> animFuncs_;
        Phase phase_ = Phase::Idle;
        bool dirty_ = false;
        bool defaultOpen_ = true;
        bool reload_ = false;
        bool changingName_ = false;
        std::function<void()> customDebug_;
        std::array<char, 256> buffName_{};

        void HeaderUiParams();
        void ElementCommonParams();
        void ElementsParam();
        void Load(const std::string& _name);
        void ResolveAnimFuncs();

        void Update(float dt = 1.f / 60.f);
        void Draw() const;
        void Save();
        void Reload();
    public:
        void Setup(const std::string& _name);
        void Editor(bool* _open = nullptr);

        /** 登録済みの Canvas を名前で取得する。未登録なら無効な ReferencePtr を返す */
        static GESTD::ReferencePtr<Canvas> Find(const std::string& _name);

        void RegisterAction(const std::string& _actionKey, const std::function<void()>& _action);
        void RegisterAnimFunc(const std::string& _key, AnimFunc _func);

        void SetActive(bool _active);
        bool IsActive() const;
        bool IsDirty() const;
        Phase GetPhase() const;

        Element* FindElementByName(const std::string& _name) const;
        Element* GetElement(size_t _index) const;
        size_t GetElementCount() const;

        std::vector<size_t> GetIndicesWithEvent(EventKey _event) const;
        void ExecuteAction(const std::string& _actionKey);
        void ExecuteActionAt(size_t _elementIndex, EventKey _event);
        ElementRect GetElementRect(size_t _elementIndex) const;

        /** @brief 指定名の要素にアクションキーを紐づける
         * @param _name      要素名
         * @param _event     対象イベント
         * @param _actionKey 空文字で解除
         */
        void SetElementEvent(const std::string& _name, EventKey _event, const std::string& _actionKey);

        /** @brief Editor ウィンドウ末尾に追加描画するデバッグ関数を登録する */
        void SetCustomDebug(std::function<void()> _func);

        std::vector<std::string> GetActionKeys() const;
        std::vector<std::string> GetAnimFuncKeys() const;

    }; // class Canvas
} // namespace Ui
#endif // UserInterface_HPP_
