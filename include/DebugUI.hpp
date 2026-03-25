#ifndef DebugUI_HPP_
#define DebugUI_HPP_
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "src/DirectX/DirectXAdapter.hpp"
#include "src/DirectX/Heap/Heap.hpp"

/** @brief ImGuiアダプタークラス
 * デバッグUIの描画とコマンド管理を提供
 */
class DebugUI {
    /** @brief デバッグUIコマンド
     */
    struct Command {
        std::string id;
        std::function<void()> command;
    };

    /** @brief ウィンドウの状態管理構造体
     */
    struct WindowState {
        bool visible = false;
        std::string group = "";

        constexpr WindowState& operator=(const bool _visible) {
            visible = _visible;
            return *this;
        }

        constexpr WindowState& operator=(const WindowState& _other) {
            visible = _other.visible;
            group = _other.group;
            return *this;
        }
    };

    std::mutex mutex_;

    std::unique_ptr<Heap> heap_;
    const DirectXAdapter* adapter_ = nullptr;

    std::vector<Command> commands_;
    std::unordered_map<std::string, WindowState> windowStates_;
    bool showMenuBar_ = false;
    bool showWindowsPanel_ = false;
    bool panelJustOpened_ = false;

public:
    ~DebugUI();

    /** @brief デバッグUIを初期化
     * @param _adapter DirectXアダプター
     */
    void Initialize(const DirectXAdapter* _adapter);

    /** @brief デバッグUIをレンダリング
     */
    void Render();

    /** @brief デバッグコマンドを登録
     * @param _id コマンドID
     * @param _command 実行する関数
     */
    void RegisterCommand(const std::string &_id, std::function<void()> _command);

    /** @brief メニューバーにウィンドウトグル項目を登録
     * @param _key 識別キー兼表示名 @param _flag
     * @param _group 同一グループの項目は Windows パネルでまとめて表示
     */
    void RegisterMenuButton(const std::string& _key, bool _flag = false, const std::string& _group = "");

    bool& IsVisible(const std::string& _key);

    /** @brief ImGuiディスプレイサイズを更新（ウィンドウリサイズ時）
     * @param _width 新しい幅
     * @param _height 新しい高さ
     */
    void UpdateDisplaySize(int _width, int _height) const;

    /** @brief マウスがデバッグUI上にあるか判定
     * Scene ウィンドウ以外の ImGui ウィンドウにホバー中の場合 true
     * カーソル非表示時にデバッグUIパネル上ではカーソルを表示するために使用
     */
    bool IsMouseOverDebugUI() const;

    /** @brief ImGui表示用テクスチャを登録し ImTextureID として使用可能なGPUハンドルを返す
     * ImGuiヒープのスロット1にSRVを作成する（スロット0はフォント用）
     * @param _resource テクスチャリソース
     * @param _format テクスチャフォーマット
     * @return GPU ディスクリプタハンドルの ptr 値（ImTextureID としてキャスト可）。失敗時は0
     */
    uint64_t RegisterTexture(ID3D12Resource* _resource, DXGI_FORMAT _format);

private:
    /** @brief 登録されたコマンドを処理
     */
    void Process();

    /** @brief メインメニューバーを描画
     */
    void RenderMainMenuBar();

    /** @brief Windowsパネルを描画
     */
    void MenuBar();

    /** @brief モダンスタイルのセットアップ
     */
    void SetupStyle();
}; // class DebugUI

#endif // DebugUI_HPP_
