#pragma once

#include <memory>
#include <vector>

#include "window.h"
#include "camera.h"
#include "worldobj.h"


namespace RenderingFramework3D {
class Renderer
{
public:
	Renderer();
	~Renderer();

	bool Initialize(Window& wnd);
	bool Cleanup();

	// global uniform data
	void SetLightDirection(const MathUtil::Vec<3>& direction);
	void SetLightDirection(unsigned pipeline, const MathUtil::Vec<3>& direction);
	void SetLightDirection(const std::vector<unsigned>& pipelines, const MathUtil::Vec<3>& light);
	
	void SetLightColour(const MathUtil::Vec<4>& colour);
	void SetLightColour(unsigned pipeline, const MathUtil::Vec<4>& colour);
	void SetLightColour(const std::vector<unsigned>& pipelines, const MathUtil::Vec<4>& colour);
	
	void SetLightIntensity(float intensity);
	void SetLightIntensity(unsigned pipeline, float intensity);
	void SetLightIntensity(const std::vector<unsigned>& pipelines, float intensity);

	void SetAmbientLightIntensity(float intensity);
	void SetAmbientLightIntensity(unsigned pipeline, float intensity);
	void SetAmbientLightIntensity(const std::vector<unsigned>& pipelines, float intensity);

	// must follow glsl alignment requirements for uniform buffer objects
	void SetCustomGlobalUniformShaderData(unsigned pipeline, unsigned binding, void* data, unsigned size, unsigned offset=0);
	void SetCustomGlobalUniformShaderData(const std::vector<unsigned>& pipeline, unsigned binding, void* data, unsigned size, unsigned offset=0);

	// rendering functions
	bool DrawObject(const WorldObject& obj, Camera& cam, unsigned pipelineID=PIPELINE_SHADED);
	bool PresentFrame();

	// custom pipeline
	bool CreateCustomPipeline(const PipelineConfig& config, unsigned& pipelineID);

private:
	friend Mesh;
	class RendererInternal;
	std::unique_ptr<RendererInternal> _internal;
};
}

