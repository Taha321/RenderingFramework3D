#pragma once
#include "types_internal.h"

namespace RenderingFramework3D {

class DeviceManager
{
public:
	enum QueueType {
		QUEUE_TYPE_GRAPHICS,
		QUEUE_TYPE_PRESENT,
	}; 

public:
	static bool Initialize();
	static void Cleanup();

	static VkInstance GetVkInstance();
	static VkDevice GetVkDevice(unsigned devID);
	static VkPhysicalDevice GetVkPhyDevice(unsigned  devID);

	static bool GetQueueIdx(unsigned devID, QueueType queue, unsigned& idx);
	static VkQueue GetVkQueue(unsigned devID, QueueType queue);
	static bool CreateCommandBuffer(unsigned  devID, QueueType queue, bool primary, VkCommandBuffer& buffer);
	
	static bool CheckQueueReady(unsigned  devID, QueueType queue, bool& ready);
	static bool WaitForQueue(unsigned  devID, QueueType queue);
	static bool SubmitCommandBuffer(unsigned  devID, QueueType queueType, VkCommandBuffer buffer, VkSemaphore waitSem, VkPipelineStageFlags waitStage, VkSemaphore signalSem, bool block=true);

	static bool CreateVkSurface(GLFWwindow* window, VkSurfaceKHR& surface);
	static bool FindSuitableDevice(VkSurfaceKHR surface, unsigned & devID, SwapChainSupportDetails& swapchainSupport);
	
	static bool CopyBuffer(unsigned devID, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

private:
	DeviceManager();
	~DeviceManager();

	bool createVKInstance();
	bool enlistPhysicalDevices();
	bool createLogicalDevice(unsigned devIdx, unsigned numIdx);
	bool createCommandPool(unsigned devIdx, unsigned queueIdx, bool primary);
	bool createCommandBuffer(unsigned devIdx, unsigned queueIdx, bool primary, VkCommandBuffer& buffer);

	static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, unsigned& gfxQueueIndex, unsigned& presentQueueIndex, unsigned& numIdx, SwapChainSupportDetails& swapchainSupport);

private:
	struct DeviceQueue {
		unsigned index;
		VkQueue vkqueue;
		VkFence fence;
		VkCommandPool cmdpool;
		VkCommandPool secondarypool;
	};
	struct Device {
		VkPhysicalDevice physDev;
		VkDevice logicalDev;
		std::vector<DeviceQueue> queues;
		VkCommandBuffer loadCmdBuffer;
		unsigned gfxQueueIdx;
		unsigned presentQueueIdx;
	};

	std::vector<Device> _devices;

	VkInstance _vk_instance;
};
}

