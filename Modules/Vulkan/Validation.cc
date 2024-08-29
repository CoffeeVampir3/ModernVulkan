module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module Validation;

import std;

export namespace Validation {
    const std::vector<std::string> validationLayerNames = {
        "VK_LAYER_KHRONOS_validation"
    };

    // This vector holds C-style strings for Vulkan API compatibility, but allows us to get rid of cstring header.
    const std::vector<const char*> validationLayers = []() {
        std::vector<const char*> layers;
        layers.reserve(validationLayerNames.size());
        for (const auto& name : validationLayerNames) {
            layers.push_back(name.c_str());
        }
        return layers;
    }();

    #ifdef NDEBUG
        constexpr bool enableValidationLayers = false;
    #else
        constexpr bool enableValidationLayers = true;
    #endif

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const std::string &layerName : validationLayers) {
            bool layerFound = false;
            for (const auto &layerProperties : availableLayers) {
                if (layerName == layerProperties.layerName) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }

        return true;
    }
}