#ifndef Input_HPP_
#define Input_HPP_
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl/client.h>

/** @brief 入力処理クラス
 ** キーボード、マウス、ジョイスティックの入力を管理
 **/
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

    //EventSystem* eventSystem_ = nullptr;
public:
    /** @brief 入力システムを初期化
     ** @param _hWnd ウィンドウハンドル
     ** @param _hInstance インスタンスハンドル
     **/
    void Initialize(HWND _hWnd, HINSTANCE _hInstance);

    /** @brief 入力状態を更新
     **/
    void Update();

    /** @brief キーが押されているかを判定
     ** @param _key キーコード
     ** @return 押されている場合true
     **/
    bool IsPress(BYTE _key) const;
    bool IsTrigger(BYTE _key) const;

    //void SetEventSystem(EventSystem* eventSystem) { /*eventSystem_ = eventSystem;*/ }
private:
    void UpdateKeyboard();

}; // class Input

#endif // Input_HPP_
