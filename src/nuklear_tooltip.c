#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                              TOOLTIP
 *
 * ===============================================================*/
NK_API nk_bool
nk_tooltip_begin(struct nk_context *ctx, float width)
{
    NK_ASSERT(ctx);
    return nk_tooltip_begin_offset(ctx, width, ctx->style.window.tooltip_origin, ctx->style.window.tooltip_offset);
}

NK_API nk_bool
nk_tooltip_begin_offset(struct nk_context *ctx, float width, enum nk_tooltip_pos position, struct nk_vec2 offset)
{
    int x,y,w,h;
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
    h = (int)NK_MAX(win->layout->row.min_height, ctx->style.font->height+2*ctx->style.window.padding.y);

    /* Default origin is top left, plus user offset */
    x = nk_ifloorf(in->mouse.pos.x + 1) - (int)win->layout->clip.x + (int)offset.x;
    y = nk_ifloorf(in->mouse.pos.y + 1) - (int)win->layout->clip.y + (int)offset.y;

    /* Adjust origin based on enum */
    switch (position) {
    case NK_TOP_LEFT:
        /* no change */
        break;
    case NK_TOP_CENTER:
        x -= w/2;
        break;
    case NK_TOP_RIGHT:
        x -= w;
        break;

    case NK_MIDDLE_LEFT:
        y -= h/2;
        break;
    case NK_MIDDLE_CENTER:
        x -= w/2;
        y -= h/2;
        break;
    case NK_MIDDLE_RIGHT:
        x -= w;
        y -= h/2;
        break;

    case NK_BOTTOM_LEFT:
        y -= h;
        break;
    case NK_BOTTOM_CENTER:
        x -= w/2;
        y -= h;
        break;
    case NK_BOTTOM_RIGHT:
        x -= w;
        y -= h;
        break;
    default:
        NK_ASSERT(0 && "Invalid tooltip position");
    }

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

/**
 * Display a default tooltip if the mouse is hovering over the rect `bounds`
 */
NK_API void
nk_do_tooltip(struct nk_context* ctx, const char* text, struct nk_rect bounds)
{
    NK_ASSERT(ctx);
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds)) {
        nk_tooltip_offset(ctx, text, ctx->style.window.tooltip_origin, ctx->style.window.tooltip_offset);
    }
}

/**
 * Display a default tooltip if the mouse hovers motionless for the default delay (ctx->style.window.tooltip_delay)
 * `timer` is used to track the time across frames.
 */
NK_API void
nk_do_tooltip_delay(struct nk_context* ctx, const char* text, struct nk_rect bounds, float* timer)
{
    NK_ASSERT(ctx);
    if (nk_input_is_mouse_hovering_still_delay_rect(ctx, bounds, timer, ctx->style.window.tooltip_delay)) {
        nk_tooltip_offset(ctx, text, ctx->style.window.tooltip_origin, ctx->style.window.tooltip_offset);
    }
}

/**
 * Display a default tooltip if the mouse hovers motionless for the default delay (ctx->style.window.tooltip_delay) unless
 * `clicked` is true. `clicked` will be reset to false (and `timer` to 0) when the mouse moves.
 * `timer` is used to track the time across frames.
 */
NK_API void
nk_do_tooltip_delay_clicked(struct nk_context* ctx, const char* text, struct nk_rect bounds, float* timer, nk_bool* clicked)
{
    NK_ASSERT(ctx);
    if (nk_input_is_mouse_hovering_still_delay_clicked_rect(ctx, bounds, timer, ctx->style.window.tooltip_delay, clicked)) {
        nk_tooltip_offset(ctx, text, ctx->style.window.tooltip_origin, ctx->style.window.tooltip_offset);
    }
}

NK_API void
nk_tooltip_offset(struct nk_context *ctx, const char *text, enum nk_tooltip_pos position, struct nk_vec2 offset)
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
    if (nk_tooltip_begin_offset(ctx, (float)text_width, position, offset)) {
        nk_layout_row_dynamic(ctx, (float)text_height, 1);
        nk_text(ctx, text, text_len, NK_TEXT_LEFT);
        nk_tooltip_end(ctx);
    }
}

NK_API void
nk_tooltip(struct nk_context *ctx, const char *text)
{
    NK_ASSERT(ctx);
    nk_tooltip_offset(ctx, text, ctx->style.window.tooltip_origin, ctx->style.window.tooltip_offset);
}
#ifdef NK_INCLUDE_STANDARD_VARARGS
NK_API void
nk_tooltipf_offset(struct nk_context *ctx, enum nk_tooltip_pos position, struct nk_vec2 offset, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nk_tooltipfv_offset(ctx, position, offset, fmt, args);
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
nk_tooltipfv_offset(struct nk_context *ctx, enum nk_tooltip_pos position, struct nk_vec2 offset, const char *fmt, va_list args)
{
    char buf[256];
    nk_strfmt(buf, NK_LEN(buf), fmt, args);
    nk_tooltip_offset(ctx, buf, position, offset);
}
NK_API void
nk_tooltipfv(struct nk_context *ctx, const char *fmt, va_list args)
{
    char buf[256];
    nk_strfmt(buf, NK_LEN(buf), fmt, args);
    nk_tooltip(ctx, buf);
}
#endif


