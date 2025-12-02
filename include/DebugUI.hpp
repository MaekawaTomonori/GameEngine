#ifndef DebugUI_HPP_
#define DebugUI_HPP_
#include <memory>
#include <mutex>

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

    /** @brief ImGuiディスプレイサイズを更新（ウィンドウリサイズ時）
     ** @param width 新しい幅
     ** @param height 新しい高さ
     **/
    void UpdateDisplaySize(int width, int height);

private:
    /** @brief 登録されたコマンドを処理
     **/
    void Process();

    /** @brief モダンスタイルのセットアップ
     **/
    void SetupModernStyle();
}; // class DebugUI

#endif // DebugUI_HPP_
