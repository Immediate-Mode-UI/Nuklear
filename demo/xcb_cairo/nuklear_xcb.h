/*****************************************************************************
 *
 * Nuklear XCB/Cairo Render Backend - v0.0.2
 * Copyright 2021 Richard Gill
 *
 * Grabbed and adapted from https://github.com/griebd/nuklear_xcb
 * Copyright 2017 Adriano Grieb
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *                                API
 *
 ****************************************************************************/

#ifndef NK_XCB_CAIRO_H
#define NK_XCB_CAIRO_H

struct nk_xcb_context;
struct nk_cairo_context;

/* With Xcb, we work mostly on events, so to do something only when
 * needed it's good to know what kind of events pulled us from sleep
 */
enum nk_xcb_event_type {
    NK_XCB_EVENT_PAINT      = 0x02,
    NK_XCB_EVENT_RESIZED    = 0x04,
    NK_XCB_EVENT_STOP       = 0x08
};

/* Xcb part: work on windows */
NK_API struct nk_xcb_context *nk_xcb_init(const char *title, int pos_x, int pos_y, int width, int height);
NK_API void nk_xcb_free(struct nk_xcb_context *xcb_ctx);

NK_API int nk_xcb_handle_event(struct nk_xcb_context *xcb_ctx, struct nk_context *nk_ctx);
NK_API void nk_xcb_render(struct nk_xcb_context *xcb_ctx);
NK_API void nk_xcb_size(struct nk_xcb_context *xcb_ctx, int *width, int *height);

/* TODO: copy/paste */

/* Cairo part: work on painting */
NK_API struct nk_cairo_context *nk_cairo_init(struct nk_color *bg, const char *font_file, double font_size, void *surface);
NK_API void nk_cairo_free(struct nk_cairo_context *cairo_ctx);

NK_API struct nk_user_font *nk_cairo_default_font(struct nk_cairo_context *cairo_ctx);
NK_API void nk_cairo_damage(struct nk_cairo_context *cairo_ctx);
NK_API int nk_cairo_render(struct nk_cairo_context *cairo_ctx, struct nk_context *ctx);

/* Bridge between xcb and cairo (so it's possible to use them like legos) */
NK_API void *nk_xcb_create_cairo_surface(struct nk_xcb_context *xcb_ctx);
NK_API void nk_xcb_resize_cairo_surface(struct nk_xcb_context *xcb_ctx, void *surface);


#endif /* NK_XCB_CAIRO_H */

/*****************************************************************************
 *
 *                           IMPLEMENTATION
 *
 ****************************************************************************/

#ifdef NK_XCB_CAIRO_IMPLEMENTATION

#include <xcb/xcb.h>
#include <xcb/xcb_util.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <cairo/cairo-xcb.h>
#include <cairo/cairo-ft.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>


#if defined _XOPEN_SOURCE && _XOPEN_SOURCE >= 600 || \
    defined _POSIX_C_SOURCE && _POSIX_C_SOURCE >= 200112L
#include <time.h>
#include <errno.h>
#ifndef NK_XCB_FPS
#define NK_XCB_FPS 30
#endif /* NK_XCB_FPS */
#define NK_XCB_NSEC 1000000000
#define NK_XCB_MIN_FRAME_TIME (NK_XCB_NSEC / NK_XCB_FPS)
#endif

#include <math.h>
#ifdef __USE_GNU
#define NK_XCB_PI M_PIl
#elif defined __USE_BSD || defined __USE_XOPEN
#define NK_XCB_PI M_PI
#else
#define NK_XCB_PI acos(-1.0)
#endif

#define NK_XCB_TO_CAIRO(x) ((double) x / 255.0)
#define NK_XCB_DEG_TO_RAD(x) ((double) x * NK_XCB_PI / 180.0)

typedef struct xkb_context xkb_context;
typedef struct xkb_keymap xkb_keymap;
typedef struct xkb_state xkb_state;
typedef struct xkbcommon_context xkbcommon_context;
typedef struct nk_xcb_context nk_xcb_context;

struct xkbcommon_context
{
	struct xkb_context *ctx;
	struct xkb_keymap *keymap;
	struct xkb_state *state;
};

NK_INTERN void xkbcommon_free(xkbcommon_context *kbdctx);
NK_INTERN xkbcommon_context *xkbcommon_init(xcb_connection_t *conn);
NK_INTERN xkb_keysym_t keycode_to_keysym(nk_xcb_context *ctx, xkb_keycode_t keycode, int pressed);

struct nk_cairo_context {
    cairo_surface_t *surface;
    cairo_t *cr;

    struct nk_user_font *font;
    struct nk_color *bg;

    void *last_buffer;
    nk_size buffer_size;
    int repaint;
};

struct nk_xcb_context {
    xcb_connection_t *conn;
    int screennum;
    xcb_window_t window;
    /* xcb_key_symbols_t *key_symbols; */
    xkbcommon_context *xkbcommon_ctx;
#ifdef NK_XCB_MIN_FRAME_TIME
    unsigned long last_render;
#endif /* NK_XCB_MIN_FRAME_TIME */
    int events;
    xcb_intern_atom_reply_t* del_atom;
    int width, height;
};

NK_API struct nk_xcb_context *nk_xcb_init(const char *title, int pos_x, int pos_y, int width, int height)
{
    int screenNum;
    xcb_connection_t *conn;
    xcb_screen_t *screen;
    xcb_window_t window;
    uint32_t values[1];
    struct nk_xcb_context *xcb_ctx;
    xcb_intern_atom_cookie_t cookie;
    xcb_intern_atom_reply_t *reply, *del_atom;

    conn = xcb_connect(NULL, &screenNum);
    if (xcb_connection_has_error(conn)) {
        xcb_disconnect(conn);
        return NULL;
    }
    screen = xcb_aux_get_screen(conn, screenNum);

    window = xcb_generate_id(conn);
    values[0] = XCB_EVENT_MASK_KEY_PRESS
        | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_BUTTON_1_MOTION
        | XCB_EVENT_MASK_BUTTON_2_MOTION
        | XCB_EVENT_MASK_BUTTON_3_MOTION
        | XCB_EVENT_MASK_BUTTON_4_MOTION
        | XCB_EVENT_MASK_BUTTON_5_MOTION
        | XCB_EVENT_MASK_BUTTON_MOTION
        | XCB_EVENT_MASK_KEYMAP_STATE
        | XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        ;
    xcb_create_window(conn, XCB_COPY_FROM_PARENT, window, screen->root,
            pos_x, pos_y, width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
            XCB_COPY_FROM_PARENT, XCB_CW_EVENT_MASK, values);

	xcb_change_property(conn,
		XCB_PROP_MODE_REPLACE,
		window,
		XCB_ATOM_WM_NAME,
		XCB_ATOM_STRING,
		8,
		strlen(title),
		title);

    cookie = xcb_intern_atom(conn, 1, 12, "WM_PROTOCOLS");
    reply = xcb_intern_atom_reply(conn, cookie, 0);
    cookie = xcb_intern_atom(conn, 0, 16, "WM_DELETE_WINDOW");
    del_atom = xcb_intern_atom_reply(conn, cookie, 0);
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, window, reply->atom, 4, 32, 1, &del_atom->atom);
    free(reply);

    xcb_map_window(conn, window);
    xcb_flush(conn);

    xcb_ctx = (struct nk_xcb_context *)malloc(sizeof (struct nk_xcb_context));
    xcb_ctx->conn = conn;
    xcb_ctx->screennum = screenNum;
    xcb_ctx->window = window;
    /* xcb_ctx->key_symbols = xcb_key_symbols_alloc(xcb_ctx->conn); */
    xcb_ctx->xkbcommon_ctx = xkbcommon_init(conn);
    xcb_ctx->del_atom = del_atom;
    xcb_ctx->width = width;
    xcb_ctx->height = height;

    return xcb_ctx;
}

NK_API void nk_xcb_free(struct nk_xcb_context *xcb_ctx)
{
	xkbcommon_free(xcb_ctx->xkbcommon_ctx);
    free(xcb_ctx->del_atom);
    /* xcb_key_symbols_free(xcb_ctx->key_symbols); */
    xcb_disconnect(xcb_ctx->conn);
    free(xcb_ctx);
}

NK_API int nk_xcb_handle_event(struct nk_xcb_context *xcb_ctx, struct nk_context *nk_ctx)
{
    int events = 0;
    xcb_generic_event_t *event;

#ifdef NK_XCB_MIN_FRAME_TIME
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC_COARSE, &tp);
    xcb_ctx->last_render = tp.tv_sec * NK_XCB_NSEC + tp.tv_nsec;
#endif /* NK_XCB_MIN_FRAME_TIME */

    event = xcb_wait_for_event(xcb_ctx->conn);

    nk_input_begin(nk_ctx);
    do {
        switch (XCB_EVENT_RESPONSE_TYPE(event)) {
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE:
            {
                int press = (XCB_EVENT_RESPONSE_TYPE(event)) == XCB_KEY_PRESS;
                xcb_key_press_event_t *kp = (xcb_key_press_event_t *)event;
                /* xcb_keysym_t sym = xcb_key_symbols_get_keysym(xcb_ctx->key_symbols, kp->detail, kp->state);*/
                xcb_keysym_t sym = keycode_to_keysym(xcb_ctx, kp->detail, press);

                switch (sym) {
                case XK_Shift_L:
                case XK_Shift_R:
                    nk_input_key(nk_ctx, NK_KEY_SHIFT, press);
                    break;
                case XK_Control_L:
                case XK_Control_R:
                    nk_input_key(nk_ctx, NK_KEY_CTRL, press);
                    break;
                case XK_Delete:
                    nk_input_key(nk_ctx, NK_KEY_DEL, press);
                    break;
                case XK_Return:
                case XK_KP_Enter:
                    nk_input_key(nk_ctx, NK_KEY_ENTER, press);
                    break;
                case XK_Tab:
                    nk_input_key(nk_ctx, NK_KEY_TAB, press);
                    break;
                case XK_BackSpace:
                    nk_input_key(nk_ctx, NK_KEY_BACKSPACE, press);
                    break;
                /* case NK_KEY_COPY */
                /* case NK_KEY_CUT */
                /* case NK_KEY_PASTE */
                case XK_Up:
                    nk_input_key(nk_ctx, NK_KEY_UP, press);
                    break;
                case XK_Down:
                    nk_input_key(nk_ctx, NK_KEY_DOWN, press);
                    break;
                case XK_Left:
                    nk_input_key(nk_ctx, NK_KEY_LEFT, press);
                    break;
                case XK_Right:
                    nk_input_key(nk_ctx, NK_KEY_RIGHT, press);
                    break;
                /* NK_KEY_TEXT_INSERT_MODE, */
                /* NK_KEY_TEXT_REPLACE_MODE, */
                case XK_Escape:
                    nk_input_key(nk_ctx, NK_KEY_TEXT_RESET_MODE, press);
                    break;
                /* NK_KEY_TEXT_LINE_START, */
                /* NK_KEY_TEXT_LINE_END, */
                case XK_Home:
                    {
                        nk_input_key(nk_ctx, NK_KEY_TEXT_START, press);
                        nk_input_key(nk_ctx, NK_KEY_SCROLL_START, press);
                    }
                    break;
                case XK_End:
                    {
                        nk_input_key(nk_ctx, NK_KEY_TEXT_END, press);
                        nk_input_key(nk_ctx, NK_KEY_SCROLL_END, press);
                    }
                    break;
                /* NK_KEY_TEXT_UNDO, */
                /* NK_KEY_TEXT_REDO, */
                /* NK_KEY_TEXT_SELECT_ALL, */
                /* NK_KEY_TEXT_WORD_LEFT, */
                /* NK_KEY_TEXT_WORD_RIGHT, */
                case XK_Page_Down:
                    nk_input_key(nk_ctx, NK_KEY_SCROLL_DOWN, press);
                    break;
                case XK_Page_Up:
                    nk_input_key(nk_ctx, NK_KEY_SCROLL_UP, press);
                    break;
                default:
                    if (press &&
                            !xcb_is_keypad_key(sym) &&
                            !xcb_is_private_keypad_key(sym) &&
                            !xcb_is_cursor_key(sym) &&
                            !xcb_is_pf_key(sym) &&
                            !xcb_is_function_key(sym) &&
                            !xcb_is_misc_function_key(sym) &&
                            !xcb_is_modifier_key(sym)
                            ) {
                        /* nk_input_char(nk_ctx, sym); */
                        nk_input_unicode(nk_ctx, sym);
                    }
                    else {
                        printf("state: %x code: %x sum: %x\n", kp->state, kp->detail, sym);
                    }
                    break;
                }
            }
            break;
        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
            {
                int press = (XCB_EVENT_RESPONSE_TYPE(event)) == XCB_BUTTON_PRESS;
                xcb_button_press_event_t *bp = (xcb_button_press_event_t *)event;
                switch (bp->detail) {
                case XCB_BUTTON_INDEX_1:
                    nk_input_button(nk_ctx, NK_BUTTON_LEFT, bp->event_x, bp->event_y, press);
                    break;
                case XCB_BUTTON_INDEX_2:
                    nk_input_button(nk_ctx, NK_BUTTON_MIDDLE, bp->event_x, bp->event_y, press);
                    break;
                case XCB_BUTTON_INDEX_3:
                    nk_input_button(nk_ctx, NK_BUTTON_RIGHT, bp->event_x, bp->event_y, press);
                    break;
                case XCB_BUTTON_INDEX_4:
                    nk_input_scroll(nk_ctx, nk_vec2(0, 1.0f));
                    break;
                case XCB_BUTTON_INDEX_5:
                    nk_input_scroll(nk_ctx, nk_vec2(0, -1.0f));
                    break;
                default: break;
                }
            }
            break;
        case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t *mn = (xcb_motion_notify_event_t *)event;
                nk_input_motion(nk_ctx, mn->event_x, mn->event_y);
            }
            break;
        case XCB_SELECTION_CLEAR:
            {
                printf("Unhandled event: %s\n", xcb_event_get_label(event->response_type));
            }
            break;
        case XCB_SELECTION_REQUEST:
            {
                printf("Unhandled event: %s\n", xcb_event_get_label(event->response_type));
            }
            break;
        case XCB_SELECTION_NOTIFY:
            {
                printf("Unhandled event: %s\n", xcb_event_get_label(event->response_type));
            }
            break;
        case XCB_CONFIGURE_NOTIFY:
            {
                xcb_configure_notify_event_t *cn = (xcb_configure_notify_event_t *)event;
                xcb_ctx->width = cn->width;
                xcb_ctx->height = cn->height;
                events |= NK_XCB_EVENT_RESIZED;
            }
            break;
        case XCB_KEYMAP_NOTIFY:
            /* xcb_refresh_keyboard_mapping(xcb_ctx->key_symbols, (xcb_mapping_notify_event_t *)event); */
            break;
        case XCB_EXPOSE:
        case XCB_REPARENT_NOTIFY:
        case XCB_MAP_NOTIFY:
            events |= NK_XCB_EVENT_PAINT;
            break;
        case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t *cm = (xcb_client_message_event_t *)event;
                if (cm->data.data32[0] == xcb_ctx->del_atom->atom)
                {
                    events = NK_XCB_EVENT_STOP;
                }
            }
            break;
        default:
            printf ("Unhandled event: %s\n", xcb_event_get_label(event->response_type));
            break;
        }
        free(event);
    }
    while ((events != NK_XCB_EVENT_STOP) && (event = xcb_poll_for_event(xcb_ctx->conn)));
    nk_input_end(nk_ctx);

    return events;
}

NK_API void nk_xcb_render(struct nk_xcb_context *xcb_ctx)
{
    xcb_flush (xcb_ctx->conn);

#ifdef NK_XCB_MIN_FRAME_TIME
    {
        struct timespec tp;
        unsigned long spent;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &tp);
        spent = tp.tv_sec * NK_XCB_NSEC + tp.tv_nsec - xcb_ctx->last_render;
        if (NK_XCB_MIN_FRAME_TIME > spent) {
            tp.tv_sec = 0;
            tp.tv_nsec = NK_XCB_MIN_FRAME_TIME - spent;
            while (clock_nanosleep(CLOCK_MONOTONIC, 0, &tp, &tp) == EINTR);
        }
    }
#endif /* NK_XCB_MIN_FRAME_TIME */
}

NK_API void nk_xcb_size(struct nk_xcb_context *xcb_ctx, int *width, int *height)
{
    *width = xcb_ctx->width;
    *height = xcb_ctx->height;
}

NK_API void *nk_xcb_create_cairo_surface(struct nk_xcb_context *xcb_ctx)
{
    xcb_screen_t *screen;
    xcb_visualtype_t *visual;

    screen = xcb_aux_get_screen(xcb_ctx->conn, xcb_ctx->screennum);
    visual = xcb_aux_get_visualtype(xcb_ctx->conn, xcb_ctx->screennum, screen->root_visual);
    return cairo_xcb_surface_create(xcb_ctx->conn, xcb_ctx->window, visual, xcb_ctx->width, xcb_ctx->height);
}

NK_API void nk_xcb_resize_cairo_surface(struct nk_xcb_context *xcb_ctx, void *surface)
{
    cairo_xcb_surface_set_size((cairo_surface_t *)surface, xcb_ctx->width, xcb_ctx->height);
}



#define NK_TO_CAIRO(x) ((double) x / 255.0)


NK_INTERN float nk_cairo_text_width(nk_handle handle, float height __attribute__ ((__unused__)), const char *text, int len)
{
    cairo_scaled_font_t *font = (cairo_scaled_font_t *)handle.ptr;
    cairo_glyph_t *glyphs = NULL;
    int num_glyphs;
    cairo_text_extents_t extents;

    cairo_scaled_font_text_to_glyphs(font, 0, 0, text, len, &glyphs, &num_glyphs, NULL, NULL, NULL);
    cairo_scaled_font_glyph_extents(font, glyphs, num_glyphs, &extents);
    cairo_glyph_free(glyphs);

    return extents.x_advance;
}

NK_API struct nk_cairo_context *nk_cairo_init(struct nk_color *bg, const char *font_file, double font_size, void *surf)
{
    cairo_surface_t *surface = (cairo_surface_t *)surf;
    struct nk_cairo_context *cairo_ctx;
    cairo_t *cr;
    cairo_font_extents_t extents;
    cairo_scaled_font_t *default_font;
    struct nk_user_font *font;

    cr = cairo_create(surface);
    font = (struct nk_user_font *)malloc(sizeof (struct nk_user_font));
    if (font_file != NULL) {
        FT_Library library;
        FT_Face face;
        cairo_font_face_t *font_face;
        static const cairo_user_data_key_t key = {0};

        FT_Init_FreeType(&library);
        FT_New_Face(library, font_file, 0, &face);
        font_face = cairo_ft_font_face_create_for_ft_face(face, 0);
        cairo_font_face_set_user_data(font_face, &key, face, (cairo_destroy_func_t)FT_Done_Face);
        cairo_set_font_face(cr, font_face);
    }
    if (font_size < 0.01) {
        font_size = 11.0;
    }
    cairo_set_font_size(cr, font_size);
    default_font = cairo_get_scaled_font(cr);
    cairo_scaled_font_extents(default_font, &extents);
    font->userdata.ptr = default_font;
    font->height = extents.height;
    font->width = nk_cairo_text_width;

    cairo_ctx = (struct nk_cairo_context *)malloc(sizeof(struct nk_cairo_context));
    cairo_ctx->surface = (cairo_surface_t *)surface;
    cairo_ctx->cr = cr;
    cairo_ctx->font = font;
    cairo_ctx->bg = bg;
    cairo_ctx->last_buffer = NULL;
    cairo_ctx->buffer_size = 0;
    cairo_ctx->repaint = nk_false;

    return cairo_ctx;
}

NK_API cairo_surface_t *nk_cairo_surface(struct nk_cairo_context *cairo_ctx)
{
    return cairo_ctx->surface;
}

NK_API struct nk_user_font *nk_cairo_default_font(struct nk_cairo_context *cairo_ctx)
{
    return cairo_ctx->font;
}

NK_API void nk_cairo_free(struct nk_cairo_context *cairo_ctx)
{
    free (cairo_ctx->last_buffer);
    cairo_destroy(cairo_ctx->cr);
    cairo_surface_destroy(cairo_ctx->surface);
    free(cairo_ctx->font);
    free(cairo_ctx);
}

NK_API void nk_cairo_damage(struct nk_cairo_context *cairo_ctx)
{
    cairo_ctx->repaint = nk_true;
}

NK_API int nk_cairo_render(struct nk_cairo_context *cairo_ctx, struct nk_context *nk_ctx)
{
    cairo_t *cr;
    const struct nk_command *cmd = NULL;
    void *cmds = nk_buffer_memory(&nk_ctx->memory);

    if (cairo_ctx->buffer_size != nk_ctx->memory.allocated) {
        cairo_ctx->buffer_size = nk_ctx->memory.allocated;
        cairo_ctx->last_buffer = realloc(cairo_ctx->last_buffer, cairo_ctx->buffer_size);
        memcpy(cairo_ctx->last_buffer, cmds, cairo_ctx->buffer_size);
    }
    else if (!memcmp(cmds, cairo_ctx->last_buffer, cairo_ctx->buffer_size)) {
        if (!cairo_ctx->repaint) {
            return nk_false;
        }
        cairo_ctx->repaint = nk_false;
    }
    else {
        memcpy(cairo_ctx->last_buffer, cmds, cairo_ctx->buffer_size);
    }

    cr = cairo_ctx->cr;
    cairo_push_group(cr);

    cairo_set_source_rgb(cr, NK_TO_CAIRO(cairo_ctx->bg->r), NK_TO_CAIRO(cairo_ctx->bg->g), NK_TO_CAIRO(cairo_ctx->bg->b));
    cairo_paint(cr);

    nk_foreach(cmd, nk_ctx) {
        switch (cmd->type) {
        case NK_COMMAND_NOP:
            break;
        case NK_COMMAND_SCISSOR:
            {
                const struct nk_command_scissor *s = (const struct nk_command_scissor *)cmd;
                cairo_reset_clip(cr);
                if (s->x >= 0) {
                    cairo_rectangle(cr, s->x - 1, s->y - 1, s->w + 2, s->h + 2);
                    cairo_clip(cr);
                }
            }
            break;
        case NK_COMMAND_LINE:
            {
                const struct nk_command_line *l = (const struct nk_command_line *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(l->color.r), NK_TO_CAIRO(l->color.g), NK_TO_CAIRO(l->color.b), NK_TO_CAIRO(l->color.a));
                cairo_set_line_width(cr, l->line_thickness);
                cairo_move_to(cr, l->begin.x, l->begin.y);
                cairo_line_to(cr, l->end.x, l->end.y);
                cairo_stroke(cr);
            }
            break;
        case NK_COMMAND_CURVE:
            {
                const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(q->color.r), NK_TO_CAIRO(q->color.g), NK_TO_CAIRO(q->color.b), NK_TO_CAIRO(q->color.a));
                cairo_set_line_width(cr, q->line_thickness);
                cairo_move_to(cr, q->begin.x, q->begin.y);
                cairo_curve_to(cr, q->ctrl[0].x, q->ctrl[0].y, q->ctrl[1].x, q->ctrl[1].y, q->end.x, q->end.y);
                cairo_stroke(cr);
            }
            break;
        case NK_COMMAND_RECT:
            {
                const struct nk_command_rect *r = (const struct nk_command_rect *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(r->color.r), NK_TO_CAIRO(r->color.g), NK_TO_CAIRO(r->color.b), NK_TO_CAIRO(r->color.a));
                cairo_set_line_width(cr, r->line_thickness);
                if (r->rounding == 0) {
                    cairo_rectangle(cr, r->x, r->y, r->w, r->h);
                }
                else {
                    int xl = r->x + r->w - r->rounding;
                    int xr = r->x + r->rounding;
                    int yl = r->y + r->h - r->rounding;
                    int yr = r->y + r->rounding;
                    cairo_new_sub_path(cr);
                    cairo_arc(cr, xl, yr, r->rounding, NK_XCB_DEG_TO_RAD(-90), NK_XCB_DEG_TO_RAD(0));
                    cairo_arc(cr, xl, yl, r->rounding, NK_XCB_DEG_TO_RAD(0), NK_XCB_DEG_TO_RAD(90));
                    cairo_arc(cr, xr, yl, r->rounding, NK_XCB_DEG_TO_RAD(90), NK_XCB_DEG_TO_RAD(180));
                    cairo_arc(cr, xr, yr, r->rounding, NK_XCB_DEG_TO_RAD(180), NK_XCB_DEG_TO_RAD(270));
                    cairo_close_path(cr);
                }
                cairo_stroke(cr);
            }
            break;
        case NK_COMMAND_RECT_FILLED:
            {
                const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(r->color.r), NK_TO_CAIRO(r->color.g), NK_TO_CAIRO(r->color.b), NK_TO_CAIRO(r->color.a));
                if (r->rounding == 0) {
                    cairo_rectangle(cr, r->x, r->y, r->w, r->h);
                } else {
                    int xl = r->x + r->w - r->rounding;
                    int xr = r->x + r->rounding;
                    int yl = r->y + r->h - r->rounding;
                    int yr = r->y + r->rounding;
                    cairo_new_sub_path(cr);
                    cairo_arc(cr, xl, yr, r->rounding, NK_XCB_DEG_TO_RAD(-90), NK_XCB_DEG_TO_RAD(0));
                    cairo_arc(cr, xl, yl, r->rounding, NK_XCB_DEG_TO_RAD(0), NK_XCB_DEG_TO_RAD(90));
                    cairo_arc(cr, xr, yl, r->rounding, NK_XCB_DEG_TO_RAD(90), NK_XCB_DEG_TO_RAD(180));
                    cairo_arc(cr, xr, yr, r->rounding, NK_XCB_DEG_TO_RAD(180), NK_XCB_DEG_TO_RAD(270));
                    cairo_close_path(cr);
                }
                cairo_fill(cr);
            }
            break;
        case NK_COMMAND_RECT_MULTI_COLOR:
            {
                /* from https://github.com/taiwins/twidgets/blob/master/src/nk_wl_cairo.c */
                const struct nk_command_rect_multi_color *r = (const struct nk_command_rect_multi_color *)cmd;
                cairo_pattern_t *pat = cairo_pattern_create_mesh();
                if (pat) {
                    cairo_mesh_pattern_begin_patch(pat);
                    cairo_mesh_pattern_move_to(pat, r->x, r->y);
                    cairo_mesh_pattern_line_to(pat, r->x, r->y + r->h);
                    cairo_mesh_pattern_line_to(pat, r->x + r->w, r->y + r->h);
                    cairo_mesh_pattern_line_to(pat, r->x + r->w, r->y);
                    cairo_mesh_pattern_set_corner_color_rgba(pat, 0, NK_TO_CAIRO(r->left.r), NK_TO_CAIRO(r->left.g), NK_TO_CAIRO(r->left.b), NK_TO_CAIRO(r->left.a));
                    cairo_mesh_pattern_set_corner_color_rgba(pat, 1, NK_TO_CAIRO(r->bottom.r), NK_TO_CAIRO(r->bottom.g), NK_TO_CAIRO(r->bottom.b), NK_TO_CAIRO(r->bottom.a));
                    cairo_mesh_pattern_set_corner_color_rgba(pat, 2, NK_TO_CAIRO(r->right.r), NK_TO_CAIRO(r->right.g), NK_TO_CAIRO(r->right.b), NK_TO_CAIRO(r->right.a));
                    cairo_mesh_pattern_set_corner_color_rgba(pat, 3, NK_TO_CAIRO(r->top.r), NK_TO_CAIRO(r->top.g), NK_TO_CAIRO(r->top.b), NK_TO_CAIRO(r->top.a));
                    cairo_mesh_pattern_end_patch(pat);

                    cairo_rectangle(cr, r->x, r->y, r->w, r->h);
                    cairo_set_source(cr, pat);
                    cairo_fill(cr);
                    cairo_pattern_destroy(pat);
                }
            }
            break;
        case NK_COMMAND_CIRCLE:
            {
                const struct nk_command_circle *c = (const struct nk_command_circle *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(c->color.r), NK_TO_CAIRO(c->color.g), NK_TO_CAIRO(c->color.b), NK_TO_CAIRO(c->color.a));
                cairo_set_line_width(cr, c->line_thickness);
                cairo_save(cr);
                cairo_translate(cr, c->x + c->w / 2.0, c->y + c->h / 2.0);
                cairo_scale(cr, c->w / 2.0, c->h / 2.0);
                cairo_arc(cr, 0, 0, 1, NK_XCB_DEG_TO_RAD(0), NK_XCB_DEG_TO_RAD(360));
                cairo_restore(cr);
                cairo_stroke(cr);
            }
            break;
        case NK_COMMAND_CIRCLE_FILLED:
            {
                const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(c->color.r), NK_TO_CAIRO(c->color.g), NK_TO_CAIRO(c->color.b), NK_TO_CAIRO(c->color.a));
                cairo_save(cr);
                cairo_translate(cr, c->x + c->w / 2.0, c->y + c->h / 2.0);
                cairo_scale(cr, c->w / 2.0, c->h / 2.0);
                cairo_arc(cr, 0, 0, 1, NK_XCB_DEG_TO_RAD(0), NK_XCB_DEG_TO_RAD(360));
                cairo_restore(cr);
                cairo_fill(cr);
            }
            break;
        case NK_COMMAND_ARC:
            {
                const struct nk_command_arc *a = (const struct nk_command_arc*) cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(a->color.r), NK_TO_CAIRO(a->color.g), NK_TO_CAIRO(a->color.b), NK_TO_CAIRO(a->color.a));
                cairo_set_line_width(cr, a->line_thickness);
                cairo_arc(cr, a->cx, a->cy, a->r, NK_XCB_DEG_TO_RAD(a->a[0]), NK_XCB_DEG_TO_RAD(a->a[1]));
                cairo_stroke(cr);
            }
            break;
        case NK_COMMAND_ARC_FILLED:
            {
                const struct nk_command_arc_filled *a = (const struct nk_command_arc_filled*)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(a->color.r), NK_TO_CAIRO(a->color.g), NK_TO_CAIRO(a->color.b), NK_TO_CAIRO(a->color.a));
                cairo_arc(cr, a->cx, a->cy, a->r, NK_XCB_DEG_TO_RAD(a->a[0]), NK_XCB_DEG_TO_RAD(a->a[1]));
                cairo_fill(cr);
            }
            break;
        case NK_COMMAND_TRIANGLE:
            {
                const struct nk_command_triangle *t = (const struct nk_command_triangle *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(t->color.r), NK_TO_CAIRO(t->color.g), NK_TO_CAIRO(t->color.b), NK_TO_CAIRO(t->color.a));
                cairo_set_line_width(cr, t->line_thickness);
                cairo_move_to(cr, t->a.x, t->a.y);
                cairo_line_to(cr, t->b.x, t->b.y);
                cairo_line_to(cr, t->c.x, t->c.y);
                cairo_close_path(cr);
                cairo_stroke(cr);
            }
            break;
        case NK_COMMAND_TRIANGLE_FILLED:
            {
                const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(t->color.r), NK_TO_CAIRO(t->color.g), NK_TO_CAIRO(t->color.b), NK_TO_CAIRO(t->color.a));
                cairo_move_to(cr, t->a.x, t->a.y);
                cairo_line_to(cr, t->b.x, t->b.y);
                cairo_line_to(cr, t->c.x, t->c.y);
                cairo_close_path(cr);
                cairo_fill(cr);
            }
            break;
        case NK_COMMAND_POLYGON:
            {
                int i;
                const struct nk_command_polygon *p = (const struct nk_command_polygon *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(p->color.r), NK_TO_CAIRO(p->color.g), NK_TO_CAIRO(p->color.b), NK_TO_CAIRO(p->color.a));
                cairo_set_line_width(cr, p->line_thickness);
                cairo_move_to(cr, p->points[0].x, p->points[0].y);
                for (i = 1; i < p->point_count; ++i) {
                    cairo_line_to(cr, p->points[i].x, p->points[i].y);
                }
                cairo_close_path(cr);
                cairo_stroke(cr);
            }
            break;
        case NK_COMMAND_POLYGON_FILLED:
            {
                int i;
                const struct nk_command_polygon_filled *p = (const struct nk_command_polygon_filled *)cmd;
                cairo_set_source_rgba (cr, NK_TO_CAIRO(p->color.r), NK_TO_CAIRO(p->color.g), NK_TO_CAIRO(p->color.b), NK_TO_CAIRO(p->color.a));
                cairo_move_to(cr, p->points[0].x, p->points[0].y);
                for (i = 1; i < p->point_count; ++i) {
                    cairo_line_to(cr, p->points[i].x, p->points[i].y);
                }
                cairo_close_path(cr);
                cairo_fill(cr);
            }
            break;
        case NK_COMMAND_POLYLINE:
            {
                int i;
                const struct nk_command_polyline *p = (const struct nk_command_polyline *)cmd;
                cairo_set_source_rgba(cr, NK_TO_CAIRO(p->color.r), NK_TO_CAIRO(p->color.g), NK_TO_CAIRO(p->color.b), NK_TO_CAIRO(p->color.a));
                cairo_set_line_width(cr, p->line_thickness);
                cairo_move_to(cr, p->points[0].x, p->points[0].y);
                for (i = 1; i < p->point_count; ++i) {
                    cairo_line_to(cr, p->points[i].x, p->points[i].y);
                }
                cairo_stroke(cr);
            }
            break;
        case NK_COMMAND_TEXT:
            {
                const struct nk_command_text *t = (const struct nk_command_text *)cmd;
                cairo_glyph_t *glyphs = NULL;
                int num_glyphs;
                cairo_text_cluster_t *clusters = NULL;
                int num_clusters;
                cairo_text_cluster_flags_t cluster_flags;
                cairo_font_extents_t extents;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(t->foreground.r), NK_TO_CAIRO(t->foreground.g), NK_TO_CAIRO(t->foreground.b), NK_TO_CAIRO(t->foreground.a));
                cairo_scaled_font_extents((cairo_scaled_font_t *)t->font->userdata.ptr, &extents);
                cairo_scaled_font_text_to_glyphs((cairo_scaled_font_t *)t->font->userdata.ptr,
                        t->x, t->y + extents.ascent, t->string, t->length,
                        &glyphs, &num_glyphs, &clusters, &num_clusters,
                        &cluster_flags);
                cairo_show_text_glyphs(cr, t->string, t->length, glyphs,
                        num_glyphs, clusters, num_clusters,
                        cluster_flags);
                cairo_glyph_free(glyphs);
                cairo_text_cluster_free(clusters);
            }
            break;
        case NK_COMMAND_IMAGE:
            {
                /* from https://github.com/taiwins/twidgets/blob/master/src/nk_wl_cairo.c */
                const struct nk_command_image *im = (const struct nk_command_image *)cmd;
                cairo_surface_t *img_surf;
                double sw = (double)im->w / (double)im->img.region[2];
                double sh = (double)im->h / (double)im->img.region[3];
                cairo_format_t format = CAIRO_FORMAT_ARGB32;
                int stride = cairo_format_stride_for_width(format, im->img.w);

                if (!im->img.handle.ptr) return nk_false;
                img_surf = cairo_image_surface_create_for_data((unsigned char *)im->img.handle.ptr, format, im->img.w, im->img.h, stride);
                if (!img_surf) return nk_false;
                cairo_save(cr);

                cairo_rectangle(cr, im->x, im->y, im->w, im->h);
                /* scale here, if after source set, the scale would not apply to source
                 * surface
                 */
                cairo_scale(cr, sw, sh);
                /* the coordinates system in cairo is not intuitive, scale, translate,
                 * are applied to source. Refer to
                 * "https://www.cairographics.org/FAQ/#paint_from_a_surface" for details
                 *
                 * if you set source_origin to (0,0), it would be like source origin
                 * aligned to dest origin, then if you draw a rectangle on (x, y, w, h).
                 * it would clip out the (x, y, w, h) of the source on you dest as well.
                 */
                cairo_set_source_surface(cr, img_surf, im->x/sw - im->img.region[0], im->y/sh - im->img.region[1]);
                cairo_fill(cr);

                cairo_restore(cr);
                cairo_surface_destroy(img_surf);
            }
            break;
        case NK_COMMAND_CUSTOM:
            {
	            const struct nk_command_custom *cu = (const struct nk_command_custom *)cmd;
                if (cu->callback) {
                    cu->callback(cr, cu->x, cu->y, cu->w, cu->h, cu->callback_data);
                }
            }
        default:
            break;
        }
    }

    cairo_pop_group_to_source(cr);
    cairo_paint(cr);
    cairo_surface_flush(cairo_ctx->surface);

    return nk_true;
}

NK_INTERN xkbcommon_context *xkbcommon_init(xcb_connection_t *conn)
{
	xkbcommon_context *kbdctx;
	int32_t device_id;

	int ret = xkb_x11_setup_xkb_extension(conn,
		XKB_X11_MIN_MAJOR_XKB_VERSION,
		XKB_X11_MIN_MINOR_XKB_VERSION,
		XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
		NULL, NULL, NULL, NULL);

    if (ret == 0)
	{
		return NULL;
	}

	kbdctx = (xkbcommon_context *)malloc(sizeof(xkbcommon_context));
	kbdctx->ctx = NULL;
	kbdctx->keymap = NULL;
	kbdctx->state = NULL;

	kbdctx->ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!kbdctx->ctx)
	{
		xkbcommon_free(kbdctx);
		return NULL;
	}

	device_id = xkb_x11_get_core_keyboard_device_id(conn);
	if (device_id == -1)
	{
		xkbcommon_free(kbdctx);
		return NULL;
	}

	kbdctx->keymap = xkb_x11_keymap_new_from_device(kbdctx->ctx, conn, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (!kbdctx->keymap)
	{
		xkbcommon_free(kbdctx);
		return NULL;
	}

	kbdctx->state = xkb_x11_state_new_from_device(kbdctx->keymap, conn, device_id);
	if (!kbdctx->state)
	{
		xkbcommon_free(kbdctx);
		return NULL;
	}

	return kbdctx;
}

NK_INTERN void xkbcommon_free(xkbcommon_context *kbdctx)
{
	if (kbdctx != NULL)
	{
		if (kbdctx->state) xkb_state_unref(kbdctx->state);
		if (kbdctx->keymap) xkb_keymap_unref(kbdctx->keymap);
		if (kbdctx->ctx) xkb_context_unref(kbdctx->ctx);

		kbdctx->ctx = NULL;
		kbdctx->keymap = NULL;
		kbdctx->state = NULL;

		free(kbdctx);
	}
}

NK_INTERN xkb_keysym_t keycode_to_keysym(nk_xcb_context *ctx, xkb_keycode_t keycode, int pressed)
{
	xkb_keysym_t keysym;
	xkbcommon_context *kbdctx = ctx->xkbcommon_ctx;

	if (kbdctx != NULL)
	{
		keysym = xkb_state_key_get_one_sym(kbdctx->state, keycode);

		/*xkb_state_component changed = */
			xkb_state_update_key(kbdctx->state, keycode, pressed ? XKB_KEY_DOWN : XKB_KEY_UP);
	}
	else
	{
		keysym = 0;
	}

	return keysym;
}

#endif /* NK_XCB_CAIRO_IMPLEMENTATION */
