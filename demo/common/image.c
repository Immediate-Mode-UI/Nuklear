/* STB Image does not compile as C89 :[ */
#define STB_IMAGE_IMPLEMENTATION
#include "../../demo/common/libs/stb_image_c89.h"

struct testImage {
	GLuint gpuTextureID;
	int width;
	int height;
};

struct testImage loadTextureOpenGL(const char *filepath)
{
	int width, height, channels;
	unsigned char *data = stbi_load(filepath, &width, &height, &channels, 0);
	if (!data)
	{
		fprintf(stderr, "Failed to load image: %s\n", filepath);
		return (struct testImage){0, 0, 0};
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	return (struct testImage){textureID, width, height};
}

int imagesLoaded = nk_false;
struct testImage bunny_wide;
struct testImage bunny_portrait;
struct testImage tile;

static void
image_demo(struct nk_context *ctx)
{
	if(!imagesLoaded){
		bunny_wide = loadTextureOpenGL("../../demo/common/img/bunny_wide.jpg");
		bunny_portrait = loadTextureOpenGL("../../demo/common/img/bunny_portrait.jpg");
		tile = loadTextureOpenGL("../../demo/common/img/tile.jpg");
		imagesLoaded = nk_true;
	}

	/* Normally, you create your nk_image with your desired settings. This demo
	   however currently takes in the GPU texture ID and recreates the nk_image
	   in place, which is not resource friendly. */
	if (nk_begin(ctx, "Image Wide", nk_rect(5, 25, 250, 700),
				 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
					 NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
	{

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_STRETCH", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_wide.gpuTextureID, bunny_wide.width, bunny_wide.height, NK_IMAGE_STRETCH));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FILL", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_wide.gpuTextureID, bunny_wide.width, bunny_wide.height, NK_IMAGE_FILL));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FIT", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_wide.gpuTextureID, bunny_wide.width, bunny_wide.height, NK_IMAGE_FIT));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_CENTER", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_wide.gpuTextureID, bunny_wide.width, bunny_wide.height, NK_IMAGE_CENTER));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_TILE", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_wide.gpuTextureID, bunny_wide.width, bunny_wide.height, NK_IMAGE_TILE));
	}
	nk_end(ctx);

	if (nk_begin(ctx, "Image portrait", nk_rect(260, 25, 230, 700),
				 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
					 NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_STRETCH", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_portrait.gpuTextureID, bunny_portrait.width, bunny_portrait.height, NK_IMAGE_STRETCH));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FILL", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_portrait.gpuTextureID, bunny_portrait.width, bunny_portrait.height, NK_IMAGE_FILL));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FIT", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_portrait.gpuTextureID, bunny_portrait.width, bunny_portrait.height, NK_IMAGE_FIT));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_CENTER", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_portrait.gpuTextureID, bunny_portrait.width, bunny_portrait.height, NK_IMAGE_CENTER));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_TILE", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(bunny_portrait.gpuTextureID, bunny_portrait.width, bunny_portrait.height, NK_IMAGE_TILE));
	}
	nk_end(ctx);

	if (nk_begin(ctx, "tile", nk_rect(500, 25, 230, 700),
				 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
					 NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_STRETCH", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(tile.gpuTextureID, tile.width, tile.height, NK_IMAGE_STRETCH));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FILL", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(tile.gpuTextureID, tile.width, tile.height, NK_IMAGE_FILL));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FIT", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(tile.gpuTextureID, tile.width, tile.height, NK_IMAGE_FIT));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_CENTER", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(tile.gpuTextureID, tile.width, tile.height, NK_IMAGE_CENTER));
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_TILE", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, nk_image_type_id(tile.gpuTextureID, tile.width, tile.height, NK_IMAGE_TILE));
	}
	nk_end(ctx);
}