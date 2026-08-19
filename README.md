# Vulkan rendering engine

Vulkan based rendering engine written mostly from scratch, utilising the SDL, GLM, tinyobjloader.h, stb_image.h, imgui and xxHash libraries.

for easy dependencies on Arch-based systems:

```bash
  sudo pacman -S base-devel shaderc sdl3 sdl3_image glm vulkan-devel
```

Currently features Bindless descriptor arrays, shadowmapping (although this one is currently broken until I finish implementing cascaded shadowmapping), scene loading and saving within engine, material loading and saving within engine, obj importing, an IMGUI user interface, and a few other things.
