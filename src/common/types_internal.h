#pragma once
#include <vector>
#include "vulkaninc.h"
#include "types.h"


namespace RenderingFramework3D {

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct UniformShaderInputLayoutInternal {
	UniformShaderInputLayout layout;
	VkDescriptorSetLayout vklayoutobject;
	VkDescriptorSetLayout vklayoutglobal;
};

struct BufferResources {
	VkDeviceMemory vkBufferMem;
	VkBuffer vkBuffer;
};

struct ImageResources {
	VkDeviceMemory vkImgMem;
	VkImage vkImage;
};
}