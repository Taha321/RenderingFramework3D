#pragma once
#include <memory>
#include <string>
#include "vec.h"

namespace RenderingFramework3D {

class Renderer;
class Window
{
public:
	Window();
	~Window();

	bool Initialize(bool fullscreen, unsigned width, unsigned height, const std::string& name);
	bool Cleanup();

	bool IsResized();
	unsigned GetWidth();
	unsigned GetHeight();

	void Update();

	bool CheckExit();

public:
	enum KeyCode {
		KEY_SPACE,
		KEY_APOSTROPHE,
		KEY_COMMA,
		KEY_MINUS,
		KEY_PERIOD,
		KEY_SLASH,
		KEY_0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_SEMICOLON,
		KEY_EQUAL,
		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,
		KEY_LBRACKET,
		KEY_BACKSLASH,
		KEY_RBRACKET,
		KEY_ESC,
		KEY_ENTER,
		KEY_TAB,
		KEY_BACKSPACE,
		KEY_INSERT,
		KEY_DELETE,
		KEY_RIGHT,
		KEY_LEFT,
		KEY_DOWN,
		KEY_UP,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,
		KEY_HOME,
		KEY_END,
		KEY_CAPSLOCK,
		KEY_SCROLLLOCK,
		KEY_NUMLOCK,
		KEY_PRINT_SCREEN,
		KEY_PAUSE,
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,
		KEY_F13,
		KEY_F14,
		KEY_F15,
		KEY_F16,
		KEY_F17,
		KEY_F18,
		KEY_F19,
		KEY_F20,
		KEY_F21,
		KEY_F22,
		KEY_F23,
		KEY_F24,
		KEY_F25,
		KEY_KP0,
		KEY_KP1,
		KEY_KP2,
		KEY_KP3,
		KEY_KP4,
		KEY_KP5,
		KEY_KP6,
		KEY_KP7,
		KEY_KP8,
		KEY_KP9,
		KEY_KP_DECIMAL,
		KEY_KP_DIVIDE,
		KEY_KP_MULT,
		KEY_KP_SUBTRACT,
		KEY_KP_ADD,
		KEY_KP_ENTER,
		KEY_KP_EQUAL,
		KEY_LSHIFT,
		KEY_LCTRL,
		KEY_LALT,
		KEY_LSUPER,
		KEY_RSHIFT,
		KEY_RCTRL,
		KEY_RALT,
		KEY_MENU,
		NUM_KEYS,
	};

	enum MouseButton {
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_MIDDLE,
		NUM_MOUSE_BUTTON,
	};

public:

	bool IsKeyPressed(KeyCode key);
	bool CheckKeyPressEvent(KeyCode key);
	bool CheckKeyReleaseEvent(KeyCode key);

	bool IsMouseButtonPressed(MouseButton button);
	bool CheckMouseButtonPressEvent(MouseButton button);
	bool CheckMouseButtonReleaseEvent(MouseButton button);

	MathUtil::Vec<2> GetMousePosition();
	MathUtil::Vec<2> GetMouseDisplacement();

	void SetMouseVisibility(bool visible);

private:
	class WindowInternal;
	std::shared_ptr<WindowInternal> _internal;

	friend Renderer;
};
}
