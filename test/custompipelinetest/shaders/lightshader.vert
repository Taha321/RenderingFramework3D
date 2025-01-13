#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outWorldPosition;
layout(location = 1) out vec4 outObjectPosition;

layout(set = 0, binding = 0) uniform TransformUniformBufferObject {
    mat4 objToWorld;	//enable uniformShaderInputLayout.ObjectInputs.useObjToWorldTransform in pipeline configuration
	mat4 worldToCam;	//enable uniformShaderInputLayout.ObjectInputs.useWorldToCamTransform in pipeline configuration
//	mat4 CamToScreen;	//enable uniformShaderInputLayout.ObjectInputs.useCamToScreenTransform in pipeline configuration
	mat4 objToScreen;	//enable uniformShaderInputLayout.ObjectInputs.useObjToScreenTransform in pipeline configuration
	vec4 objectScale;
};


void main() {
	vec4 scaledPosition = objectScale * inPosition;

	vec4 lightPos_cam = worldToCam * objToWorld[3];
	vec4 vertpos_cam = worldToCam * objToWorld * scaledPosition;

	float radius = length(scaledPosition.xyz);

	if((lightPos_cam.z-radius) > 0 && vertpos_cam.z <= 0) {
		// scaledPosition.xyz = scaledPosition.xyz - vertpos_cam.z * worldToCam.xyz; 
		// scaledPosition = vec4(scaledPosition.xyz - vertpos_cam.z * worldToCam.xyz, 1);
		scaledPosition += vec4((vertpos_cam.z+1) * vec3(worldToCam[0].z, worldToCam[1].z, worldToCam[2].z), 1); 
	}

	outWorldPosition = objToWorld * scaledPosition;
	outObjectPosition = scaledPosition;

    gl_Position = objToScreen * scaledPosition;
}