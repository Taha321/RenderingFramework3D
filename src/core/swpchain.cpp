#include "swpchain.h"
#include "devicemgr.h"


namespace RenderingFramework3D {

Swapchain::Swapchain()
    :
    _init(false),
    _extent(),
    _surface(VK_NULL_HANDLE),
    _support(),
    _surface_format(),
    _present_mode(),
    _swapchain_images(),
    _swapchain_imageviews(),
    _swapchain_framebuffers(),
    _current_image_index(0),
    _render_pass(VK_NULL_HANDLE),
    _swapchain(VK_NULL_HANDLE)
{}

bool Swapchain::Initialize(unsigned dev, SwapChainConfig config) {
    _dev_id = dev;
    _surface = config.surface;
    _extent = config.extent;
    _support = config.swchainSupport;

    _init = true;
    _init = _init && createSwapchain();
    _init = _init && createImageViews();
    _init = _init && createRenderPass();
    _init = _init && createFrameBuffers();

    return _init;
}


bool Swapchain::Cleanup() {
    if (cleanupSwapchain() == false) {
        return false;
    }
    if (destroyRenderPass() == false) {
        return false;
    }
    return true;
}

bool Swapchain::UpdateSwapChain(VkExtent2D extent) {
    _extent = extent;
    if (cleanupSwapchain() == false) {
        return false;
    }
    if (createSwapchain() == false) {
        return false;
    }
    if (createImageViews() == false) {
        return false;
    }
    if (createFrameBuffers() == false) {
        return false;
    }
    return true;
}

VkRenderPass Swapchain::GetRenderPass() {
    return _render_pass;
}

bool Swapchain::UpdateFrameBufferIndex(VkSemaphore imageAvailableSem, bool& needUpdate) {
    if (_init) {
        VkDevice dev = DeviceManager::GetVkDevice(_dev_id);
        if (dev == VK_NULL_HANDLE) {
            return false;
        }
        unsigned imageIndex=0;
        VkResult result = vkAcquireNextImageKHR(dev, _swapchain, UINT64_MAX, imageAvailableSem, VK_NULL_HANDLE, &_current_image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            needUpdate = true;
            return true;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            return false;
        }
        needUpdate = false;
        return true;
    }
    return false;
}


bool Swapchain::AddCommandBindRenderpass(VkCommandBuffer cmdBuffer) {
    if (_init) {

        //if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        //    cleanupSwapchain();
        //    createSwapChain();
        //    return false;
        //}
        //else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        //    printf("failed to acquire swap chain image!");
        //    return false;
        //}
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _render_pass;
        renderPassInfo.framebuffer = _swapchain_framebuffers[_current_image_index];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = _extent;

        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = { 1.0f,0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        return true;
    }
    return false;
}

bool Swapchain::PresentFrame(VkSemaphore waitSem) {
    if (_init) {
        VkSemaphore waitSemaphores[] = { waitSem };
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = waitSemaphores;
        VkSwapchainKHR swapChains[] = { _swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &_current_image_index;
        presentInfo.pResults = nullptr;
        
        VkQueue vkqueue = DeviceManager::GetVkQueue(_dev_id, DeviceManager::QUEUE_TYPE_GRAPHICS);
        if (vkqueue == VK_NULL_HANDLE) {
            return false;
        }
        
        vkQueuePresentKHR(vkqueue, &presentInfo);
        return true;
    }
    return false;
}

bool Swapchain::createSwapchain() {
    uint32_t count = 0;

    _surface_format = chooseFormat();
    _present_mode = choosePresentMode();

    auto physdev = DeviceManager::GetVkPhyDevice(_dev_id);
    if(physdev == VK_NULL_HANDLE) {
        return false;
    }
    auto dev = DeviceManager::GetVkDevice(_dev_id);
    if (dev == VK_NULL_HANDLE) {
        return false;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = _surface;
    swapChainCreateInfo.minImageCount = _support.capabilities.minImageCount + 1;
    swapChainCreateInfo.imageFormat = _surface_format.format;
    swapChainCreateInfo.imageColorSpace = _surface_format.colorSpace;
    swapChainCreateInfo.imageExtent = _extent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    unsigned presQueue;
    unsigned gfxQueue;
    
    if(DeviceManager::GetQueueIdx(_dev_id, DeviceManager::QUEUE_TYPE_PRESENT, presQueue) == false) {
        return false;
    }
    if(DeviceManager::GetQueueIdx(_dev_id, DeviceManager::QUEUE_TYPE_GRAPHICS, gfxQueue) == false) {
        return false;
    }
    
    uint32_t queueFamilyIndices[] = {presQueue, gfxQueue};
    if (presQueue != gfxQueue) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
        swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    swapChainCreateInfo.preTransform = _support.capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = _present_mode;
    swapChainCreateInfo.clipped = VK_TRUE;
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(dev, &swapChainCreateInfo, nullptr, &_swapchain) != VK_SUCCESS) {
        return false;
    }
    if (vkGetSwapchainImagesKHR(dev, _swapchain, &count, nullptr) != VK_SUCCESS) {
        return false;
    }
    _swapchain_images.resize(count);
    if (vkGetSwapchainImagesKHR(dev, _swapchain, &count, _swapchain_images.data()) != VK_SUCCESS) {
        return false;
    }
    if (createImage(physdev, dev, _extent.width, _extent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depth_image.vkImage, _depth_image.vkImgMem) == false) {
        return false;
    }

    return true;
}

bool Swapchain::createImageViews() {
    auto dev = DeviceManager::GetVkDevice(_dev_id);
    if (dev == VK_NULL_HANDLE) {
        return false;
    }

    _swapchain_imageviews.resize(_swapchain_images.size());
    for (int i = 0; i < _swapchain_images.size(); i++) {
        if (createImageView(dev, _swapchain_images[i], _surface_format.format,VK_IMAGE_ASPECT_COLOR_BIT, _swapchain_imageviews[i]) == false) {
            return false;
        }
    }

    if (createImageView(dev, _depth_image.vkImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, _depth_imageview) == false) {
        return false;
    }

    return true;
}

bool Swapchain::createRenderPass() {
    auto dev = DeviceManager::GetVkDevice(_dev_id);
    if (dev == VK_NULL_HANDLE) {
        return false;
    }

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _surface_format.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(dev, &renderPassInfo, nullptr, &_render_pass) != VK_SUCCESS) {
        return false;
    }
    return true;
}

bool Swapchain::createFrameBuffers() {
    auto dev = DeviceManager::GetVkDevice(_dev_id);
    if (dev == VK_NULL_HANDLE) {
        return false;
    }

    _swapchain_framebuffers.resize(_swapchain_imageviews.size());
    for (size_t i = 0; i < _swapchain_imageviews.size(); i++) {
        std::vector<VkImageView> attachments = {
            _swapchain_imageviews[i],
            _depth_imageview
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _render_pass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _extent.width;
        framebufferInfo.height = _extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(dev, &framebufferInfo, nullptr, &_swapchain_framebuffers[i]) != VK_SUCCESS) {
            return false;
        }
    }
    return true;
}

VkSurfaceFormatKHR Swapchain::chooseFormat() {
    VkSurfaceFormatKHR format = {};
    format.format = VK_FORMAT_B8G8R8A8_SRGB;
    format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    return format;
}

VkPresentModeKHR Swapchain::choosePresentMode() {
    for (const auto& mode : _support.presentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}


bool Swapchain::cleanupSwapchain() {
    VkDevice dev = DeviceManager::GetVkDevice(_dev_id);
    if (dev == VK_NULL_HANDLE) {
        return false;
    }
    for (auto& fb : _swapchain_framebuffers) {
        vkDestroyFramebuffer(dev, fb, nullptr);
    }
    for (auto& imgv : _swapchain_imageviews) {
        vkDestroyImageView(dev, imgv, nullptr);
    }
    vkDestroyImageView(dev, _depth_imageview, nullptr);
    vkDestroyImage(dev, _depth_image.vkImage, nullptr);
    vkFreeMemory(dev, _depth_image.vkImgMem, nullptr);

    vkDestroySwapchainKHR(dev, _swapchain, nullptr);

    return true;
}
bool Swapchain::destroyRenderPass() {
    VkDevice dev = DeviceManager::GetVkDevice(_dev_id);
    if (dev == VK_NULL_HANDLE) {
        return false;
    }
    vkDestroyRenderPass(dev, _render_pass, nullptr);
    return true;
}
}