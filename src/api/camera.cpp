#include <cmath>
#include "camera.h"


namespace RenderingFramework3D {

using namespace MathUtil;

static constexpr float base_scale_pers = 0.02;
static constexpr float base_scale_iso = 0.1;

Camera::Camera(const ViewPort& view, ProjectionMode mode)
	:
	_world_to_cam(GetIdentity<4>()),
	_cam_to_screen(GetIdentity<4>()),
	_transform(GetIdentity<4>()),
	_proj_mode(mode)
{
	_view_port = view;
	_zmax = 1000;
	_zmin = 6;
	_scale = 1;
	_update_cam_to_screen = true;
	_update_world_to_cam = true;
}

void Camera::SetPosition(const Vec<3>& position) {
	_transform(0, 3) = position(0);
	_transform(1, 3) = position(1);
	_transform(2, 3) = position(2);

	_update_world_to_cam = true;
}

void Camera::Move(const Vec<3>& displacement) {
	_transform(0, 3) += displacement(0);
	_transform(1, 3) += displacement(1);
	_transform(2, 3) += displacement(2);

	_update_world_to_cam = true;
}

void Camera::SetOrientationEulerXYZ(const Vec<3>& angles) {
    float cx = cos(angles(0)), sx = sin(angles(0));
    float cy = cos(angles(1)), sy = sin(angles(1));
    float cz = cos(angles(2)), sz = sin(angles(2));

    _transform(0,0) = cy * cz;
    _transform(0,1) = -cy * sz;
    _transform(0,2) = sy;

    _transform(1,0) = cx * sz + sx * sy * cz;
    _transform(1,1) = cx * cz - sx * sy * sz;
    _transform(1,2) = -sx * cy;

    _transform(2,0) = sx * sz - cx * sy * cz;
    _transform(2,1) = sx * cz + cx * sy * sz;
    _transform(2,2) = cx * cy;

	_update_world_to_cam = true;
}

void Camera::SetRotationMatrix(const MathUtil::Matrix<3,3>& matrix) {
    _transform(0,0) = matrix(0,0);
    _transform(0,1) = matrix(0,1);
    _transform(0,2) = matrix(0,2);

    _transform(1,0) = matrix(1,0);
    _transform(1,1) = matrix(1,1);
    _transform(1,2) = matrix(1,2);

    _transform(2,0) = matrix(2,0);
    _transform(2,1) = matrix(2,1);
    _transform(2,2) = matrix(2,2);
	
	_update_world_to_cam = true;
}

void Camera::Rotate(const Vec<3>& axis, float radians) {

	auto u = axis;
	u.Normalize();
	Matrix<4,4> transform(0);
	transform(3, 3) = 1;

	//cos+ux^2(1-cos)	uxuy(1-cos)-uzsin	uxuz(1-cos)+uysin
	//uyux(1-cos)+uzsin	cos+uy^2(1-cos)		uyuz(1-cos)-uxsin
	//uzux(1-cos)-uysin	uzuy(1-cos)+uxsin	cos+uz^2(1-cos)
	float tempx = _transform(0, 3), tempy = _transform(1, 3), tempz = _transform(2, 3);

	float costheta = std::cos(radians);
	float sintheta = std::sin(radians);

	transform(0, 0) = costheta + u(0) * u(0) * (1 - costheta);
	transform(1, 0) = u(1) * u(0) * (1 - costheta) + u(2) * sintheta;
	transform(2, 0) = u(2) * u(0) * (1 - costheta) - u(1) * sintheta;

	transform(0, 1) = u(0) * u(1) * (1 - costheta) - u(2) * sintheta;
	transform(1, 1) = costheta + u(1) * u(1) * (1 - costheta);
	transform(2, 1) = u(2) * u(1) * (1 - costheta) + u(0) * sintheta;

	transform(0, 2) = u(0) * u(2) * (1 - costheta) + u(1) * sintheta;
	transform(1, 2) = u(1) * u(2) * (1 - costheta) - u(0) * sintheta;
	transform(2, 2) = costheta + u(2) * u(2) * (1 - costheta);

	_transform = transform * _transform;
	_transform(0, 3) = tempx;
	_transform(1, 3) = tempy;
	_transform(2, 3) = tempz;

	_update_world_to_cam = true;
}

void Camera::SetProjectionMode(ProjectionMode mode){
	_proj_mode = mode;
	_update_cam_to_screen = true;
}

void Camera::SetViewPort(const ViewPort& view) {
	_view_port = view;
	_update_cam_to_screen = true;
}

void Camera::SetClipDistance(float zmin, float zmax) {
	_zmax = zmax;
	_zmin = zmin;
	_update_cam_to_screen = true;
}

void Camera::SetCameraZoom(float zoom) {
	_scale = zoom;
	_update_cam_to_screen = true;
}


ProjectionMode Camera::GetProjectionMode() const {
	return _proj_mode;
}

const ViewPort& Camera::GetCameraViewPort() const{
	return _view_port;
}


float Camera::GetFarPlane() const {
	return _zmax;
}
float Camera::GetNearPlane() const {
	return _zmin;
}

const Matrix<4, 4>& Camera::GetTransform() const {
	return _transform;
}

Vec<3> Camera::GetCameraAxisX() const {
	return Vec<3>({ _transform(0,0), _transform(1,0) , _transform(2,0) });
}
Vec<3> Camera::GetCameraAxisY() const {
	return Vec<3>({ _transform(0,1), _transform(1,1) , _transform(2,1) });
}
Vec<3> Camera::GetCameraAxisZ() const {
	return Vec<3>({ _transform(0,2), _transform(1,2) , _transform(2,2) });
}

Vec<4> Camera::GetPosition() const {
	return Vec<4>({_transform(0,0), _transform(1,3), _transform(2,3), _transform(3,3)});
}
const Matrix<4,4>& Camera::GetCamToScreenTransform() {
	if (_update_cam_to_screen) {
		_update_cam_to_screen = false;
		update_cam_to_screen();
	}
	return _cam_to_screen;
}

const Matrix<4,4>& Camera::GetWorldToCameraTransform() {
	if (_update_world_to_cam) {
		_update_world_to_cam = false;
		update_world_to_cam();
	}
	return _world_to_cam;
}

void Camera::update_cam_to_screen() {
	switch(_proj_mode) {
		case PROJ_MODE_PERSPECTIVE:
			update_cam_to_screen_pers();
			break;
		case PROJ_MODE_ISOMETRIC:
			update_cam_to_screen_iso();
			break;
	}
}

void Camera::update_cam_to_screen_pers() {
	_cam_to_screen(0, 0) = 2 * _zmin / (_view_port.width * base_scale_pers * _scale);
	_cam_to_screen(1, 1) = -2 * _zmin / (_view_port.height * base_scale_pers * _scale);
	_cam_to_screen(2, 2) = (_zmin + _zmax) / _zmax;
	_cam_to_screen(2, 3) = -_zmin;
	_cam_to_screen(3, 2) = 1;
	_cam_to_screen(3, 3) = 0;
}
void Camera::update_cam_to_screen_iso() {
	_cam_to_screen(0, 0) = 2 / (_view_port.width * base_scale_iso * _scale);
	_cam_to_screen(1, 1) = -2 / (_view_port.height * base_scale_iso * _scale);
	_cam_to_screen(2, 2) = 1 / (_zmax * _scale);
	_cam_to_screen(2, 3) = 0;
	_cam_to_screen(3, 2) = 0;
	_cam_to_screen(3, 3) = 1;
}

void Camera::update_world_to_cam() {
	_world_to_cam(0, 0) = _transform(0, 0);
	_world_to_cam(0, 1) = _transform(1, 0);
	_world_to_cam(0, 2) = _transform(2, 0);

	_world_to_cam(1, 0) = _transform(0, 1);
	_world_to_cam(1, 1) = _transform(1, 1);
	_world_to_cam(1, 2) = _transform(2, 1);
	
	_world_to_cam(2, 0) = _transform(0, 2);
	_world_to_cam(2, 1) = _transform(1, 2);
	_world_to_cam(2, 2) = _transform(2, 2);

	_world_to_cam(0, 3) = -_world_to_cam(0, 0) * _transform(0, 3) - _world_to_cam(0, 1) * _transform(1, 3) - _world_to_cam(0, 2) * _transform(2, 3);
	_world_to_cam(1, 3) = -_world_to_cam(1, 0) * _transform(0, 3) - _world_to_cam(1, 1) * _transform(1, 3) - _world_to_cam(1, 2) * _transform(2, 3);
	_world_to_cam(2, 3) = -_world_to_cam(2, 0) * _transform(0, 3) - _world_to_cam(2, 1) * _transform(1, 3) - _world_to_cam(2, 2) * _transform(2, 3);
}

}