/* STB Image does not compile as C89 :[ */
#define STB_IMAGE_IMPLEMENTATION
#include "../../demo/common/libs/stb_image_c89.h"


/* abusing nk_image type to avoid dependency issues */
#ifndef load_image
struct nk_image load_image(const char* filepath)
{
	int w, h;
	struct nk_image img = {0};
	img.handle.ptr = stbi_load(filepath, &w, &h, NULL, STBI_rgb_alpha);
	if (!img.handle.ptr) {
		fprintf(stderr, "Failed to load image: %s\n", filepath);
		return img;
	}

	img.w = w;
	img.h = h;
	return img;
}
#endif

#ifdef USING_OPENGL
#define create_nk_image createTextureOpenGL
struct nk_image createTextureOpenGL(struct nk_image img)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Use GL_RGB or GL_RGBA for internal format because GLES2 only supports
	 * GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA for both format args
	 *
	 * RGBA because that's the format img is in
	 * */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.w, img.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.handle.ptr);
	return nk_image_type_id(textureID, img.w, img.h, NK_IMAGE_STRETCH);
}
#endif

/*
// assumes RGBA for now, creates range [0,0] to [uv.x, uv.y] so if you want to start
// at go from [-.5, -.5] to [1.5, 1.5], just give [2.5,2.5] and use nk_subimage to select
// [.2, .2] to [1.0, 1.0] for the same appearance

*/
struct nk_image nk_tile_image(struct nk_image img, struct nk_vec2 uv)
{
	int w,h,i,j;
	nk_uint* tiled; /* 32-bit uint */
	nk_uint* orig;
	struct nk_image out = {0};

	NK_ASSERT(img.handle.ptr);
	NK_ASSERT(img.w && img.h);
	NK_ASSERT(uv.x > 0 && uv.y > 0);

	w = nk_iceilf(img.w * uv.x);
	h = nk_iceilf(img.h * uv.y);

	orig = img.handle.ptr;
	tiled = malloc(w*h*sizeof(nk_uint));
	if (!tiled) { return out; }
	for (i=0; i<h; ++i) {
		for (j=0; j<w; ++j) {
			tiled[i*w + j] = orig[(i % img.h) * img.w + j % img.w];
		}
	}

	out.handle.ptr = tiled;
	out.w = w;
	out.h = h;

	out.type = NK_IMAGE_TILE;

	return out;
}



#ifndef create_nk_image
#error "Must define create_nk_image() for the image demo!"
#endif

int imagesLoaded = nk_false;
struct nk_image bunny_wide;
struct nk_image bunny_portrait;
struct nk_image tile;

struct nk_image bw_stretch, bw_fill, bw_fit, bw_center, bw_tile;
struct nk_image bp_stretch, bp_fill, bp_fit, bp_center, bp_tile;
struct nk_image tile_stretch, tile_fill, tile_fit, tile_center, tile_tile;

static void
image_demo(struct nk_context *ctx)
{
	if (!imagesLoaded) {
		/* It would be more convenient if we could pass the type as a third parameter
		 * to nk_image() but that would be a breaking change and require a few more
		 * internal changes */

		bunny_wide = load_image("../../demo/common/img/bunny_wide.jpg");
		bunny_portrait = load_image("../../demo/common/img/bunny_portrait.jpg");
		tile = load_image("../../demo/common/img/tile.jpg");

		/* stretch as the default */
		bw_stretch = create_nk_image(bunny_wide);
		bp_stretch = create_nk_image(bunny_portrait);
		tile_stretch = create_nk_image(tile);

		bw_fill = bw_stretch; bw_fill.type = NK_IMAGE_FILL;
		bw_fit = bw_stretch; bw_fit.type = NK_IMAGE_FIT;
		bw_center = bw_stretch; bw_center.type = NK_IMAGE_CENTER;
		bw_tile = bw_stretch; bw_tile.type = NK_IMAGE_TILE;

		bp_fill = bp_stretch; bp_fill.type = NK_IMAGE_FILL;
		bp_fit = bp_stretch; bp_fit.type = NK_IMAGE_FIT;
		bp_center = bp_stretch; bp_center.type = NK_IMAGE_CENTER;
		bp_tile = bp_stretch; bp_tile.type = NK_IMAGE_TILE;

		tile_fill = tile_stretch; tile_fill.type = NK_IMAGE_FILL;
		tile_fit = tile_stretch; tile_fit.type = NK_IMAGE_FIT;
		tile_center = tile_stretch; tile_center.type = NK_IMAGE_CENTER;
		tile_tile = tile_stretch; tile_tile.type = NK_IMAGE_TILE;

#ifdef NO_TILING
		{
		struct nk_image tmp = nk_tile_image(bunny_wide, nk_vec2(10, 1));
		bw_tile = create_nk_image(tmp); /* free(tmp.handle.ptr); */
		bw_tile.type = NK_IMAGE_TILE;

		tmp = nk_tile_image(bunny_portrait, nk_vec2(15, 1));
		bp_tile = create_nk_image(tmp); /* free(tmp.handle.ptr); */
		bp_tile.type = NK_IMAGE_TILE;

		tmp = nk_tile_image(tile, nk_vec2(50, 4));
		tile_tile = create_nk_image(tmp); /* free(tmp.handle.ptr); */
		tile_tile.type = NK_IMAGE_TILE;
		}
#endif

		/*
		 * Can't free because xlib doesn't actualy copy the data
		//free(bunny_wide.handle.ptr);
		//free(bunny_portrait.handle.ptr);
		//free(tile.handle.ptr);
		*/

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
		nk_image(ctx, bw_stretch);

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

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_TILE", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bw_tile);
	}
	nk_end(ctx);

	if (nk_begin(ctx, "Image portrait", nk_rect(260, 25, 230, 700),
	             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
	             NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_STRETCH", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bp_stretch);

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

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_TILE", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, bp_tile);
	}
	nk_end(ctx);

	if (nk_begin(ctx, "tile", nk_rect(500, 25, 230, 700),
	             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
	             NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_STRETCH", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, tile_stretch);

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

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "NK_IMAGE_TILE", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 100, 1);
		nk_image(ctx, tile_tile);
	}
	nk_end(ctx);
}
