#ifndef DebugUI_HPP_
#define DebugUI_HPP_
#include <memory>
#include <mutex>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/Heap.hpp"

/// <summary>
/// ImGuiアダプタークラス
/// デバッグUIの描画とコマンド管理を提供
/// </summary>
class DebugUI {
    /// <summary>
    /// デバッグUIコマンド
    /// </summary>
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

    /// <summary>
    /// デバッグUIを初期化
    /// </summary>
    /// <param name="_adapter">DirectXアダプター</param>
    void Initialize(const DirectXAdapter* _adapter);

    /// <summary>
    /// デバッグUIをレンダリング
    /// </summary>
    void Render();

    /// <summary>
    /// デバッグコマンドを登録
    /// </summary>
    /// <param name="_id">コマンドID</param>
    /// <param name="_command">実行する関数</param>
    void RegisterCommand(const std::string &_id, std::function<void()> _command);

    /// <summary>
    /// ImGuiディスプレイサイズを更新（ウィンドウリサイズ時）
    /// </summary>
    /// <param name="width">新しい幅</param>
    /// <param name="height">新しい高さ</param>
    void UpdateDisplaySize(int width, int height);

private:
    /// <summary>
    /// 登録されたコマンドを処理
    /// </summary>
    void Process();

    /// <summary>
    /// モダンスタイルのセットアップ
    /// </summary>
    void SetupModernStyle();
}; // class DebugUI

#endif // DebugUI_HPP_
