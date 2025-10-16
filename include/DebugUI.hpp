#ifndef DebugUI_HPP_
#define DebugUI_HPP_
#include <memory>
#include <mutex>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/Heap.hpp"

struct ImDrawData;

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
    void Initialize(const DirectXAdapter* dx);
    void Render();

    void RegisterCommand(const std::string &_id, std::function<void()> _command);

private:
    void Process();
    void SetupModernStyle();
}; // class DebugUI

#endif // DebugUI_HPP_
