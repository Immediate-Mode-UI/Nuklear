Contrary to OpenGL Vulkan needs precompiled shaders in the SPIR-V format which makes it a bit more difficult to inline the shadercode.

After executing `make` the result should be a self contained `nuklear_sdl_vulkan.h`. Copy the result file to the parent directory and the "release" should be done.

You will need to have `xxd`, `glslc` and `awk` installed for this.
