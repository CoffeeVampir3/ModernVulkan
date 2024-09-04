module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

export module Textures;

import std;
import Buffers;

namespace Vulkan {
    void createTextureImage(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, const std::filesystem::path& texturePath) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(
            texturePath.string().c_str(), 
            &texWidth, 
            &texHeight, 
            &texChannels, 
            STBI_rgb_alpha);
        //Not doing width*height*channels so that all images always are loaded with an alpha channel even if the og had none.
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        std::tie(stagingBuffer, stagingBufferMemory) = createBuffer(physicalDevice, logicalDevice, imageSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        vkMapMemory(logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(logicalDevice, stagingBufferMemory);

        stbi_image_free(pixels);

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;

    }
}