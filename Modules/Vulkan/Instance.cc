module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module Instance;

import Validation;

export namespace Vulkan {
    VkInstance createInstance() {
        VkInstance instance = VK_NULL_HANDLE;
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;

        if constexpr (Validation::enableValidationLayers) {
            if (!Validation::checkValidationLayerSupport()) return VK_NULL_HANDLE;
            
            createInfo.enabledLayerCount = static_cast<uint32_t>(Validation::validationLayers.size());
            createInfo.ppEnabledLayerNames = Validation::validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        return instance;
    }
}