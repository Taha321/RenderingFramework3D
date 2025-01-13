#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec3 outNormal;

layout(set = 0, binding = 0) uniform TransformUniformBufferObject {
    mat4 objToWorld;	//enable uniformShaderInputLayout.ObjectInputs.useObjToWorldTransform in pipeline configuration
//	mat4 worldToCam;	//enable uniformShaderInputLayout.ObjectInputs.useWorldToCamTransform in pipeline configuration
//	mat4 CamToScreen;	//enable uniformShaderInputLayout.ObjectInputs.useCamToScreenTransform in pipeline configuration
	mat4 objToScreen;	//enable uniformShaderInputLayout.ObjectInputs.useObjToScreenTransform in pipeline configuration
	vec4 objectScale;
};


void main() {
	vec4 scaledPosition = objectScale * inPosition;
    gl_Position = objToScreen * scaledPosition;

	mat3 objToWorld3d = mat3(objToWorld[0].xyz, objToWorld[1].xyz, objToWorld[2].xyz);
	outPosition = objToWorld * scaledPosition;
	outNormal = objToWorld3d * inNormal;
}