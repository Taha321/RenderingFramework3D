#pragma once
#include "types.h"
#include "matrix.h"
#include "vec.h"


namespace RenderingFramework3D {


class Camera
{
public:
	Camera(const ViewPort& view, ProjectionMode mode=PROJ_MODE_PERSPECTIVE);

	void SetPosition(const MathUtil::Vec<3>& position);
	void Move(const MathUtil::Vec<3>& displacement);
	void SetOrientationEulerXYZ(const MathUtil::Vec<3>& angles);
	void SetRotationMatrix(const MathUtil::Matrix<3,3>& matrix);
	void Rotate(const MathUtil::Vec<3>& axis, float radians);

	void SetProjectionMode(ProjectionMode mode);
	void SetViewPort(const ViewPort& view);
	void SetCameraZoom(float zoom);
	void SetClipDistance(float zmin, float zmax);

	ProjectionMode GetProjectionMode() const;
	const ViewPort& GetCameraViewPort() const;
	float GetFarPlane() const;
	float GetNearPlane() const;
	const MathUtil::Matrix<4, 4>& GetTransform() const;

	MathUtil::Vec<3> GetCameraAxisX() const;
	MathUtil::Vec<3> GetCameraAxisY() const;
	MathUtil::Vec<3> GetCameraAxisZ() const;

	MathUtil::Vec<4> GetPosition() const;
	const MathUtil::Matrix<4, 4>& GetCamToScreenTransform();
	const MathUtil::Matrix<4, 4>& GetWorldToCameraTransform();

private:
	void update_cam_to_screen();
	void update_world_to_cam();

	void update_cam_to_screen_pers();
	void update_cam_to_screen_iso();

private:
	MathUtil::Matrix<4,4> _world_to_cam;
	MathUtil::Matrix<4,4> _cam_to_screen;

	MathUtil::Matrix<4, 4> _transform;
	//view port
	ViewPort _view_port;

	float _scale;

	float _zmax;
	float _zmin;

	bool _update_cam_to_screen = true;
	bool _update_world_to_cam = true;

	ProjectionMode _proj_mode;
};

}