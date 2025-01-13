#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "matrix.h"


#define PI 	3.14159265


namespace RenderingFramework3D {

#define PIPELINE_SHADED 0
#define PIPELINE_UNSHADED 1
#define PIPELINE_WIREFRAME 2
#define PIPELINE_LINKED_LINES 3

enum GLSLType {
	GLSL_BOOL,
	GLSL_INT,
	GLSL_UINT,
	GLSL_FLOAT,
	GLSL_DOUBLE,
};


struct CustomVertInputLayout {
	//input location in glsl vertex shader
	unsigned shaderInputSlot;

	//data type
	GLSLType type;

	//number of components
	unsigned components;
};

struct VertDataLayout {
	bool useVertBuffer = true;
	//in vertex shader: layout(location = 0) in
	uint8_t vertInputSlot = 0;

	bool useVertNormBuffer = true;
	//in vertex shader: layout(location = 1) in
	uint8_t vertNormInputSlot = 1;
	
	std::vector<CustomVertInputLayout> customVertInputLayouts;
};


struct CustomUniformShaderInputLayout {
	bool vertInput;
	bool fragInput;
	uint8_t size;
	unsigned bindSlot;
};

struct UniformShaderInputLayout {
	//set 0
	struct {
		bool useObjToScreenTransform = true;
		bool useObjToWorldTransform = true;
		bool useWorldToCamTransform = false;
		bool useCamToScreenTransform = false;
		bool useObjectScale = true;
		//in shader: layout(set=0,binding=0)
		uint8_t transformBindSlot = 0;
		bool transformVertInput=true;
		bool transformFragInput=false;

		bool useMaterialData = true;
		//in shader: layout(set=0,binding=1)
		uint8_t materialDataBindSlot = 1;
		bool materialVertInput=false;
		bool materialFragInput=true;

		bool useCamTransform = true;
		//in shader: layout(set=0,binding=2)
		uint8_t camTransformBindSlot = 2;
		bool camTransformVertInput = false;
		bool camTransformFragInput = true;

		std::vector<CustomUniformShaderInputLayout> CustomUniformShaderInput;

	} ObjectInputs;
	
	//set 1
	struct {
		bool useDirectionalLight = true;
		//in shader: layout(set=1,binding=0)
		uint8_t dirLightBindSlot = 0;
		bool dirlightVertInput=false;
		bool dirlightFragInput=true;

		std::vector<CustomUniformShaderInputLayout> CustomUniformShaderInput;

	} GlobalInputs;
};

enum DefaultFragShader {
	DEFAULT_FRAG_SHADER_LIT,
	DEFAULT_FRAG_SHADER_UNLIT,
	NUM_DEFAULT_FRAG_SHADER
};

enum DefaultVertShader {
	DEFAULT_VERT_SHADER_LIT,
	DEFAULT_VERT_SHADER_UNLIT,
	NUM_DEFAULT_VERT_SHADER
};

enum PrimitiveType {
	PRIM_TYPE_TRIANGLE_FILLED,
	PRIM_TYPE_TRIANGLE_WIREFRAME,
	PRIM_TYPE_LINE_LINKED,
	PRIM_TYPE_LINE_UNLINKED,
};

struct PipelineConfig {
	bool useDefaultVertData = true;

	//Layout of per vertex data passed to the Vertex shader, ignored if useDefaultVertData is true
	VertDataLayout vertDataLayout;

	UniformShaderInputLayout uniformShaderInputLayout;

	bool alphaBlendEnable = true;
	PrimitiveType primitiveType = PRIM_TYPE_TRIANGLE_FILLED;

	bool useDefaultShaders = true;
	DefaultFragShader defFragShaderSelect = DEFAULT_FRAG_SHADER_LIT;
	DefaultVertShader defVertShaderSelect = DEFAULT_VERT_SHADER_LIT;

	//glsl shaders compiled to spirv, ignored if default shaders are used
	std::string customVertexShaderPath;
	std::string customFragmentShaderPath;
};


struct ViewPort {
	unsigned posX;
	unsigned posY;
	unsigned width;
	unsigned height;

	bool operator==(const ViewPort& comp){
		return (posX==comp.posX) && (posY==comp.posY) && (width==comp.width) && (height == comp.height);
	}
	bool operator!=(const ViewPort& comp) {
		return (posX!=comp.posX) || (posY!=comp.posY) || (width!=comp.width) || (height != comp.height);
	}
};

enum ProjectionMode {
	PROJ_MODE_PERSPECTIVE,
	PROJ_MODE_ISOMETRIC
};

}