#pragma once
#include <memory>
#include <vector>
#include "types.h"
#include "matrix.h"
#include "vec.h"

#include "worldobj.h"
#include "mesh_internal.h"



namespace RenderingFramework3D {

class WorldObject::WorldObjectInternal
{
public:
	WorldObjectInternal() = default;
	WorldObjectInternal(const std::shared_ptr<Mesh::MeshInternal>& mesh);

	void SetMesh(const std::shared_ptr<Mesh::MeshInternal>& mesh);

	void SetNumVertIndices(unsigned num);
	unsigned GetNumVertIndices() const;

	void SetBackFaceCulling(bool enable);
	bool GetBackFaceCulling() const;

	void SetPosition(const MathUtil::Vec<3>& position);
	void Move(const MathUtil::Vec<3> displacement);
	void Rotate(const MathUtil::Vec<3>& axis, float radians);
	void SetScale(float x, float y, float z);
	void SetScaleX(float x);
	void SetScaleY(float y);
	void SetScaleZ(float z);

	Material& GetMaterial();
	const Material& GetMaterial() const;

	//must follow glsl alignment requirements for uniform buffer objects
	void SetCustomUniformShaderInputData(unsigned binding, const void* data, unsigned bytes, unsigned offset=0);

	MathUtil::Vec<3> GetPosition() const;
	const MathUtil::Vec<4>& GetObjectScale() const;
	const MathUtil::Matrix<4,4>& GetTransform() const;
	const std::vector<uint8_t>& GetCustomData(unsigned binding) const;

	const std::shared_ptr<Mesh::MeshInternal>& GetMesh() const;


private:
	MathUtil::Matrix<4, 4> _transform;
	MathUtil::Vec<4> _scale;
	Material _material;
	unsigned _num_indices;
	bool _cull_mode;
	std::unordered_map <unsigned, std::vector<uint8_t>> _custom_uniform_data;

	std::shared_ptr<Mesh::MeshInternal> _mesh;
};
}