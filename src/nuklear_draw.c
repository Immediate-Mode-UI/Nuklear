#include "nuklear.h"
#include "nuklear_internal.h"

/* ==============================================================
 *
 *                          DRAW
 *
 * ===============================================================*/
NK_LIB void
nk_command_buffer_init(struct nk_command_buffer *cb,
    struct nk_buffer *b, enum nk_command_clipping clip)
{
    NK_ASSERT(cb);
    NK_ASSERT(b);
    if (!cb || !b) return;
    cb->base = b;
    cb->use_clipping = (int)clip;
    cb->begin = b->allocated;
    cb->end = b->allocated;
    cb->last = b->allocated;
#ifdef NK_INCLUDE_COMMAND_USERDATA
    cb->userdata.ptr = 0;
#endif
    cb->draw_config = 0;
}
NK_LIB void
nk_command_buffer_reset(struct nk_command_buffer *b)
{
    NK_ASSERT(b);
    if (!b) return;
    b->begin = 0;
    b->end = 0;
    b->last = 0;
    b->clip = nk_null_rect;
#ifdef NK_INCLUDE_COMMAND_USERDATA
    b->userdata.ptr = 0;
#endif
    b->draw_config = 0;
}
NK_LIB void*
nk_command_buffer_push(struct nk_command_buffer* b,
    enum nk_command_type t, nk_size size)
{
    NK_STORAGE const nk_size align = NK_ALIGNOF(struct nk_command);
    struct nk_command *cmd;
    nk_size alignment;
    void *unaligned;
    void *memory;

    NK_ASSERT(b);
    NK_ASSERT(b->base);
    if (!b) return 0;
    cmd = (struct nk_command*)nk_buffer_alloc(b->base,NK_BUFFER_FRONT,size,align);
    if (!cmd) return 0;

    /* make sure the offset to the next command is aligned */
    b->last = (nk_size)((nk_byte*)cmd - (nk_byte*)b->base->memory.ptr);
    unaligned = (nk_byte*)cmd + size;
    memory = NK_ALIGN_PTR(unaligned, align);
    alignment = (nk_size)((nk_byte*)memory - (nk_byte*)unaligned);
#ifdef NK_ZERO_COMMAND_MEMORY
    NK_MEMSET(cmd, 0, size + alignment);
#endif

    cmd->type = t;
    cmd->next = b->base->allocated + alignment;
#ifdef NK_INCLUDE_COMMAND_USERDATA
    cmd->userdata = b->userdata;
#endif
    b->end = cmd->next;
    return cmd;
}
NK_API void
nk_push_scissor(struct nk_command_buffer *b, struct nk_rect r)
{
    struct nk_command_scissor *cmd;
    NK_ASSERT(b);
    if (!b) return;

    b->clip.x = r.x;
    b->clip.y = r.y;
    b->clip.w = r.w;
    b->clip.h = r.h;
    cmd = (struct nk_command_scissor*)
        nk_command_buffer_push(b, NK_COMMAND_SCISSOR, sizeof(*cmd));

    if (!cmd) return;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NK_MAX(0, r.w);
    cmd->h = (unsigned short)NK_MAX(0, r.h);
}
NK_API void
nk_stroke_line(struct nk_command_buffer *b, float x0, float y0,
    float x1, float y1, float line_thickness, struct nk_color c)
{
    struct nk_command_line *cmd;
    NK_ASSERT(b);
    if (!b || line_thickness <= 0) return;
    cmd = (struct nk_command_line*)
        nk_command_buffer_push(b, NK_COMMAND_LINE, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->begin.x = (short)x0;
    cmd->begin.y = (short)y0;
    cmd->end.x = (short)x1;
    cmd->end.y = (short)y1;
    cmd->color = c;
}
NK_API void
nk_stroke_curve(struct nk_command_buffer *b, float ax, float ay,
    float ctrl0x, float ctrl0y, float ctrl1x, float ctrl1y,
    float bx, float by, float line_thickness, struct nk_color col)
{
    struct nk_command_curve *cmd;
    NK_ASSERT(b);
    if (!b || col.a == 0 || line_thickness <= 0) return;

    cmd = (struct nk_command_curve*)
        nk_command_buffer_push(b, NK_COMMAND_CURVE, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->begin.x = (short)ax;
    cmd->begin.y = (short)ay;
    cmd->ctrl[0].x = (short)ctrl0x;
    cmd->ctrl[0].y = (short)ctrl0y;
    cmd->ctrl[1].x = (short)ctrl1x;
    cmd->ctrl[1].y = (short)ctrl1y;
    cmd->end.x = (short)bx;
    cmd->end.y = (short)by;
    cmd->color = col;
}
NK_API void
nk_stroke_rect(struct nk_command_buffer *b, struct nk_rect rect,
    float rounding, float line_thickness, struct nk_color c)
{
    struct nk_command_rect *cmd;
    NK_ASSERT(b);
    if (!b || c.a == 0 || rect.w == 0 || rect.h == 0 || line_thickness <= 0) return;
    if (b->use_clipping) {
        const struct nk_rect *clip = &b->clip;
        if (!NK_INTERSECT(rect.x, rect.y, rect.w, rect.h,
            clip->x, clip->y, clip->w, clip->h)) return;
    }
    cmd = (struct nk_command_rect*)
        nk_command_buffer_push(b, NK_COMMAND_RECT, sizeof(*cmd));
    if (!cmd) return;
    cmd->rounding = (unsigned short)rounding;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->x = (short)rect.x;
    cmd->y = (short)rect.y;
    cmd->w = (unsigned short)NK_MAX(0, rect.w);
    cmd->h = (unsigned short)NK_MAX(0, rect.h);
    cmd->color = c;
}
NK_API void
nk_fill_rect(struct nk_command_buffer *b, struct nk_rect rect,
    float rounding, struct nk_color c)
{
    struct nk_command_rect_filled *cmd;
    NK_ASSERT(b);
    if (!b || c.a == 0 || rect.w == 0 || rect.h == 0) return;
    if (b->use_clipping) {
        const struct nk_rect *clip = &b->clip;
        if (!NK_INTERSECT(rect.x, rect.y, rect.w, rect.h,
            clip->x, clip->y, clip->w, clip->h)) return;
    }

    cmd = (struct nk_command_rect_filled*)
        nk_command_buffer_push(b, NK_COMMAND_RECT_FILLED, sizeof(*cmd));
    if (!cmd) return;
    cmd->rounding = (unsigned short)rounding;
    cmd->x = (short)rect.x;
    cmd->y = (short)rect.y;
    cmd->w = (unsigned short)NK_MAX(0, rect.w);
    cmd->h = (unsigned short)NK_MAX(0, rect.h);
    cmd->color = c;
}
NK_API void
nk_fill_rect_multi_color(struct nk_command_buffer *b, struct nk_rect rect,
    struct nk_color left, struct nk_color top, struct nk_color right,
    struct nk_color bottom)
{
    struct nk_command_rect_multi_color *cmd;
    NK_ASSERT(b);
    if (!b || rect.w == 0 || rect.h == 0) return;
    if (b->use_clipping) {
        const struct nk_rect *clip = &b->clip;
        if (!NK_INTERSECT(rect.x, rect.y, rect.w, rect.h,
            clip->x, clip->y, clip->w, clip->h)) return;
    }

    cmd = (struct nk_command_rect_multi_color*)
        nk_command_buffer_push(b, NK_COMMAND_RECT_MULTI_COLOR, sizeof(*cmd));
    if (!cmd) return;
    cmd->x = (short)rect.x;
    cmd->y = (short)rect.y;
    cmd->w = (unsigned short)NK_MAX(0, rect.w);
    cmd->h = (unsigned short)NK_MAX(0, rect.h);
    cmd->left = left;
    cmd->top = top;
    cmd->right = right;
    cmd->bottom = bottom;
}
NK_API void
nk_stroke_circle(struct nk_command_buffer *b, struct nk_rect r,
    float line_thickness, struct nk_color c)
{
    struct nk_command_circle *cmd;
    if (!b || r.w == 0 || r.h == 0 || line_thickness <= 0) return;
    if (b->use_clipping) {
        const struct nk_rect *clip = &b->clip;
        if (!NK_INTERSECT(r.x, r.y, r.w, r.h, clip->x, clip->y, clip->w, clip->h))
            return;
    }

    cmd = (struct nk_command_circle*)
        nk_command_buffer_push(b, NK_COMMAND_CIRCLE, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NK_MAX(r.w, 0);
    cmd->h = (unsigned short)NK_MAX(r.h, 0);
    cmd->color = c;
}
NK_API void
nk_fill_circle(struct nk_command_buffer *b, struct nk_rect r, struct nk_color c)
{
    struct nk_command_circle_filled *cmd;
    NK_ASSERT(b);
    if (!b || c.a == 0 || r.w == 0 || r.h == 0) return;
    if (b->use_clipping) {
        const struct nk_rect *clip = &b->clip;
        if (!NK_INTERSECT(r.x, r.y, r.w, r.h, clip->x, clip->y, clip->w, clip->h))
            return;
    }

    cmd = (struct nk_command_circle_filled*)
        nk_command_buffer_push(b, NK_COMMAND_CIRCLE_FILLED, sizeof(*cmd));
    if (!cmd) return;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NK_MAX(r.w, 0);
    cmd->h = (unsigned short)NK_MAX(r.h, 0);
    cmd->color = c;
}
NK_API void
nk_stroke_arc(struct nk_command_buffer *b, float cx, float cy, float radius,
    float a_min, float a_max, float line_thickness, struct nk_color c)
{
    struct nk_command_arc *cmd;
    if (!b || c.a == 0 || line_thickness <= 0) return;
    cmd = (struct nk_command_arc*)
        nk_command_buffer_push(b, NK_COMMAND_ARC, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->cx = (short)cx;
    cmd->cy = (short)cy;
    cmd->r = (unsigned short)radius;
    cmd->a[0] = a_min;
    cmd->a[1] = a_max;
    cmd->color = c;
}
NK_API void
nk_fill_arc(struct nk_command_buffer *b, float cx, float cy, float radius,
    float a_min, float a_max, struct nk_color c)
{
    struct nk_command_arc_filled *cmd;
    NK_ASSERT(b);
    if (!b || c.a == 0) return;
    cmd = (struct nk_command_arc_filled*)
        nk_command_buffer_push(b, NK_COMMAND_ARC_FILLED, sizeof(*cmd));
    if (!cmd) return;
    cmd->cx = (short)cx;
    cmd->cy = (short)cy;
    cmd->r = (unsigned short)radius;
    cmd->a[0] = a_min;
    cmd->a[1] = a_max;
    cmd->color = c;
}
NK_API void
nk_stroke_triangle(struct nk_command_buffer *b, float x0, float y0, float x1,
    float y1, float x2, float y2, float line_thickness, struct nk_color c)
{
    struct nk_command_triangle *cmd;
    NK_ASSERT(b);
    if (!b || c.a == 0 || line_thickness <= 0) return;
    if (b->use_clipping) {
        const struct nk_rect *clip = &b->clip;
        if (!NK_INBOX(x0, y0, clip->x, clip->y, clip->w, clip->h) &&
            !NK_INBOX(x1, y1, clip->x, clip->y, clip->w, clip->h) &&
            !NK_INBOX(x2, y2, clip->x, clip->y, clip->w, clip->h))
            return;
    }

    cmd = (struct nk_command_triangle*)
        nk_command_buffer_push(b, NK_COMMAND_TRIANGLE, sizeof(*cmd));
    if (!cmd) return;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->a.x = (short)x0;
    cmd->a.y = (short)y0;
    cmd->b.x = (short)x1;
    cmd->b.y = (short)y1;
    cmd->c.x = (short)x2;
    cmd->c.y = (short)y2;
    cmd->color = c;
}
NK_API void
nk_fill_triangle(struct nk_command_buffer *b, float x0, float y0, float x1,
    float y1, float x2, float y2, struct nk_color c)
{
    struct nk_command_triangle_filled *cmd;
    NK_ASSERT(b);
    if (!b || c.a == 0) return;
    if (!b) return;
    if (b->use_clipping) {
        const struct nk_rect *clip = &b->clip;
        if (!NK_INBOX(x0, y0, clip->x, clip->y, clip->w, clip->h) &&
            !NK_INBOX(x1, y1, clip->x, clip->y, clip->w, clip->h) &&
            !NK_INBOX(x2, y2, clip->x, clip->y, clip->w, clip->h))
            return;
    }

    cmd = (struct nk_command_triangle_filled*)
        nk_command_buffer_push(b, NK_COMMAND_TRIANGLE_FILLED, sizeof(*cmd));
    if (!cmd) return;
    cmd->a.x = (short)x0;
    cmd->a.y = (short)y0;
    cmd->b.x = (short)x1;
    cmd->b.y = (short)y1;
    cmd->c.x = (short)x2;
    cmd->c.y = (short)y2;
    cmd->color = c;
}
NK_API void
nk_stroke_polygon(struct nk_command_buffer *b,  float *points, int point_count,
    float line_thickness, struct nk_color col)
{
    int i;
    nk_size size = 0;
    struct nk_command_polygon *cmd;

    NK_ASSERT(b);
    if (!b || col.a == 0 || line_thickness <= 0) return;
    size = sizeof(*cmd) + sizeof(short) * 2 * (nk_size)point_count;
    cmd = (struct nk_command_polygon*) nk_command_buffer_push(b, NK_COMMAND_POLYGON, size);
    if (!cmd) return;
    cmd->color = col;
    cmd->line_thickness = (unsigned short)line_thickness;
    cmd->point_count = (unsigned short)point_count;
    for (i = 0; i < point_count; ++i) {
        cmd->points[i].x = (short)points[i*2];
        cmd->points[i].y = (short)points[i*2+1];
    }
}
NK_API void
nk_fill_polygon(struct nk_command_buffer *b, float *points, int point_count,
    struct nk_color col)
{
    int i;
    nk_size size = 0;
    struct nk_command_polygon_filled *cmd;

    NK_ASSERT(b);
    if (!b || col.a == 0) return;
    size = sizeof(*cmd) + sizeof(short) * 2 * (nk_size)point_count;
    cmd = (struct nk_command_polygon_filled*)
        nk_command_buffer_push(b, NK_COMMAND_POLYGON_FILLED, size);
    if (!cmd) return;
    cmd->color = col;
    cmd->point_count = (unsigned short)point_count;
    for (i = 0; i < point_count; ++i) {
        cmd->points[i].x = (short)points[i*2+0];
        cmd->points[i].y = (short)points[i*2+1];
    }
}
NK_API void
nk_stroke_polyline(struct nk_command_buffer *b, float *points, int point_count,
    float line_thickness, struct nk_color col)
{
    int i;
    nk_size size = 0;
    struct nk_command_polyline *cmd;

    NK_ASSERT(b);
    if (!b || col.a == 0 || line_thickness <= 0) return;
    size = sizeof(*cmd) + sizeof(short) * 2 * (nk_size)point_count;
    cmd = (struct nk_command_polyline*) nk_command_buffer_push(b, NK_COMMAND_POLYLINE, size);
    if (!cmd) return;
    cmd->color = col;
    cmd->point_count = (unsigned short)point_count;
    cmd->line_thickness = (unsigned short)line_thickness;
    for (i = 0; i < point_count; ++i) {
        cmd->points[i].x = (short)points[i*2];
        cmd->points[i].y = (short)points[i*2+1];
    }
}
NK_API void
nk_draw_image(struct nk_command_buffer *b, struct nk_rect r,
    const struct nk_image *img, struct nk_color col)
{
    struct nk_command_image *cmd;
    NK_ASSERT(b);
    if (!b) return;
    if (b->use_clipping) {
        const struct nk_rect *c = &b->clip;
        if (c->w == 0 || c->h == 0 || !NK_INTERSECT(r.x, r.y, r.w, r.h, c->x, c->y, c->w, c->h))
            return;
    }

    cmd = (struct nk_command_image*)
        nk_command_buffer_push(b, NK_COMMAND_IMAGE, sizeof(*cmd));
    if (!cmd) return;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NK_MAX(0, r.w);
    cmd->h = (unsigned short)NK_MAX(0, r.h);
    cmd->img = *img;
    cmd->col = col;
}
NK_API void
nk_draw_nine_slice(struct nk_command_buffer *b, struct nk_rect r,
    const struct nk_nine_slice *slc, struct nk_color col)
{
    struct nk_image img;
    const struct nk_image *slcimg = (const struct nk_image*)slc;
    nk_ushort rgnX, rgnY, rgnW, rgnH;
    rgnX = slcimg->region[0];
    rgnY = slcimg->region[1];
    rgnW = slcimg->region[2];
    rgnH = slcimg->region[3];

    /* top-left */
    img.handle = slcimg->handle;
    img.w = slcimg->w;
    img.h = slcimg->h;
    img.region[0] = rgnX;
    img.region[1] = rgnY;
    img.region[2] = slc->l;
    img.region[3] = slc->t;

    nk_draw_image(b,
        nk_rect(r.x, r.y, (float)slc->l, (float)slc->t),
        &img, col);

#define IMG_RGN(x, y, w, h) img.region[0] = (nk_ushort)(x); img.region[1] = (nk_ushort)(y); img.region[2] = (nk_ushort)(w); img.region[3] = (nk_ushort)(h);

    /* top-center */
    IMG_RGN(rgnX + slc->l, rgnY, rgnW - slc->l - slc->r, slc->t);
    nk_draw_image(b,
        nk_rect(r.x + (float)slc->l, r.y, (float)(r.w - slc->l - slc->r), (float)slc->t),
        &img, col);

    /* top-right */
    IMG_RGN(rgnX + rgnW - slc->r, rgnY, slc->r, slc->t);
    nk_draw_image(b,
        nk_rect(r.x + r.w - (float)slc->r, r.y, (float)slc->r, (float)slc->t),
        &img, col);

    /* center-left */
    IMG_RGN(rgnX, rgnY + slc->t, slc->l, rgnH - slc->t - slc->b);
    nk_draw_image(b,
        nk_rect(r.x, r.y + (float)slc->t, (float)slc->l, (float)(r.h - slc->t - slc->b)),
        &img, col);

    /* center */
    IMG_RGN(rgnX + slc->l, rgnY + slc->t, rgnW - slc->l - slc->r, rgnH - slc->t - slc->b);
    nk_draw_image(b,
        nk_rect(r.x + (float)slc->l, r.y + (float)slc->t, (float)(r.w - slc->l - slc->r), (float)(r.h - slc->t - slc->b)),
        &img, col);

    /* center-right */
    IMG_RGN(rgnX + rgnW - slc->r, rgnY + slc->t, slc->r, rgnH - slc->t - slc->b);
    nk_draw_image(b,
        nk_rect(r.x + r.w - (float)slc->r, r.y + (float)slc->t, (float)slc->r, (float)(r.h - slc->t - slc->b)),
        &img, col);

    /* bottom-left */
    IMG_RGN(rgnX, rgnY + rgnH - slc->b, slc->l, slc->b);
    nk_draw_image(b,
        nk_rect(r.x, r.y + r.h - (float)slc->b, (float)slc->l, (float)slc->b),
        &img, col);

    /* bottom-center */
    IMG_RGN(rgnX + slc->l, rgnY + rgnH - slc->b, rgnW - slc->l - slc->r, slc->b);
    nk_draw_image(b,
        nk_rect(r.x + (float)slc->l, r.y + r.h - (float)slc->b, (float)(r.w - slc->l - slc->r), (float)slc->b),
        &img, col);

    /* bottom-right */
    IMG_RGN(rgnX + rgnW - slc->r, rgnY + rgnH - slc->b, slc->r, slc->b);
    nk_draw_image(b,
        nk_rect(r.x + r.w - (float)slc->r, r.y + r.h - (float)slc->b, (float)slc->r, (float)slc->b),
        &img, col);

#undef IMG_RGN
}
NK_API void
nk_push_custom(struct nk_command_buffer *b, struct nk_rect r,
    nk_command_custom_callback cb, nk_handle usr)
{
    struct nk_command_custom *cmd;
    NK_ASSERT(b);
    if (!b) return;
    if (b->use_clipping) {
        const struct nk_rect *c = &b->clip;
        if (c->w == 0 || c->h == 0 || !NK_INTERSECT(r.x, r.y, r.w, r.h, c->x, c->y, c->w, c->h))
            return;
    }

    cmd = (struct nk_command_custom*)
        nk_command_buffer_push(b, NK_COMMAND_CUSTOM, sizeof(*cmd));
    if (!cmd) return;
    cmd->x = (short)r.x;
    cmd->y = (short)r.y;
    cmd->w = (unsigned short)NK_MAX(0, r.w);
    cmd->h = (unsigned short)NK_MAX(0, r.h);
    cmd->callback_data = usr;
    cmd->callback = cb;
}
NK_API void
nk_draw_set_color_inline(struct nk_context *ctx, enum nk_color_inline_type color_inline)
{
    NK_ASSERT(ctx);
    if (!ctx) return;
    ctx->draw_config.color_inline = color_inline;
}
NK_API nk_bool
nk_draw_push_color_inline(struct nk_context *ctx, enum nk_color_inline_type color_inline)
{
    struct nk_config_stack_color_inline *color_inline_stack;
    struct nk_config_stack_color_inline_element *element;

    NK_ASSERT(ctx);
    if (!ctx) return 0;

    color_inline_stack = &ctx->stacks.color_inline;
    NK_ASSERT(color_inline_stack->head < (int)NK_LEN(color_inline_stack->elements));
    if (color_inline_stack->head >= (int)NK_LEN(color_inline_stack->elements))
        return 0;

    element = &color_inline_stack->elements[color_inline_stack->head++];
    element->old_value = ctx->draw_config.color_inline;
    ctx->draw_config.color_inline = color_inline;
    return 1;
}
NK_API nk_bool
nk_draw_pop_color_inline(struct nk_context *ctx)
{
    struct nk_config_stack_color_inline *color_inline_stack;
    struct nk_config_stack_color_inline_element *element;

    NK_ASSERT(ctx);
    if (!ctx) return 0;

    color_inline_stack = &ctx->stacks.color_inline;
    NK_ASSERT(color_inline_stack->head > 0);
    if (color_inline_stack->head < 1)
        return 0;

    element = &color_inline_stack->elements[--color_inline_stack->head];
    ctx->draw_config.color_inline = element->old_value;
    return 1;
}
NK_API nk_bool nk_draw_push_map_name_color(struct nk_context *ctx, struct nk_map_name_color *c)
{
    struct nk_map_name_color_stack *stack;

    NK_ASSERT(ctx);
    NK_ASSERT(c);

    if (!ctx || !c)
        return 0;

    stack = &ctx->draw_config.map_name_color;
    NK_ASSERT(stack->head < (int)NK_LEN(stack->elements));
    if (stack->head >= (int)NK_LEN(stack->elements))
        return 0;

    stack->elements[stack->head++] = c;

    return 1;
}
NK_API struct nk_map_name_color *nk_draw_get_map_name_color(struct nk_context* ctx, int index)
{
    struct nk_map_name_color_stack *stack;

    NK_ASSERT(ctx);
    if (!ctx) return 0;

    stack = &ctx->draw_config.map_name_color;
    NK_ASSERT(stack->head > index);
    if (stack->head <= index)
        return 0;

    return stack->elements[stack->head - 1 - index];
}
NK_API int nk_draw_get_map_name_color_index_range(struct nk_context *ctx)
{
    struct nk_map_name_color_stack *stack;

    NK_ASSERT(ctx);
    if (!ctx) return 0;

    stack = &ctx->draw_config.map_name_color;
    return stack->head;
}
NK_API struct nk_map_name_color *nk_draw_pop_map_name_color(struct nk_context *ctx)
{
    struct nk_map_name_color_stack *stack;

    NK_ASSERT(ctx);

    if (!ctx)
        return 0;

    stack = &ctx->draw_config.map_name_color;
    NK_ASSERT(stack->head > 0);
    if (stack->head < 1)
        return 0;

    return stack->elements[--stack->head];
}
NK_API struct nk_name_color *nk_draw_get_name_color(struct nk_map_name_color_stack *stack, const char *name, int len)
{
    int i, j;
    struct nk_map_name_color *c;
    struct nk_name_color *cv;
    nk_hash hash;

    NK_ASSERT(stack);

    if (!stack || !name)
        return 0;

    NK_ASSERT(stack->head > 0);
    if (stack->head < 1)
        return 0;

    i = stack->head;
    len = NK_MIN(len, NK_NAME_COLOR_MAX_NAME - 1);
    hash = nk_murmur_hash(name, len, NK_COLOR_INLINE_TAG);
    do {
        c = stack->elements[--i];
        cv = (struct nk_name_color *)c->buffer.memory.ptr;
        /* more recently pushed are on the back, so we start from the back */
        for (j = c->count; j > 0;) {
            --j;
            if (cv[j].name == hash) {
                if (nk_stricmpn(cv[j].name_string, name, len) == 0) {
                    return &cv[j];
                }
            }
        }
    } while (i > 0);
    return 0;
}
NK_API int
nk_draw_raw_text(struct nk_command_buffer *b, struct nk_rect *r,
    const char *text, int len, const struct nk_user_font *font,
    struct nk_color bg, struct nk_color fg, nk_bool wrap,
    nk_bool wrap_at_sep_only, float *w)
{
    struct nk_command_text *cmd = 0;
    enum nk_color_inline_type color_inline;
    int i, j, len_esc = len;
    const char *text_esc = text;
    float font_width;

    NK_ASSERT(b);
    NK_ASSERT(font);

    if (!b || !font || !text || !len || (bg.a == 0 && fg.a == 0)) return 0;

    if (b->draw_config) {
        color_inline = b->draw_config->color_inline;
    } else {
        color_inline = NK_COLOR_INLINE_NONE;
    }

    if (color_inline != NK_COLOR_INLINE_NONE) {
        cmd = (struct nk_command_text*)
            nk_command_buffer_push(b, NK_COMMAND_TEXT, sizeof(*cmd) + (nk_size)(len + 1));
        if (!cmd) return 0;

        for (i = 0, j = 0; i < len; ++i)
            if (text[i] != NK_ESC_CHAR)
                cmd->string[j++] = text[i];
        text = cmd->string;
        len = j;
    }

    /* make sure text fits inside bounds */
    font_width = font->width(font->userdata, font->height, text, len);
    if (font_width > r->w) {
        NK_INTERN nk_rune separator[] = {' '};
        int glyphs = 0, draw_len;
        nk_bool clamped_at_sep;

        if (wrap) {
            draw_len = nk_text_clamp(font, text, len, r->w, &glyphs, &font_width, separator, NK_LEN(separator), &clamped_at_sep);
            if (wrap_at_sep_only && !clamped_at_sep)
                draw_len = 0;
        } else {
            draw_len = nk_text_clamp(font, text, len, r->w, &glyphs, &font_width, 0, 0, 0);
        }

        if (draw_len != len) {
            len = draw_len;
            for (i = 0, j = 0; j < len; ++i)
                if (text_esc[i] != NK_ESC_CHAR)
                    ++j;
            len_esc = j;
        }
    }
    if (w)
        *w = font_width;

    if (color_inline == NK_COLOR_INLINE_NONE) {
        if (!len)
            return 0;

        cmd = (struct nk_command_text*)
            nk_command_buffer_push(b, NK_COMMAND_TEXT, sizeof(*cmd) + (nk_size)(len + 1));
        if (!cmd) return 0;

        NK_MEMCPY(cmd->string, text, (nk_size)len);
    }

    cmd->x = (short)r->x;
    cmd->y = (short)r->y;
    cmd->w = (unsigned short)r->w;
    cmd->h = (unsigned short)r->h;
    cmd->background = bg;
    cmd->foreground = fg;
    cmd->font = font;
    cmd->length = len;
    cmd->string[len] = '\0';
    cmd->height = font->height;

    r->x += font_width;
    r->w -= font_width;

    return len_esc;
}
#define NK_COLOR_INLINE_HEX_CODE_READ_LOOP(N) do { \
    if ((color_name_end = k++) >= length) \
        goto end_branch; \
} while ( \
    NK_CHAR_IS_HEX_DIGIT(c = string[color_name_end]) && ++found < N \
)
#define NK_COLOR_INLINE_GET_COLOR_FROM_STACK(DST, TAG_INDEX, DEF) do { \
    DST = stack->head[TAG_INDEX] > 0 && \
        stack->head[TAG_INDEX] <= NK_INLINE_TAG_STACK_SIZE ? \
        stack->colors[TAG_INDEX][stack->head[TAG_INDEX] - 1] : DEF; \
} while (0)
#define NK_COLOR_INLINE_DRAW_TEXT() do { if (j + esc_count < i) { \
    NK_COLOR_INLINE_GET_COLOR_FROM_STACK(current_bg, NK_INLINE_TAG_BGCOLOR, bg); \
    NK_COLOR_INLINE_GET_COLOR_FROM_STACK(current_fg, NK_INLINE_TAG_COLOR, fg); \
    len = i - j; \
    draw_len = nk_draw_raw_text(b, r, string + j, len, font, current_bg, current_fg, wrap, wrap_at_sep_only, &w); \
    wrap_at_sep_only = nk_true; \
    if (draw_len < len) \
        return j + draw_len; \
} esc_count = 0; } while (0)
NK_API int
nk_draw_coded_text(struct nk_command_buffer *b, struct nk_rect *r,
    const char *string, int length, const struct nk_user_font *font,
    struct nk_color bg, struct nk_color fg, nk_bool wrap, struct nk_inline_tag_stack *stack)
{
    struct nk_name_color *name_color;
    struct nk_color color, current_fg, current_bg;
    enum nk_color_inline_type color_inline;
    struct nk_map_name_color_stack *name_color_stack;
    const char *tag_begin[NK_INLINE_TAG_MAX] = {"[color=", "[bgcolor="};
    const char *tag_end[NK_INLINE_TAG_MAX] = {"[/color]", "[/bgcolor]"};

    int i, j = 0, k, l, esc_count = 0, len, draw_len, found, color_name_begin, color_name_end;
    char c;
    float w;
    nk_bool wrap_at_sep_only = nk_false;

    NK_ASSERT(b);
    NK_ASSERT(font);
    if (!b || !font || !string || !length || (bg.a == 0 && fg.a == 0)) return 0;
    if (b->use_clipping) {
        const struct nk_rect *c = &b->clip;
        if (c->w == 0 || c->h == 0 || !NK_INTERSECT(r->x, r->y, r->w, r->h, c->x, c->y, c->w, c->h))
            return 0;
    }

    if (b->draw_config) {
        color_inline = b->draw_config->color_inline;
        name_color_stack = &b->draw_config->map_name_color;
    } else {
        color_inline = NK_COLOR_INLINE_NONE;
        name_color_stack = 0;
    }

    if (color_inline == NK_COLOR_INLINE_NONE || color_inline >= NK_COLOR_INLINE_MAX) {
        i = length;
        goto end;
    }
    if (color_inline == NK_COLOR_INLINE_ESCAPE_TAG)
        found = 0;
    for (i = 0; i < length; ++i) {
begin:
        if (string[i] == NK_ESC_CHAR) {
            ++esc_count;
            if (color_inline == NK_COLOR_INLINE_TAG) {
                if (++i < length) {
                    if (string[i] == '[')
                        continue;
                    goto begin;
                }
                break;
            } else if (color_inline == NK_COLOR_INLINE_ESCAPE_TAG) {
                found = 1;
                continue;
            }
        }
        if (color_inline == NK_COLOR_INLINE_ESCAPE_TAG && found == 0)
            continue;
        for (l = 0; l < NK_INLINE_TAG_MAX; ++l) {
            if (length - i > nk_strlen(tag_begin[l]) &&
                nk_stricmpn(&string[i], tag_begin[l], nk_strlen(tag_begin[l])) == 0
            ) {
                k = i + nk_strlen(tag_begin[l]);
                if (string[k] == '"') {
                    color_name_begin = ++k;
                    /* found = (NK_NAME_COLOR_MAX_NAME - 1) + 1 + 1 */
                    /* first (+ 1) is to read one past end for '"' */
                    /* second (+ 1) is to terminate loop immediately after one past end */
                    found = NK_NAME_COLOR_MAX_NAME + 1;
                    while (--found) {
                        if ((color_name_end = k++) >= length)
                            goto end_branch;
                        if (string[color_name_end] == '"') {
                            if (k >= length)
                                goto end_branch;
                            if (string[k] == ']') {
                                found = 1;
                                break;
                            }
                        }
                    }
                    if (found) {
                        name_color = nk_draw_get_name_color(name_color_stack,
                            string + color_name_begin,
                            color_name_end - color_name_begin);
                        if (name_color) {
                            color = name_color->color;
                        } else {
                            found = 0;
                        }
                    }
                } else if (string[k] == '#') {
                    found = 0;
                    color_name_begin = ++k;
                    NK_COLOR_INLINE_HEX_CODE_READ_LOOP(6);
                    if (k >= length)
                        goto end_branch;
                    found = 0;
                    if (NK_CHAR_IS_HEX_DIGIT(c)) {
                        if (string[k] == ']') {
                            color = nk_rgb_hex(string + color_name_begin);
                            found = 1;
                        } else {
                            NK_COLOR_INLINE_HEX_CODE_READ_LOOP(2);
                            if (k >= length)
                                goto end_branch;
                            found = 0;
                            if (NK_CHAR_IS_HEX_DIGIT(c)) {
                                if (string[k] == ']') {
                                    color = nk_rgba_hex(string + color_name_begin);
                                    found = 1;
                                }
                            }
                        }
                    }
                }
                if (found) {
                    NK_COLOR_INLINE_DRAW_TEXT();
                    i = j = k + 1;
                    if (i >= length)
                        goto end;
                    if (stack->head[l] < NK_INLINE_TAG_STACK_SIZE) {
                        stack->colors[l][stack->head[l]] = color;
                    }
                    ++stack->head[l];
                    found = 0;
                    goto begin;
                }
            }
end_branch:
            found = 0;
            if (length - i >= nk_strlen(tag_end[l]) &&
                nk_stricmpn(&string[i], tag_end[l], nk_strlen(tag_end[l])) == 0
            ) {
                if (stack->head[l] > 0) {
                    NK_COLOR_INLINE_DRAW_TEXT();
                    --stack->head[l];
                    j = i += nk_strlen(tag_end[l]);
                    if (i < length)
                        goto begin;
                    else
                        goto end;
                }
            }
        } /* for l */
    } /* for i */
end:
    NK_COLOR_INLINE_DRAW_TEXT();
    return i;
}
#undef NK_COLOR_INLINE_DRAW_TEXT
#undef NK_COLOR_INLINE_GET_COLOR_FROM_STACK
#undef NK_COLOR_INLINE_HEX_CODE_READ_LOOP
NK_API void
nk_draw_text(struct nk_command_buffer *b, struct nk_rect r,
    const char *string, int length, const struct nk_user_font *font,
    struct nk_color bg, struct nk_color fg)
{
    struct nk_inline_tag_stack stack = {0};
    nk_draw_coded_text(b, &r, string, length, font, bg, fg, nk_false, &stack);
}
