module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module Buffers;

import std;
import Descriptors;
import Logging;

namespace Vulkan {
    static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        return -1;
    }

    export std::tuple<VkBuffer, VkDeviceMemory> createBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
        VkBuffer buffer;
        VkDeviceMemory bufferMemory;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory);
        vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);

        return std::make_tuple(buffer, bufferMemory);
    }

    export void copyBuffer(
        VkDevice logicalDevice, 
        VkQueue graphicsQueue, 
        VkCommandPool commandPool, 
        VkBuffer srcBuffer, 
        VkBuffer dstBuffer,
        VkDeviceSize size) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
    }

    export struct StagedBuffer {
        VkBuffer buffer;
        VkBuffer staging;
        VkDeviceMemory bufferMemory;
        VkDeviceMemory stagingMemory;
        VkDeviceSize bufferSize;
        VkDeviceSize vertexSize;
        VkDeviceSize indexSize;
        int numVertices;
        int numIndices;
        void* bufferData;

        VkDevice allocatedDevice;
        bool mapped{false};

        void allocate(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize bufferSizeToAllocate) {
            bufferSize = bufferSizeToAllocate;
            allocatedDevice = logicalDevice;

            std::tie(this->staging, this->stagingMemory) = createBuffer(
                physicalDevice, allocatedDevice, bufferSize, 
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            std::tie(this->buffer, this->bufferMemory) = createBuffer(
                physicalDevice, allocatedDevice, bufferSize, 
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );
        }

        void free() {
            vkDestroyBuffer(allocatedDevice, buffer, nullptr);
            vkDestroyBuffer(allocatedDevice, staging, nullptr);
            vkFreeMemory(allocatedDevice, bufferMemory, nullptr);
            vkFreeMemory(allocatedDevice, stagingMemory, nullptr);
        }

        void map() {
            if(mapped) {
                Logging::failure("Attempted to map an already mapped vertex buffer.");
                return;
            }
            vkMapMemory(allocatedDevice, stagingMemory, 0, bufferSize, 0, &bufferData);
            mapped = true;
        }

        void unmap() {
            if(!mapped) {
                Logging::failure("Attempted to unmap an unmapped buffer.");
                return;
            }
            vkUnmapMemory(allocatedDevice, stagingMemory);
            mapped = false;
        }

        void put(
            const std::vector<Descriptors::Vertex>& vertices, 
            const std::vector<uint16_t>& indices
            ) {
            vertexSize = sizeof(vertices[0]) * vertices.size();
            indexSize = sizeof(indices[0]) * indices.size();
            numVertices = vertices.size();
            numIndices = indices.size();

            std::memcpy(bufferData, vertices.data(), (size_t) vertexSize);
            void* indexDest = std::bit_cast<std::byte*>(bufferData) + vertexSize;
            std::memcpy(indexDest, indices.data(),  (size_t) indexSize);
        }

        void stagingToBuffer(VkQueue commandQueue, VkCommandPool commandPool) {
            Vulkan::copyBuffer(allocatedDevice, commandQueue, commandPool, staging, buffer, bufferSize);
        }
    };

    export std::tuple<VkBuffer, VkDeviceMemory, VkDeviceSize> createVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, std::vector<Descriptors::Vertex> vertices) {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        auto [vertexBuffer, vertexBufferMemory] = createBuffer(
            physicalDevice, logicalDevice, bufferSize, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        return std::make_tuple(vertexBuffer, vertexBufferMemory, bufferSize);
    }

    export std::tuple<VkBuffer, VkDeviceMemory, VkDeviceSize> createIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, std::vector<uint16_t> indices) {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
        auto [indexBuffer, indexBufferMemory] = createBuffer(
            physicalDevice, logicalDevice, bufferSize, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        return std::make_tuple(indexBuffer, indexBufferMemory, bufferSize);
    }

    export std::tuple<VkBuffer, VkDeviceMemory> createStagingBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize bufferSize) {
        auto [stagingBuffer, stagingBufferMemory] = createBuffer(
            physicalDevice, logicalDevice, bufferSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        return std::make_tuple(stagingBuffer, stagingBufferMemory);
    }
}