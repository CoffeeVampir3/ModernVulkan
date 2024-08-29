module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module SwapChain;

import std;
import Queues;

namespace Vulkan {

    export struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    export SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    export struct RenderingSwapChain {
        VkSwapchainKHR vulkanSwapChain;
        VkExtent2D extent;
        VkFormat format;
        VkSurfaceKHR surface;
        GLFWwindow* window;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        std::vector<VkFramebuffer> framebuffers;

        void destroy(VkDevice logicalDevice) {
            for (auto& framebuffer : framebuffers) {
                vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
            }
            for (auto& imageView : imageViews) {
                vkDestroyImageView(logicalDevice, imageView, nullptr);
            }
            vkDestroySwapchainKHR(logicalDevice, vulkanSwapChain, nullptr);
        }

        void populateFramebuffers(
            VkDevice logicalDevice,
            VkRenderPass renderPass
        ) {
            framebuffers.resize(imageViews.size());
            for (size_t i = 0; i < imageViews.size(); i++) {
                VkImageView attachments[] = {imageViews[i]};

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = extent.width;
                framebufferInfo.height = extent.height;
                framebufferInfo.layers = 1;

                vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &framebuffers[i]);
            }
        }

        void populateImageViews(
            VkDevice logicalDevice
        ) {
            imageViews.resize(images.size());
            for (size_t i = 0; i < images.size(); i++) {
                VkImageViewCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                createInfo.image = images[i];
                createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                createInfo.format = format;
                createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                createInfo.subresourceRange.baseMipLevel = 0;
                createInfo.subresourceRange.levelCount = 1;
                createInfo.subresourceRange.baseArrayLayer = 0;
                createInfo.subresourceRange.layerCount = 1;
                vkCreateImageView(logicalDevice, &createInfo, nullptr, &imageViews[i]);
            }
        }

        void build(
            VkPhysicalDevice physicalDevice, 
            VkDevice logicalDevice,
            VkSurfaceKHR vulkanSurface, 
            GLFWwindow* glfwWindow
            ) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, vulkanSurface);
            VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
            VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

            surface = vulkanSurface;
            window = glfwWindow;
            format = surfaceFormat.format;
            extent = chooseSwapExtent(swapChainSupport.capabilities, glfwWindow);

            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

            //It should be noted that "0" in this case means no limit not zero supported, so min is not going to work here for that reason.
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount >= swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surface;
            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;
            createInfo.oldSwapchain = VK_NULL_HANDLE;

            vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &vulkanSwapChain);

            vkGetSwapchainImagesKHR(logicalDevice, vulkanSwapChain, &imageCount, nullptr);
            images.resize(imageCount);
            vkGetSwapchainImagesKHR(logicalDevice, vulkanSwapChain, &imageCount, images.data());

            populateImageViews(logicalDevice);
        }

        bool valid() {
            if (vulkanSwapChain == VK_NULL_HANDLE) {
                return false;
            }
            for (auto& imageView : imageViews) {
                if(imageView == VK_NULL_HANDLE) {
                    return false;
                }
            }
            for (auto& framebuffer : framebuffers) {
                if(framebuffer == VK_NULL_HANDLE) {
                    return false;
                }
            }
            return true;
        }

        void rebuild(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkRenderPass renderPass) {
            //Pause minimized window
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            while (width == 0 || height == 0) {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            }
            vkDeviceWaitIdle(logicalDevice);

            destroy(logicalDevice);

            build(physicalDevice, logicalDevice, surface, window);
            populateFramebuffers(logicalDevice, renderPass);
        }
    };
}