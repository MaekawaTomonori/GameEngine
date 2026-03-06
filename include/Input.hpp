#ifndef Input_HPP_
#define Input_HPP_
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl/client.h>

#include "Math/Vector2.hpp"

/** @brief 入力処理クラス
 * キーボード、マウス、ジョイスティックの入力を管理
 */
class Input {
    HINSTANCE hInstance_{};
    HWND hWnd_{};

    Microsoft::WRL::ComPtr<IDirectInput8> directInput_;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> joystick_;
    BYTE keyState_[256]{};
    BYTE preState_[256]{};
    DIMOUSESTATE mouseState_{};
    DIMOUSESTATE preMouseState_{};

    Vector2 mousePosition_{};

    bool cursorVisible_ = true;
    bool appliedCursorVisible_ = true;

    /** Scene ウィンドウ表示時のマウス座標変換データ
     */
    struct SceneViewTransform {
        bool active = false;
        Vector2 imagePos{};   // ImGui画像のスクリーン座標（左上）
        Vector2 imageSize{};  // ImGui画像のサイズ
        Vector2 gameSize{};   // ゲーム解像度
    } sceneViewTransform_{};

    //EventSystem* eventSystem_ = nullptr;
public:
    /** @brief 入力システムを初期化
     * @param _hWnd ウィンドウハンドル
     * @param _hInstance インスタンスハンドル
     */
    void Initialize(HWND _hWnd, HINSTANCE _hInstance);

    /** @brief 入力状態を更新
     */
    void Update();

    /** @brief キーが押されているかを判定
     * @param _key キーコード
     * @return 押されている場合true
     */
    bool IsPress(BYTE _key) const;
    bool IsTrigger(BYTE _key) const;

    /** @brief マウス座標を取得
     * Scene ウィンドウが有効な場合はゲーム空間座標に変換して返す
     */
    Vector2 GetMousePosition() const;

    /** @brief Scene ウィンドウ用のマウス座標変換を設定
     * @param _active 変換を有効にするか
     * @param _imagePos ImGui画像のスクリーン座標（左上）
     * @param _imageSize ImGui画像のサイズ
     * @param _gameSize ゲーム解像度
     */
    void SetSceneViewTransform(bool _active, Vector2 _imagePos, Vector2 _imageSize, Vector2 _gameSize);

    /** @brief カーソルの表示/非表示を設定
     * @param _visible true で表示、false で非表示
     * @note Debug ビルドでは非表示時も SceneView 外ではカーソルを表示する
     */
    void SetCursorVisible(bool _visible);

    /** @brief カーソル表示状態を即時反映
     * @note Framework::Draw() の renderer_->Render() 後に呼ぶこと
     *       (SceneView の矩形情報は ImGui レンダリング後に確定するため)
     */
    void ApplyCursorVisibility();

    //void SetEventSystem(EventSystem* eventSystem) { /*eventSystem_ = eventSystem;*/ }
private:
    void UpdateKeyboard();
    void UpdateMouse();

}; // class Input

/** DirectInputキーコードマッピング
 */
static const struct {
    BYTE code;
    const char* name;
} KeyCodeMap[] = {
    {.code = DIK_ESCAPE, .name = "ESC"},
    {.code = DIK_1, .name = "1"}, {.code = DIK_2, .name = "2"}, {.code = DIK_3, .name = "3"}, {.code = DIK_4, .name = "4"}, {.code = DIK_5, .name = "5"},
    {.code = DIK_6, .name = "6"}, {.code = DIK_7, .name = "7"}, {.code = DIK_8, .name = "8"}, {.code = DIK_9, .name = "9"}, {.code = DIK_0, .name = "0"},
    {.code = DIK_MINUS, .name = "-"}, {.code = DIK_EQUALS, .name = "="}, {.code = DIK_BACK, .name = "BACKSPACE"},
    {.code = DIK_TAB, .name = "TAB"},
    {.code = DIK_Q, .name = "Q"}, {.code = DIK_W, .name = "W"}, {.code = DIK_E, .name = "E"}, {.code = DIK_R, .name = "R"}, {.code = DIK_T, .name = "T"},
    {.code = DIK_Y, .name = "Y"}, {.code = DIK_U, .name = "U"}, {.code = DIK_I, .name = "I"}, {.code = DIK_O, .name = "O"}, {.code = DIK_P, .name = "P"},
    {.code = DIK_LBRACKET, .name = "["}, {.code = DIK_RBRACKET, .name = "]"},
    {.code = DIK_RETURN, .name = "ENTER"},
    {.code = DIK_LCONTROL, .name = "L-CTRL"},
    {.code = DIK_A, .name = "A"}, {.code = DIK_S, .name = "S"}, {.code = DIK_D, .name = "D"}, {.code = DIK_F, .name = "F"}, {.code = DIK_G, .name = "G"},
    {.code = DIK_H, .name = "H"}, {.code = DIK_J, .name = "J"}, {.code = DIK_K, .name = "K"}, {.code = DIK_L, .name = "L"},
    {.code = DIK_SEMICOLON, .name = ";"}, {.code = DIK_APOSTROPHE, .name = "'"},
    {.code = DIK_GRAVE, .name = "`"},
    {.code = DIK_LSHIFT, .name = "L-SHIFT"},
    {.code = DIK_BACKSLASH, .name = "\\"},
    {.code = DIK_Z, .name = "Z"}, {.code = DIK_X, .name = "X"}, {.code = DIK_C, .name = "C"}, {.code = DIK_V, .name = "V"}, {.code = DIK_B, .name = "B"},
    {.code = DIK_N, .name = "N"}, {.code = DIK_M, .name = "M"},
    {.code = DIK_COMMA, .name = ","}, {.code = DIK_PERIOD, .name = "."}, {.code = DIK_SLASH, .name = "/"},
    {.code = DIK_RSHIFT, .name = "R-SHIFT"},
    {.code = DIK_MULTIPLY, .name = "NUMPAD *"},
    {.code = DIK_LMENU, .name = "L-ALT"},
    {.code = DIK_SPACE, .name = "SPACE"},
    {.code = DIK_CAPITAL, .name = "CAPS LOCK"},
    {.code = DIK_F1, .name = "F1"}, {.code = DIK_F2, .name = "F2"}, {.code = DIK_F3, .name = "F3"}, {.code = DIK_F4, .name = "F4"},
    {.code = DIK_F5, .name = "F5"}, {.code = DIK_F6, .name = "F6"}, {.code = DIK_F7, .name = "F7"}, {.code = DIK_F8, .name = "F8"},
    {.code = DIK_F9, .name = "F9"}, {.code = DIK_F10, .name = "F10"}, {.code = DIK_F11, .name = "F11"}, {.code = DIK_F12, .name = "F12"},
    {.code = DIK_NUMLOCK, .name = "NUM LOCK"},
    {.code = DIK_SCROLL, .name = "SCROLL LOCK"},
    {.code = DIK_NUMPAD7, .name = "NUMPAD 7"}, {.code = DIK_NUMPAD8, .name = "NUMPAD 8"}, {.code = DIK_NUMPAD9, .name = "NUMPAD 9"},
    {.code = DIK_SUBTRACT, .name = "NUMPAD -"},
    {.code = DIK_NUMPAD4, .name = "NUMPAD 4"}, {.code = DIK_NUMPAD5, .name = "NUMPAD 5"}, {.code = DIK_NUMPAD6, .name = "NUMPAD 6"},
    {.code = DIK_ADD, .name = "NUMPAD +"},
    {.code = DIK_NUMPAD1, .name = "NUMPAD 1"}, {.code = DIK_NUMPAD2, .name = "NUMPAD 2"}, {.code = DIK_NUMPAD3, .name = "NUMPAD 3"},
    {.code = DIK_NUMPAD0, .name = "NUMPAD 0"},
    {.code = DIK_DECIMAL, .name = "NUMPAD ."},
    {.code = DIK_RCONTROL, .name = "R-CTRL"},
    {.code = DIK_DIVIDE, .name = "NUMPAD /"},
    {.code = DIK_RMENU, .name = "R-ALT"},
    {.code = DIK_HOME, .name = "HOME"},
    {.code = DIK_UP, .name = "UP"},
    {.code = DIK_PRIOR, .name = "PAGE UP"},
    {.code = DIK_LEFT, .name = "LEFT"},
    {.code = DIK_RIGHT, .name = "RIGHT"},
    {.code = DIK_END, .name = "END"},
    {.code = DIK_DOWN, .name = "DOWN"},
    {.code = DIK_NEXT, .name = "PAGE DOWN"},
    {.code = DIK_INSERT, .name = "INSERT"},
    {.code = DIK_DELETE, .name = "DELETE"},
};

#endif // Input_HPP_
