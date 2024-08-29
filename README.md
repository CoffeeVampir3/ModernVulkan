Modern C++23 approach to vulkan using modules. Based on <https://vulkan-tutorial.com/Introduction> 

#### Dependencies:

`Pacman`:
```
sudo pacman -S vulkan-devel glm libxi libxxf86vm shaderc
sudo pacman -S glfw-wayland # glfw-x11 for X11 users
```

`Dnf:`
```
sudo dnf install vulkan-tools vulkan-loader-devel mesa-vulkan-devel vulkan-validation-layers-devel glfw-devel glm-devel glslc
```

`Apt`:
(For shaders you'll need to install glslc yourself: https://github.com/google/shaderc/blob/main/downloads.md)
```
sudo apt install vulkan-tools vulkan-validationlayers-dev spirv-tools libvulkan-dev libglfw3-dev libglm-dev
```

#### Building:

Build the shaders via `shaders.sh` then build the project via `make.sh`

For non-linux users see <https://vulkan-tutorial.com/Development_environment> for vulkan/environment setup.
