#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                              SELECTABLE
 *
 * ===============================================================*/
NK_LIB void
nk_draw_selectable(struct nk_command_buffer *out,
    nk_flags state, const struct nk_style_selectable *style, nk_bool active,
    const struct nk_rect *bounds,
    const struct nk_rect *icon, const struct nk_image *img, enum nk_symbol_type sym,
    struct nk_slice string, nk_flags align, const struct nk_user_font *font)
{
    const struct nk_style_item *background;
    struct nk_text text;
    text.padding = style->padding;

    /* select correct colors/images */
    if (!active) {
        if (state & NK_WIDGET_STATE_ACTIVED) {
            background = &style->pressed;
            text.text = style->text_pressed;
        } else if (state & NK_WIDGET_STATE_HOVER) {
            background = &style->hover;
            text.text = style->text_hover;
        } else {
            background = &style->normal;
            text.text = style->text_normal;
        }
    } else {
        if (state & NK_WIDGET_STATE_ACTIVED) {
            background = &style->pressed_active;
            text.text = style->text_pressed_active;
        } else if (state & NK_WIDGET_STATE_HOVER) {
            background = &style->hover_active;
            text.text = style->text_hover_active;
        } else {
            background = &style->normal_active;
            text.text = style->text_normal_active;
        }
    }
    /* draw selectable background and text */
    switch (background->type) {
        case NK_STYLE_ITEM_IMAGE:
            text.background = nk_rgba(0, 0, 0, 0);
            nk_draw_image(out, *bounds, &background->data.image, nk_white);
            break;
        case NK_STYLE_ITEM_NINE_SLICE:
            text.background = nk_rgba(0, 0, 0, 0);
            nk_draw_nine_slice(out, *bounds, &background->data.slice, nk_white);
            break;
        case NK_STYLE_ITEM_COLOR:
            text.background = background->data.color;
            nk_fill_rect(out, *bounds, style->rounding, background->data.color);
            break;
    }
    if (icon) {
        if (img) nk_draw_image(out, *icon, img, nk_white);
        else nk_draw_symbol(out, sym, *icon, text.background, text.text, 1, font);
    }
    nk_widget_text(out, *bounds, string, &text, align, font);
}
NK_LIB nk_bool
nk_do_selectable(nk_flags *state, struct nk_command_buffer *out,
    struct nk_rect bounds, struct nk_slice str, nk_flags align, nk_bool *value,
    const struct nk_style_selectable *style, const struct nk_input *in,
    const struct nk_user_font *font)
{
    int old_value;
    struct nk_rect touch;

    NK_ASSERT(state);
    NK_ASSERT(out);
    NK_ASSERT(str.ptr);
    NK_ASSERT(str.len);
    NK_ASSERT(value);
    NK_ASSERT(style);
    NK_ASSERT(font);

    if (!state || !out || !str.ptr || !str.len || !value || !style || !font) return 0;
    old_value = *value;

    /* remove padding */
    touch.x = bounds.x - style->touch_padding.x;
    touch.y = bounds.y - style->touch_padding.y;
    touch.w = bounds.w + style->touch_padding.x * 2;
    touch.h = bounds.h + style->touch_padding.y * 2;

    /* update button */
    if (nk_button_behavior(state, touch, in, NK_BUTTON_DEFAULT))
        *value = !(*value);

    /* draw selectable */
    if (style->draw_begin) style->draw_begin(out, style->userdata);
    nk_draw_selectable(out, *state, style, *value, &bounds, 0,0,NK_SYMBOL_NONE, str, align, font);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return old_value != *value;
}
NK_LIB nk_bool
nk_do_selectable_image(nk_flags *state, struct nk_command_buffer *out,
    struct nk_rect bounds, struct nk_slice str, nk_flags align, nk_bool *value,
    const struct nk_image *img, const struct nk_style_selectable *style,
    const struct nk_input *in, const struct nk_user_font *font)
{
    nk_bool old_value;
    struct nk_rect touch;
    struct nk_rect icon;

    NK_ASSERT(state);
    NK_ASSERT(out);
    NK_ASSERT(str.ptr);
    NK_ASSERT(str.len);
    NK_ASSERT(value);
    NK_ASSERT(style);
    NK_ASSERT(font);

    if (!state || !out || !str.ptr || !str.len || !value || !style || !font) return 0;
    old_value = *value;

    /* toggle behavior */
    touch.x = bounds.x - style->touch_padding.x;
    touch.y = bounds.y - style->touch_padding.y;
    touch.w = bounds.w + style->touch_padding.x * 2;
    touch.h = bounds.h + style->touch_padding.y * 2;
    if (nk_button_behavior(state, touch, in, NK_BUTTON_DEFAULT))
        *value = !(*value);

    icon.y = bounds.y + style->padding.y;
    icon.w = icon.h = bounds.h - 2 * style->padding.y;
    if (align & NK_TEXT_ALIGN_LEFT) {
        icon.x = (bounds.x + bounds.w) - (2 * style->padding.x + icon.w);
        icon.x = NK_MAX(icon.x, 0);
    } else icon.x = bounds.x + 2 * style->padding.x;

    icon.x += style->image_padding.x;
    icon.y += style->image_padding.y;
    icon.w -= 2 * style->image_padding.x;
    icon.h -= 2 * style->image_padding.y;

    /* draw selectable */
    if (style->draw_begin) style->draw_begin(out, style->userdata);
    nk_draw_selectable(out, *state, style, *value, &bounds, &icon, img, NK_SYMBOL_NONE, str, align, font);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return old_value != *value;
}
NK_LIB nk_bool
nk_do_selectable_symbol(nk_flags *state, struct nk_command_buffer *out,
    struct nk_rect bounds, struct nk_slice str, nk_flags align, nk_bool *value,
    enum nk_symbol_type sym, const struct nk_style_selectable *style,
    const struct nk_input *in, const struct nk_user_font *font)
{
    int old_value;
    struct nk_rect touch;
    struct nk_rect icon;

    NK_ASSERT(state);
    NK_ASSERT(out);
    NK_ASSERT(str.ptr);
    NK_ASSERT(str.len);
    NK_ASSERT(value);
    NK_ASSERT(style);
    NK_ASSERT(font);

    if (!state || !out || !str.ptr || !str.len || !value || !style || !font) return 0;
    old_value = *value;

    /* toggle behavior */
    touch.x = bounds.x - style->touch_padding.x;
    touch.y = bounds.y - style->touch_padding.y;
    touch.w = bounds.w + style->touch_padding.x * 2;
    touch.h = bounds.h + style->touch_padding.y * 2;
    if (nk_button_behavior(state, touch, in, NK_BUTTON_DEFAULT))
        *value = !(*value);

    icon.y = bounds.y + style->padding.y;
    icon.w = icon.h = bounds.h - 2 * style->padding.y;
    if (align & NK_TEXT_ALIGN_LEFT) {
        icon.x = (bounds.x + bounds.w) - (2 * style->padding.x + icon.w);
        icon.x = NK_MAX(icon.x, 0);
    } else icon.x = bounds.x + 2 * style->padding.x;

    icon.x += style->image_padding.x;
    icon.y += style->image_padding.y;
    icon.w -= 2 * style->image_padding.x;
    icon.h -= 2 * style->image_padding.y;

    /* draw selectable */
    if (style->draw_begin) style->draw_begin(out, style->userdata);
    nk_draw_selectable(out, *state, style, *value, &bounds, &icon, 0, sym, str, align, font);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return old_value != *value;
}

NK_API nk_bool
nk_selectable_label(struct nk_context *ctx, struct nk_slice str,
    nk_flags align, nk_bool *value)
{
    struct nk_window *win;
    struct nk_panel *layout;
    const struct nk_input *in;
    const struct nk_style *style;

    enum nk_widget_layout_states state;
    struct nk_rect bounds;

    NK_ASSERT(ctx);
    NK_ASSERT(value);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !value)
        return 0;

    win = ctx->current;
    layout = win->layout;
    style = &ctx->style;

    state = nk_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NK_WIDGET_ROM || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    return nk_do_selectable(&ctx->last_widget_state, &win->buffer, bounds,
                str, align, value, &style->selectable, in, style->font);
}
NK_API nk_bool
nk_selectable_image_label(struct nk_context *ctx, struct nk_image img,
    struct nk_slice str, nk_flags align, nk_bool *value)
{
    struct nk_window *win;
    struct nk_panel *layout;
    const struct nk_input *in;
    const struct nk_style *style;

    enum nk_widget_layout_states state;
    struct nk_rect bounds;

    NK_ASSERT(ctx);
    NK_ASSERT(value);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !value)
        return 0;

    win = ctx->current;
    layout = win->layout;
    style = &ctx->style;

    state = nk_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NK_WIDGET_ROM || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    return nk_do_selectable_image(&ctx->last_widget_state, &win->buffer, bounds,
                str, align, value, &img, &style->selectable, in, style->font);
}
NK_API nk_bool
nk_selectable_symbol_label(struct nk_context *ctx, enum nk_symbol_type sym,
    struct nk_slice str, nk_flags align, nk_bool *value)
{
    struct nk_window *win;
    struct nk_panel *layout;
    const struct nk_input *in;
    const struct nk_style *style;

    enum nk_widget_layout_states state;
    struct nk_rect bounds;

    NK_ASSERT(ctx);
    NK_ASSERT(value);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout || !value)
        return 0;

    win = ctx->current;
    layout = win->layout;
    style = &ctx->style;

    state = nk_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NK_WIDGET_ROM || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    return nk_do_selectable_symbol(&ctx->last_widget_state, &win->buffer, bounds,
                str, align, value, sym, &style->selectable, in, style->font);
}
NK_API nk_bool nk_select_label(struct nk_context *ctx, struct nk_slice str,
    nk_flags align, nk_bool value)
{
    nk_selectable_label(ctx, str, align, &value);return value;
}
NK_API nk_bool nk_select_image_label(struct nk_context *ctx, struct nk_image img,
    struct nk_slice str, nk_flags align, nk_bool value)
{
    nk_selectable_image_label(ctx, img, str, align, &value);return value;
}
NK_API nk_bool
nk_select_symbol_label(struct nk_context *ctx, enum nk_symbol_type sym,
    struct nk_slice title, nk_flags align, nk_bool value)
{
    nk_selectable_symbol_label(ctx, sym, title, align, &value);return value;
}

