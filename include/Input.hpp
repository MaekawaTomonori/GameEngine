#ifndef Input_HPP_
#define Input_HPP_
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl/client.h>

class Input {
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> joystick_;
	BYTE keyState_[256];
	BYTE preState_[256];
	DIMOUSESTATE mouseState_;
	DIMOUSESTATE preMouseState_;

	//EventSystem* eventSystem_ = nullptr;
public:
	void Initialize();
	void Update();

	//void SetEventSystem(EventSystem* eventSystem) { /*eventSystem_ = eventSystem;*/ }
private:
}; // class Input

#endif // Input_HPP_
