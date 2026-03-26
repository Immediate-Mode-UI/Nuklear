create_shader_inlined_header: nuklearshaders/nuklear.vert.spv nuklearshaders/nuklear.frag.spv
	awk -v st='// NUKLEAR_SHADERS_START' -v et='// NUKLEAR_SHADERS_END' -v repl="$$(xxd -i nuklearshaders/nuklear.vert.spv && xxd -i nuklearshaders/nuklear.frag.spv)" '$$0 == st{del=1} $$0 == et{$$0 = repl; del=0} !del' nuklear_sdl_vulkan.in.h > nuklear_sdl_vulkan.h

nuklearshaders/nuklear.vert.spv: nuklearshaders/nuklear.vert
	glslc --target-env=vulkan nuklearshaders/nuklear.vert -o nuklearshaders/nuklear.vert.spv

nuklearshaders/nuklear.frag.spv: nuklearshaders/nuklear.frag
	glslc --target-env=vulkan nuklearshaders/nuklear.frag -o nuklearshaders/nuklear.frag.spv

clean:
	rm nuklearshaders/nuklear.vert.spv nuklearshaders/nuklear.frag.spv nuklear_sdl_vulkan.h
