#pragma once
#include <vector>
#include "types_internal.h"
#include "renderer.h"
#include "worldobj_internal.h"
#include "pipeline.h"

namespace RenderingFramework3D {
class Renderer::RendererInternal {
public:
	
	RendererInternal();

	bool Initialize(std::shared_ptr<Window::WindowInternal>& wnd);
	bool Cleanup();

	void SetLightDirection(const MathUtil::Vec<3>& direction);
	void SetLightDirection(unsigned pipeline, const MathUtil::Vec<3>& light);
	
	void SetLightColour(const MathUtil::Vec<4>& colour);
	void SetLightColour(unsigned pipeline, const MathUtil::Vec<4>& colour);
	
	void SetLightIntensity(float intensity);
	void SetLightIntensity(unsigned pipeline, float intensity);
	
	void SetAmbientLightIntensity(float intensity);
	void SetAmbientLightIntensity(unsigned pipeline, float intensity);
	
	void SetCustomGlobalUniformShaderData(unsigned pipeline, unsigned binding, void* data, unsigned size, unsigned offset);
	
	bool DrawObject(const WorldObject& obj, Camera& cam, unsigned pipelineID);
	bool PresentFrame();

	bool IsReady() const;

	bool CreatePipeline(const PipelineConfig& config, unsigned& pipelineID);

	unsigned GetDeviceID() const;

private:
	bool addCommandSetCullMode(VkCommandBuffer cmdBuffer, bool cull);
	bool addCommandBindViewPort(VkCommandBuffer cmdBuffer, const Camera& cam);
	bool submitGraphicsCommands(bool wait_for_image = false);
	bool commandBufferStart();

private:
	bool _init;
	std::vector<Pipeline> _pipelines;
	Swapchain _swapchain;

	VkCommandBuffer _cmd_buffer;
	VkSemaphore _image_available_sem;
	VkSemaphore _render_complete_sem;

	std::weak_ptr<Window::WindowInternal> _window;

	struct {
		bool startPass;
		ViewPort vp;
		bool cull;
		unsigned pipeline;
	} _draw_state;


	unsigned _dev_id;
};
}