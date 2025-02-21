#include <cmath>
#include "types.h"
#include "worldobj_internal.h"



namespace RenderingFramework3D {

using namespace MathUtil;

WorldObject::WorldObject() {
    _internal = std::make_shared<WorldObjectInternal>();
}

WorldObject::WorldObject(Mesh& mesh) {
    _internal = std::make_shared<WorldObjectInternal>(mesh._internal);
}

WorldObject::WorldObject(WorldObject& src) {
    _internal = std::make_shared<WorldObjectInternal>(*src._internal);
}
WorldObject::WorldObject(WorldObject&& src) {
    _internal = std::move(src._internal);
}

WorldObject& WorldObject::operator=(WorldObject& src) {
    _internal = std::make_shared<WorldObjectInternal>(*src._internal);
    return *this;
}
WorldObject& WorldObject::operator=(WorldObject&& src) {
    _internal = std::move(src._internal);
    return *this;
}

WorldObject::~WorldObject() {}



void WorldObject::SetMesh(Mesh& mesh) {
    _internal->SetMesh(mesh._internal);
}

void WorldObject::SetNumVertIndices(unsigned num) {
    _internal->SetNumVertIndices(num);
}

unsigned WorldObject::GetNumVertIndices() const{
    return _internal->GetNumVertIndices();
}

void WorldObject::SetBackFaceCulling(bool enable) {
    _internal->SetBackFaceCulling(enable);
}
bool WorldObject::GetBackFaceCulling() const {
    return _internal->GetBackFaceCulling();
}

void WorldObject::SetPosition(const Vec<3>& position) {
    _internal->SetPosition(position);
}

void WorldObject::Move(const Vec<3>& displacement) {
    _internal->Move(displacement);
}

void WorldObject::SetOrientationEulerXYZ(const MathUtil::Vec<3>& angles) {
    _internal->SetOrientationEulerXYZ(angles);
}

void WorldObject::SetRotationMatrix(const MathUtil::Matrix<3,3>& matrix) {
    _internal->SetRotationMatrix(matrix);
}

void WorldObject::Rotate(const Vec<3>& axis, float radians) {
    _internal->Rotate(axis, radians);
}

void WorldObject::SetCustomUniformShaderInputData(unsigned binding, const void* data, unsigned bytes, unsigned offset) {
    _internal->SetCustomUniformShaderInputData(binding, data, bytes, offset);
}

void WorldObject::SetScale(float x, float y, float z) {
    _internal->SetScale(x,y,z);
}
void WorldObject::SetScaleX(float x) {
    _internal->SetScaleX(x);
}
void WorldObject::SetScaleY(float y) {
    _internal->SetScaleY(y);
}
void WorldObject::SetScaleZ(float z) {
    _internal->SetScaleZ(z);
}

void WorldObject::AttachReferenceFrame(const WorldObject& ref) {
    _internal->AttachReferenceFrame(ref._internal);
}

void WorldObject::DetachReferenceFrame() {
    _internal->DetachReferenceFrame();
}

Vec<3> WorldObject::GetPosition() const {
    return _internal->GetPosition();
}

Vec<3> WorldObject::GetLocalPosition() const {
    return _internal->GetLocalPosition();
}

Matrix<4,4> WorldObject::GetTransform() const {
    return _internal->GetTransform();
}

const Matrix<4, 4>& WorldObject::GetLocalTransform() const {
    return _internal->GetLocalTransform();
}

const Vec<4>& WorldObject::GetObjectScale() const {
    return _internal->GetObjectScale();
}

Material& WorldObject::GetMaterial() {
    return _internal->GetMaterial();
}

const Material& WorldObject::GetMaterial() const {
    return _internal->GetMaterial();
}

const std::vector<uint8_t>& WorldObject::GetCustomData(unsigned binding) const {
    return _internal->GetCustomData(binding);
}

Mesh WorldObject::GetMesh() const {
    return Mesh(_internal->GetMesh());
}
}
