#ifndef Window_HPP_
#define Window_HPP_

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <Windows.h>
#include <string>

/// <summary>
/// ウィンドウクラス
/// Windowsウィンドウの生成と管理を提供
/// </summary>
class Window {
    std::wstring title_ = L"Title";

    HWND hWnd_ = nullptr;
    HINSTANCE hInstance_ = nullptr;
    bool isBorderless_ = false;

    RECT previousRect_{};
    LONG previousStyle_ = 0;

public:
    ~Window() = default;

    /// <summary>
    /// ウィンドウを作成
    /// </summary>
    /// <returns>成功した場合true</returns>
    bool Create();

    /// <summary>
    /// ウィンドウが有効かを判定
    /// </summary>
    /// <returns>有効な場合true</returns>
    bool IsEnabled();

    /// <summary>
    /// ウィンドウハンドルを取得
    /// </summary>
    /// <returns>ウィンドウハンドル</returns>
    HWND GetWindowHandle() const;

    /// <summary>
    /// インスタンスハンドルを取得
    /// </summary>
    /// <returns>インスタンスハンドル</returns>
    HINSTANCE GetInstanceHandle() const;

    /// <summary>
    /// ウィンドウサイズを設定
    /// </summary>
    /// <param name="width">幅</param>
    /// <param name="height">高さ</param>
    void SetSize(int width, int height) const;

    /// <summary>
    /// ウィンドウタイトルを設定
    /// </summary>
    /// <param name="_title">タイトル文字列</param>
    void SetTitle(const std::string& _title);
    /// <summary>
    /// ボーダレスウィンドウモードを切り替え
    /// </summary>
    void ToggleBorderless();

    /// <summary>
    /// ボーダレスモードかどうか
    /// </summary>
    /// <returns>ボーダレスモードの場合true</returns>
    bool IsBorderless() const { return isBorderless_; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// ボーダレスフルスクリーン化
    /// </summary>
    void BorderlessFullScreen();

    /// <summary>
    /// 通常ウィンドウモードに戻す
    /// </summary>
    void RestoreWindowMode() const;

}; // class Window

#endif // Window_HPP_
