#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                              TOGGLE
 *
 * ===============================================================*/
NK_LIB nk_bool
nk_toggle_behavior(const struct nk_input *in, struct nk_rect select,
    nk_flags *state, nk_bool active)
{
    nk_widget_state_reset(state);
    if (nk_button_behavior(state, select, in, NK_BUTTON_DEFAULT)) {
        *state = NK_WIDGET_STATE_ACTIVE;
        active = !active;
    }
    if (*state & NK_WIDGET_STATE_HOVER && !nk_input_is_mouse_prev_hovering_rect(in, select))
        *state |= NK_WIDGET_STATE_ENTERED;
    else if (nk_input_is_mouse_prev_hovering_rect(in, select))
        *state |= NK_WIDGET_STATE_LEFT;
    return active;
}
NK_LIB void
nk_draw_checkbox(struct nk_command_buffer *out,
    nk_flags state, const struct nk_style_toggle *style, nk_bool active,
    const struct nk_rect *label, const struct nk_rect *selector,
    const struct nk_rect *cursors, const char *string, int len,
    const struct nk_user_font *font, nk_flags text_alignment)
{
    const struct nk_style_item *background;
    const struct nk_style_item *cursor;
    struct nk_text text;

    /* select correct colors/images */
    if (state & NK_WIDGET_STATE_HOVER) {
        background = &style->hover;
        cursor = &style->cursor_hover;
        text.text = style->text_hover;
    } else if (state & NK_WIDGET_STATE_ACTIVED) {
        background = &style->hover;
        cursor = &style->cursor_hover;
        text.text = style->text_active;
    } else {
        background = &style->normal;
        cursor = &style->cursor_normal;
        text.text = style->text_normal;
    }

    text.text = nk_rgb_factor(text.text, style->color_factor);
    text.text = nk_rgb_factor(text.text, style->color_factor);
    text.padding.x = 0;
    text.padding.y = 0;
    nk_widget_text(out, *label, string, len, &text, text_alignment, font);
    
    /* draw background and cursor */
    if (background->type == NK_STYLE_ITEM_COLOR) {
        nk_stroke_rect(out, *selector, 2, 2, nk_rgb_factor(background->data.color, style->color_factor));
    } else nk_draw_image(out, *selector, &background->data.image, nk_rgb_factor(nk_white, style->color_factor));
    if (active) {
        if (cursor->type == NK_STYLE_ITEM_IMAGE)
            nk_draw_image(out, *cursors, &cursor->data.image, nk_rgb_factor(nk_white, style->color_factor));
        else nk_fill_rect(out, *cursors, 0, nk_rgb_factor(background->data.color, style->color_factor));
    }


}
NK_LIB void
nk_draw_option(struct nk_command_buffer *out,
    nk_flags state, const struct nk_style_toggle *style, nk_bool active,
    const struct nk_rect *label, const struct nk_rect *selector,
    const struct nk_rect *cursors, const char *string, int len,
    const struct nk_user_font *font, nk_flags text_alignment)
{
    const struct nk_style_item *background;
    const struct nk_style_item *cursor;
    struct nk_text text;

    /* select correct colors/images */
    if (state & NK_WIDGET_STATE_HOVER) {
        background = &style->hover;
        cursor = &style->cursor_hover;
        text.text = style->text_hover;
    } else if (state & NK_WIDGET_STATE_ACTIVED) {
        background = &style->hover;
        cursor = &style->cursor_hover;
        text.text = style->text_active;
    } else {
        background = &style->normal;
        cursor = &style->cursor_normal;
        text.text = style->text_normal;
    }

    text.text = nk_rgb_factor(text.text, style->color_factor);
    text.padding.x = 0;
    text.padding.y = 0;
    text.background = style->text_background;
    nk_widget_text(out, *label, string, len, &text, text_alignment, font);

    /* draw background and cursor */
    if (background->type == NK_STYLE_ITEM_COLOR) {
        nk_stroke_circle(out, *selector, 2, nk_rgb_factor(background->data.color, style->color_factor));
    } else nk_draw_image(out, *selector, &background->data.image, nk_rgb_factor(nk_white, style->color_factor));
    if (active) {
        if (cursor->type == NK_STYLE_ITEM_IMAGE)
            nk_draw_image(out, *cursors, &cursor->data.image, nk_rgb_factor(nk_white, style->color_factor));
        else nk_fill_circle(out, *cursors, background->data.color);
    }
}
NK_LIB nk_bool
nk_do_toggle(nk_flags *state,
    struct nk_command_buffer *out, struct nk_rect r,
    nk_bool *active, const char *str, int len, enum nk_toggle_type type,
    const struct nk_style_toggle *style, const struct nk_input *in,
    const struct nk_user_font *font, nk_flags widget_alignment, nk_flags text_alignment)
{
    int was_active;
    struct nk_rect bounds;
    struct nk_rect select;
    struct nk_rect cursor;
    struct nk_rect label;

    NK_ASSERT(style);
    NK_ASSERT(out);
    NK_ASSERT(font);
    if (!out || !style || !font || !active)
        return 0;

    r.w = NK_MAX(r.w, font->height + 2 * style->padding.x);
    r.h = NK_MAX(r.h, font->height + 2 * style->padding.y);

    /* add additional touch padding for touch screen devices */
    bounds.x = r.x - style->touch_padding.x;
    bounds.y = r.y - style->touch_padding.y;
    bounds.w = r.w + 2 * style->touch_padding.x;
    bounds.h = r.h + 2 * style->touch_padding.y;

    /* calculate the selector space */
    select.w = font->height;
    select.h = select.w;

    if (widget_alignment & NK_WIDGET_ALIGN_RIGHT) {
        select.x = r.x + r.w - font->height;

        /* label in front of the selector */
        label.x = r.x;
        label.w = r.w - select.w - style->spacing * 2;
    } else if (widget_alignment & NK_WIDGET_ALIGN_CENTERED) {
        select.x = r.x + (r.w - select.w) / 2;

        /* label in front of selector */
        label.x = r.x;
        label.w = (r.w - select.w - style->spacing * 2) / 2;
    } else { /* Default: NK_WIDGET_ALIGN_LEFT */
        select.x = r.x;

        /* label behind the selector */
        label.x = select.x + select.w + style->spacing;
        label.w = NK_MAX(r.x + r.w, label.x) - label.x;
    }

    if (widget_alignment & NK_WIDGET_ALIGN_TOP) {
        select.y = r.y;
    } else if (widget_alignment & NK_WIDGET_ALIGN_BOTTOM) {
        select.y = r.y + r.h - select.h - 2 * style->padding.y;
    } else { /* Default: NK_WIDGET_ALIGN_MIDDLE */
        select.y = r.y + r.h/2.0f - select.h/2.0f;
    }

    label.y = select.y;
    label.h = select.w;

    /* calculate the bounds of the cursor inside the selector */
    cursor.x = select.x + style->padding.x + style->border;
    cursor.y = select.y + style->padding.y + style->border;
    cursor.w = select.w - (2 * style->padding.x + 2 * style->border);
    cursor.h = select.h - (2 * style->padding.y + 2 * style->border);

    /* update selector */
    was_active = *active;
    *active = nk_toggle_behavior(in, bounds, state, *active);

    /* draw selector */
    if (style->draw_begin)
        style->draw_begin(out, style->userdata);
    if (type == NK_TOGGLE_CHECK) {
        nk_draw_checkbox(out, *state, style, *active, &label, &select, &cursor, str, len, font, text_alignment);
    } else {
        nk_draw_option(out, *state, style, *active, &label, &select, &cursor, str, len, font, text_alignment);
    }
    if (style->draw_end)
        style->draw_end(out, style->userdata);
    return (was_active != *active);
}
/*----------------------------------------------------------------
 *
 *                          CHECKBOX
 *
 * --------------------------------------------------------------*/
NK_API nk_bool
nk_check_text(struct nk_context *ctx, const char *text, int len, nk_bool active)
{
    struct nk_window *win;
    struct nk_panel *layout;
    const struct nk_input *in;
    const struct nk_style *style;

    struct nk_rect bounds;
    enum nk_widget_layout_states state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return active;

    win = ctx->current;
    style = &ctx->style;
    layout = win->layout;

    state = nk_widget(&bounds, ctx);
    if (!state) return active;
    in = (state == NK_WIDGET_ROM || state == NK_WIDGET_DISABLED || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    nk_do_toggle(&ctx->last_widget_state, &win->buffer, bounds, &active,
        text, len, NK_TOGGLE_CHECK, &style->checkbox, in, style->font, NK_WIDGET_LEFT, NK_TEXT_LEFT);
    return active;
}
NK_API nk_bool
nk_check_text_align(struct nk_context *ctx, const char *text, int len, nk_bool active, nk_flags widget_alignment, nk_flags text_alignment)
{
    struct nk_window *win;
    struct nk_panel *layout;
    const struct nk_input *in;
    const struct nk_style *style;

    struct nk_rect bounds;
    enum nk_widget_layout_states state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return active;

    win = ctx->current;
    style = &ctx->style;
    layout = win->layout;

    state = nk_widget(&bounds, ctx);
    if (!state) return active;
    in = (state == NK_WIDGET_ROM || state == NK_WIDGET_DISABLED || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    nk_do_toggle(&ctx->last_widget_state, &win->buffer, bounds, &active,
        text, len, NK_TOGGLE_CHECK, &style->checkbox, in, style->font, widget_alignment, text_alignment);
    return active;
}
NK_API unsigned int
nk_check_flags_text(struct nk_context *ctx, const char *text, int len,
    unsigned int flags, unsigned int value)
{
    int old_active;
    NK_ASSERT(ctx);
    NK_ASSERT(text);
    if (!ctx || !text) return flags;
    old_active = (int)((flags & value) & value);
    if (nk_check_text(ctx, text, len, old_active))
        flags |= value;
    else flags &= ~value;
    return flags;
}
NK_API nk_bool
nk_checkbox_text(struct nk_context *ctx, const char *text, int len, nk_bool *active)
{
    int old_val;
    NK_ASSERT(ctx);
    NK_ASSERT(text);
    NK_ASSERT(active);
    if (!ctx || !text || !active) return 0;
    old_val = *active;
    *active = nk_check_text(ctx, text, len, *active);
    return old_val != *active;
}
NK_API nk_bool
nk_checkbox_text_align(struct nk_context *ctx, const char *text, int len, nk_bool *active, nk_flags widget_alignment, nk_flags text_alignment)
{
    int old_val;
    NK_ASSERT(ctx);
    NK_ASSERT(text);
    NK_ASSERT(active);
    if (!ctx || !text || !active) return 0;
    old_val = *active;
    *active = nk_check_text_align(ctx, text, len, *active, widget_alignment, text_alignment);
    return old_val != *active;
}
NK_API nk_bool
nk_checkbox_flags_text(struct nk_context *ctx, const char *text, int len,
    unsigned int *flags, unsigned int value)
{
    nk_bool active;
    NK_ASSERT(ctx);
    NK_ASSERT(text);
    NK_ASSERT(flags);
    if (!ctx || !text || !flags) return 0;

    active = (int)((*flags & value) & value);
    if (nk_checkbox_text(ctx, text, len, &active)) {
        if (active) *flags |= value;
        else *flags &= ~value;
        return 1;
    }
    return 0;
}
NK_API nk_bool nk_check_label(struct nk_context *ctx, const char *label, nk_bool active)
{
    return nk_check_text(ctx, label, nk_strlen(label), active);
}
NK_API unsigned int nk_check_flags_label(struct nk_context *ctx, const char *label,
    unsigned int flags, unsigned int value)
{
    return nk_check_flags_text(ctx, label, nk_strlen(label), flags, value);
}
NK_API nk_bool nk_checkbox_label(struct nk_context *ctx, const char *label, nk_bool *active)
{
    return nk_checkbox_text(ctx, label, nk_strlen(label), active);
}
NK_API nk_bool nk_checkbox_label_align(struct nk_context *ctx, const char *label, nk_bool *active, nk_flags widget_alignment, nk_flags text_alignment)
{
    return nk_checkbox_text_align(ctx, label, nk_strlen(label), active, widget_alignment, text_alignment);
}
NK_API nk_bool nk_checkbox_flags_label(struct nk_context *ctx, const char *label,
    unsigned int *flags, unsigned int value)
{
    return nk_checkbox_flags_text(ctx, label, nk_strlen(label), flags, value);
}
/*----------------------------------------------------------------
 *
 *                          OPTION
 *
 * --------------------------------------------------------------*/
NK_API nk_bool
nk_option_text(struct nk_context *ctx, const char *text, int len, nk_bool is_active)
{
    struct nk_window *win;
    struct nk_panel *layout;
    const struct nk_input *in;
    const struct nk_style *style;

    struct nk_rect bounds;
    enum nk_widget_layout_states state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return is_active;

    win = ctx->current;
    style = &ctx->style;
    layout = win->layout;

    state = nk_widget(&bounds, ctx);
    if (!state) return (int)state;
    in = (state == NK_WIDGET_ROM || state == NK_WIDGET_DISABLED || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    nk_do_toggle(&ctx->last_widget_state, &win->buffer, bounds, &is_active,
        text, len, NK_TOGGLE_OPTION, &style->option, in, style->font, NK_WIDGET_LEFT, NK_TEXT_LEFT);
    return is_active;
}
NK_API nk_bool
nk_option_text_align(struct nk_context *ctx, const char *text, int len, nk_bool is_active, nk_flags widget_alignment, nk_flags text_alignment)
{
    struct nk_window *win;
    struct nk_panel *layout;
    const struct nk_input *in;
    const struct nk_style *style;

    struct nk_rect bounds;
    enum nk_widget_layout_states state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return is_active;

    win = ctx->current;
    style = &ctx->style;
    layout = win->layout;

    state = nk_widget(&bounds, ctx);
    if (!state) return (int)state;
    in = (state == NK_WIDGET_ROM || state == NK_WIDGET_DISABLED || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    nk_do_toggle(&ctx->last_widget_state, &win->buffer, bounds, &is_active,
        text, len, NK_TOGGLE_OPTION, &style->option, in, style->font, widget_alignment, text_alignment);
    return is_active;
}
NK_API nk_bool
nk_radio_text(struct nk_context *ctx, const char *text, int len, nk_bool *active)
{
    int old_value;
    NK_ASSERT(ctx);
    NK_ASSERT(text);
    NK_ASSERT(active);
    if (!ctx || !text || !active) return 0;
    old_value = *active;
    *active = nk_option_text(ctx, text, len, old_value);
    return old_value != *active;
}
NK_API nk_bool
nk_radio_text_align(struct nk_context *ctx, const char *text, int len, nk_bool *active, nk_flags widget_alignment, nk_flags text_alignment)
{
    int old_value;
    NK_ASSERT(ctx);
    NK_ASSERT(text);
    NK_ASSERT(active);
    if (!ctx || !text || !active) return 0;
    old_value = *active;
    *active = nk_option_text_align(ctx, text, len, old_value, widget_alignment, text_alignment);
    return old_value != *active;
}
NK_API nk_bool
nk_option_label(struct nk_context *ctx, const char *label, nk_bool active)
{
    return nk_option_text(ctx, label, nk_strlen(label), active);
}
NK_API nk_bool
nk_option_label_align(struct nk_context *ctx, const char *label, nk_bool active, nk_flags widget_alignment, nk_flags text_alignment)
{
    return nk_option_text_align(ctx, label, nk_strlen(label), active, widget_alignment, text_alignment);
}
NK_API nk_bool
nk_radio_label(struct nk_context *ctx, const char *label, nk_bool *active)
{
    return nk_radio_text(ctx, label, nk_strlen(label), active);
}
NK_API nk_bool
nk_radio_label_align(struct nk_context *ctx, const char *label, nk_bool *active, nk_flags widget_alignment, nk_flags text_alignment)
{
    return nk_radio_text_align(ctx, label, nk_strlen(label), active, widget_alignment, text_alignment);
}

