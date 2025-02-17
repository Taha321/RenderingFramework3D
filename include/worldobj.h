#pragma once
#include <memory>
#include <vector>
#include "types.h"
#include "matrix.h"
#include "vec.h"

#include "mesh.h"

namespace RenderingFramework3D {
class Renderer;

struct Material {
	MathUtil::Vec<4> colour;
	float diffuseConstant=0.8;
	float specularConstant=1.8;
	float shininess=50;
};

class WorldObject
{
public:
	WorldObject();
	WorldObject(Mesh& mesh);	
	WorldObject(WorldObject& src);
	WorldObject(WorldObject&& src);

	WorldObject& operator=(WorldObject& src);
	WorldObject& operator=(WorldObject&& src);
	~WorldObject();


	void SetMesh(Mesh& mesh);

	void SetNumVertIndices(unsigned num);
	unsigned GetNumVertIndices() const;

	void SetBackFaceCulling(bool enable);
	bool GetBackFaceCulling() const;

	// movement and rotation wrt. parent object frame
	void SetPosition(const MathUtil::Vec<3>& position);
	void Move(const MathUtil::Vec<3>& displacement);
	void SetOrientationEulerXYZ(const MathUtil::Vec<3>& angles);
	void SetRotationMatrix(const MathUtil::Matrix<3,3>& matrix);
	void Rotate(const MathUtil::Vec<3>& axis, float radians);

	// scale object
	void SetScale(float x, float y, float z);
	void SetScaleX(float x);
	void SetScaleY(float y);
	void SetScaleZ(float z);

	// attach a parent object reference frame
	void AttachReferenceFrame(const WorldObject& ref);
	void DetachReferenceFrame();

	Material& GetMaterial();
	const Material& GetMaterial() const;

	// must follow glsl alignment requirements for uniform buffer objects
	void SetCustomUniformShaderInputData(unsigned binding, const void* data, unsigned bytes, unsigned offset=0);

	// transform and position in world reference frame
	MathUtil::Vec<3> GetPosition() const;
	MathUtil::Matrix<4,4> GetTransform() const;

	// transform and postition wrt. parent object frame
	MathUtil::Vec<3> GetLocalPosition() const;
	const MathUtil::Matrix<4,4>& GetLocalTransform() const;

	const MathUtil::Vec<4>& GetObjectScale() const;
	const std::vector<uint8_t>& GetCustomData(unsigned binding) const;

	Mesh GetMesh() const;

private:
	friend Renderer;

	class WorldObjectInternal;
	std::shared_ptr<WorldObjectInternal> _internal;
};
}