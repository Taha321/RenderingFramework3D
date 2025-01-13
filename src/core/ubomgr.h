#pragma once
#include <stdint.h>
#include <vector>
#include <stack>
#include <set>
#include "util.h"
#include "devicemgr.h"


namespace RenderingFramework3D {

enum ObjectUniformBufferType {
	OBJ_UB_TYPE_TRANSFORM,
	OBJ_UB_TYPE_MATERIAL,
	OBJ_UB_TYPE_CAM,
	OBJ_UB_TYPE_CUSTOM
};

enum GlobalUniformBufferType {
	GLOB_UB_TYPE_DIRLIGHT,
	GLOB_UB_TYPE_CUSTOM
};

class UniformBufferAllocator {
public:
	UniformBufferAllocator(unsigned poolsize = 100);   
	bool Initialize(unsigned dev, const UniformShaderInputLayoutInternal& layout);
	bool Cleanup();
	
	bool AllocateObjectUniformBufferSet(unsigned& id);
	void FreeObjectUniformBufferSet(unsigned id);
	void FreeAllObjectUniformBufferSet();

	bool AddCommandBindUniformBufferSet(unsigned id, VkPipelineLayout pipelineLayout, VkCommandBuffer cmdBuffer);

	void* GetObjectUniformBuffer(unsigned id, ObjectUniformBufferType type, unsigned& size, unsigned custom_idx = 0);
	void* GetGlobalUniformBuffer(GlobalUniformBufferType type, unsigned& size, unsigned custom_idx = 0);

private:
	bool addNewPool();
	bool createGlobalSet();

private:
	struct DescriptorSetBufferResources {
		unsigned bindSlot;
		unsigned size;
		BufferResources buffer;
		void* mappedBuffer;
	};

	struct ObjectUniformBufferSet {
		bool used = false;
		VkDescriptorSet vkdesc = VK_NULL_HANDLE;
		DescriptorSetBufferResources transformBuffer;
		DescriptorSetBufferResources materialBuffer;
		DescriptorSetBufferResources camTransformBuffer;
		std::unordered_map<unsigned, DescriptorSetBufferResources> customBuffers;
	};

	struct GlobalUniformBufferSet {
		bool used = false;
		VkDescriptorSet vkdesc = VK_NULL_HANDLE;
		DescriptorSetBufferResources dirlightBuffer;
		std::unordered_map<unsigned, DescriptorSetBufferResources> customBuffers;
	};

private:
	bool _init;
	std::vector<VkDescriptorPool> _vk_pool_obj;
	std::vector<std::vector<ObjectUniformBufferSet>> _obj_sets;
	unsigned _pool_size;
	std::stack<unsigned> _available;

	VkDescriptorPool _vk_pool_global;
	GlobalUniformBufferSet _global_set;

	UniformShaderInputLayoutInternal _layout;
	unsigned _dev_id;
};
}
