#version 450


layout(location = 0) out vec4 outColour;


//enable uniformShaderInputLayout.ObjectInputs.useMaterialData in pipeline configuration
layout(set = 0, binding = 1) uniform materialUniformBufferObject {
	vec4 objColour;
	float diffuseConstant;
	float specularConstant;
	float shininess;
};



void main() {
    outColour = objColour;
}