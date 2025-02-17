#include <cmath>
#include "types.h"
#include "worldobj_internal.h"



namespace RenderingFramework3D {

using namespace MathUtil;


WorldObject::WorldObjectInternal::WorldObjectInternal()
    :
    _transform(GetIdentity<4>()),
    _material(),
    _scale({1,1,1,1}),
    _num_indices(0),
    _cull_mode(true)
{

}

WorldObject::WorldObjectInternal::WorldObjectInternal(const std::shared_ptr<Mesh::MeshInternal>& mesh)
    :
    _transform(GetIdentity<4>()),
    _material(),
    _scale({1,1,1,1}),
    _num_indices(0),
    _cull_mode(true),
    _mesh(mesh)
{}


void WorldObject::WorldObjectInternal::SetMesh(const std::shared_ptr<Mesh::MeshInternal>& mesh) {
    _mesh = mesh;
}



void WorldObject::WorldObjectInternal::SetNumVertIndices(unsigned num) {
    _num_indices = num;
}

unsigned WorldObject::WorldObjectInternal::GetNumVertIndices() const{
    return _num_indices;
}

void WorldObject::WorldObjectInternal::SetBackFaceCulling(bool enable) {
    _cull_mode = enable;
}
bool WorldObject::WorldObjectInternal::GetBackFaceCulling() const {
    return _cull_mode;
}

void WorldObject::WorldObjectInternal::SetPosition(const Vec<3>& position) {
    _transform(0, 3) = position(0);
    _transform(1, 3) = position(1);
    _transform(2, 3) = position(2);
}

void WorldObject::WorldObjectInternal::Move(const Vec<3>& displacement) {
    _transform(0, 3) += displacement(0);
    _transform(1, 3) += displacement(1);
    _transform(2, 3) += displacement(2);
}

void WorldObject::WorldObjectInternal::SetOrientationEulerXYZ(const Vec<3>& angles) {
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
}


void WorldObject::WorldObjectInternal::SetRotationMatrix(const MathUtil::Matrix<3,3>& matrix) {
    _transform(0,0) = matrix(0,0);
    _transform(0,1) = matrix(0,1);
    _transform(0,2) = matrix(0,2);

    _transform(1,0) = matrix(1,0);
    _transform(1,1) = matrix(1,1);
    _transform(1,2) = matrix(1,2);

    _transform(2,0) = matrix(2,0);
    _transform(2,1) = matrix(2,1);
    _transform(2,2) = matrix(2,2);
}

void WorldObject::WorldObjectInternal::Rotate(const Vec<3>& axis, float radians) {

    auto u = axis;
    u.Normalize();
    //axis(2, 0) = 1;
    Matrix<4,4> transform;
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

    transform(3, 0) = 0;
    transform(3, 1) = 0;
    transform(3, 2) = 0;
    transform(0, 3) = 0;
    transform(1, 3) = 0;
    transform(2, 3) = 0;

    //_transform.print();
    _transform = transform * _transform;
    _transform(0, 3) = tempx;
    _transform(1, 3) = tempy;
    _transform(2, 3) = tempz;
    //_transform.print();
}

void WorldObject::WorldObjectInternal::SetCustomUniformShaderInputData(unsigned binding, const void* data, unsigned bytes, unsigned offset) {
    if (_custom_uniform_data.find(binding) == _custom_uniform_data.end()) {
        _custom_uniform_data.insert({ binding, std::vector<uint8_t>(bytes + offset) });
    }
    memcpy(_custom_uniform_data[binding].data()+offset, data, bytes);
    float* ptr = (float*)_custom_uniform_data[binding].data();
}

void WorldObject::WorldObjectInternal::SetScale(float x, float y, float z) {
    _scale = Vec<4>({x,y,z,1});
}
void WorldObject::WorldObjectInternal::SetScaleX(float x) {
    _scale(0) = x;
}
void WorldObject::WorldObjectInternal::SetScaleY(float y) {
    _scale(1) = y;
}
void WorldObject::WorldObjectInternal::SetScaleZ(float z) {
    _scale(2) = z;
}

void WorldObject::WorldObjectInternal::AttachReferenceFrame(const std::shared_ptr<WorldObjectInternal>& ref) {
    _parent = std::const_pointer_cast<const WorldObjectInternal>(ref);
}

void WorldObject::WorldObjectInternal::DetachReferenceFrame() {
    _parent.reset();
}

Vec<3> WorldObject::WorldObjectInternal::GetPosition() const {
    if(auto ref = _parent.lock()) {
        Vec<4> pos4 = ref->GetTransform() * Vec<4>({_transform(0,3), _transform(1,3), _transform(2,3),1});
        return Vec<3>({pos4(0), pos4(1), pos4(2)});
    }
    return Vec<3>({_transform(0,3), _transform(1,3), _transform(2,3)});
}

Vec<3> WorldObject::WorldObjectInternal::GetLocalPosition() const {
    return Vec<3>({_transform(0,3), _transform(1,3), _transform(2,3)});
}

Matrix<4,4> WorldObject::WorldObjectInternal::GetTransform() const {
    if(auto ref = _parent.lock()) {
        return ref->GetTransform() * _transform;
    }
    return _transform;
}

const Matrix<4, 4>& WorldObject::WorldObjectInternal::GetLocalTransform() const {
    return _transform;
}

const Vec<4>& WorldObject::WorldObjectInternal::GetObjectScale() const {
    return _scale;
}


Material& WorldObject::WorldObjectInternal::GetMaterial() {
    return _material;
}

const Material& WorldObject::WorldObjectInternal::GetMaterial() const {
    return _material;
}

const std::vector<uint8_t>& WorldObject::WorldObjectInternal::GetCustomData(unsigned binding) const {
    if (_custom_uniform_data.find(binding) != _custom_uniform_data.end()) {
        return _custom_uniform_data.at(binding);
    }
    static std::vector<uint8_t> dummy;
    return dummy;
}

const std::shared_ptr<Mesh::MeshInternal>& WorldObject::WorldObjectInternal::GetMesh() const {
    return _mesh;
}
}
