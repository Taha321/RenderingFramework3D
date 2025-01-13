#include <unordered_map>
#include <mutex>
#include "wnd_internal.h"

namespace RenderingFramework3D {

static unsigned _window_count = 0;
std::mutex _wnd_map_mt;
std::unordered_map<GLFWwindow*, Window::WindowInternal*> Window::WindowInternal::_window_map;

static Window::KeyCode keyCodeRemap(unsigned glfwkey);
static Window::MouseButton mouseButtonCodeRemap(unsigned glfwbtn);

Window::WindowInternal::WindowInternal()
	:
	_glfw_wnd(nullptr),
	_init(false),
	_key_states(),
	_delta_mouse_pos(0),
	_current_mouse_pos(0),
	_resized(false),
	_width(0),
	_height(0),
	_mouse_hidden(false)
{}

Window::WindowInternal::~WindowInternal() {

}

bool Window::WindowInternal::Initialize(bool fullscreen, unsigned width, unsigned height, const std::string& name) {
	if (_window_count == 0) {
		if (glfwInit() == GLFW_FALSE) {
			return false;
		}
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}
	_width = width;
	_height = height;

	if(fullscreen) {
		_glfw_wnd = glfwCreateWindow(width, height, name.c_str(), glfwGetPrimaryMonitor(), nullptr);
	} else {
		_glfw_wnd = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	}
	if (_glfw_wnd == NULL) {
		return false;
	}

	{
		std::lock_guard<std::mutex> g(_wnd_map_mt);
		_window_map[_glfw_wnd] = this;
	}
	_resized = false;
	glfwSetKeyCallback(_glfw_wnd, WindowInternal::keyCallback);
	glfwSetWindowSizeCallback(_glfw_wnd, WindowInternal::resizeCallback);
	glfwSetMouseButtonCallback(_glfw_wnd, WindowInternal::mouseButtonCallback);
	
	_init = true;
	_window_count++;
	return true;
}

bool Window::WindowInternal::Cleanup() {
	if (_init) {
		{
			std::lock_guard<std::mutex> g(_wnd_map_mt);
			_window_map.erase(_glfw_wnd);
		}
		glfwDestroyWindow(_glfw_wnd);
		_glfw_wnd = nullptr;
		_window_count--;
		_init = false;
		if (_window_count == 0) {
			glfwTerminate();
		}
	}
	return true;
}


bool Window::WindowInternal::IsResized() {
	bool ret = _resized;
	_resized = false;
	return ret;
}

unsigned Window::WindowInternal::GetWidth() {
	return _width;
}
unsigned Window::WindowInternal::GetHeight() {
	return _height;
}

bool Window::WindowInternal::CheckExit() {
	if(_init==false) {
		return true;
	}
	return glfwWindowShouldClose(_glfw_wnd);
}


void Window::WindowInternal::Update() {
	if (_init) {
		_key_pressed.reset();
		_key_released.reset();
		_btn_pressed.reset();
		_btn_released.reset();

		double mousex, mousey;
		glfwGetCursorPos(_glfw_wnd, &mousex, &mousey);
		Vec<2> newPos = Vec<2>({(float)mousex, (float)mousey});
		_delta_mouse_pos = newPos - _current_mouse_pos;

		if(_mouse_hidden == false){
			_current_mouse_pos = newPos;
		} else {
			glfwSetCursorPos( _glfw_wnd, (double)_width/2 , (double)_height/2);
			_current_mouse_pos = Vec<2>({(float)(_width/2), (float)(_height/2)});
		}
		
		glfwPollEvents();
	}
}

bool Window::WindowInternal::IsKeyPressed(KeyCode key) {
	if (key < NUM_KEYS) {
		return _key_states[(unsigned)key];
	}
	return false;
}

bool Window::WindowInternal::CheckKeyPressEvent(KeyCode key) {
	if (key < NUM_KEYS) {
		return _key_pressed[(unsigned)key];
	}
	return false;
}
bool Window::WindowInternal::CheckKeyReleaseEvent(KeyCode key) {
	if (key < NUM_KEYS) {
		return _key_released[(unsigned)key];
	}
	return false;
}

bool Window::WindowInternal::IsMouseButtonPressed(MouseButton button) {
	if(button < NUM_MOUSE_BUTTON) {
		return _btn_states[(unsigned)button];
	}
	return false;
}
bool Window::WindowInternal::CheckMouseButtonPressEvent(MouseButton button) {
	if(button < NUM_MOUSE_BUTTON) {
		return _btn_pressed[(unsigned)button];
	}
	return false;
}
bool Window::WindowInternal::CheckMouseButtonReleaseEvent(MouseButton button) {
	if(button < NUM_MOUSE_BUTTON) {
		return _btn_released[(unsigned)button];
	}
	return false;
}


Vec<2> Window::WindowInternal::GetMousePosition() {
	return _current_mouse_pos;
}

Vec<2> Window::WindowInternal::GetMouseDisplacement() {
	return _delta_mouse_pos;
}

void Window::WindowInternal::SetMouseVisibility(bool visible) {
	_mouse_hidden = visible==false;
	if(visible == false) _current_mouse_pos = Vec<2>({(float)_width/2, (float)_height/2});
	glfwSetInputMode(_glfw_wnd, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

GLFWwindow* Window::WindowInternal::GetGlfwHandle() {
	return _glfw_wnd;
}

void  Window::WindowInternal::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Window::WindowInternal* wnd;
	{
		std::lock_guard<std::mutex> g(_wnd_map_mt);
		wnd = _window_map[window];
	}
	if (wnd) {
		auto rkey = keyCodeRemap(key);
		if (action == GLFW_RELEASE) {
			wnd->_key_states[rkey] = false;
			wnd->_key_released[rkey] = true;
		} else if(action == GLFW_PRESS){
			wnd->_key_states[rkey] = true;
			wnd->_key_pressed[rkey] = true;
		}
	}
}

void Window::WindowInternal::resizeCallback(GLFWwindow* window, int width, int height) {
	Window::WindowInternal* wnd;
	{
		std::lock_guard<std::mutex> g(_wnd_map_mt);
		wnd = _window_map[window];
	}
	if (wnd) {
		wnd->_resized = true;
		wnd->_width = width;
		wnd->_height = height;
	}
}
void Window::WindowInternal::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	Window::WindowInternal* wnd;
	{
		std::lock_guard<std::mutex> g(_wnd_map_mt);
		wnd = _window_map[window];
	}
	if (wnd) {
		auto btn = mouseButtonCodeRemap(button);
		if (action == GLFW_RELEASE) {
			wnd->_btn_states[btn] = false;
			wnd->_btn_released[btn] = true;
		}
		else if (action == GLFW_PRESS) {
			wnd->_btn_states[btn] = true;
			wnd->_btn_pressed[btn] = true;
		}
	}
}

static inline Window::MouseButton mouseButtonCodeRemap(unsigned glfwbtn) {
	switch (glfwbtn) {
		case GLFW_MOUSE_BUTTON_LEFT:
			return Window::MOUSE_LEFT;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return Window::MOUSE_RIGHT;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return Window::MOUSE_MIDDLE;
	}
	return Window::NUM_MOUSE_BUTTON;
}

static inline Window::KeyCode keyCodeRemap(unsigned glfwkey) {
	switch(glfwkey) {
		case GLFW_KEY_SPACE:
			return Window::KEY_SPACE;
		case GLFW_KEY_APOSTROPHE:
			return Window::KEY_APOSTROPHE;
		case GLFW_KEY_COMMA:
			return Window::KEY_COMMA;
		case GLFW_KEY_MINUS:
			return Window::KEY_MINUS;
		case GLFW_KEY_PERIOD:
			return Window::KEY_PERIOD;
		case GLFW_KEY_SLASH:
			return Window::KEY_SLASH;
		case GLFW_KEY_0:
			return Window::KEY_0;
		case GLFW_KEY_1:
			return Window::KEY_1;
		case GLFW_KEY_2:
			return Window::KEY_2;
		case GLFW_KEY_3:
			return Window::KEY_3;
		case GLFW_KEY_4:
			return Window::KEY_4;
		case GLFW_KEY_5:
			return Window::KEY_5;
		case GLFW_KEY_6:
			return Window::KEY_6;
		case GLFW_KEY_7:
			return Window::KEY_7;
		case GLFW_KEY_8:
			return Window::KEY_8;
		case GLFW_KEY_9:
			return Window::KEY_9;
		case GLFW_KEY_SEMICOLON:
			return Window::KEY_SEMICOLON;
		case GLFW_KEY_EQUAL:
			return Window::KEY_EQUAL;
		case GLFW_KEY_A:
			return Window::KEY_A;
		case GLFW_KEY_B:
			return Window::KEY_B;
		case GLFW_KEY_C:
			return Window::KEY_C;
		case GLFW_KEY_D:
			return Window::KEY_D;
		case GLFW_KEY_E:
			return Window::KEY_E;
		case GLFW_KEY_F:
			return Window::KEY_F;
		case GLFW_KEY_G:
			return Window::KEY_G;
		case GLFW_KEY_H:
			return Window::KEY_H;
		case GLFW_KEY_I:
			return Window::KEY_I;
		case GLFW_KEY_J:
			return Window::KEY_J;
		case GLFW_KEY_K:
			return Window::KEY_K;
		case GLFW_KEY_L:
			return Window::KEY_L;
		case GLFW_KEY_M:
			return Window::KEY_M;
		case GLFW_KEY_N:
			return Window::KEY_N;
		case GLFW_KEY_O:
			return Window::KEY_O;
		case GLFW_KEY_P:
			return Window::KEY_P;
		case GLFW_KEY_Q:
			return Window::KEY_Q;
		case GLFW_KEY_R:
			return Window::KEY_R;
		case GLFW_KEY_S:
			return Window::KEY_S;
		case GLFW_KEY_T:
			return Window::KEY_T;
		case GLFW_KEY_U:
			return Window::KEY_U;
		case GLFW_KEY_V:
			return Window::KEY_V;
		case GLFW_KEY_W:
			return Window::KEY_W;
		case GLFW_KEY_X:
			return Window::KEY_X;
		case GLFW_KEY_Y:
			return Window::KEY_Y;
		case GLFW_KEY_Z:
			return Window::KEY_Z;
		case GLFW_KEY_LEFT_BRACKET:
			return Window::KEY_LBRACKET;
		case GLFW_KEY_BACKSLASH:
			return Window::KEY_BACKSLASH;
		case GLFW_KEY_RIGHT_BRACKET:
			return Window::KEY_RBRACKET;
		case GLFW_KEY_ESCAPE:
			return Window::KEY_ESC;
		case GLFW_KEY_ENTER:
			return Window::KEY_ENTER;
		case GLFW_KEY_TAB:
			return Window::KEY_TAB;
		case GLFW_KEY_BACKSPACE:
			return Window::KEY_BACKSPACE;
		case GLFW_KEY_INSERT:
			return Window::KEY_INSERT;
		case GLFW_KEY_DELETE:
			return Window::KEY_DELETE;
		case GLFW_KEY_RIGHT:
			return Window::KEY_RIGHT;
		case GLFW_KEY_LEFT:
			return Window::KEY_LEFT;
		case GLFW_KEY_DOWN:
			return Window::KEY_DOWN;
		case GLFW_KEY_UP:
			return Window::KEY_UP;
		case GLFW_KEY_PAGE_UP:
			return Window::KEY_PAGE_UP;
		case GLFW_KEY_PAGE_DOWN:
			return Window::KEY_DOWN;
		case GLFW_KEY_HOME:
			return Window::KEY_HOME;
		case GLFW_KEY_END:
			return Window::KEY_END;
		case GLFW_KEY_CAPS_LOCK:
			return Window::KEY_CAPSLOCK;
		case GLFW_KEY_SCROLL_LOCK:
			return Window::KEY_SCROLLLOCK;
		case GLFW_KEY_NUM_LOCK:
			return Window::KEY_NUMLOCK;
		case GLFW_KEY_PRINT_SCREEN:
			return Window::KEY_PRINT_SCREEN;
		case GLFW_KEY_PAUSE:
			return Window::KEY_PAUSE;
		case GLFW_KEY_F1:
			return Window::KEY_F1;
		case GLFW_KEY_F2:
			return Window::KEY_F2;
		case GLFW_KEY_F3:
			return Window::KEY_F3;
		case GLFW_KEY_F4:
			return Window::KEY_F4;
		case GLFW_KEY_F5:
			return Window::KEY_F5;
		case GLFW_KEY_F6:
			return Window::KEY_F6;
		case GLFW_KEY_F7:
			return Window::KEY_F7;
		case GLFW_KEY_F8:
			return Window::KEY_F8;
		case GLFW_KEY_F9:
			return Window::KEY_F9;
		case GLFW_KEY_F10:
			return Window::KEY_F10;
		case GLFW_KEY_F11:
			return Window::KEY_F11;
		case GLFW_KEY_F12:
			return Window::KEY_F12;
		case GLFW_KEY_F13:
			return Window::KEY_F13;
		case GLFW_KEY_F14:
			return Window::KEY_F14;
		case GLFW_KEY_F15:
			return Window::KEY_F15;
		case GLFW_KEY_F16:
			return Window::KEY_F16;
		case GLFW_KEY_F17:
			return Window::KEY_F17;
		case GLFW_KEY_F18:
			return Window::KEY_F18;
		case GLFW_KEY_F19:
			return Window::KEY_F19;
		case GLFW_KEY_F20:
			return Window::KEY_F20;
		case GLFW_KEY_F21:
			return Window::KEY_F21;
		case GLFW_KEY_F22:
			return Window::KEY_F22;
		case GLFW_KEY_F23:
			return Window::KEY_F23;
		case GLFW_KEY_F24:
			return Window::KEY_F24;
		case GLFW_KEY_F25:
			return Window::KEY_F25;
		case GLFW_KEY_KP_0:
			return Window::KEY_KP0;
		case GLFW_KEY_KP_1:
			return Window::KEY_KP1;
		case GLFW_KEY_KP_2:
			return Window::KEY_KP2;
		case GLFW_KEY_KP_3:
			return Window::KEY_KP3;
		case GLFW_KEY_KP_4:
			return Window::KEY_KP4;
		case GLFW_KEY_KP_5:
			return Window::KEY_KP5;
		case GLFW_KEY_KP_6:
			return Window::KEY_KP6;
		case GLFW_KEY_KP_7:
			return Window::KEY_KP7;
		case GLFW_KEY_KP_8:
			return Window::KEY_KP8;
		case GLFW_KEY_KP_9:
			return Window::KEY_KP9;
		case GLFW_KEY_KP_DECIMAL:
			return Window::KEY_KP_DECIMAL;
		case GLFW_KEY_KP_DIVIDE:
			return Window::KEY_KP_DIVIDE;
		case GLFW_KEY_KP_MULTIPLY:
			return Window::KEY_KP_MULT;
		case GLFW_KEY_KP_SUBTRACT:
			return Window::KEY_KP_SUBTRACT;
		case GLFW_KEY_KP_ADD:
			return Window::KEY_KP_ADD;
		case GLFW_KEY_KP_ENTER:
			return Window::KEY_KP_ENTER;
		case GLFW_KEY_KP_EQUAL:
			return Window::KEY_KP_EQUAL;
		case GLFW_KEY_LEFT_SHIFT:
			return Window::KEY_LSHIFT;
		case GLFW_KEY_LEFT_CONTROL:
			return Window::KEY_LCTRL;
		case GLFW_KEY_LEFT_ALT:
			return Window::KEY_LALT;
		case GLFW_KEY_RIGHT_SHIFT:
			return Window::KEY_RSHIFT;
		case GLFW_KEY_RIGHT_CONTROL:
			return Window::KEY_RCTRL;
		case GLFW_KEY_RIGHT_ALT:
			return Window::KEY_RALT;
		case GLFW_KEY_MENU:
			return Window::KEY_MENU;
	}
	return Window::NUM_KEYS;
}
}