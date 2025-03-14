/* STB Image does not compile as C89 :[ */
#define STB_IMAGE_IMPLEMENTATION
#include "../../demo/common/libs/stb_image_c89.h"


#ifdef USING_OPENGL
#define load_nk_image loadTextureOpenGL
struct nk_image loadTextureOpenGL(const char *filepath)
{
	int width, height, channels;
	GLuint textureID;
	unsigned char *data = stbi_load(filepath, &width, &height, &channels, 0);
	if (!data)
	{
		fprintf(stderr, "Failed to load image: %s\n", filepath);
		return nk_image_type_id(0, 0, 0, 0);
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Use GL_RGB or GL_RGBA for internal format because GLES2 only supports
	 * GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA for both format args */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	return nk_image_type_id(textureID, width, height, NK_IMAGE_STRETCH);
}
#endif



#ifndef load_nk_image
#error "Must define load_nk_image() for the image demo!"
#endif

int imagesLoaded = nk_false;
struct nk_image bunny_wide;
struct nk_image bunny_portrait;
struct nk_image tile;

struct nk_image bw_fill, bw_fit, bw_center, bw_tile;
struct nk_image bp_fill, bp_fit, bp_center, bp_tile;
struct nk_image tile_fill, tile_fit, tile_center, tile_tile;

static void
image_demo(struct nk_context *ctx)
{
	if (!imagesLoaded) {
		/* It would be more convenient if we could pass the type as a third parameter
		 * to nk_image() but that would be a breaking change and require a few more
		 * internal changes */

		/* stretch as the default */
		bunny_wide = load_nk_image("../../demo/common/img/bunny_wide.jpg");
		bunny_portrait = load_nk_image("../../demo/common/img/bunny_portrait.jpg");
		tile = load_nk_image("../../demo/common/img/tile.jpg");

		bw_fill = bunny_wide; bw_fill.type = NK_IMAGE_FILL;
		bw_fit = bunny_wide; bw_fit.type = NK_IMAGE_FIT;
		bw_center = bunny_wide; bw_center.type = NK_IMAGE_CENTER;
		bw_tile = bunny_wide; bw_tile.type = NK_IMAGE_TILE;

		bp_fill = bunny_portrait; bp_fill.type = NK_IMAGE_FILL;
		bp_fit = bunny_portrait; bp_fit.type = NK_IMAGE_FIT;
		bp_center = bunny_portrait; bp_center.type = NK_IMAGE_CENTER;
		bp_tile = bunny_portrait; bp_tile.type = NK_IMAGE_TILE;

		tile_fill = tile; tile_fill.type = NK_IMAGE_FILL;
		tile_fit = tile; tile_fit.type = NK_IMAGE_FIT;
		tile_center = tile; tile_center.type = NK_IMAGE_CENTER;
		tile_tile = tile; tile_tile.type = NK_IMAGE_TILE;

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
		nk_image(ctx, bunny_wide);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FILL", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bw_fill);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FIT", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bw_fit);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_CENTER", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bw_center);

#ifndef NO_TILING
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_TILE", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bw_tile);
#endif
	}
	nk_end(ctx);

	if (nk_begin(ctx, "Image portrait", nk_rect(260, 25, 230, 700),
	             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
	             NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_STRETCH", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bunny_portrait);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FILL", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bp_fill);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FIT", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bp_fit);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_CENTER", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bp_center);

#ifndef NO_TILING
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_TILE", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bp_tile);
#endif
	}
	nk_end(ctx);

	if (nk_begin(ctx, "tile", nk_rect(500, 25, 230, 700),
	             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
	             NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_STRETCH", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, tile);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FILL", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, tile_fill);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_FIT", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, tile_fit);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_CENTER", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, tile_center);

#ifndef NO_TILING
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_TILE", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, tile_tile);
#endif
	}
	nk_end(ctx);
}
