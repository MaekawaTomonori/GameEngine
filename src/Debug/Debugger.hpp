#ifndef Debugger_HPP_
#define Debugger_HPP_

#ifdef _DEBUG

#include <functional>
#include <memory>
#include <string>

#include "WatchDebugger.hpp"
#include "WeakPtr.hpp"

class DirectXAdapter;
class DebugUI;
class FrameDebugger;
class JsonParams;

/** @brief デバッグシステムの統合管理クラス（Debugビルド専用）
 * DebugUI / FrameDebugger / PerformanceProfiler / WatchDebugger を一括管理する
 *
 *   // グループなし
 *   Debugger::Watch("FPS", &fps_);
 *
 *   // グループ（メソッドチェイン）
 *   Debugger::WatchGroup("Player")
 *       .Watch("HP",    &hp_)               // const* → 読み取り専用
 *       .Watch("Speed", &speed_)             // T*     → 編集可能
 *       .Watch("Speed", &speed_, "player");  // 終了時に Assets/Data/player.json へ保存
 */
class Debugger {
    std::unique_ptr<DebugUI>       ui_;
    std::unique_ptr<FrameDebugger> frame_;

public:
    Debugger();
    ~Debugger();

    /** @brief 初期化
     * @param _adapter DirectXアダプター
     */
    void Initialize(GESTD::WeakPtr<DirectXAdapter> _adapter);

    /*@brief 各デバッガーの ImGui ウィンドウを登録
     */
    void Debug();

    /*@brief このフレームで Scene の Update を実行すべきか 
     */
    bool ShouldUpdate();

    /** @brief ImGui 描画 
     */
    void Render();

    /** @brief ウィンドウリサイズ時の ImGui ディスプレイサイズ更新 
     */
    void UpdateDisplaySize(int _width, int _height);

    /** @brief DebugUI ポインタを返す
     */
    GESTD::WeakPtr<DebugUI> GetUI() const { return GESTD::WeakPtr<DebugUI>(ui_); }

    /** @brief FrameDebugger ポインタを返す
     */
    GESTD::WeakPtr<FrameDebugger> GetFrame() const { return GESTD::WeakPtr<FrameDebugger>(frame_); }

    /** @brief 読み取り専用ウォッチ（const ポインタ）
     */
    template<typename T>
    static void Watch(const std::string& _label, const T* _ptr) {
        WatchDebugger::Watch(_label, _ptr);
    }

    /** @brief 編集可能ウォッチ
     * @param _label ラベル
     * @param _ptr   ポインタ
     * @param _jsonFile 保存先ファイル
     * @param _jsonKey JSON キー
     */
    template<typename T>
    static void Watch(const std::string& _label, T* _ptr, const std::string& _jsonFile = "", const std::string& _jsonKey  = "") {
        WatchDebugger::WatchEdit(_label, _ptr, _jsonFile, _jsonKey);
    }


    /** @brief グループ化 Watch のメソッドチェイン RAII ラッパー
     * スコープを抜けると自動的にグループを閉じる
     */
    class WatchGroup {
    public:
        /** @brief グループ開始
         */
        explicit WatchGroup(const std::string& _label) {
            WatchDebugger::BeginGroup(_label);
        }

        /** @brief グループ開始（JSON 書き込みあり）
         */
        WatchGroup(const std::string& _label, JsonParams* _json, const std::string& _name, const std::string& _group) {
            WatchDebugger::BeginGroup(_label, _json, _name, _group);
        }

        ~WatchGroup() { WatchDebugger::EndGroup(); }

        WatchGroup(const WatchGroup&)            = delete;
        WatchGroup& operator=(const WatchGroup&) = delete;

        /** @brief 読み取り専用ウォッチ（const ポインタ）*/
        template<typename T>
        WatchGroup& Watch(const std::string& _label, const T* _ptr) {
            WatchDebugger::Watch(_label, _ptr);
            return *this;
        }

        /** @brief 編集可能ウォッチ（非 const ポインタ）*/
        template<typename T>
        WatchGroup& Watch(const std::string& _label, T* _ptr,
                          const std::string& _jsonFile = "",
                          const std::string& _jsonKey  = "") {
            WatchDebugger::WatchEdit(_label, _ptr, _jsonFile, _jsonKey);
            return *this;
        }
    };

}; // class Debugger

#endif // _DEBUG
#endif // Debugger_HPP_
