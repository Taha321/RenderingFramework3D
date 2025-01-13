#pragma once
#include <mutex>
#include <fstream>
#include <iostream>
#include <array>
#include <vector>
#include "types_internal.h"


namespace RenderingFramework3D {

bool findMemoryType(VkPhysicalDevice physdev, uint32_t typeFilter, VkMemoryPropertyFlags properties, unsigned& idx);
bool createBuffer(VkPhysicalDevice physdev, VkDevice dev, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
bool createImage(VkPhysicalDevice physdev, VkDevice dev, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
bool createImageView(VkDevice dev, VkImage img, VkFormat fmt, VkImageAspectFlags aspectFlags, VkImageView& imgView);
VkShaderModule createShaderModule(VkDevice dev, std::vector<uint8_t> code);
std::vector<uint8_t> readFile(const std::string& filename);
VkFormat getVkFormat(GLSLType type, unsigned components);
unsigned getVertDataSize(GLSLType type, unsigned components);
}


