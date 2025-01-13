#include <iostream>
#include "devicemgr.h"


namespace RenderingFramework3D {
static bool _init = false;
static DeviceManager* _instance=nullptr;

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

DeviceManager::DeviceManager()
    :
    _devices(),
    _vk_instance(VK_NULL_HANDLE)
{
    _init = true;
    _init = _init && createVKInstance();
    _init = _init && enlistPhysicalDevices();
}

DeviceManager::~DeviceManager() {
    for (auto& dev : _devices) {
        if (dev.logicalDev != VK_NULL_HANDLE) {
            for (auto& queue : dev.queues) {
                queue.vkqueue = VK_NULL_HANDLE;
                if (queue.cmdpool != VK_NULL_HANDLE) vkDestroyCommandPool(dev.logicalDev, queue.cmdpool, nullptr);
                if (queue.secondarypool != VK_NULL_HANDLE) vkDestroyCommandPool(dev.logicalDev, queue.secondarypool, nullptr);
                if (queue.fence != VK_NULL_HANDLE) vkDestroyFence(dev.logicalDev, queue.fence, nullptr);
            }
            vkDestroyDevice(dev.logicalDev, nullptr);
        }
    }
    vkDestroyInstance(_vk_instance, nullptr);
    _devices.clear();
}

bool DeviceManager::Initialize() {
    if (_init == false) {
        _instance = new DeviceManager();
        if (_init == false) {
            delete _instance;
            _instance = nullptr;
            return false;
        }
    }
    return true;
}
void DeviceManager::Cleanup() {
    if (_init && _instance) {
        delete _instance;
        _init = false;
    }
}

VkDevice DeviceManager::GetVkDevice(unsigned  id) {
    if (_instance && id < _instance->_devices.size()) {
        return _instance->_devices[id].logicalDev;
    }
    return VK_NULL_HANDLE;
}
VkPhysicalDevice DeviceManager::GetVkPhyDevice(unsigned  id) {
    if (_instance && id < _instance->_devices.size()) {
        return _instance->_devices[id].physDev;
    }
	return VK_NULL_HANDLE;
}
VkInstance DeviceManager::GetVkInstance() {
    if (_instance) {
        return _instance->_vk_instance;
    }
    return VK_NULL_HANDLE;
}

bool DeviceManager::GetQueueIdx(unsigned devID, QueueType queue, unsigned& idx) {
    if(_instance) {
        if(devID >= _instance->_devices.size()) {
            return false;
        }

        idx = _instance->_devices[devID].gfxQueueIdx;

        return true;
    }
    return false;
}
	

VkQueue DeviceManager::GetVkQueue(unsigned devID, QueueType queue) {
    if (_instance) {
        unsigned idx = -1;
        switch(queue) {
            case QUEUE_TYPE_GRAPHICS:
                idx = _instance->_devices[devID].gfxQueueIdx;
                break;
            case QUEUE_TYPE_PRESENT:
                idx = _instance->_devices[devID].presentQueueIdx;
                break;
        }

        return _instance->_devices[devID].queues[idx].vkqueue;
    }
    return VK_NULL_HANDLE;
}

bool DeviceManager::CreateCommandBuffer(unsigned id, QueueType queueType, bool primary, VkCommandBuffer& buffer) {
    if (_instance) {
        if (id >= _instance->_devices.size() || _instance->_devices[id].logicalDev == VK_NULL_HANDLE) {
            return false;
        }

        unsigned queue = -1;
        switch(queueType) {
            case QUEUE_TYPE_GRAPHICS:
                queue = _instance->_devices[id].gfxQueueIdx;
                break;
            case QUEUE_TYPE_PRESENT:
                queue = _instance->_devices[id].presentQueueIdx;
                break;
        }

        if (primary) {
            if (_instance->_devices[id].queues[queue].cmdpool == VK_NULL_HANDLE) {
                if (_instance->createCommandPool(id, queue, primary) == false) {
                    return false;
                }
            }
        } else {
            if (_instance->_devices[id].queues[queue].secondarypool == VK_NULL_HANDLE) {
                if (_instance->createCommandPool(id, queue, primary) == false) {
                    return false;
                }
            }
        }
        return _instance->createCommandBuffer(id, queue, primary, buffer);
    }
    return false;
}

bool DeviceManager::CheckQueueReady(unsigned id, QueueType queueType, bool& ready) {
    ready = false;
    if (_instance) {
        if (id >= _instance->_devices.size() || _instance->_devices[id].logicalDev == VK_NULL_HANDLE) {
            return false;
        }

        unsigned queue = -1;
        switch(queueType) {
            case QUEUE_TYPE_GRAPHICS:
                queue = _instance->_devices[id].gfxQueueIdx;
                break;
            case QUEUE_TYPE_PRESENT:
                queue = _instance->_devices[id].presentQueueIdx;
                break;
        }
        auto ret = vkGetFenceStatus(_instance->_devices[id].logicalDev, _instance->_devices[id].queues[queue].fence);

        if (ret == VK_SUCCESS) {
            ready = true;
        } else if(ret == VK_ERROR_DEVICE_LOST) {
            return false;
        }
        return true;
    }
    return false;
}
bool DeviceManager::WaitForQueue(unsigned id, QueueType queueType) {
    if (_instance) {
        if (id >= _instance->_devices.size() || _instance->_devices[id].logicalDev == VK_NULL_HANDLE) {
            return false;
        }
        unsigned queue = -1;
        switch(queueType) {
            case QUEUE_TYPE_GRAPHICS:
                queue = _instance->_devices[id].gfxQueueIdx;
                break;
            case QUEUE_TYPE_PRESENT:
                queue = _instance->_devices[id].presentQueueIdx;
                break;
        }

        if (vkWaitForFences(_instance->_devices[id].logicalDev, 1, &_instance->_devices[id].queues[queue].fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
            return false;
        }
        return true;
    }
    return false;
}

bool DeviceManager::SubmitCommandBuffer(unsigned id, QueueType queueType, VkCommandBuffer buffer, VkSemaphore waitSem, VkPipelineStageFlags waitStage, VkSemaphore signalSem, bool block) {
    if (_instance) {
        
        if (id >= _instance->_devices.size() || _instance->_devices[id].logicalDev == VK_NULL_HANDLE) {
            return false;
        }

        unsigned queue = -1;
        switch(queueType) {
            case QUEUE_TYPE_GRAPHICS:
                queue = _instance->_devices[id].gfxQueueIdx;
                break;
            case QUEUE_TYPE_PRESENT:
                queue = _instance->_devices[id].presentQueueIdx;
                break;
        }

        bool ready = false;

        if(_instance->CheckQueueReady(id, queueType, ready) == false) {
            return false;
        }
        if (ready == false) {
            if (block && _instance->WaitForQueue(id, queueType) == false) {
                return false;
            }
            ready = true;
        }
        if (ready) {
            if (vkResetFences(_instance->_devices[id].logicalDev, 1, &_instance->_devices[id].queues[queue].fence) != VK_SUCCESS) {
                return false;
            }
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &buffer;

            if (waitSem != VK_NULL_HANDLE) {
                submitInfo.waitSemaphoreCount = 1;
                submitInfo.pWaitDstStageMask = &waitStage;
                submitInfo.pWaitSemaphores = &waitSem;
            }
            if (signalSem != VK_NULL_HANDLE) {
                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = &signalSem;
            }
            if (vkQueueSubmit(_instance->_devices[id].queues[queue].vkqueue, 1, &submitInfo, _instance->_devices[id].queues[queue].fence) != VK_SUCCESS) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool DeviceManager::CreateVkSurface(GLFWwindow* window, VkSurfaceKHR& surface) {
    if (_instance) {
        if (glfwCreateWindowSurface(_instance->_vk_instance, window, nullptr, &surface) != VK_SUCCESS) {
            return false;
        }
        return true;
    }
    return false;
}

bool DeviceManager::FindSuitableDevice(VkSurfaceKHR surface, unsigned& id,  SwapChainSupportDetails& swapchainSupport) {
    if (_instance) {
        unsigned idx = 0;
        for (auto&  dev : _instance->_devices) {
            unsigned numQueueFamily;
            SwapChainSupportDetails swDetails;
            if (isDeviceSuitable(dev.physDev, surface, dev.gfxQueueIdx, dev.presentQueueIdx, numQueueFamily, swDetails)) {
                if (dev.logicalDev == VK_NULL_HANDLE && _instance->createLogicalDevice(idx, numQueueFamily) == false) {
                    return false;
                }

                if(_instance->createCommandPool(idx, dev.gfxQueueIdx, false)==false) {
                    return false;
                }

                if(_instance->createCommandBuffer(idx, dev.gfxQueueIdx, false, dev.loadCmdBuffer) == false) {
                    return false;
                }

                swapchainSupport = swDetails;
                id = idx;
                return true;
            }
            idx++;
        }
    }
    return false;
}


bool DeviceManager::CopyBuffer(unsigned devID, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	if(_instance == nullptr) {
        return false;
    }

    if (devID > _instance->_devices.size() || srcBuffer == VK_NULL_HANDLE || dstBuffer == VK_NULL_HANDLE) {
		return false;
	}

    if (_instance->_devices[devID].logicalDev == VK_NULL_HANDLE) {
        return false;
    }

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    unsigned queue = _instance->_devices[devID].gfxQueueIdx;

	if (vkBeginCommandBuffer(_instance->_devices[devID].loadCmdBuffer, &beginInfo) != VK_SUCCESS) {
		return false;
	}

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(_instance->_devices[devID].loadCmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	if (vkEndCommandBuffer(_instance->_devices[devID].loadCmdBuffer) != VK_SUCCESS) {
		return false;
	}

	if (DeviceManager::SubmitCommandBuffer(devID, QUEUE_TYPE_GRAPHICS, _instance->_devices[devID].loadCmdBuffer, VK_NULL_HANDLE, 0, VK_NULL_HANDLE) == false) {
		return false;
	}
	if (DeviceManager::WaitForQueue(devID, QUEUE_TYPE_GRAPHICS) == false) {
		return false;
	}

	return true;
}

bool DeviceManager::createVKInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "vulkan application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "no engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;
    return (vkCreateInstance(&createInfo, nullptr, &_vk_instance) == VK_SUCCESS);
}

bool DeviceManager::enlistPhysicalDevices() {
    uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(_vk_instance, &count, nullptr) != VK_SUCCESS) {
        return false;
    }
    std::vector<VkPhysicalDevice> physDevs(count);
    if (vkEnumeratePhysicalDevices(_vk_instance, &count, physDevs.data()) != VK_SUCCESS) {
        return false;
    }
    _devices.resize(count);
    for (unsigned i = 0; i < count; i++) {
        _devices[i].physDev = physDevs[i];
        _devices[i].logicalDev = VK_NULL_HANDLE;
    }
    return true;
}

bool DeviceManager::createLogicalDevice(unsigned devIdx, unsigned numIdx) {
    if (devIdx >= _devices.size()) {
        return false;
    }
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queues(numIdx);
    
    for (int i = 0; i < numIdx; i++) {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = i;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queues[i] = queueInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = queues.size();
    deviceCreateInfo.pQueueCreateInfos = queues.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkResult result;
    if ((result=vkCreateDevice(_devices[devIdx].physDev, &deviceCreateInfo, nullptr, &_devices[devIdx].logicalDev)) != VK_SUCCESS) {
        std::cout << result << std::endl;
        return false;
    }

    //std::cout << "here" << std::endl;
    unsigned idx = 0;
    _devices[devIdx].queues.resize(numIdx, {0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE });
    for (auto& queue : _devices[devIdx].queues) {
        queue.index = idx;
        vkGetDeviceQueue(_devices[devIdx].logicalDev, idx, 0, &queue.vkqueue);

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        if (vkCreateFence(_devices[devIdx].logicalDev, &fenceInfo, nullptr, &queue.fence) != VK_SUCCESS) {
            return false;
        }

        idx++;
    }

    return true;
}

bool DeviceManager::createCommandPool(unsigned devIdx, unsigned queueIdx, bool primary) {
    if (devIdx >= _devices.size() || _devices[devIdx].logicalDev == VK_NULL_HANDLE || queueIdx >= _devices[devIdx].queues.size()) {
        return false;
    }

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (primary == false) poolInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = queueIdx;

    if (primary) {
        if (vkCreateCommandPool(_devices[devIdx].logicalDev, &poolInfo, nullptr, &_devices[devIdx].queues[queueIdx].cmdpool) != VK_SUCCESS) {
            return false;
        }
    } else {
        if (vkCreateCommandPool(_devices[devIdx].logicalDev, &poolInfo, nullptr, &_devices[devIdx].queues[queueIdx].secondarypool) != VK_SUCCESS) {
            return false;
        }
    }
    return true;
}
bool DeviceManager::createCommandBuffer(unsigned devIdx, unsigned queueIdx, bool primary, VkCommandBuffer& buffer) {
    if (devIdx >= _devices.size() || _devices[devIdx].logicalDev == VK_NULL_HANDLE || queueIdx >= _devices[devIdx].queues.size()) {
        return false;
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    if(primary) {
        allocInfo.commandPool = _devices[devIdx].queues[queueIdx].cmdpool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    } else {
        allocInfo.commandPool = _devices[devIdx].queues[queueIdx].secondarypool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    }
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(_devices[devIdx].logicalDev, &allocInfo, &buffer) != VK_SUCCESS) {
        return false;
    }
    return true;
}


bool DeviceManager::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, unsigned& gfxQueueIndex, unsigned& presentQueueIndex, unsigned& numIdx, SwapChainSupportDetails& swapchainSupport) {
    bool gfxqfound = false, presqfound = false;
    unsigned count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());
    numIdx = count;

    for (int i = 0; i < queueFamilies.size(); i++) {
        VkBool32 presentSupport = false;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport) != VK_SUCCESS) {
            return false;
        }
        if (presentSupport) {
            presentQueueIndex = i;
            presqfound = true;
        }
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            gfxQueueIndex = i;
            gfxqfound = true;
        }
        if (presqfound == true && gfxqfound == true) {
            break;
        }
    }
    if (presqfound == false || gfxqfound == false) {
        return false;
    }
    count = 0;
    bool check = true;
    check = check && (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainSupport.capabilities) == VK_SUCCESS);
    check = check && (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr) == VK_SUCCESS);
    swapchainSupport.formats.resize(count);
    check = check && (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, swapchainSupport.formats.data()) == VK_SUCCESS);
    
    count = 0;
    check = check && (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr) == VK_SUCCESS);
    swapchainSupport.presentModes.resize(count);
    check = check && (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, swapchainSupport.presentModes.data()) == VK_SUCCESS);
    
    if (check == false) {
        return false;
    }

    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    uint32_t extensionCount;
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
        return false;
    }
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()) != VK_SUCCESS) {
        return false;
    }

    for (auto& extension : deviceExtensions) {
        bool found = false;
        for (auto& availableExtension : availableExtensions) {
            if (strcmp(availableExtension.extensionName, extension) == 0) {
                found = true;
                break;
            }
        }
        if (found == false) {
            return false;
        }
    }

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    //if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    //    return false;
    //}
    bool found = false;
    for (const auto& format : swapchainSupport.formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            found = true;
            break;
        }
    }
    if (found == false) {
        return false;
    }
    return true;
}
}