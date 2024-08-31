module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module Presentation;

import std;
import Commands;
import SwapChain;
import Logging;

export namespace Vulkan {

    struct RenderSync {
        VkSemaphore imageAvailableSemaphore; 
        VkSemaphore renderFinishedSemaphore; 
        VkFence inFlightFence;

        void destroy(VkDevice logicalDevice) {
            vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, nullptr); 
            vkDestroySemaphore(logicalDevice, renderFinishedSemaphore, nullptr);
            vkDestroyFence(logicalDevice, inFlightFence, nullptr);
        }
    };

    std::optional<std::vector<RenderSync>> createFrameSyncObjects(VkDevice logicalDevice, uint32_t numSynchronizersToCreate);

    bool drawFrame(        
        VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkPipeline graphicsPipeline, 
        VkQueue graphicsQueue, 
        RenderingSwapChain& swapChain,
        VkRenderPass renderPass,
        VkCommandBuffer commandBuffer, 
        RenderSync synchronizers,
        VkBuffer vertexBuffer,
        VkBuffer indexBuffer,
        bool& framebufferResized
    );

}

namespace Vulkan {
    std::optional<std::vector<RenderSync>> createFrameSyncObjects(VkDevice logicalDevice, uint32_t numSynchronizersToCreate) {
        std::vector<RenderSync> synchronizers{numSynchronizersToCreate};

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //Creates the fence in a signaled state.

        for (int i = 0;  i < numSynchronizersToCreate; i++) {
            if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &synchronizers[i].imageAvailableSemaphore) != VK_SUCCESS ||
                vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &synchronizers[i].renderFinishedSemaphore) != VK_SUCCESS || 
                vkCreateFence(logicalDevice, &fenceInfo, nullptr, &synchronizers[i].inFlightFence) != VK_SUCCESS) {
                Logging::failure("Failed to create synchronization objects.");
                return {};
            }
        }

        return synchronizers;
    }

    bool drawFrame(
        VkPhysicalDevice physicalDevice,
        VkDevice logicalDevice,
        VkPipeline graphicsPipeline, 
        VkQueue graphicsQueue,
        RenderingSwapChain& swapChain,
        VkRenderPass renderPass,
        VkCommandBuffer commandBuffer, 
        RenderSync synchronizers,
        VkBuffer vertexBuffer,
        VkBuffer indexBuffer,
        bool& framebufferResized
        ) {
        vkWaitForFences(logicalDevice, 1, &synchronizers.inFlightFence, VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(
            logicalDevice, 
            swapChain.vulkanSwapChain, 
            UINT64_MAX, 
            synchronizers.imageAvailableSemaphore, 
            VK_NULL_HANDLE, 
            &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            Logging::info("Acquiring a new swapchain as the current one is out of date.");
            swapChain.rebuild(physicalDevice, logicalDevice, renderPass);
            if(!swapChain.valid()) {
                Logging::failure("Newly acquired resized Swapchain was not valid.");
                return false;
            }
            return true;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            Logging::failure("Failed to acquire a swap chain image on draw!");
            return false;
        }

        vkResetFences(logicalDevice, 1, &synchronizers.inFlightFence);

        vkResetCommandBuffer(commandBuffer, 0);
        Vulkan::recordCommandBuffer(
            commandBuffer, imageIndex, graphicsPipeline, renderPass, 
            swapChain.framebuffers, swapChain.extent, vertexBuffer, indexBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {synchronizers.imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores[] = {synchronizers.renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, synchronizers.inFlightFence) != VK_SUCCESS) {
            Logging::failure("Failed to submit draw frame queue.");
            return false;
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain.vulkanSwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(graphicsQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            Logging::info("Acquiring a new swapchain as the current one is out of date.");
            swapChain.rebuild(physicalDevice, logicalDevice, renderPass);
            if(!swapChain.valid()) {
                Logging::failure("Newly acquired resized Swapchain was not valid.");
                return false;
            }
            return true;
        } else if (result != VK_SUCCESS) {
            Logging::failure("Failed to present image from swap chain on draw.");
            return false;
        }

        return true;
    }
}