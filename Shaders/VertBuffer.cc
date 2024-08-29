module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

export module Buffers;

import <vector>;

export namespace Buffers {
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

    // constexpr auto BufferFormatV1 = VK_FORMAT_R32_SFLOAT;
    // constexpr auto BufferFormatV2 = VK_FORMAT_R32G32_SFLOAT;
    // constexpr auto BufferFormatV3 = VK_FORMAT_R32G32B32_SFLOAT;
    // constexpr auto BufferFormatV4 = VK_FORMAT_R32G32B32A32_SFLOAT;
    // constexpr auto BufferFormatIV1 = VK_FORMAT_R32_SINT;
    // constexpr auto BufferFormatIV2 = VK_FORMAT_R32G32_SINT;
    // constexpr auto BufferFormatIV3 = VK_FORMAT_R32G32B32_SINT;
    // constexpr auto BufferFormatIV4 = VK_FORMAT_R32G32B32A32_SINT;
    // constexpr auto BufferFormatUV1 = VK_FORMAT_R32_UINT;
    // constexpr auto BufferFormatUV2 = VK_FORMAT_R32G32_UINT;
    // constexpr auto BufferFormatUV3 = VK_FORMAT_R32G32B32_UINT;
    // constexpr auto BufferFormatUV4 = VK_FORMAT_R32G32B32A32_UINT;
    // constexpr auto BufferFormatFloat = VK_FORMAT_R32_SFLOAT;
    // constexpr auto BufferFormatInt = VK_FORMAT_R32_SINT;
    // constexpr auto BufferFormatUint = VK_FORMAT_R32_UINT;

    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};

            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static VkVertexInputAttributeDescription* getAttributeDescriptions() {
            static VkVertexInputAttributeDescription attributeDescriptions[2] = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            return attributeDescriptions;
        }
    };

    const std::vector<Vertex> vertices = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
}
