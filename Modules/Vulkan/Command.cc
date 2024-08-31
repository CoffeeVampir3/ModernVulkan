module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module Commands;

import std;
import Queues;
import Logging;
import Descriptors;

export namespace Vulkan {
    VkCommandPool createCommandPool(VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
    std::vector<VkCommandBuffer> createCommandBuffers(VkDevice logicalDevice, VkCommandPool commandPool, uint32_t numBuffersToCreate);

    bool recordCommandBuffer(
        VkCommandBuffer commandBuffer, 
        uint32_t imageIndex, 
        VkPipeline graphicsPipeline, 
        VkRenderPass renderPass, 
        std::vector<VkFramebuffer> swapChainFramebuffers, 
        VkExtent2D swapChainExtent,
        VkBuffer vertexBuffer,
        VkBuffer indexBuffer
    );
}

namespace Vulkan {
    VkCommandPool createCommandPool(VkPhysicalDevice physicalDevice, VkDevice logicalDevice) {
        VkCommandPool commandPool;
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool);
        return commandPool;
    }

    std::vector<VkCommandBuffer> createCommandBuffers(VkDevice logicalDevice, VkCommandPool commandPool, uint32_t numBuffersToCreate) {
        std::vector<VkCommandBuffer> commandBuffers(numBuffersToCreate, VK_NULL_HANDLE);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = numBuffersToCreate;

        vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data());
        return commandBuffers;
    }

    bool recordCommandBuffer(
        VkCommandBuffer commandBuffer, uint32_t imageIndex, VkPipeline graphicsPipeline, 
        VkRenderPass renderPass, std::vector<VkFramebuffer> swapChainFramebuffers, VkExtent2D swapChainExtent,
        VkBuffer vertexBuffer, VkBuffer indexBuffer) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            Logging::failure("Failed to begin command buffer.");
            return false;
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        return vkEndCommandBuffer(commandBuffer) != VK_SUCCESS;
    }
}