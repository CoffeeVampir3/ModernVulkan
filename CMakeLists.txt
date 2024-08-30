# This requires a recent nightly build.
# This will be part of CMake 3.30.0.
cmake_minimum_required(VERSION 3.29.0 FATAL_ERROR)

# Enables the Standard module support. This needs to be done
# before selecting the languages.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_CXX_MODULE_STD ON)

project("VulkanApp"
  LANGUAGES CXX
)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)


# Set language version used
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS ON)

# Create the executable target
add_executable(VulkanApp main.cpp)

file(GLOB_RECURSE MODULE_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/Modules/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.cc"
)

# Add modules using FILE_SET
target_sources(VulkanApp
    PUBLIC
    FILE_SET CXX_MODULES TYPE CXX_MODULES 
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES ${MODULE_FILES}
)

# Add the Modules directory to include directories
target_include_directories(VulkanApp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules
)

target_link_libraries(VulkanApp
    PRIVATE
    Vulkan::Vulkan
    glfw
    dl
    pthread
    X11
    Xxf86vm
    Xrandr
    Xi
)