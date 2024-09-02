#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

import std;

import Instance;
import PhysicalDevice;
import LogicalDevice;
import Surface;
import SwapChain;
import GraphicsPipeline;
import RenderPass;
import Commands;
import Presentation;
import Logging;
import Descriptors;
import Buffers;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

//64Mb of statically allocated vertex buffer in this case.
constexpr uint32_t INDEXED_VERTEX_BUFFER_STATIC_ALLOCATION_SIZE = 64 * 1024 * 1024;

const std::vector<const char *> requiredDeviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

using deferred = std::function<void()>;

int main() {
  // Language feature when sadge.
  std::stack<deferred> defer;
  #define DEFER(func) defer.push([&]() { func; })

  Logging::info("GLFW initialization.");
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  auto window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  DEFER(
    glfwDestroyWindow(window); 
    glfwTerminate();
    Logging::info("GLFW destroyed");
  );

  //Callback for when the window is resized and we need to rebuild the swapchains and framebuffers for the new extents.
  bool framebufferResized = false;
  glfwSetWindowUserPointer(window, &framebufferResized);
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
    auto *resized = reinterpret_cast<bool *>(glfwGetWindowUserPointer(window));
    *resized = true;
    Logging::info("GLFW window was resized.");
  });

  Logging::info("Vulkan initialization.");
  auto instance = Vulkan::createInstance();
  DEFER(
    vkDestroyInstance(instance, nullptr);
    Logging::info("Vulkan destroyed.");
  );
  if (instance == VK_NULL_HANDLE) {
    Logging::failure("Unable to create a vulkan instance.");
    return -1;
  }

  auto surface = Vulkan::createSurface(instance, window);
  DEFER(vkDestroySurfaceKHR(instance, surface, nullptr));
  if (surface == VK_NULL_HANDLE) {
    Logging::failure("Could not create a window surface.");
    return -1;
  }

  auto physicalDevice = Vulkan::pickPhysicalDevice(instance, surface, requiredDeviceExtensions);
  if (physicalDevice == VK_NULL_HANDLE) {
    Logging::failure("Could not acquire a valid physical device.");
    return -1;
  }

  auto [logicalDevice, graphicsQueue] = Vulkan::createLogicalDevice(physicalDevice, requiredDeviceExtensions);
  DEFER(vkDestroyDevice(logicalDevice, nullptr));
  if (logicalDevice == VK_NULL_HANDLE || graphicsQueue == VK_NULL_HANDLE) {
    Logging::failure("Could not create a logical device.");
    return -1;
  }

  Vulkan::RenderingSwapChain swapChain;
  swapChain.build(physicalDevice, logicalDevice, surface, window);

  auto renderPass = Vulkan::createRenderPass(logicalDevice, swapChain.format);
  DEFER(vkDestroyRenderPass(logicalDevice, renderPass, nullptr));
  if (renderPass == VK_NULL_HANDLE) {
    Logging::failure("Failed to create graphics pipeline.");
    return -1;
  }

  swapChain.populateFramebuffers(logicalDevice, renderPass);
  DEFER(swapChain.destroy(logicalDevice));
  if (!swapChain.valid()) {
    Logging::failure("Failed to create framebuffers.");
    return -1;
  }

  auto descriptorSetLayout = Descriptors::createPipelineDescriptorLayout(logicalDevice);
  DEFER(
    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
  );

  auto [graphicsPipelineLayout, graphicsPipeline] = Vulkan::createGraphicsPipeline(logicalDevice, renderPass, descriptorSetLayout);
  DEFER(
    vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, graphicsPipelineLayout, nullptr)
  );
  if (graphicsPipelineLayout == VK_NULL_HANDLE ||
      graphicsPipeline == VK_NULL_HANDLE) {
    Logging::failure("Failed to create graphics pipeline.");
    return -1;
  }

  auto commandPool = Vulkan::createCommandPool(physicalDevice, logicalDevice);
  DEFER(vkDestroyCommandPool(logicalDevice, commandPool, nullptr));
  if (commandPool == VK_NULL_HANDLE) {
    Logging::failure("Failed to create a command pool.");
    return -1;
  }

  auto commandBuffers = Vulkan::createCommandBuffers(logicalDevice, commandPool, MAX_FRAMES_IN_FLIGHT);
  for (auto &commandBuffer : commandBuffers) {
    if (commandBuffer == VK_NULL_HANDLE) {
      Logging::failure("Failed to create command buffers.");
      return -1;
    }
  }

  auto maybeSynchronizers = Vulkan::createFrameSyncObjects(logicalDevice, MAX_FRAMES_IN_FLIGHT);
  if (!maybeSynchronizers) {
    Logging::failure("Failed to create synchronization objects.");
    return -1;
  }
  auto synchronizers = maybeSynchronizers.value();
  DEFER(
    for (auto synchronizer : synchronizers) { 
      synchronizer.destroy(logicalDevice); 
    }
  );

  std::vector<Descriptors::Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
  };

  std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
  };

  auto indexedVertexBuffer = Vulkan::StagedBuffer{};
  indexedVertexBuffer.allocate(physicalDevice, logicalDevice, INDEXED_VERTEX_BUFFER_STATIC_ALLOCATION_SIZE);
  indexedVertexBuffer.map();
  DEFER( 
    indexedVertexBuffer.unmap();
    indexedVertexBuffer.free();
  );

  auto descriptorPool = Descriptors::createDescriptorPool(logicalDevice, MAX_FRAMES_IN_FLIGHT);
  DEFER(
    vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
  );

  auto descriptorSets = Descriptors::createDescriptorSets(logicalDevice, descriptorSetLayout, descriptorPool, MAX_FRAMES_IN_FLIGHT);

  std::array<Vulkan::UniformBuffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers;
  for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    uniformBuffers[i].allocate(physicalDevice, logicalDevice);
    uniformBuffers[i].bindDescriptor(logicalDevice, descriptorSets[i]);
  }
  DEFER(
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      uniformBuffers[i].free(logicalDevice);
    }
  );

  uint32_t currentFrame = 0;
  // PRIMARY LOOP
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    Vulkan::UniformBuffer currentUniformBuffer = uniformBuffers[currentFrame];
    currentUniformBuffer.updateUniformBuffer(swapChain.extent);

    bool frameSuccessful = Vulkan::drawFrame(
      physicalDevice, 
      logicalDevice, 
      graphicsPipeline, 
      graphicsPipelineLayout,
      graphicsQueue,
      swapChain,
      renderPass, 
      commandBuffers[currentFrame],
      synchronizers[currentFrame],
      indexedVertexBuffer,
      currentUniformBuffer,
      framebufferResized
    );

    if (!frameSuccessful) {
      Logging::failure("Failed to draw frame.");
      return -1;
    }
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    indexedVertexBuffer.put(vertices, indices);
    indexedVertexBuffer.stagingToBuffer(graphicsQueue, commandPool);
  }

  vkDeviceWaitIdle(logicalDevice);

  while (!defer.empty()) {
    auto deferred_func = defer.top();
    deferred_func();
    defer.pop();
  }

  return 0;
}