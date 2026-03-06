#ifndef UserInterface_HPP_
#define UserInterface_HPP_
#include <array>

#include "Ui/Element.hpp"
#include "Ui/EventSystem.hpp"

namespace Ui {
    class Canvas {
        static constexpr std::string_view JSON_PATH = "Assets/Data/UI/";

        std::string name_;

        Vector2 position_ {};

        EventSystem eventSystem_;

        std::vector<std::unique_ptr<Element>> elements_;

        bool dirty_ = false;

        bool active_ = true;
        bool editing_ = false;
        bool defaultOpen_ = true;
        bool reload_ = false;

        std::array<char, 256> buffName_{};

    public:
        void Setup(const std::string& _name);
        void Update();
        void Draw() const;

        void StandaloneDebug();
        void Debug();

        void RegisterAction(const std::string& _actionKey, const std::function<void()>& _action);

        void SetActive(bool _active);
        bool IsActive() const;
        bool IsDirty() const;

        std::vector<size_t> GetIndicesWithEvent(EventKey _event) const;
        void ExecuteAction(const std::string& _actionKey);
        void ExecuteActionAt(size_t _elementIndex, EventKey _event);

        struct ElementRect { Vector2 position; Vector2 size; };
        ElementRect GetElementRect(size_t _elementIndex) const;

        std::vector<std::string> GetActionKeys() const;

        Element* FindElementByName(const std::string& _name);
        Element* GetElement(size_t _index) const;
        size_t GetElementCount() const;

    private:
        void Editor();

        void HeaderUiParams();
        void ElementCommonParams();
        void ElementsParam();

        void Load(const std::string& _name);
        void Save();

    }; // class Canvas
} // namespace Ui
#endif // UserInterface_HPP_
