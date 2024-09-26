/* ===============================================================
 *
 *                          Skin Demo
 *
 * ===============================================================*/
/* This shouldn't actually be committed like this, but rather a
 * a separate ../common/<*>.c , to make it backend agnostic. I just
 * need something quick to demo the skin. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "../../nuklear.h"
#include "nuklear_glfw_gl3.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

struct {
    struct nk_style_button primary;
    struct nk_style_button secondary;
    struct nk_style_button danger;
    struct nk_style_button disabled;
} breeze_button;

static void error_callback(int e, const char *d)
{printf("Error %d: %s\n", e, d);}

int main(void)
{
    /* Platform */
    struct nk_glfw glfw = {0};
    static GLFWwindow *win;
    int width = 0, height = 0;
    struct nk_context *ctx;

    /* GLFW */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stdout, "[GFLW] failed to init!\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "SkinDemo", NULL, NULL);
    glfwMakeContextCurrent(win);
    glfwGetWindowSize(win, &width, &height);

    /* OpenGL */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(1);
    }

    /* Font */
    ctx = nk_glfw3_init(&glfw, win, NK_GLFW3_INSTALL_CALLBACKS);
    {
	struct nk_font_atlas *atlas;
	nk_glfw3_font_stash_begin(&glfw, &atlas);
	struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../extra_font/Roboto-Regular.ttf", 27.25, 0);
	nk_glfw3_font_stash_end(&glfw);
	nk_style_set_font(ctx, &droid->handle);
    }

    /* Import Style Texture */
    /* Not C89 anymore ( ;   ;   )*/
    int x,y,n;
    unsigned char *data = stbi_load("../../example/skins/Breeze.png", &x, &y, &n, 4);
    GLuint gpu_tex;
    
    glGenTextures(1, &gpu_tex);
    glBindTexture(GL_TEXTURE_2D, gpu_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    ctx->style.text.color = nk_rgb(239, 240, 241);
    ctx->style.window.padding = nk_vec2(42, 42);
    ctx->style.window.fixed_background = nk_style_item_nine_slice(
	nk_sub9slice_id(gpu_tex, x, y,
			(struct nk_rect){ 480, 0, 96, 96},
			42, 42, 42, 42));

    breeze_button.primary = ctx->style.button;
    breeze_button.primary.normal = nk_style_item_nine_slice(
	nk_sub9slice_id(gpu_tex, x, y,
			(struct nk_rect){ 6, 6, 84, 84},
			26, 26, 26, 26));
    breeze_button.primary.hover = nk_style_item_nine_slice(
	nk_sub9slice_id(gpu_tex, x, y,
			(struct nk_rect){ 102, 6, 84, 84},
			26, 26, 26, 26));
    breeze_button.primary.active = nk_style_item_nine_slice(
	nk_sub9slice_id(gpu_tex, x, y,
			(struct nk_rect){ 198, 6, 84, 84},
			26, 26, 26, 26));
    breeze_button.primary.text_normal = nk_rgb( 239, 240, 241);
    breeze_button.primary.text_hover = nk_rgb( 255, 255, 255);
    breeze_button.primary.text_active = nk_rgb( 239, 240, 241);

    breeze_button.danger = ctx->style.button;
    breeze_button.danger.normal = nk_style_item_nine_slice(
	nk_sub9slice_id(gpu_tex, x, y,
			(struct nk_rect){ 6, 102, 84, 84},
			26, 26, 26, 26));
    breeze_button.danger.hover = nk_style_item_nine_slice(
	nk_sub9slice_id(gpu_tex, x, y,
			(struct nk_rect){ 102, 102, 84, 84},
			26, 26, 26, 26));
    breeze_button.danger.active = nk_style_item_nine_slice(
	nk_sub9slice_id(gpu_tex, x, y,
			(struct nk_rect){ 198, 102, 84, 84},
			26, 26, 26, 26));
    breeze_button.danger.text_normal = nk_rgb( 239, 240, 241);
    breeze_button.danger.text_hover = nk_rgb( 255, 255, 255);
    breeze_button.danger.text_active = nk_rgb( 239, 240, 241);
    
    glClearColor(0, 0, 0, 0);
    while (!glfwWindowShouldClose(win))
	{
	    /* Input */
	    glfwPollEvents();
	    nk_glfw3_new_frame(&glfw);

	    /* GUI */
	    if (nk_begin(ctx, "Demo", nk_rect(100, 100, 520, 220),
			 NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE))
		{
		    nk_layout_row_dynamic(ctx, 32, 1);
		    nk_label(ctx, "Add or delete \"Thing\"", NK_TEXT_LEFT);
		    nk_layout_row_dynamic(ctx, nk_window_get_content_region(ctx).h - 200, 2);
		    nk_layout_row_begin(ctx, NK_DYNAMIC, 84, 3);
		    {
			nk_layout_row_push(ctx, 0.3);
			nk_button_label_styled(ctx, &breeze_button.danger, "Destroy!");
			nk_layout_row_push(ctx, 0.1);
			nk_spacer(ctx);
			nk_layout_row_push(ctx, 0.6);
			nk_button_label_styled(ctx, &breeze_button.primary, "Add Thing");
			nk_layout_row_end(ctx);
		    }
		}
	    nk_end(ctx);

	    /* Draw */
	    glfwGetWindowSize(win, &width, &height);
	    glViewport(0, 0, width, height);
	    glClear(GL_COLOR_BUFFER_BIT);

	    /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
	     * with blending, scissor, face culling, depth test and viewport and
	     * defaults everything back into a default state.
	     * Make sure to either a.) save and restore or b.) reset your own state after
	     * rendering the UI. */
	    nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
	    glfwSwapBuffers(win);
	}
    nk_glfw3_shutdown(&glfw);
    glfwTerminate();
    return 0;
}

