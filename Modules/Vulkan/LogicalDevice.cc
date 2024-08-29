module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module LogicalDevice;

import std;
import Queues;

export namespace Vulkan {

    std::tuple<VkDevice, VkQueue> createLogicalDevice(
        VkPhysicalDevice physicalDevice, 
        const std::vector<const char*> requiredDeviceExtensions
    );

}

namespace Vulkan {
    std::tuple<VkDevice, VkQueue> createLogicalDevice(VkPhysicalDevice physicalDevice, const std::vector<const char*> requiredDeviceExtensions) {
        VkDevice logicalDevice;
        VkQueue graphicsQueue;

        QueueFamilyIndices queueFamilies = findQueueFamilies(physicalDevice);

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();

        vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);
        vkGetDeviceQueue(logicalDevice, queueFamilies.graphicsFamily.value(), 0, &graphicsQueue);

        return std::make_tuple(logicalDevice, graphicsQueue);
    }
}