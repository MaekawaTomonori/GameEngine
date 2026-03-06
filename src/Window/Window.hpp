#ifndef Window_HPP_
#define Window_HPP_

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <Windows.h>
#include <string>

/** @brief ウィンドウクラス
 * Windowsウィンドウの生成と管理を提供
 */
class Window {
    std::wstring title_ = L"Title";

    HWND hWnd_ = nullptr;
    HINSTANCE hInstance_ = nullptr;
    bool isBorderless_ = false;

    RECT previousRect_{};
    RECT previousClient_{};
    LONG previousStyle_ = 0;

public:
    ~Window() = default;

    /** @brief ウィンドウを作成
     * @return 成功した場合true
     */
    bool Create();

    /** @brief ウィンドウが有効かを判定
     * @return 有効な場合true
     */
    bool IsEnabled();

    /** @brief ウィンドウハンドルを取得
     * @return ウィンドウハンドル
     */
    HWND GetWindowHandle() const;

    /** @brief インスタンスハンドルを取得
     * @return インスタンスハンドル
     */
    HINSTANCE GetInstanceHandle() const;

    /** @brief ウィンドウサイズを設定
     * @param width 幅
     * @param height 高さ
     */
    void SetSize(int _width, int _height) const;

    /** @brief ウィンドウタイトルを設定
     * @param _title タイトル文字列
     */
    void SetTitle(const std::string& _title);
    /** @brief ボーダレスウィンドウモードを切り替え
     */
    void ToggleBorderless();

    /** @brief ボーダレスモードかどうか
     * @return ボーダレスモードの場合true
     */
    bool IsBorderless() const { return isBorderless_; }

private:
    static LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);

    /** @brief ボーダレスフルスクリーン化
     */
    void BorderlessFullScreen();

    /** @brief 通常ウィンドウモードに戻す
     */
    void RestoreWindowMode() const;

}; // class Window

#endif // Window_HPP_
