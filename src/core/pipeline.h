#pragma once
#include <vector>

#include "util.h"
#include "types_internal.h"
#include "swpchain.h"
#include "ubomgr.h"
#include "camera.h"
#include "worldobj.h"

namespace RenderingFramework3D{
class Pipeline
{
public:
	Pipeline();
	~Pipeline();

	bool Initialize(unsigned dev, const PipelineConfig& config, VkRenderPass renderPass);
	bool Cleanup();

	bool IsReady();

	bool SetLightDir(const MathUtil::Vec<3>& lightDir);
	bool SetLightColour(const MathUtil::Vec<4>& lightColour);
	bool SetLightIntensity(float intensity);
	bool SetAmbientLightIntensity(float intensity);

	bool SetCustomGlobalData(unsigned binding, void* data, unsigned size, unsigned offset);
	

	const UniformShaderInputLayout& GetUniformBufferSetLayout() const;
	
	bool AddCommandBindPipeline(VkCommandBuffer cmdBuffer);
	bool AddCommandBindUniformBufferSet(VkCommandBuffer cmdBuffer, const WorldObject& obj, Camera& cam);

	bool EndRenderPass();

private:
	bool createPipeline(const PipelineConfig& config, VkRenderPass renderPass);
	void createVertexInputInfo(const PipelineConfig& config, const std::vector<CustomVertInputLayout>& customSorted, VkVertexInputBindingDescription& bindingDescription, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);
	void createObjectUniformBufferBindList(std::vector<VkDescriptorSetLayoutBinding>& uboLayoutBindingList);
	void createGlobalUniformBufferBindList(std::vector<VkDescriptorSetLayoutBinding>& uboLayoutBindingList);
	bool createDescriptorSetLayout(VkDevice dev, const std::vector <VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayout& vkdesclayout);

private:
	bool _init;
	VkPipelineLayout _pipelinelayout;
	VkPipeline _graphics_pipeline;

	UniformShaderInputLayoutInternal _uniform_shader_input_layout;

	UniformBufferAllocator _ubo_allocator;

	unsigned _dev_id;
};
}