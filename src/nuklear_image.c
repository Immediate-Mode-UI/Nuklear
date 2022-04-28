#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                          IMAGE
 *
 * ===============================================================*/
NK_API nk_handle
nk_handle_ptr(void *ptr)
{
    nk_handle handle = {0};
    handle.ptr = ptr;
    return handle;
}
NK_API nk_handle
nk_handle_id(int id)
{
    nk_handle handle;
    nk_zero_struct(handle);
    handle.id = id;
    return handle;
}
NK_API struct nk_image
nk_subimage_from_ptr(void *ptr, nk_ushort w, nk_ushort h, enum nk_image_type type, struct nk_rect r)
{
    struct nk_image s;
    nk_zero(&s, sizeof(s));
    s.handle.ptr = ptr;
    NK_ASSERT(w); 		/* Size may not be zero */
    NK_ASSERT(h); 		/* Size may not be zero */
    s.w = w; s.h = h;
    /* Because a subimage just uses modified UVs to show a small portion of an
       nk_image, using CENTER or TILED will result in the rest of the image
       bleeding through. */
    NK_ASSERT(type != NK_IMAGE_CENTER && type != NK_IMAGE_TILE);
    s.type = type;
    s.region[0] = (nk_ushort)r.x;
    s.region[1] = (nk_ushort)r.y;
    /* A subregion with dimensions 0 is not valid and won't display. */
    NK_ASSERT( r.w );
    NK_ASSERT( r.h );
    s.region[2] = (nk_ushort)r.w;
    s.region[3] = (nk_ushort)r.h;
    return s;
}
NK_API struct nk_image
nk_subimage_from_id(int id, nk_ushort w, nk_ushort h, enum nk_image_type type, struct nk_rect r)
{
    struct nk_image s;
    nk_zero(&s, sizeof(s));
    s.handle.id = id;
    NK_ASSERT(w); 		/* Size may not be zero */
    NK_ASSERT(h); 		/* Size may not be zero */
    s.w = w; s.h = h;
    /* Because a subimage just uses modified UVs to show a small portion of an
       nk_image, using CENTER or TILED will result in the rest of the image
       bleeding through. */
    NK_ASSERT(type != NK_IMAGE_CENTER && type != NK_IMAGE_TILE);
    s.type = type;
    s.region[0] = (nk_ushort)r.x;
    s.region[1] = (nk_ushort)r.y;
    /* A subregion with dimensions 0 is not valid and won't display. */
    NK_ASSERT( r.w );
    NK_ASSERT( r.h );
    s.region[2] = (nk_ushort)r.w;
    s.region[3] = (nk_ushort)r.h;
    return s;
}
NK_API struct nk_image
nk_subimage_from_handle(nk_handle handle, nk_ushort w, nk_ushort h, enum nk_image_type type, struct nk_rect r)
{
    struct nk_image s;
    nk_zero(&s, sizeof(s));
    s.handle = handle;
    NK_ASSERT(w); 		/* Size may not be zero */
    NK_ASSERT(h); 		/* Size may not be zero */
    s.w = w; s.h = h;
    /* Because a subimage just uses modified UVs to show a small portion of an
       nk_image, using CENTER or TILED will result in the rest of the image
       bleeding through. */
    NK_ASSERT(type != NK_IMAGE_CENTER && type != NK_IMAGE_TILE);
    s.type = type;
    s.region[0] = (nk_ushort)r.x;
    s.region[1] = (nk_ushort)r.y;
    /* A subregion with dimensions 0 is not valid and won't display. */
    NK_ASSERT( r.w );
    NK_ASSERT( r.h );
    s.region[2] = (nk_ushort)r.w;
    s.region[3] = (nk_ushort)r.h;
    return s;
}
NK_API struct nk_image
nk_image_from_handle(nk_handle handle, nk_ushort w, nk_ushort h, enum nk_image_type type)
{
    struct nk_image s;
    nk_zero(&s, sizeof(s));
    s.handle = handle;
    NK_ASSERT(w); 		/* Size may not be zero */
    NK_ASSERT(h); 		/* Size may not be zero */
    s.w = w; s.h = h;
    s.type = type;
    s.region[0] = 0;
    s.region[1] = 0;
    s.region[2] = 0;
    s.region[3] = 0;
    return s;
}
NK_API struct nk_image
nk_image_from_ptr(void *ptr, nk_ushort w, nk_ushort h, enum nk_image_type type)
{
    struct nk_image s;
    nk_zero(&s, sizeof(s));
    NK_ASSERT(ptr);
    s.handle.ptr = ptr;
    NK_ASSERT(w); 		/* Size may not be zero */
    NK_ASSERT(h); 		/* Size may not be zero */
    s.w = w; s.h = h;
    s.type = type;
    s.region[0] = 0;
    s.region[1] = 0;
    s.region[2] = 0;
    s.region[3] = 0;
    return s;
}
NK_API struct nk_image
nk_image_from_id(int id, nk_ushort w, nk_ushort h, enum nk_image_type type)
{
    struct nk_image s;
    nk_zero(&s, sizeof(s));
    s.handle.id = id;
    NK_ASSERT(w); 		/* Size may not be zero */
    NK_ASSERT(h); 		/* Size may not be zero */
    s.w = w; s.h = h;
    s.type = type;
    s.region[0] = 0;
    s.region[1] = 0;
    s.region[2] = 0;
    s.region[3] = 0;
    return s;
}
NK_API nk_bool
nk_image_is_subimage(const struct nk_image* img)
{
    NK_ASSERT(img);
    return !(img->region[0] == 0 && img->region[1] == 0 &&
	     img->region[2] == 0 && img->region[3] == 0);
}
NK_API void
nk_image(struct nk_context *ctx, struct nk_image img)
{
    struct nk_window *win;
    struct nk_rect bounds;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    if (!nk_widget(&bounds, ctx)) return;
    nk_draw_image(&win->buffer, bounds, &img, nk_white);
}
NK_API void
nk_image_color(struct nk_context *ctx, struct nk_image img, struct nk_color col)
{
    struct nk_window *win;
    struct nk_rect bounds;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    if (!nk_widget(&bounds, ctx)) return;
    nk_draw_image(&win->buffer, bounds, &img, col);
}

