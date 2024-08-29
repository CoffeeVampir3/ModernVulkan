Modern C++23 approach to vulkan using modules. Based on <https://vulkan-tutorial.com/Introduction> 

### GCC 15:
Note, that this project is currently building under experimental GCC-15 compiler you can get and build from here: <https://gcc.gnu.org/git.html>

Quick way:
```
git clone git://gcc.gnu.org/git/gcc.git
cd gcc
contrib/download_prerequisites
mkdir gcc-build
cd gcc-build
../configure --prefix=$HOME/.local --program-suffix=-15 --disable-bootstrap --disable-multilib --enable-languages=c,c++,lto
make -j$(nproc)
make install
```

Don't forget to add `export LD_LIBRARY_PATH="$HOME/.local/lib:$HOME/.local/lib64:$LD_LIBRARY_PATH"` if your distro doesn't normally use home local.

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