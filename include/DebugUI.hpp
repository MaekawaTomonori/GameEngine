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
    ID3D12GraphicsCommandList* cList_ = nullptr;

    std::vector<Command> commands_;

public:
    ~DebugUI();

    /// <summary>
    /// デバッグUIを初期化
    /// </summary>
    /// <param name="dx">DirectXアダプター</param>
    void Initialize(const DirectXAdapter* dx);

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
