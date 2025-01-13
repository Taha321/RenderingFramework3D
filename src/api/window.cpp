#include "wnd_internal.h"


namespace RenderingFramework3D {

using namespace MathUtil;

Window::Window() {
	_internal = std::make_shared<WindowInternal>();
}

Window::~Window() {}

bool Window::Initialize(bool fullscreen, unsigned width, unsigned height, const std::string& name) {
	return _internal->Initialize(fullscreen, width, height,name);
}

bool Window::Cleanup() {
	return _internal->Cleanup();
}


bool Window::IsResized() {
	return _internal->IsResized();
}

unsigned Window::GetWidth() {
	return _internal->GetWidth();
}
unsigned Window::GetHeight() {
	return _internal->GetHeight();
}

bool Window::CheckExit() {
	return _internal->CheckExit();
}


void Window::Update() {
	_internal->Update();
}

bool Window::IsKeyPressed(KeyCode key) {
	return _internal->IsKeyPressed(key);
}

bool Window::CheckKeyPressEvent(KeyCode key) {
	return _internal->CheckKeyPressEvent(key);
}
bool Window::CheckKeyReleaseEvent(KeyCode key) {
	return _internal->CheckKeyReleaseEvent(key);
}

bool Window::IsMouseButtonPressed(MouseButton button) {
	return _internal->IsMouseButtonPressed(button);
}
bool Window::CheckMouseButtonPressEvent(MouseButton button) {
	return _internal->CheckMouseButtonPressEvent(button);
}
bool Window::CheckMouseButtonReleaseEvent(MouseButton button) {
	return _internal->CheckMouseButtonReleaseEvent(button);
}


Vec<2> Window::GetMousePosition() {
	return _internal->GetMousePosition();
}

Vec<2> Window::GetMouseDisplacement() {
	return _internal->GetMouseDisplacement();
}

void Window::SetMouseVisibility(bool visible) {
	_internal->SetMouseVisibility(visible);
}
}