#pragma once
#include "util.h"
#include "devicemgr.h"


namespace RenderingFramework3D {

struct SwapChainConfig {
	VkSurfaceKHR surface;
	VkExtent2D extent;
	SwapChainSupportDetails swchainSupport;
};

class Swapchain
{
public:
	Swapchain();

	bool Initialize(unsigned dev, SwapChainConfig config);
	bool Cleanup();
	bool UpdateSwapChain(VkExtent2D extent);

	VkRenderPass GetRenderPass();

	bool UpdateFrameBufferIndex(VkSemaphore imageAvailableSem, bool& needUpdate);
	bool AddCommandBindRenderpass(VkCommandBuffer cmdBuffer);

	bool PresentFrame(VkSemaphore waitSem);

private:
	bool createSwapchain();
	bool createImageViews();
	bool createRenderPass();
	bool createFrameBuffers();

	VkSurfaceFormatKHR chooseFormat();
	VkPresentModeKHR choosePresentMode();

	bool cleanupSwapchain();
	bool destroyRenderPass();

private:
	bool _init;

	VkExtent2D _extent;
	VkSurfaceKHR _surface;

	SwapChainSupportDetails _support;
	VkSurfaceFormatKHR _surface_format;
	VkPresentModeKHR _present_mode;

	std::vector<VkImage> _swapchain_images;
	std::vector<VkImageView> _swapchain_imageviews;
	std::vector<VkFramebuffer> _swapchain_framebuffers;

	ImageResources _depth_image;
	VkImageView _depth_imageview;

	VkRenderPass _render_pass;

	VkSwapchainKHR _swapchain;

	unsigned _current_image_index;

	unsigned _dev_id;
};
}
