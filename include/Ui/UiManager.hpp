#ifndef UiManager_HPP_
#define UiManager_HPP_
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "ReferencePtr.hpp"
#include "DebugUI.hpp"
#include "Input.hpp"
#include "Ui/UserInterface.hpp"
#include "src/Renderer/Renderer.hpp"


namespace Ui {

    class Manager {
    public:
        /** @brief Registry の1エントリ */
        struct Entry {
            std::string name;
            GESTD::ReferencePtr<Canvas> canvas;
            /** @brief canvas が有効（ロード済み）かどうか */
            bool IsLoaded() const { return static_cast<bool>(canvas); }
        };

    private:
        GESTD::ReferencePtr<DebugUI> debugUI_;
        GESTD::ReferencePtr<Input>   input_;

        std::vector<Entry> entries_;
        std::vector<std::unique_ptr<Canvas>> owned_;
        int selectedIndex_ = -1;
        std::array<char, 256> newNameBuff_ = {};

    public:
        void Setup(const GESTD::ReferencePtr<DebugUI>& _debugUi,
                   const GESTD::ReferencePtr<Input>&   _input);

        /** @brief フォーカスを持つ Canvas を切り替える（Canvas 内部から呼ばれる） */
        void SetFocusedCanvas(const Canvas* _canvas) const;

        void Update();
        void Draw() const;

        /** @brief Assets/Data/UI/ の JSON ファイルを走査してリストに追加する */
        void ScanJsonFiles();

        /** @brief 同名エントリがあれば canvas を更新、なければ追加する */
        void Register(const std::string& _name, const GESTD::ReferencePtr<Canvas>& _canvas);

        /** @brief 名前で Canvas を取得する。未登録なら無効な ReferencePtr を返す */
        GESTD::ReferencePtr<Canvas> GetCanvas(const std::string& _name) const;

        

        void Debug();

    private:
        void DrawListWindow();
        void DrawDetailWindow();
        void CreateCanvas(const std::string& _name);
        void LoadCanvas(int _index);
        void UnloadCanvas(int _index);
        bool IsOwned(int _index) const;
    };

} // namespace Ui

#endif // UiManager_HPP_
