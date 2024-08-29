module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module PhysicalDevice;

import std;
import Queues;
import SwapChain;

export namespace Vulkan {
    VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*> requiredDeviceExtensions);
}

namespace Vulkan {
    bool checkDeviceExtensionSupport(auto device, auto requiredDeviceExtensions) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool isDeviceSuitable(auto device, auto surface, auto requiredDeviceExtensions) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        auto extensionsSupported = checkDeviceExtensionSupport(device, requiredDeviceExtensions);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*> requiredDeviceExtensions) {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device, surface, requiredDeviceExtensions)) {
                physicalDevice = device;
                break;
            }
        }

        return physicalDevice;
    }
}