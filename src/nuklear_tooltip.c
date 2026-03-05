#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                              TOOLTIP
 *
 * ===============================================================*/
NK_LIB struct nk_vec2
nk_tooltip_get_default_offset(const struct nk_context *ctx)
{
    struct nk_vec2 offset = {0};
    NK_ASSERT(ctx);
    if (!ctx) return offset;
    if (ctx->style.cursor_active) {
        /* nuklear is drawing its own cursor so we can reuse its size (best case!) */
        offset.x = ctx->style.window.padding.x + ctx->style.cursor_active->size.x;
        offset.x = ctx->style.window.padding.y + ctx->style.cursor_active->size.y;
    } else if (ctx->style.font) {
        /* assume that cursor size is similar to font height (flawed but reasonable)*/
        offset.y = ctx->style.window.padding.x + ctx->style.font->height;
        offset.x = ctx->style.window.padding.y + ctx->style.font->height;
    }
    return offset;
}
NK_LIB nk_flags
nk_tooltip_get_default_flags(const struct nk_context *ctx)
{
    NK_UNUSED(ctx);
    return NK_TOOLTIP_BELOW|NK_TOOLTIP_ON_RIGHT;
}
NK_API nk_bool
nk_tooltip_begin(struct nk_context *ctx, float width)
{
    return nk_tooltip_begin_offset(ctx, width,
                                   nk_tooltip_get_default_flags(ctx),
                                   nk_tooltip_get_default_offset(ctx));
}
NK_API nk_bool
nk_tooltip_begin_offset(struct nk_context *ctx, float width, nk_flags tooltip, struct nk_vec2 offset)
{
    int x,y,w,h,mul_x,mul_y;
    struct nk_window *win;
    const struct nk_input *in;
    struct nk_rect bounds;
    int ret;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    /* make sure that no nonblocking popup is currently active */
    win = ctx->current;
    in = &ctx->input;
    if (win->popup.win && ((int)win->popup.type & (int)NK_PANEL_SET_NONBLOCK))
        return 0;

    w = nk_iceilf(width);
    h = ctx->current->layout->row.min_height;

    /* find axis multipliers based on bitmask state */
    mul_x = 0;
    mul_x -= !!(tooltip & NK_TOOLTIP_ON_LEFT );
    mul_x += !!(tooltip & NK_TOOLTIP_ON_RIGHT);
    NK_ASSERT(mul_x == -1 || mul_x == 0 || mul_x == 1);
    mul_y = 0;
    mul_y -= !!(tooltip & NK_TOOLTIP_ABOVE);
    mul_y += !!(tooltip & NK_TOOLTIP_BELOW);
    NK_ASSERT(mul_y == -1 || mul_y == 0 || mul_y == 1);

    /* turn relative offset into absolute, unless it's already absolute
     * notice that offset axis is ignored in cases where mul==0
     * (if you don't like this behavior, make sure to use ABS_OFFSET flag)*/
    if (!(tooltip & NK_TOOLTIP_ABS_OFFSET)) {
        offset.x *= mul_x;
        offset.y *= mul_y;
    }

    /* find origin */
    x = -w/2 + (mul_x * w/2);
    x += nk_ifloorf(in->mouse.pos.x + 1) - (int)win->layout->clip.x;
    x += offset.x;
    y = -h/2 + (mul_y * h/2);
    y += nk_ifloorf(in->mouse.pos.y + 1) - (int)win->layout->clip.y;
    y += offset.y;

    bounds.x = (float)x;
    bounds.y = (float)y;
    bounds.w = (float)w;
    bounds.h = (float)nk_iceilf(nk_null_rect.h);

    ret = nk_popup_begin(ctx, NK_POPUP_DYNAMIC,
        "__##Tooltip##__", NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER, bounds);
    if (ret) win->layout->flags &= ~(nk_flags)NK_WINDOW_ROM;
    win->popup.type = NK_PANEL_TOOLTIP;
    ctx->current->layout->type = NK_PANEL_TOOLTIP;
    return ret;
}
NK_API void
nk_tooltip_end(struct nk_context *ctx)
{
    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    if (!ctx || !ctx->current) return;
    ctx->current->seq--;
    nk_popup_close(ctx);
    nk_popup_end(ctx);
}
NK_API void
nk_tooltip_offset(struct nk_context *ctx, const char *text, nk_flags tooltip, struct nk_vec2 offset)
{
    const struct nk_style *style;
    struct nk_vec2 padding;

    int text_len;
    float text_width;
    float text_height;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    NK_ASSERT(text);
    if (!ctx || !ctx->current || !ctx->current->layout || !text)
        return;

    /* fetch configuration data */
    style = &ctx->style;
    padding = style->window.padding;

    /* calculate size of the text and tooltip */
    text_len = nk_strlen(text);
    text_width = style->font->width(style->font->userdata,
                    style->font->height, text, text_len);
    text_width += (4 * padding.x);
    text_height = (style->font->height + 2 * padding.y);

    /* execute tooltip and fill with text */
    if (nk_tooltip_begin_offset(ctx, (float)text_width, tooltip, offset)) {
        nk_layout_row_dynamic(ctx, (float)text_height, 1);
        nk_text(ctx, text, text_len, NK_TEXT_LEFT);
        nk_tooltip_end(ctx);
    }
}
NK_API void
nk_tooltip(struct nk_context *ctx, const char *text)
{
    nk_tooltip_offset(ctx, text,
                      nk_tooltip_get_default_flags(ctx),
                      nk_tooltip_get_default_offset(ctx));
}
#ifdef NK_INCLUDE_STANDARD_VARARGS
NK_API void
nk_tooltipf_offset(struct nk_context *ctx, nk_flags tooltip, struct nk_vec2 offset, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nk_tooltipfv_offset(ctx, tooltip, offset, fmt, args);
    va_end(args);
}
NK_API void
nk_tooltipf(struct nk_context *ctx, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nk_tooltipfv(ctx, fmt, args);
    va_end(args);
}
NK_API void
nk_tooltipfv_offset(struct nk_context *ctx, nk_flags tooltip, struct nk_vec2 offset, const char *fmt, va_list args)
{
    char buf[256];
    nk_strfmt(buf, NK_LEN(buf), fmt, args);
    nk_tooltip_offset(ctx, buf, tooltip, offset);
}
NK_API void
nk_tooltipfv(struct nk_context *ctx, const char *fmt, va_list args)
{
    char buf[256];
    nk_strfmt(buf, NK_LEN(buf), fmt, args);
    nk_tooltip(ctx, buf);
}
#endif


