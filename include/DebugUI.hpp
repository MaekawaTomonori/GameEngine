#ifndef DebugUI_HPP_
#define DebugUI_HPP_
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/Heap.hpp"

/** @brief ImGuiアダプタークラス
 ** デバッグUIの描画とコマンド管理を提供
 **/
class DebugUI {
    /** @brief デバッグUIコマンド
     **/
    struct Command {
        std::string id;
        std::function<void()> command;
    };

    std::mutex mutex_;

    std::unique_ptr<Heap> heap_;
    const DirectXAdapter* adapter_ = nullptr;

    std::vector<Command> commands_;
    std::unordered_map<std::string, bool> windowStates_;
    bool showMenuBar_ = false;
    bool showWindowsPanel_ = false;
    bool panelJustOpened_ = false;

public:
    ~DebugUI();

    /** @brief デバッグUIを初期化
     ** @param _adapter DirectXアダプター
     **/
    void Initialize(const DirectXAdapter* _adapter);

    /** @brief デバッグUIをレンダリング
     **/
    void Render();

    /** @brief デバッグコマンドを登録
     ** @param _id コマンドID
     ** @param _command 実行する関数
     **/
    void RegisterCommand(const std::string &_id, std::function<void()> _command);

    /** @brief メニューバーにウィンドウトグル項目を登録
     ** @param _key 識別キー兼表示名 @param _flag
     **/
    void RegisterMenuButton(const std::string& _key, bool _flag = false);

    void ToggleMenu(const std::string& _key);
    bool& IsVisible(const std::string& _key);

    /** @brief ImGuiディスプレイサイズを更新（ウィンドウリサイズ時）
     ** @param _width 新しい幅
     ** @param _height 新しい高さ
     **/
    void UpdateDisplaySize(int _width, int _height) const;

private:
    /** @brief 登録されたコマンドを処理
     **/
    void Process();

    /** @brief メインメニューバーを描画
     **/
    void RenderMainMenuBar();

    /** @brief モダンスタイルのセットアップ
     **/
    void SetupStyle();
}; // class DebugUI

#endif // DebugUI_HPP_
