# nuklear glfw vulkan

## Theory of operation

The nuklear glfw vulkan integration creates an independent graphics pipeline that will render the nuklear UI to separate render targets.
The application is responsible to fully manage these render targets. So it must ensure they are properly sized (and resized when requested).

Furthermore it is assumed that you will have a swap chain in place and the number of nuklear overlay images and number of swap chain images match.

This is how you can integrate it in your application:

```
/*
Setup: overlay_images have been created and their number match with the number
of the swap_chain_images of your application. The overlay_images in this
example have the same format as your swap_chain images (optional)
*/
    struct nk_context *ctx = nk_glfw3_init(
        demo.win, demo.device, demo.physical_device, demo.indices.graphics,
        demo.overlay_image_views, demo.swap_chain_images_len,
        demo.swap_chain_image_format, NK_GLFW3_INSTALL_CALLBACKS,
        MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
[...]
/*
in your draw loop draw you can then render to the overlay image at
`image_index`
your own application can then wait for the semaphore and produce
the swap_chain_image at `image_index`
this should simply sample from the overlay_image (see example)
*/
nk_semaphore semaphore =
    nk_glfw3_render(demo.graphics_queue, image_index,
                    demo.image_available, NK_ANTI_ALIASING_ON);
    if (!render(&demo, &bg, nk_semaphore, image_index)) {
        fprintf(stderr, "render failed\n");
        return false;
    }
```

You must call `nk_glfw3_resize` whenever the size of the overlay_images resize.

## Using images

Images can be used by providing a VkImageView as an nk_image_ptr to nuklear:

```
img = nk_image_ptr(demo.demo_texture_image_view);
```

Note that they must have SHADER_READ_OPTIMAL layout

It is currently not possible to specify how they are being sampled. The nuklear glfw vulkan integration uses a fixed sampler that does linear filtering.
