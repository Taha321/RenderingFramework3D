#include "util.h"


namespace RenderingFramework3D {

bool findMemoryType(VkPhysicalDevice physdev, uint32_t typeFilter, VkMemoryPropertyFlags properties, unsigned& idx) {
	VkPhysicalDeviceMemoryProperties memProperties;

	vkGetPhysicalDeviceMemoryProperties(physdev, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			idx = i;
			return true;
		}
	}

	return false;
}

bool createBuffer(VkPhysicalDevice physdev, VkDevice dev, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(dev, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(dev, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	if (findMemoryType(physdev, memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex) == false) {
		return false;
	}

	if (vkAllocateMemory(dev, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		return false;
	}
	if (vkBindBufferMemory(dev, buffer, bufferMemory, 0) != VK_SUCCESS) {
		return false;
	}
	return true;
}

bool createImage(VkPhysicalDevice physdev, VkDevice dev, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


	if (vkCreateImage(dev, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(dev, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	if (findMemoryType(physdev, memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex) == false) {
		return false;
	}

	if (vkAllocateMemory(dev, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		return false;
	}

	vkBindImageMemory(dev, image, imageMemory, 0);
	return true;
}

bool createImageView(VkDevice dev, VkImage img, VkFormat fmt, VkImageAspectFlags aspectFlags, VkImageView& imgView) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = img;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = fmt;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(dev, &viewInfo, nullptr, &imgView) != VK_SUCCESS) {
		return false;
	}
	return true;
}

VkShaderModule createShaderModule(VkDevice dev, std::vector<uint8_t> code) {

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;

	if (vkCreateShaderModule(dev, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}

std::vector<uint8_t> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		printf("file %s not found\n", filename.c_str());
		throw std::runtime_error("failed to open file!");
	}

	size_t  size = (size_t)file.tellg();
	std::vector<uint8_t> buffer(size);
	file.seekg(0);
	file.read((char*)(buffer.data()), size);
	file.close();
	return buffer;
}

VkFormat getVkFormat(GLSLType type, unsigned components) {
	if (components > 4) components = 4;
	else if (components < 1) components = 1;

	if (type == GLSL_BOOL || type == GLSL_UINT) {
		VkFormat list[] = { VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32A32_UINT };

		return list[components - 1];
	}
	else if (type == GLSL_INT) {
		VkFormat list[] = { VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT };

		return list[components - 1];
	}
	else if (type == GLSL_FLOAT) {
		VkFormat list[] = { VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT };

		return list[components - 1];
	}
	else if (type == GLSL_DOUBLE) {
		VkFormat list[] = { VK_FORMAT_R64_SFLOAT, VK_FORMAT_R64G64_SFLOAT, VK_FORMAT_R64G64B64_SFLOAT, VK_FORMAT_R64G64B64A64_SFLOAT };
		return list[components - 1];
	}

	return VK_FORMAT_UNDEFINED;
}

unsigned getVertDataSize(GLSLType type, unsigned components) {
	if (components > 4) components = 4;
	else if (components < 1) components = 1;

	if (type == GLSL_DOUBLE) {
		return 8 * components;
	} else {
		return 4 * components;
	}
}

}
