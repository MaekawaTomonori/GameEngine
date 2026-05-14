#ifndef UserInterface_HPP_
#define UserInterface_HPP_
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ReferencePtr.hpp"
#include "Ui/Element.hpp"
#include "Ui/EventSystem.hpp"
#include "Ui/UiAnimation.hpp"
#include "Ui/Module/IModule.hpp"

class Input;

namespace Ui {

    class Canvas {
        static constexpr std::string_view JSON_PATH = "Assets/Data/UI/";

        friend class Manager;
    public:
        enum class Phase     { Inactive, Showing, Idle, Hiding };
        enum class CursorDir { Up, Down, Left, Right };
        struct ElementRect   { Vector2 position; Vector2 size; };

    private:
        // --- 入力・操作コンテキスト（モジュール未使用時は null） ---
        struct InputContext {
            struct NavNeighbors { int up = -1, down = -1, left = -1, right = -1; };

            std::vector<std::unique_ptr<IModule>> modules;
            std::string defaultFocusName;
            int  cursorIndex = -1;
            bool hasFocus    = false;

            std::vector<int>          navSelectables;
            std::vector<NavNeighbors> navGraph;
            bool navDirty = true;
        };

        std::string name_;
        Vector2 position_ {};
        EventSystem eventSystem_;
        std::vector<std::unique_ptr<Element>> elements_;
        GESTD::LifetimeSentinel lifetime_;
        std::unordered_map<std::string, AnimFunc> animFuncs_;
        Phase phase_       = Phase::Idle;
        bool  dirty_       = false;
        bool  defaultOpen_ = true;
        bool  reload_      = false;
        bool  changingName_= false;
        std::function<void()> customDebug_;
        std::array<char, 256> buffName_{};

        std::unique_ptr<InputContext> input_;

        // Editor helpers
        void HeaderUiParams();
        void ElementCommonParams();
        void ElementsParam();
        void ModulesEditor();

        // Load / Save / internal lifecycle
        void Load(const std::string& _name);
        void Save();
        void Reload();
        void ResolveAnimFuncs();
        void LoadModulesFromJson(const nlohmann::json& _root);
        void SaveModulesToJson(nlohmann::json& _root) const;

        // Navigation graph
        void    RebuildNavGraph();
        int     FindNavNeighbor(int _fromElemIdx, float _dx, float _dy) const;
        Vector2 GetElementCenter(int _elemIdx) const;

        // Cursor internal
        void InitCursor();
        int  ResolveDefaultFocus() const;

        // Framework/Manager lifecycle (private)
        void Update(float dt = 1.f / 60.f);
        void Draw() const;
        void ProcessInput(const Input& _input);

    public:
        void Setup(const std::string& _name);
        void Editor(bool* _open = nullptr);

        /** 登録済みの Canvas を名前で取得する。未登録なら無効な ReferencePtr を返す */
        static GESTD::ReferencePtr<Canvas> Find(const std::string& _name);

        void RegisterAction(const std::string& _actionKey, const std::function<void()>& _action);
        void RegisterAnimFunc(const std::string& _key, AnimFunc _func);

        void  SetActive(bool _active);
        bool  IsActive() const;
        bool  IsDirty()  const;
        Phase GetPhase()  const;

        // Module management
        void     AddModule(std::unique_ptr<IModule> _module);
        IModule* FindModule(std::string_view _type) const;

        // Focus
        void SetFocused(bool _focused);
        bool HasFocus() const;

        // Cursor (called by modules)
        void    MoveCursorTo(int _elementIdx);
        void    MoveCursorDir(CursorDir _dir);
        void    ExecuteCursor();
        int     GetCursorIndex() const;
        Vector2 GetPosition()    const { return position_; }

        // DefaultFocus
        void               SetDefaultFocusName(const std::string& _name);
        const std::string& GetDefaultFocusName() const;

        Element* FindElementByName(const std::string& _name) const;
        Element* GetElement(size_t _index) const;
        size_t   GetElementCount() const;

        std::vector<size_t> GetIndicesWithEvent(EventKey _event) const;
        void ExecuteAction(const std::string& _actionKey);
        void ExecuteActionAt(size_t _elementIndex, EventKey _event);
        ElementRect GetElementRect(size_t _elementIndex) const;

        void SetElementEvent(const std::string& _name, EventKey _event, const std::string& _actionKey);
        void SetCustomDebug(std::function<void()> _func);

        std::vector<std::string> GetActionKeys()   const;
        std::vector<std::string> GetAnimFuncKeys() const;

    }; // class Canvas
} // namespace Ui
#endif // UserInterface_HPP_
