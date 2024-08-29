module;
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module Surface;

export namespace Vulkan {

    VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window);
    
}

namespace Vulkan {
    VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window) {
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(instance, window, nullptr, &surface);
        return surface;
    }
}