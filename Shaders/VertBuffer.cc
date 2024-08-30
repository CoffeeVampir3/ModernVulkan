module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

export module Descriptors;

import std;

/*
    <https://vulkan-tutorial.com/en/Vertex_buffers/Vertex_input_description>
    The format parameter describes the type of data for the attribute. 
    A bit confusingly, the formats are specified using the same enumeration as color formats. 
    The following shader types and formats are commonly used together:
        float: VK_FORMAT_R32_SFLOAT
        vec2: VK_FORMAT_R32G32_SFLOAT
        vec3: VK_FORMAT_R32G32B32_SFLOAT
        vec4: VK_FORMAT_R32G32B32A32_SFLOAT
*/

export namespace DescriptorFormat {
    constexpr auto V1 = VK_FORMAT_R32_SFLOAT;
    constexpr auto V2 = VK_FORMAT_R32G32_SFLOAT;
    constexpr auto V3 = VK_FORMAT_R32G32B32_SFLOAT;
    constexpr auto V4 = VK_FORMAT_R32G32B32A32_SFLOAT;
    constexpr auto IV1 = VK_FORMAT_R32_SINT;
    constexpr auto IV2 = VK_FORMAT_R32G32_SINT;
    constexpr auto IV3 = VK_FORMAT_R32G32B32_SINT;
    constexpr auto IV4 = VK_FORMAT_R32G32B32A32_SINT;
    constexpr auto UV1 = VK_FORMAT_R32_UINT;
    constexpr auto UV2 = VK_FORMAT_R32G32_UINT;
    constexpr auto UV3 = VK_FORMAT_R32G32B32_UINT;
    constexpr auto UV4 = VK_FORMAT_R32G32B32A32_UINT;
    constexpr auto Float = VK_FORMAT_R32_SFLOAT;
    constexpr auto Int = VK_FORMAT_R32_SINT;
    constexpr auto Uint = VK_FORMAT_R32_UINT;
}

export namespace Descriptors {
    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription bindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};

            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = DescriptorFormat::V2;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = DescriptorFormat::V3;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };
}
