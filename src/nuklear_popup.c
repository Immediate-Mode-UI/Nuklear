#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                              POPUP
 *
 * ===============================================================*/
NK_LIB struct nk_rect
nk_fit_popup_rect(const struct nk_context *ctx, struct nk_rect body,
    struct nk_rect anchor, enum nk_popup_fit fit, float known_h,
    nk_bool stay_up)
{
    struct nk_rect d;
    float overlap;
    float space_below;
    float space_above;
    float d_right;
    float d_bottom;
    float req_h;
    float need_h;

    if (!ctx) return body;
    d = ctx->display_bounds;
    if (d.w <= 0 || d.h <= 0) return body;

    d_right = d.x + d.w;
    d_bottom = d.y + d.h;
    req_h = body.h;
    need_h = (known_h > 0.0f) ? known_h : req_h;
    overlap = (anchor.y + anchor.h) - body.y;

    /* horizontal: flip/align, then slide. Do not shrink width; dynamic
     * rows (e.g. the overview color contextual) size to the panel and
     * would smush. If the surface is narrower than the popup, clip. */
    if (fit == NK_POPUP_FIT_FLIP) {
        if (body.x + body.w > d_right)
            body.x = anchor.x + anchor.w - body.w;
    } else if (fit == NK_POPUP_FIT_TOOLTIP) {
        if (body.x + body.w > d_right) {
            float flipped = anchor.x - body.w;
            if (flipped >= d.x)
                body.x = flipped;
        }
    }
    if (body.x + body.w > d_right)
        body.x = d_right - body.w;
    if (body.x < d.x)
        body.x = d.x;

    space_below = d_bottom - body.y;
    space_above = anchor.y - d.y;
    if (space_below < 0.0f) space_below = 0.0f;
    if (space_above < 0.0f) space_above = 0.0f;

    if (fit == NK_POPUP_FIT_FLIP) {
        int flip = 0;
        /* Caller size.y is a maximum (ADVANCED is 600px for expanded trees),
         * not the current content. Flip only when last frame's actual height
         * does not fit below. First open (known_h == 0) always drops down.
         * stay_up keeps a drop-up for the rest of this open so collapsing
         * a tree does not jump back to a dropdown. */
        if (stay_up && space_above > 0.0f)
            flip = 1;
        else if (known_h > 0.0f && known_h > space_below) {
            if (known_h <= space_above)
                flip = 1;
            else if (space_above > space_below)
                flip = 1;
        }
        if (flip) {
            float use_h = req_h;
            if (use_h > space_above) use_h = space_above;
            if (known_h < use_h) use_h = known_h;
            if (use_h < 1.0f) use_h = 1.0f;
            body.y = anchor.y - use_h;
            if (overlap > 0.0f) body.y += overlap;
            body.h = use_h;
        } else if (known_h > space_below) {
            body.h = (space_below > 1.0f) ? space_below : 1.0f;
        }
    } else if (fit == NK_POPUP_FIT_SLIDE) {
        if (body.y + need_h > d_bottom)
            body.y = d_bottom - need_h;
        if (body.y < d.y)
            body.y = d.y;
        if (body.y + body.h > d_bottom)
            body.h = d_bottom - body.y;
        if (body.h < 1.0f) body.h = 1.0f;
    } else {
        /* tooltip: flip then slide; do not shrink (clipping is acceptable) */
        if (body.y + need_h > d_bottom) {
            float flipped = anchor.y - need_h;
            if (flipped >= d.y)
                body.y = flipped;
        }
        if (body.y + need_h > d_bottom)
            body.y = d_bottom - need_h;
        if (body.y < d.y)
            body.y = d.y;
    }
    return body;
}
NK_API nk_bool
nk_popup_begin(struct nk_context *ctx, enum nk_popup_type type,
    const char *title, nk_flags flags, struct nk_rect rect)
{
    struct nk_window *popup;
    struct nk_window *win;
    struct nk_panel *panel;

    int title_len;
    nk_hash title_hash;
    nk_size allocated;

    NK_ASSERT(ctx);
    NK_ASSERT(title);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    win = ctx->current;
    panel = win->layout;
    NK_ASSERT(!((int)panel->type & (int)NK_PANEL_SET_POPUP) && "popups are not allowed to have popups");
    (void)panel;
    title_len = (int)nk_strlen(title);
    title_hash = nk_murmur_hash(title, (int)title_len, NK_PANEL_POPUP);

    popup = win->popup.win;
    if (!popup) {
        popup = (struct nk_window*)nk_create_window(ctx);
        popup->parent = win;
        win->popup.win = popup;
        win->popup.active = 0;
        win->popup.type = NK_PANEL_POPUP;
    }

    /* make sure we have correct popup */
    if (win->popup.name != title_hash) {
        if (!win->popup.active) {
            nk_zero(popup, sizeof(*popup));
            win->popup.name = title_hash;
            win->popup.active = 1;
            win->popup.type = NK_PANEL_POPUP;
        } else return 0;
    }

    /* popup position is local to window */
    ctx->current = popup;
    rect.x += win->layout->clip.x;
    rect.y += win->layout->clip.y;

    /* setup popup data */
    popup->parent = win;
    popup->bounds = rect;
    popup->seq = ctx->seq;
    popup->layout = (struct nk_panel*)nk_create_panel(ctx);
    popup->flags = flags;
    popup->flags |= NK_WINDOW_BORDER;
    if (type == NK_POPUP_DYNAMIC)
        popup->flags |= NK_WINDOW_DYNAMIC;

    popup->buffer = win->buffer;
    nk_start_popup(ctx, win);
    allocated = ctx->memory.allocated;
    nk_push_scissor(&popup->buffer, nk_null_rect);

    if (nk_panel_begin(ctx, title, NK_PANEL_POPUP)) {
        /* popup is running therefore invalidate parent panels */
        struct nk_panel *root;
        root = win->layout;
        while (root) {
            root->flags |= NK_WINDOW_ROM;
            root->flags &= ~(nk_flags)NK_WINDOW_REMOVE_ROM;
            root = root->parent;
        }
        win->popup.active = 1;
        popup->layout->offset_x = &popup->scrollbar.x;
        popup->layout->offset_y = &popup->scrollbar.y;
        popup->layout->parent = win->layout;
        return 1;
    } else {
        /* popup was closed/is invalid so cleanup */
        struct nk_panel *root;
        root = win->layout;
        while (root) {
            root->flags |= NK_WINDOW_REMOVE_ROM;
            root = root->parent;
        }
        win->popup.buf.active = 0;
        win->popup.active = 0;
        ctx->memory.allocated = allocated;
        ctx->current = win;
        nk_free_panel(ctx, popup->layout);
        popup->layout = 0;
        return 0;
    }
}
NK_LIB nk_bool
nk_nonblock_begin(struct nk_context *ctx,
    nk_flags flags, struct nk_rect body, struct nk_rect header,
    enum nk_panel_type panel_type)
{
    struct nk_window *popup;
    struct nk_window *win;
    struct nk_panel *panel;
    int is_active = nk_true;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    /* popups cannot have popups */
    win = ctx->current;
    panel = win->layout;
    NK_ASSERT(!((int)panel->type & (int)NK_PANEL_SET_POPUP));
    (void)panel;
    popup = win->popup.win;
    if (!popup) {
        /* create window for nonblocking popup */
        popup = (struct nk_window*)nk_create_window(ctx);
        popup->parent = win;
        win->popup.win = popup;
        win->popup.type = panel_type;
        nk_command_buffer_init(&popup->buffer, &ctx->memory, NK_CLIPPING_ON);
    } else {
        /* close the popup if user pressed outside or in the header */
        int pressed, in_body, in_header;
#ifdef NK_BUTTON_TRIGGER_ON_RELEASE
        pressed = nk_input_is_mouse_released(&ctx->input, NK_BUTTON_LEFT);
#else
        pressed = nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT);
#endif
        in_body = nk_input_is_mouse_hovering_rect(&ctx->input, body);
        in_header = nk_input_is_mouse_hovering_rect(&ctx->input, header);
        if (pressed && (!in_body || in_header))
            is_active = nk_false;
    }
    win->popup.header = header;

    if (!is_active) {
        /* remove read only mode from all parent panels */
        struct nk_panel *root = win->layout;
        while (root) {
            root->flags |= NK_WINDOW_REMOVE_ROM;
            root = root->parent;
        }
        win->popup.buf.active = 0;
        win->popup.last_h = 0;
        win->popup.pinned_up = nk_false;
        return is_active;
    }
    popup->bounds = body;
    popup->parent = win;
    popup->layout = (struct nk_panel*)nk_create_panel(ctx);
    popup->flags = flags;
    popup->flags |= NK_WINDOW_BORDER;
    popup->flags |= NK_WINDOW_DYNAMIC;
    popup->seq = ctx->seq;
    win->popup.active = 1;
    NK_ASSERT(popup->layout);

    nk_start_popup(ctx, win);
    popup->buffer = win->buffer;
    nk_push_scissor(&popup->buffer, nk_null_rect);
    ctx->current = popup;

    nk_panel_begin(ctx, 0, panel_type);
    win->buffer = popup->buffer;
    popup->layout->parent = win->layout;
    popup->layout->offset_x = &popup->scrollbar.x;
    popup->layout->offset_y = &popup->scrollbar.y;

    /* set read only mode to all parent panels */
    {struct nk_panel *root;
    root = win->layout;
    while (root) {
        root->flags |= NK_WINDOW_ROM;
        root = root->parent;
    }}
    return is_active;
}
NK_API void
nk_popup_close(struct nk_context *ctx)
{
    struct nk_window *popup;
    NK_ASSERT(ctx);
    if (!ctx || !ctx->current) return;

    popup = ctx->current;
    NK_ASSERT(popup->parent);
    NK_ASSERT((int)popup->layout->type & (int)NK_PANEL_SET_POPUP);
    popup->flags |= NK_WINDOW_HIDDEN;
}
NK_API void
nk_popup_end(struct nk_context *ctx)
{
    struct nk_window *win;
    struct nk_window *popup;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return;

    popup = ctx->current;
    if (!popup->parent) return;
    win = popup->parent;
    if (popup->flags & NK_WINDOW_HIDDEN) {
        struct nk_panel *root;
        root = win->layout;
        while (root) {
            root->flags |= NK_WINDOW_REMOVE_ROM;
            root = root->parent;
        }
        win->popup.active = 0;
    }
    nk_push_scissor(&popup->buffer, nk_null_rect);
    nk_end(ctx);

    win->buffer = popup->buffer;
    nk_finish_popup(ctx, win);
    ctx->current = win;
    nk_push_scissor(&win->buffer, win->layout->clip);
}
NK_API void
nk_popup_get_scroll(const struct nk_context *ctx, nk_uint *offset_x, nk_uint *offset_y)
{
    struct nk_window *popup;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return;

    popup = ctx->current;
    if (offset_x)
      *offset_x = popup->scrollbar.x;
    if (offset_y)
      *offset_y = popup->scrollbar.y;
}
NK_API void
nk_popup_set_scroll(struct nk_context *ctx, nk_uint offset_x, nk_uint offset_y)
{
    struct nk_window *popup;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return;

    popup = ctx->current;
    popup->scrollbar.x = offset_x;
    popup->scrollbar.y = offset_y;
}
