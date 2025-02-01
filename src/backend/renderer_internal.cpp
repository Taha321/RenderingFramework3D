#include <iostream>
#include "renderer_internal.h"
#include "wnd_internal.h"


namespace RenderingFramework3D {

using namespace MathUtil;


static unsigned _renderer_count = 0;

Renderer::RendererInternal::RendererInternal()
	:
	_init(false),
	_pipelines(),
	_swapchain(),
	_cmd_buffer(VK_NULL_HANDLE),
	_image_available_sem(VK_NULL_HANDLE),
	_render_complete_sem(VK_NULL_HANDLE),
	_window(),
	_dev_id(0)
{}
bool Renderer::RendererInternal::Initialize(std::shared_ptr<Window::WindowInternal>& wnd) {
	if (_init == true) {
		return true;
	}

	_window = wnd;
	if (auto wndShared = _window.lock()) {

		if (DeviceManager::Initialize() == false) {
			return false;
		}

		int width = 0, height = 0;

		GLFWwindow* glfwWnd = wndShared->GetGlfwHandle();
		if (glfwWnd == nullptr) {
			return false;
		}

		glfwGetFramebufferSize(wndShared->GetGlfwHandle(), &width, &height);
		VkExtent2D extent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		if (DeviceManager::CreateVkSurface(wndShared->GetGlfwHandle(), surface) == false) {
			return false;
		}
		SwapChainSupportDetails swpSupport{};
		if (DeviceManager::FindSuitableDevice(surface, _dev_id, swpSupport) == false) {
			return false;
		}
		if (DeviceManager::CreateCommandBuffer(_dev_id, DeviceManager::QUEUE_TYPE_GRAPHICS, true, _cmd_buffer) == false) {
			return false;
		}
		if (_swapchain.Initialize(_dev_id, { surface,extent,swpSupport }) == false) {
			return false;
		}

		VkDevice dev = DeviceManager::GetVkDevice(_dev_id);
		if (dev == VK_NULL_HANDLE) {
			return false;
		}
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(dev, &semaphoreInfo, nullptr, &_image_available_sem) != VK_SUCCESS) {
			return false;
		}
		if (vkCreateSemaphore(dev, &semaphoreInfo, nullptr, &_render_complete_sem) != VK_SUCCESS) {
			return false;
		}

		PipelineConfig config;
		config.useDefaultShaders = true;
		config.useDefaultVertData = true;
		
		unsigned pipelineID;
		if (CreatePipeline(config, pipelineID) == false) {
			return false;
		}

		config.uniformShaderInputLayout.ObjectInputs.useObjToWorldTransform = false;
		config.defFragShaderSelect = DEFAULT_FRAG_SHADER_UNLIT;
		config.defVertShaderSelect = DEFAULT_VERT_SHADER_UNLIT;
		if (CreatePipeline(config, pipelineID) == false) {
			return false;
		}
		
		config.uniformShaderInputLayout.ObjectInputs.useObjToWorldTransform = false;
		config.defFragShaderSelect = DEFAULT_FRAG_SHADER_UNLIT;
		config.defVertShaderSelect = DEFAULT_VERT_SHADER_UNLIT;
		config.primitiveType=PRIM_TYPE_TRIANGLE_WIREFRAME;
		if (CreatePipeline(config, pipelineID) == false) {
			return false;
		}

		config.uniformShaderInputLayout.ObjectInputs.useObjToWorldTransform = false;
		config.defFragShaderSelect = DEFAULT_FRAG_SHADER_UNLIT;
		config.defVertShaderSelect = DEFAULT_VERT_SHADER_UNLIT;
		config.primitiveType = PRIM_TYPE_LINE_LINKED;
		if (CreatePipeline(config, pipelineID) == false) {
			return false;
		}

		_init = true;
		_renderer_count++;
		_draw_state.startPass=true;
		return true;
	}
	return false;
}

bool Renderer::RendererInternal::Cleanup() {
	if (_init == false) {
		return true;
	}
	_init = false;
	_renderer_count--;

	for (auto& pipeline : _pipelines) {
		pipeline.Cleanup();
	}
	_pipelines.clear();

	_swapchain.Cleanup();

	if (_renderer_count <= 0) {
		DeviceManager::Cleanup();
	}
	return true;
}

bool Renderer::RendererInternal::IsReady() const {
	return _init;
}

void Renderer::RendererInternal::SetLightDirection(const Vec<3>& light) {
	for(auto& pipeline : _pipelines) {
		pipeline.SetLightDir(light);
	}
}
void Renderer::RendererInternal::SetLightDirection(unsigned pipeline, const Vec<3>& light) {
	if(pipeline < _pipelines.size() && _pipelines[pipeline].IsReady()) {
		_pipelines[pipeline].SetLightDir(light);
	}
}

void Renderer::RendererInternal::SetLightColour(const Vec<4>& colour) {
	for(auto& pipeline : _pipelines) {
		pipeline.SetLightColour(colour);
	}
}
void Renderer::RendererInternal::SetLightColour(unsigned pipeline, const Vec<4>& colour) {
	if(pipeline < _pipelines.size() && _pipelines[pipeline].IsReady()) {
		_pipelines[pipeline].SetLightColour(colour);
	}
}

void Renderer::RendererInternal::SetLightIntensity(float intensity) {
	for(auto& pipeline : _pipelines) {
		pipeline.SetLightIntensity(intensity/10);
	}
}
void Renderer::RendererInternal::SetLightIntensity(unsigned pipeline, float intensity) {
	if(pipeline < _pipelines.size() && _pipelines[pipeline].IsReady()) {
		_pipelines[pipeline].SetLightIntensity(intensity/10);
	}
}

void Renderer::RendererInternal::SetAmbientLightIntensity(float intensity) {
	for(auto& pipeline : _pipelines) {
		pipeline.SetAmbientLightIntensity(intensity);
	}
}
void Renderer::RendererInternal::SetAmbientLightIntensity(unsigned pipeline, float intensity) {
	if(pipeline < _pipelines.size() && _pipelines[pipeline].IsReady()) {
		_pipelines[pipeline].SetAmbientLightIntensity(intensity);
	}
}

void Renderer::RendererInternal::SetCustomGlobalUniformShaderData(unsigned pipeline, unsigned binding, void* data, unsigned size, unsigned offset) {
	if (pipeline < _pipelines.size() && _pipelines[pipeline].IsReady()) {
		_pipelines[pipeline].SetCustomGlobalData(binding, data, size, offset);
	}
}

bool Renderer::RendererInternal::DrawObject(const WorldObject& obj, Camera& cam, unsigned pipelineID) {
	
	if (_init) {
		if (pipelineID >= _pipelines.size()) {
			return false;
		}
		if(obj.GetMesh()._internal == nullptr) {
			return false;
		}
		
		bool first = _draw_state.startPass;
		if (_draw_state.startPass ) {

			_draw_state.startPass = false;
			if (DeviceManager::WaitForQueue(_dev_id, DeviceManager::QUEUE_TYPE_GRAPHICS) == false) {
				return false;
			}
			if (commandBufferStart() == false) {
				return false;
			}
			bool needUpdate;
			if (_swapchain.UpdateFrameBufferIndex(_image_available_sem, needUpdate) == false) {
				return false;
			}
			if (needUpdate) {
				if (auto wndShared = _window.lock()) {
					int width = 0, height = 0;

					GLFWwindow* glfwWnd = wndShared->GetGlfwHandle();
					if (glfwWnd == nullptr) {
						return false;
					}

					glfwGetFramebufferSize(wndShared->GetGlfwHandle(), &width, &height);
					VkExtent2D extent = {
						static_cast<uint32_t>(width),
						static_cast<uint32_t>(height)
					};
					if (_swapchain.UpdateSwapChain(extent) == false) {
						return false;
					}
					if (_swapchain.UpdateFrameBufferIndex(_image_available_sem, needUpdate) == false) {
						return false;
					}
					if (needUpdate) {
						return false;
					}
				} else {
					return false;
				}
			}

			if (_swapchain.AddCommandBindRenderpass(_cmd_buffer) == false) {
				return false;
			}
		}
		if(first || _draw_state.cull != obj.GetBackFaceCulling()) {
			_draw_state.cull = obj.GetBackFaceCulling();
			if(addCommandSetCullMode(_cmd_buffer, _draw_state.cull) == false) {
				return false;
			}
		}
		if(first || _draw_state.vp != cam.GetCameraViewPort()) {
			_draw_state.vp = cam.GetCameraViewPort();
			if (addCommandBindViewPort(_cmd_buffer, cam) == false) {
				return false;
			}
		}
		if(first || _draw_state.pipeline != pipelineID) {
			_draw_state.pipeline = pipelineID;
			if (_pipelines[pipelineID].AddCommandBindPipeline(_cmd_buffer) == false) {
				return false;
			}
		}
		//bind descriptor set
		if (_pipelines[pipelineID].AddCommandBindUniformBufferSet(_cmd_buffer, obj, cam) == false) {
			return false;
		}
		//draw call
		if (obj.GetMesh()._internal->AddCommandDrawMesh(_cmd_buffer, obj.GetNumVertIndices()) == false) {
			return false;
		}
		return true;
	}
	return false;
}

bool Renderer::RendererInternal::PresentFrame() {
	if (_init) {
		if (_draw_state.startPass == true) {
			return true;
		}

		if (submitGraphicsCommands(true) == false) {
			return false;
		}

		if (_swapchain.PresentFrame(_render_complete_sem) == false) {
			return false;
		}

		for (auto& pipeline : _pipelines) {
			pipeline.EndRenderPass();
		}
		_draw_state.startPass = true;
		return true;
	}
	return false;
}

bool Renderer::RendererInternal::CreatePipeline(const PipelineConfig& config, unsigned& pipelineID) {
	unsigned idx = 0;
	for (auto& pipeline : _pipelines) {
		if (pipeline.IsReady() == false) {
			if (pipeline.Initialize(_dev_id, config, _swapchain.GetRenderPass()) == false) {
				return false;
			}
			pipeline.SetLightDir(Vec<3>({0,0,1}));
			pipeline.SetLightColour(Vec<4>({1,1,1,1}));
			pipeline.SetLightIntensity(1);
			pipeline.SetAmbientLightIntensity(0.1);
			
			pipelineID = idx;
			return true;
		}
		idx++;
	}

	_pipelines.push_back(Pipeline());
	if (_pipelines.back().Initialize(_dev_id, config, _swapchain.GetRenderPass()) == false) {
		
		return false;
	}
		
	pipelineID = _pipelines.size() - 1;
	_pipelines[pipelineID].SetLightDir(Vec<3>({0,0,1}));
	_pipelines[pipelineID].SetLightColour(Vec<4>({1,1,1,1}));
	_pipelines[pipelineID].SetLightIntensity(1);
	_pipelines[pipelineID].SetAmbientLightIntensity(0.1);

	return true;
}

unsigned Renderer::RendererInternal::GetDeviceID() const {
	return _dev_id;
}

bool Renderer::RendererInternal::addCommandSetCullMode(VkCommandBuffer cmdBuffer, bool cull) {
	if (_init) {
		if(cull)vkCmdSetCullMode(cmdBuffer, VK_CULL_MODE_BACK_BIT);
		else vkCmdSetCullMode(cmdBuffer, VK_CULL_MODE_NONE);
		return true;
	}
	return false;
}

bool Renderer::RendererInternal::addCommandBindViewPort(VkCommandBuffer cmdBuffer, const Camera& cam) {
	if (_init) {
		const auto& vp = cam.GetCameraViewPort();
		VkViewport viewport{};
		viewport.x = static_cast<float>(vp.posX);
		viewport.y = static_cast<float>(vp.posY);
		viewport.width = static_cast<float>(vp.width);
		viewport.height = static_cast<float>(vp.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0;
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { static_cast<int32_t>(vp.posX), static_cast<int32_t>(vp.posY) };
		scissor.extent.width = vp.width;
		scissor.extent.height = vp.height;
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
		return true;
	}
	return false;
}

bool Renderer::RendererInternal::submitGraphicsCommands(bool wait_for_image) {
	if (_init) {
		vkCmdEndRenderPass(_cmd_buffer);
		
		if (vkEndCommandBuffer(_cmd_buffer) != VK_SUCCESS) {
			printf("failed to close command buffer\n");
			return false;
		}

		VkSemaphore sem = VK_NULL_HANDLE;
		if (wait_for_image) {
			sem = _render_complete_sem;
		}

		if (DeviceManager::SubmitCommandBuffer(_dev_id, DeviceManager::QUEUE_TYPE_GRAPHICS, _cmd_buffer, _image_available_sem, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, sem, true) == false) {
			return false;
		}
		return true;
	}
	return false;
}

bool Renderer::RendererInternal::commandBufferStart() {
	if (_init) {
		if (vkResetCommandBuffer(_cmd_buffer, 0) != VK_SUCCESS) {
			return false;
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional
		if (vkBeginCommandBuffer(_cmd_buffer, &beginInfo) != VK_SUCCESS) {
			return false;
		}
		
		return true;
	}
	return false;
}
}
