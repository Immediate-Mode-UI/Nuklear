#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                               KNOB
 *
 * ===============================================================*/

NK_LIB float
nk_knob_behavior(nk_flags *state, struct nk_input *in,
    struct nk_rect bounds, float knob_min, float knob_max, float knob_value,
    float knob_step, float knob_steps,
    enum nk_heading zero_direction, float dead_zone_percent)
{
    struct nk_vec2 origin;
    float angle = 0.0f;
    origin.x = bounds.x + (bounds.w / 2);
    origin.y = bounds.y + (bounds.h / 2);

    nk_widget_state_reset(state);

    /* handle click and drag input */
    if(in &&
       in->mouse.buttons[NK_BUTTON_LEFT].down &&
       nk_input_has_mouse_click_down_in_rect(in, NK_BUTTON_LEFT, bounds, nk_true)){
        /* calculate angle from origin and rotate */
        const float direction_rads[4] = {
            NK_PI * 2.5f, /* 90  NK_UP */
            NK_PI * 2.0f, /* 0   NK_RIGHT */
            NK_PI * 1.5f, /* 270 NK_DOWN */
            NK_PI,        /* 180 NK_LEFT */
        };
        *state = NK_WIDGET_STATE_ACTIVE;

        angle = NK_ATAN2(in->mouse.pos.y - origin.y, in->mouse.pos.x - origin.x) + direction_rads[zero_direction];
        angle -= (angle > NK_PI * 2) ? NK_PI * 3 : NK_PI;

        /* account for dead space applied when drawing */
        angle *= 1.0f / (1.0f - dead_zone_percent);
        angle = NK_CLAMP(-NK_PI, angle, NK_PI);

        /* convert -pi -> pi range to 0.0 -> 1.0 */
        angle = (angle + NK_PI) / (NK_PI * 2);

        /* click to closest step */
        knob_value = knob_min + ( (int)(angle * knob_steps + (knob_step / 2)) ) * knob_step;
        knob_value = NK_CLAMP(knob_min, knob_value, knob_max);
    }

    /* knob widget state */
    if (nk_input_is_mouse_hovering_rect(in, bounds)){
        *state = NK_WIDGET_STATE_HOVERED;
        /* handle scroll and arrow inputs */
        if (in->mouse.scroll_delta.y > 0 ||
           (in->keyboard.keys[NK_KEY_UP].down && in->keyboard.keys[NK_KEY_UP].clicked)) {
            knob_value += knob_step;
        }

        if (in->mouse.scroll_delta.y < 0 ||
           (in->keyboard.keys[NK_KEY_DOWN].down && in->keyboard.keys[NK_KEY_DOWN].clicked)) {
            knob_value -= knob_step;
        }
        /* easiest way to disable scrolling of parent panels..knob eats scrolling */
        in->mouse.scroll_delta.y = 0;
        knob_value = NK_CLAMP(knob_min, knob_value, knob_max);
    }
    if (*state & NK_WIDGET_STATE_HOVER &&
        !nk_input_is_mouse_prev_hovering_rect(in, bounds))
        *state |= NK_WIDGET_STATE_ENTERED;
    else if (nk_input_is_mouse_prev_hovering_rect(in, bounds))
        *state |= NK_WIDGET_STATE_LEFT;

    return knob_value;
}
NK_LIB void
nk_draw_knob(struct nk_command_buffer *out, nk_flags state,
    const struct nk_style_knob *style, const struct nk_rect *bounds, float min, float value, float max,
    enum nk_heading zero_direction, float dead_zone_percent)
{
    const struct nk_style_item *background;
    struct nk_color knob_color, cursor;

    NK_UNUSED(min);
    NK_UNUSED(max);
    NK_UNUSED(value);

    if (state & NK_WIDGET_STATE_ACTIVED) {
        background = &style->active;
        knob_color = style->knob_active;
        cursor = style->cursor_active;
    } else if (state & NK_WIDGET_STATE_HOVER) {
        background = &style->hover;
        knob_color = style->knob_hover;
        cursor = style->cursor_hover;
    } else {
        background = &style->normal;
        knob_color = style->knob_normal;
        cursor = style->cursor_normal;
    }

    /* draw background */
    switch(background->type) {
        case NK_STYLE_ITEM_IMAGE:
            nk_draw_image(out, *bounds, &background->data.image, nk_rgb_factor(nk_white, style->color_factor));
            break;
        case NK_STYLE_ITEM_NINE_SLICE:
            nk_draw_nine_slice(out, *bounds, &background->data.slice, nk_rgb_factor(nk_white, style->color_factor));
            break;
        case NK_STYLE_ITEM_COLOR:
            nk_fill_rect(out, *bounds, 0, nk_rgb_factor(background->data.color, style->color_factor));
            nk_stroke_rect(out, *bounds, 0, style->border, nk_rgb_factor(style->border_color, style->color_factor));
            break;
    }

    /* draw knob */
    nk_fill_circle(out, *bounds, nk_rgb_factor(knob_color, style->color_factor));
    if(style->knob_border > 0){
        struct nk_rect border_bounds = *bounds;
        border_bounds.x += style->knob_border / 2;
        border_bounds.y += style->knob_border / 2;
        border_bounds.w -= style->knob_border;
        border_bounds.h -= style->knob_border;
        nk_stroke_circle(out, border_bounds, style->knob_border, nk_rgb_factor(style->knob_border_color, style->color_factor));
    }
    { /* calculate cursor line cords */
    float half_circle_size = (bounds->w / 2);
    float angle = (value - min) / (max - min);
    float alive_zone =  1.0f - dead_zone_percent;
    struct nk_vec2 cursor_start, cursor_end;
    const float direction_rads[4] = {
        NK_PI * 1.5f, /* 90  NK_UP */
        0.0f,         /* 0   NK_RIGHT */
        NK_PI * 0.5f, /* 270 NK_DOWN */
        NK_PI,        /* 180 NK_LEFT */
    };
    /* calculate + apply dead zone */
    angle = (angle * alive_zone) + (dead_zone_percent / 2);

    /* percentage 0.0 -> 1.0 to radians, rads are 0.0 to (2*pi) NOT -pi to pi */
    angle *= NK_PI * 2;

    /* apply zero angle */
    angle += direction_rads[zero_direction];
    if(angle > NK_PI * 2)
        angle -= NK_PI * 2;

    cursor_start.x = bounds->x + half_circle_size + (angle > NK_PI);
    cursor_start.y = bounds->y + half_circle_size + (angle < NK_PI_HALF || angle > (NK_PI * 1.5f));

    cursor_end.x = cursor_start.x + (half_circle_size * NK_COS(angle));
    cursor_end.y = cursor_start.y + (half_circle_size * NK_SIN(angle));

    /* cut off half of the cursor */
    cursor_start.x = (cursor_start.x + cursor_end.x) / 2;
    cursor_start.y = (cursor_start.y + cursor_end.y) / 2;

    /* draw cursor */
    nk_stroke_line(out, cursor_start.x, cursor_start.y, cursor_end.x, cursor_end.y, 2, nk_rgb_factor(cursor, style->color_factor));
    }
}
NK_LIB float
nk_do_knob(nk_flags *state,
    struct nk_command_buffer *out, struct nk_rect bounds,
    float min, float val, float max, float step,
    enum nk_heading zero_direction, float dead_zone_percent,
    const struct nk_style_knob *style, struct nk_input *in)
{
    float knob_range;
    float knob_min;
    float knob_max;
    float knob_value;
    float knob_steps;

    NK_ASSERT(style);
    NK_ASSERT(out);
    if (!out || !style)
        return 0;

    /* remove padding from knob bounds */
    bounds.y = bounds.y + style->padding.y;
    bounds.x = bounds.x + style->padding.x;
    bounds.h = NK_MAX(bounds.h, 2*style->padding.y);
    bounds.w = NK_MAX(bounds.w, 2*style->padding.x);
    bounds.w -= 2 * style->padding.x;
    bounds.h -= 2 * style->padding.y;
    if(bounds.h < bounds.w){
        bounds.x += (bounds.w - bounds.h) / 2;
        bounds.w = bounds.h;
    }

    /* make sure the provided values are correct */
    knob_max = NK_MAX(min, max);
    knob_min = NK_MIN(min, max);
    knob_value = NK_CLAMP(knob_min, val, knob_max);
    knob_range = knob_max - knob_min;
    knob_steps = knob_range / step;

    knob_value = nk_knob_behavior(state, in, bounds, knob_min, knob_max, knob_value, step, knob_steps, zero_direction, dead_zone_percent);

    /* draw knob */
    if (style->draw_begin) style->draw_begin(out, style->userdata);
    nk_draw_knob(out, *state, style, &bounds, knob_min, knob_value, knob_max, zero_direction, dead_zone_percent);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return knob_value;
}
NK_API nk_bool
nk_knob_float(struct nk_context *ctx, float min_value, float *value, float max_value,
    float value_step, enum nk_heading zero_direction, float dead_zone_degrees)
{
    struct nk_window *win;
    struct nk_panel *layout;
    struct nk_input *in;
    const struct nk_style *style;

    int ret = 0;
    float old_value;
    struct nk_rect bounds;
    enum nk_widget_layout_states state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    NK_ASSERT(value);
    NK_ASSERT(NK_BETWEEN(dead_zone_degrees, 0.0f, 360.0f));
    if (!ctx || !ctx->current || !ctx->current->layout || !value)
        return ret;

    win = ctx->current;
    style = &ctx->style;
    layout = win->layout;

    state = nk_widget(&bounds, ctx);
    if (!state) return ret;
    in = (state == NK_WIDGET_DISABLED || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;

    old_value = *value;
    *value = nk_do_knob(&ctx->last_widget_state, &win->buffer, bounds, min_value,
                old_value, max_value, value_step, zero_direction, dead_zone_degrees / 360.0f, &style->knob, in);

    return (old_value > *value || old_value < *value);
}
NK_API nk_bool
nk_knob_int(struct nk_context *ctx, int min, int *val, int max, int step,
    enum nk_heading zero_direction, float dead_zone_degrees)
{
    int ret;
    float value = (float)*val;
    ret = nk_knob_float(ctx, (float)min, &value, (float)max, (float)step, zero_direction, dead_zone_degrees);
    *val =  (int)value;
    return ret;
}
