# RenderingFramework3D
A simple but extensible 3D rendering framework using Vulkan and C++ that allows for rendering 3D objects with a balance between ease of use and flexibility.The framework is designed for GPU accelerated realtime 2D or 3D rendering of similations or simple games. It provides a modular API with very few dependencies making it easy to integrate into applications. This project is a work in progress with potential new features to be added in future.

# Features
- __Simple API:__ Easy-to-use API for window creation, load meshes, move around objects in 3D space and rendering, while abstracting low-level hardware related details of Vulkan API. Some default pipelines are available to choose from if needed; enable/disable lighting, wireframe rendering or line rendering.

- __Camera System:__ Provides an intuitive camera system to move the camera POV in 3D space, scale camera image, configure view-port and adjust FOV. Choose between isometric or perspective projection for 2D or 3D rendering.

- __Rendering with Vulkan:__ Built using the Vulkan API, providing hardware-accelerated rendering.

- **Supported Platforms:** The framework supports x86_64 architectures on Windows and Linux, based on the compatibility of the underlying libraries.


- __Customizable Pipelines:__ Support to create custom pipelines with custom vertex and fragment shaders written in GLSL and compiled into SPIR-V. Some rasterizer configurations are also available.

- __Dynamic Meshes:__ Ability to modify mesh vertex data dynamically after initially loading into GPU memory.


# Dependencies

This framework relies on the following libraries:

- ### [Vulkan SDK 1.3.296.0](https://vulkan.lunarg.com/)

    Vulkan API for high-performance GPU rendering.

- ### [GLFW](https://www.glfw.org/)

    A cross-platform library for creating windows, managing OpenGL/Vulkan contexts, and handling user input.


## Build

### Prerequisites
Ensure the following are installed before building the project:
- **C/C++ Toolchain**: Compatible compilers include MSVC, Clang, or  G++ (GNU C++ Compiler).
- **[CMake v3.15+](https://cmake.org/download/)**: For project configuration and build automation.
- **[Vulkan SDK 1.3.296.0](https://vulkan.lunarg.com/)**: Required for Vulkan API support.
- **Linux-Specific Dependencies**:
  - **X11** (Xlib/XCB) or **Wayland**: Necessary as a backend for GLFW.


### Build Instructions

1. Navigate to the `build` directory of the repository:

2. Run the following commands to configure and build the project, specifying the Vulkan SDK directory:

```bash
cmake .. -DVULKAN_DIR=<path_to_vulkan_sdk>
cmake --build ./
```
Replace `<path_to_vulkan_sdk>` with the path to your installed Vulkan SDK

### Output

After a successful build:
- A static library will be generated in the `build/lib` directory:
  - **Windows**: `rfw3d.lib`
  - **Linux**: `librfw3d.a`
- Test programs will be available in the `build/bin` directory:
  - The source files for these tests can be found in the `test/` directory.
  - These examples demonstrate the use of the **RenderingFramework3D** API.


# TODO
- Add Support for loading textures and applying to objects.
- Add support for multiple renderpasses for more advanced effects like shadow mapping, mirror reflections or translucent surfaces etc.
- Low-level optimizations such as queue and command buffer management, synchronization, and memory management for improved performance.
- More thorough testing required to run on Linux