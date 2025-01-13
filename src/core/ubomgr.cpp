#include "ubomgr.h"

#define DESCRIPTORSET_POOLSIZE 100



namespace RenderingFramework3D {

UniformBufferAllocator::UniformBufferAllocator(unsigned poolsize) 
	:
	_init(false),
	_vk_pool_obj(),
	_obj_sets(),
	_pool_size(poolsize),
	_available(),
	_vk_pool_global(VK_NULL_HANDLE),
	_global_set(),
	_layout(),
	_dev_id()
{}

bool UniformBufferAllocator::Initialize(unsigned dev, const UniformShaderInputLayoutInternal& layout) {
	if (_init == false) {
		_dev_id = dev;
		_layout = layout;
		_init = true;
		if (createGlobalSet() == false) {
			return false;
		}
		return addNewPool();
	}
	return true;
}

bool UniformBufferAllocator::Cleanup() {
	if (_init) {
		//deallocate all vk_pool
		VkDevice dev = DeviceManager::GetVkDevice(_dev_id);
		if (dev == VK_NULL_HANDLE) {
			return false;
		}

		for (auto& pool : _vk_pool_obj) {
			vkDestroyDescriptorPool(dev, pool, nullptr);
			pool = VK_NULL_HANDLE;
		}

		_vk_pool_obj.clear();
		_obj_sets.clear();
		while (_available.size()) {
			_available.pop();
		}
		_init = false;
	}
	return true;
}

bool UniformBufferAllocator::AllocateObjectUniformBufferSet(unsigned& id) {
	if (_init) {
		if (_available.size()) {
			unsigned available_id = _available.top();
			_available.pop();
			unsigned pool_idx = available_id / _pool_size;
			unsigned desc_idx = available_id % _pool_size;

			_obj_sets[pool_idx][desc_idx].used = true;
			id = available_id;
		} else {
			unsigned pool_idx = _vk_pool_obj.size() - 1;
			id = pool_idx * _pool_size;
			if (addNewPool() == false) {
				return false;
			}
		}
		return true;
	}
	return false;
}
void UniformBufferAllocator::FreeObjectUniformBufferSet(unsigned id) {
	if (id < (_pool_size * _obj_sets.size())) {
		unsigned pool_idx = id / _pool_size;
		unsigned desc_idx = id % _pool_size;

		if (_obj_sets[pool_idx][desc_idx].used) {
			_obj_sets[pool_idx][desc_idx].used = false;
			_available.push(id);
		}
	}
}

void UniformBufferAllocator::FreeAllObjectUniformBufferSet() {
	unsigned pool_size = _pool_size * _obj_sets.size();
	for (int id = 0; id < _pool_size * _obj_sets.size(); id++) {
		unsigned pool_idx = id / _pool_size;
		unsigned desc_idx = id % _pool_size;

		if (_obj_sets[pool_idx][desc_idx].used) {
			_obj_sets[pool_idx][desc_idx].used = false;
			_available.push(id);
		}
		if (_available.size() >= pool_size) {
			return;
		}
	}
}


bool UniformBufferAllocator::AddCommandBindUniformBufferSet(unsigned id, VkPipelineLayout pipelineLayout, VkCommandBuffer cmdBuffer) {
	if (_init) {
		if (id >= (_pool_size * _obj_sets.size())) {
			return false;
		}
		unsigned pool_idx = id / _pool_size;
		unsigned desc_idx = id % _pool_size;

		if (_obj_sets[pool_idx][desc_idx].used == false) {
			return false;
		}

		std::array<VkDescriptorSet,2> sets = { _obj_sets[pool_idx][desc_idx].vkdesc, _global_set.vkdesc };
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, sets.size(), sets.data(), 0, nullptr);
		return true;
	}
	return false;
}

void* UniformBufferAllocator::GetObjectUniformBuffer(unsigned id, ObjectUniformBufferType type, unsigned& size, unsigned custom_idx) {
	size = 0;
	if (id < (_pool_size * _obj_sets.size())) {

		unsigned pool_idx = id / _pool_size;
		unsigned desc_idx = id % _pool_size;

		if (_obj_sets[pool_idx][desc_idx].used) {
			if (type == OBJ_UB_TYPE_TRANSFORM) {
				size = _obj_sets[pool_idx][desc_idx].transformBuffer.size;
				return _obj_sets[pool_idx][desc_idx].transformBuffer.mappedBuffer;
			}
			else if (type == OBJ_UB_TYPE_MATERIAL) {
				size = _obj_sets[pool_idx][desc_idx].materialBuffer.size;
				return _obj_sets[pool_idx][desc_idx].materialBuffer.mappedBuffer;
			}
			else if (type == OBJ_UB_TYPE_CAM) {
				size = _obj_sets[pool_idx][desc_idx].camTransformBuffer.size;
				return _obj_sets[pool_idx][desc_idx].camTransformBuffer.mappedBuffer;
			}
			else if (_obj_sets[pool_idx][desc_idx].customBuffers.find(custom_idx) != _obj_sets[pool_idx][desc_idx].customBuffers.end()) {
				size = _obj_sets[pool_idx][desc_idx].customBuffers[custom_idx].size;
				return _obj_sets[pool_idx][desc_idx].customBuffers[custom_idx].mappedBuffer;
			}
		}
	}
	return nullptr;
}
void* UniformBufferAllocator::GetGlobalUniformBuffer(GlobalUniformBufferType type, unsigned& size, unsigned custom_idx) {
	size = 0;
	if (type == GLOB_UB_TYPE_DIRLIGHT) {
		size = _global_set.dirlightBuffer.size;
		return _global_set.dirlightBuffer.mappedBuffer;
	} else if (_global_set.customBuffers.find(custom_idx) != _global_set.customBuffers.end()) {
		size = _global_set.customBuffers[custom_idx].size;
		return _global_set.customBuffers[custom_idx].mappedBuffer;
	}
	return nullptr;
}

static bool allocateDescriptorBuffer(unsigned devId, VkDescriptorSet descSet, unsigned bindSlot, unsigned bufferSize,
	VkBuffer& buffer, VkDeviceMemory& bufferMemory, void** bufferMapped);

bool UniformBufferAllocator::addNewPool() {
	if (_init) {
		unsigned descCount = _layout.layout.ObjectInputs.CustomUniformShaderInput.size() + 3;
		std::vector<VkDescriptorPoolSize> descPoolSizeList;
		descPoolSizeList.reserve(descCount);

		if (_layout.layout.ObjectInputs.useObjToScreenTransform ||
			_layout.layout.ObjectInputs.useObjToWorldTransform ||
			_layout.layout.ObjectInputs.useWorldToCamTransform ||
			_layout.layout.ObjectInputs.useCamToScreenTransform) {

			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = _pool_size;
			descPoolSizeList.emplace_back(poolSize);

		}
		if (_layout.layout.ObjectInputs.useMaterialData) {
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = _pool_size;
			descPoolSizeList.emplace_back(poolSize);
		}
		if (_layout.layout.ObjectInputs.useCamTransform) {
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = _pool_size;
			descPoolSizeList.emplace_back(poolSize);
		}

		for (auto& d : _layout.layout.ObjectInputs.CustomUniformShaderInput) {
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = _pool_size;
			descPoolSizeList.emplace_back(poolSize);
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = descPoolSizeList.size();
		poolInfo.pPoolSizes = descPoolSizeList.data();
		poolInfo.maxSets = _pool_size;

		VkDescriptorPool pool;
		VkDevice dev = DeviceManager::GetVkDevice(_dev_id);

		if (dev == VK_NULL_HANDLE) {
			return false;
		}

		if (vkCreateDescriptorPool(dev, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
			return false;
		}

		//allocate vulkan descriptor pool vk_pool
		_vk_pool_obj.push_back(pool);
		_obj_sets.push_back(std::vector<ObjectUniformBufferSet>(_pool_size));

		unsigned pool_idx = _vk_pool_obj.size() - 1;
		unsigned idx = 0;
		for (auto& set : _obj_sets[pool_idx]) {
			//allocate vulkan descriptor(vkdesc) using vk_pool
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = pool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &_layout.vklayoutobject;
			if (vkAllocateDescriptorSets(dev, &allocInfo, &set.vkdesc) != VK_SUCCESS) {
				return false;
			}

			//allocate vk buffers and map mapped_buffer
			unsigned bufferSize = 0;
			if (_layout.layout.ObjectInputs.useObjToScreenTransform) {
				bufferSize += sizeof(float) * 16;
			}
			if (_layout.layout.ObjectInputs.useObjToWorldTransform) {
				bufferSize += sizeof(float) * 16;
			}
			if (_layout.layout.ObjectInputs.useWorldToCamTransform) {
				bufferSize += sizeof(float) * 16;
			}
			if (_layout.layout.ObjectInputs.useCamToScreenTransform) {
				bufferSize += sizeof(float) * 16;
			}
			if (_layout.layout.ObjectInputs.useObjectScale) {
				bufferSize += sizeof(float) * 4;
			}

			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
			void* bufferMapped = nullptr;

			if (bufferSize) {
				if (allocateDescriptorBuffer(_dev_id, set.vkdesc, _layout.layout.ObjectInputs.transformBindSlot, bufferSize,
					buffer, bufferMemory, &bufferMapped) == false) {
					set.transformBuffer.buffer.vkBuffer = VK_NULL_HANDLE;
					set.transformBuffer.buffer.vkBufferMem = VK_NULL_HANDLE;
					return false;
				}

				set.transformBuffer.bindSlot = _layout.layout.ObjectInputs.transformBindSlot;
				set.transformBuffer.size = bufferSize;
				set.transformBuffer.buffer.vkBuffer = buffer;
				set.transformBuffer.buffer.vkBufferMem = bufferMemory;
				set.transformBuffer.mappedBuffer = bufferMapped;
			}

			if (_layout.layout.ObjectInputs.useMaterialData) {
				bufferSize = sizeof(float) * 8;
				if (bufferSize && allocateDescriptorBuffer(_dev_id, set.vkdesc, _layout.layout.ObjectInputs.materialDataBindSlot, bufferSize,
					buffer, bufferMemory, &bufferMapped) == false) {
					set.materialBuffer.buffer.vkBuffer = VK_NULL_HANDLE;
					set.materialBuffer.buffer.vkBufferMem = VK_NULL_HANDLE;
					return false;
				}
				set.materialBuffer.bindSlot = _layout.layout.ObjectInputs.materialDataBindSlot;
				set.materialBuffer.size = bufferSize;
				set.materialBuffer.buffer.vkBuffer = buffer;
				set.materialBuffer.buffer.vkBufferMem = bufferMemory;
				set.materialBuffer.mappedBuffer = bufferMapped;
			}
			if (_layout.layout.ObjectInputs.useCamTransform) {
				bufferSize = sizeof(float) * 16;
				if (bufferSize && allocateDescriptorBuffer(_dev_id, set.vkdesc, _layout.layout.ObjectInputs.camTransformBindSlot, bufferSize,
					buffer, bufferMemory, &bufferMapped) == false) {
					set.camTransformBuffer.buffer.vkBuffer = VK_NULL_HANDLE;
					set.camTransformBuffer.buffer.vkBufferMem = VK_NULL_HANDLE;
					return false;
				}
				set.camTransformBuffer.bindSlot = _layout.layout.ObjectInputs.camTransformBindSlot;
				set.camTransformBuffer.size = bufferSize;
				set.camTransformBuffer.buffer.vkBuffer = buffer;
				set.camTransformBuffer.buffer.vkBufferMem = bufferMemory;
				set.camTransformBuffer.mappedBuffer = bufferMapped;
			}

			if (_layout.layout.ObjectInputs.CustomUniformShaderInput.size()) {
				for (auto& desc : _layout.layout.ObjectInputs.CustomUniformShaderInput) {
					buffer = VK_NULL_HANDLE;
					bufferMemory = VK_NULL_HANDLE;
					bufferMapped = nullptr;
					if (desc.size && allocateDescriptorBuffer(_dev_id, set.vkdesc, desc.bindSlot, desc.size,
						buffer, bufferMemory, &bufferMapped) == false) {
						return false;
					}
					set.customBuffers[desc.bindSlot].bindSlot = desc.bindSlot;
					set.customBuffers[desc.bindSlot].size = bufferSize;
					set.customBuffers[desc.bindSlot].buffer.vkBufferMem = bufferMemory;
					set.customBuffers[desc.bindSlot].buffer.vkBuffer = buffer;
					set.customBuffers[desc.bindSlot].mappedBuffer = bufferMapped;
				}
			}
			set.used = false;
			_available.push(idx + pool_idx * _pool_size);
			idx++;
		}
		return true;
	} 
	return false;
}


static bool allocateDescriptorBuffer(unsigned devId, VkDescriptorSet descSet, unsigned bindSlot, unsigned bufferSize,
							  VkBuffer& buffer, VkDeviceMemory& bufferMemory, void** bufferMapped) {
	VkDevice dev = DeviceManager::GetVkDevice(devId);
	if (dev == VK_NULL_HANDLE) {
		return false;
	}
	VkPhysicalDevice physdev = DeviceManager::GetVkPhyDevice(devId);
	if (physdev == VK_NULL_HANDLE) {
		return false;
	}

	if (createBuffer(physdev, dev, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory) == false) {
		return false;
	}
	if (vkMapMemory(dev, bufferMemory, 0, bufferSize, 0, bufferMapped) != VK_SUCCESS) {
		printf("failed to map ubo memory");
		return false;
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = bufferSize;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descSet;
	descriptorWrite.dstBinding = bindSlot;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	vkUpdateDescriptorSets(dev, 1, &descriptorWrite, 0, nullptr);

	return true;
}


bool UniformBufferAllocator::createGlobalSet() {
	if (_init == false) {
		return false;
	}
	unsigned descCount = _layout.layout.GlobalInputs.CustomUniformShaderInput.size() + 1;
	std::vector<VkDescriptorPoolSize> descPoolSizeList;
	descPoolSizeList.reserve(descCount);

	if (_layout.layout.GlobalInputs.useDirectionalLight) {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 1;
		descPoolSizeList.emplace_back(poolSize);
	}

	for (auto& d : _layout.layout.GlobalInputs.CustomUniformShaderInput) {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 1;
		descPoolSizeList.emplace_back(poolSize);
	}

	if (descPoolSizeList.size() <= 0) {
		_global_set.used = false;
		return true;
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = descPoolSizeList.size();
	poolInfo.pPoolSizes = descPoolSizeList.data();
	poolInfo.maxSets = 1;

	VkDevice dev = DeviceManager::GetVkDevice(_dev_id);

	if (dev == VK_NULL_HANDLE) {
		return false;
	}
	
	if (vkCreateDescriptorPool(dev, &poolInfo, nullptr, &_vk_pool_global) != VK_SUCCESS) {
		return false;
	}
	
	//allocate vulkan descriptor(vkdesc) using vk_pool
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _vk_pool_global;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_layout.vklayoutglobal;
	if (vkAllocateDescriptorSets(dev, &allocInfo, &_global_set.vkdesc) != VK_SUCCESS) {
		return false;
	}

	//allocate vk buffers and map mapped_buffer
	unsigned bufferSize = 0;
	if (_layout.layout.GlobalInputs.useDirectionalLight) {
		bufferSize += sizeof(float)*36;
	}

	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
	void* bufferMapped = nullptr;

	if (bufferSize && allocateDescriptorBuffer(_dev_id, _global_set.vkdesc, _layout.layout.GlobalInputs.dirLightBindSlot, bufferSize,
		buffer, bufferMemory, &bufferMapped) == false) {
		_global_set.dirlightBuffer.buffer.vkBuffer = VK_NULL_HANDLE;
		_global_set.dirlightBuffer.buffer.vkBufferMem = VK_NULL_HANDLE;
		return false;
	}

	_global_set.dirlightBuffer.bindSlot = _layout.layout.GlobalInputs.dirLightBindSlot;
	_global_set.dirlightBuffer.size = bufferSize;
	_global_set.dirlightBuffer.buffer.vkBuffer = buffer;
	_global_set.dirlightBuffer.buffer.vkBufferMem = bufferMemory;
	_global_set.dirlightBuffer.mappedBuffer = bufferMapped;


	for (auto& desc : _layout.layout.GlobalInputs.CustomUniformShaderInput) {
		buffer = VK_NULL_HANDLE;
		bufferMemory = VK_NULL_HANDLE;
		bufferMapped = nullptr;
		if (desc.size && allocateDescriptorBuffer(_dev_id, _global_set.vkdesc, desc.bindSlot, desc.size,
			buffer, bufferMemory, &bufferMapped) == false) {
			return false;
		}
		_global_set.customBuffers[desc.bindSlot].bindSlot = desc.bindSlot;
		_global_set.customBuffers[desc.bindSlot].size = desc.size;
		_global_set.customBuffers[desc.bindSlot].buffer.vkBufferMem = bufferMemory;
		_global_set.customBuffers[desc.bindSlot].buffer.vkBuffer = buffer;
		_global_set.customBuffers[desc.bindSlot].mappedBuffer = bufferMapped;
	}

	_global_set.used = false;
	return true;
}
}


