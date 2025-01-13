#pragma once
#include <bitset>
#include "window.h"
#include "util.h"


namespace RenderingFramework3D {

using namespace MathUtil;

class Window::WindowInternal
{
public:
	WindowInternal();
	~WindowInternal();

	GLFWwindow* GetGlfwHandle();
	
	bool Initialize(bool fullscreen, unsigned width, unsigned height, const std::string& name);
	bool Cleanup();

	bool IsResized();
	unsigned GetWidth();
	unsigned GetHeight();

	void Update();

	bool CheckExit();

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
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void resizeCallback(GLFWwindow* window, int width, int height);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
private:
	bool _init;
	GLFWwindow* _glfw_wnd;

	std::bitset<NUM_KEYS> _key_states;
	std::bitset<NUM_KEYS> _key_pressed;
	std::bitset<NUM_KEYS> _key_released;

	std::bitset<NUM_MOUSE_BUTTON> _btn_states;
	std::bitset<NUM_MOUSE_BUTTON> _btn_pressed;
	std::bitset<NUM_MOUSE_BUTTON> _btn_released;

	Vec<2> _current_mouse_pos;
	Vec<2> _delta_mouse_pos;

	bool _resized;
	unsigned _width;
	unsigned _height;
	bool _mouse_hidden;

	static std::unordered_map<GLFWwindow*, WindowInternal*> _window_map;
	friend Window;
};
}
