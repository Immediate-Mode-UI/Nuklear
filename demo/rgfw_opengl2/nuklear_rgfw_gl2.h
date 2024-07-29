/*
 * Nuklear - v1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2017 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_RGFW_GL2_H_
#define NK_RGFW_GL2_H_

#include "RGFW.h"

enum nk_RGFW_init_state{
    NK_RGFW_DEFAULT = 0,
    NK_RGFW_INSTALL_CALLBACKS
};
NK_API struct nk_context*   nk_RGFW_init(struct RGFW_window* win, enum nk_RGFW_init_state);
NK_API void                 nk_RGFW_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_RGFW_font_stash_end(void);

NK_API void                 nk_RGFW_new_frame(void);
NK_API void                 nk_RGFW_render(enum nk_anti_aliasing);
NK_API void                 nk_RGFW_shutdown(void);

NK_API void                 nk_RGFW_key_callback(RGFW_window* win, u32 keycode, char keyName[16], unsigned char lockState, unsigned char pressed);
NK_API void                 nk_RGFW_mouse_button_callback(RGFW_window*  window, unsigned char button, double scroll, unsigned char pressed);
NK_API void                 nk_rgfw_scroll_callback(RGFW_window* win, double xoff, double yoff);

#endif

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_RGFW_GL2_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>

#ifndef NK_RGFW_TEXT_MAX
#define NK_RGFW_TEXT_MAX 256
#endif
#ifndef NK_RGFW_DOUBLE_CLICK_LO
#define NK_RGFW_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_RGFW_DOUBLE_CLICK_HI
#define NK_RGFW_DOUBLE_CLICK_HI 0.2
#endif

struct nk_RGFW_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    GLuint font_tex;
};

struct nk_RGFW_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct nk_RGFW {
    RGFW_window* win;
    int width, height;
    int display_width, display_height;
    struct nk_RGFW_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_RGFW_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int is_double_click_down;
    struct nk_vec2 double_click_pos;
} RGFW;

NK_INTERN void
nk_RGFW_device_upload_atlas(const void *image, int width, int height)
{
    struct nk_RGFW_device *dev = &RGFW.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void
nk_RGFW_render(enum nk_anti_aliasing AA)
{
    /* setup global state */
    struct nk_RGFW_device *dev = &RGFW.ogl;
    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_TRANSFORM_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* setup viewport/project */
    glViewport(0,0,(GLsizei)RGFW.display_width,(GLsizei)RGFW.display_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, RGFW.width, RGFW.height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    {
        GLsizei vs = sizeof(struct nk_RGFW_vertex);
        size_t vp = offsetof(struct nk_RGFW_vertex, position);
        size_t vt = offsetof(struct nk_RGFW_vertex, uv);
        size_t vc = offsetof(struct nk_RGFW_vertex, col);

        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        const nk_draw_index *offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* fill convert configuration */
        struct nk_convert_config config;
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_RGFW_vertex, position)},
            {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_RGFW_vertex, uv)},
            {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_RGFW_vertex, col)},
            {NK_VERTEX_LAYOUT_END}
        };
        memset(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(struct nk_RGFW_vertex);
        config.vertex_alignment = NK_ALIGNOF(struct nk_RGFW_vertex);
        config.tex_null = dev->tex_null;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = AA;
        config.line_AA = AA;

        /* convert shapes into vertexes */
        nk_buffer_init_default(&vbuf);
        nk_buffer_init_default(&ebuf);
        nk_convert(&RGFW.ctx, &dev->cmds, &vbuf, &ebuf, &config);

        /* setup vertex buffer pointer */
        {const void *vertices = nk_buffer_memory_const(&vbuf);
        glVertexPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vp));
        glTexCoordPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vt));
        glColorPointer(4, GL_UNSIGNED_BYTE, vs, (const void*)((const nk_byte*)vertices + vc));}

        /* iterate over and execute each draw command */
        offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);
        nk_draw_foreach(cmd, &RGFW.ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                (GLint)(cmd->clip_rect.x * RGFW.fb_scale.x),
                (GLint)((RGFW.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * RGFW.fb_scale.y),
                (GLint)(cmd->clip_rect.w * RGFW.fb_scale.x),
                (GLint)(cmd->clip_rect.h * RGFW.fb_scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&RGFW.ctx);
        nk_buffer_clear(&dev->cmds);
        nk_buffer_free(&vbuf);
        nk_buffer_free(&ebuf);
    }

    /* default OpenGL state */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

char RGFW_keyCodeToChar(u32 keyCode, b8 caps) {
    static const char map[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        '`', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', 
        '9', '-', '=', 0, '\t', 
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
        'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
        'x', 'y', 'z', '.', ',', '/', '[', ']', 
        ';', '\n', '\'', '\\', 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        '/', '*', '-', '1', '2', '3', 
        '3', '5', '6', '7', '8',  '9', '0', '\n'
    };

    static const char mapCaps[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        '~', ')', '!', '@', '#',
        '$', '%', '^', '&', '*', 
        '(', '_', '+', 0, '0', 
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
        'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
        'X', 'Y', 'Z', '>', '<', '?', '{', '}', 
        ':', '\n', '"', '|', 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        '?', '*', '-', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    if (caps == RGFW_FALSE)
        return map[keyCode];
    
    return mapCaps[keyCode];
}

NK_API void
nk_RGFW_key_callback(RGFW_window* win, u32 keycode, char keyName[16], u8 lockState, b8 pressed)
{
    if (pressed == RGFW_FALSE)
        return;
    
    RGFW_UNUSED(keyName);
    
    #define RGFW_xor(x, y) (( (x) && (!(y)) ) ||  ((y) && (!(x)) ))
    b8 caps4caps = (lockState & RGFW_CAPSLOCK) && ((keycode >= RGFW_a) && (keycode <= RGFW_z));
    char ch = RGFW_keyCodeToChar(keycode, RGFW_xor((RGFW_isPressed(win, RGFW_ShiftL) || RGFW_isPressed(win, RGFW_ShiftR)), caps4caps));
    #undef RGFW_xor

    if (RGFW.text_len < NK_RGFW_TEXT_MAX)
        RGFW.text[RGFW.text_len++] = ch;
}

NK_API void
nk_rgfw_scroll_callback(RGFW_window* win, double xoff, double yoff)
{
    (void)win; (void)xoff;
    RGFW.scroll.x += (float)xoff;
    RGFW.scroll.y += (float)yoff;
}

NK_API void
nk_RGFW_mouse_button_callback(RGFW_window*  window, u8 button, double scroll, b8 pressed)
{
    double x, y;
    if (button != RGFW_mouseLeft && button < RGFW_mouseScrollUp) return;
    RGFW_point p = RGFW_window_getMousePoint(window);
    x = (double)p.x;
    y = (double)p.y;

    if (button >= RGFW_mouseScrollUp) {
        return nk_rgfw_scroll_callback(window, 0, scroll);
    }

    if (pressed == RGFW_TRUE)  {
        double dt = RGFW_getTime() - RGFW.last_button_click;
        if (dt > NK_RGFW_DOUBLE_CLICK_LO && dt < NK_RGFW_DOUBLE_CLICK_HI) {
            RGFW.is_double_click_down = nk_true;
            RGFW.double_click_pos = nk_vec2((float)x, (float)y);
        }
        RGFW.last_button_click = RGFW_getTime();
    } else RGFW.is_double_click_down = nk_false;
}

static char* clipboard_string = NULL;

NK_INTERN void
nk_RGFW_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    if (clipboard_string != NULL) {
        free(clipboard_string);
        clipboard_string = NULL;
    }
    
    size_t size;
    clipboard_string = RGFW_readClipboard(&size);
    if (clipboard_string) nk_textedit_paste(edit, (const char*)clipboard_string, size);
    (void)usr;
}

NK_INTERN void
nk_RGFW_clipboard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    RGFW_writeClipboard(str, len);
    free(str);
}

NK_API struct nk_context*
nk_RGFW_init(RGFW_window* win, enum nk_RGFW_init_state init_state)
{
    RGFW.win = win;
    if (init_state == NK_RGFW_INSTALL_CALLBACKS) {
        RGFW_setKeyCallback(nk_RGFW_key_callback);
        RGFW_setMouseButtonCallback(nk_RGFW_mouse_button_callback);
    }
    nk_init_default(&RGFW.ctx, 0);
    RGFW.ctx.clip.copy = nk_RGFW_clipboard_copy;
    RGFW.ctx.clip.paste = nk_RGFW_clipboard_paste;
    RGFW.ctx.clip.userdata = nk_handle_ptr(0);
    nk_buffer_init_default(&RGFW.ogl.cmds);

    RGFW.is_double_click_down = nk_false;
    RGFW.double_click_pos = nk_vec2(0, 0);

    return &RGFW.ctx;
}

NK_API void
nk_RGFW_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&RGFW.atlas);
    nk_font_atlas_begin(&RGFW.atlas);
    *atlas = &RGFW.atlas;
}

NK_API void
nk_RGFW_font_stash_end(void)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&RGFW.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_RGFW_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&RGFW.atlas, nk_handle_id((int)RGFW.ogl.font_tex), &RGFW.ogl.tex_null);
    if (RGFW.atlas.default_font)
        nk_style_set_font(&RGFW.ctx, &RGFW.atlas.default_font->handle);
}

NK_API void
nk_RGFW_new_frame(void)
{
    int i;
    double x, y;
    struct nk_context *ctx = &RGFW.ctx;
    struct RGFW_window* win = RGFW.win;

    RGFW.width = win->r.w;
    RGFW.height = win->r.h;

    RGFW_area screenSize = RGFW_getScreenSize();
    RGFW.display_width = win->r.w;
    RGFW.display_height = win->r.h;

    RGFW.fb_scale.x = (float)RGFW.display_width/(float)RGFW.width;
    RGFW.fb_scale.y = (float)RGFW.display_height/(float)RGFW.height;

    nk_input_begin(ctx);
    for (i = 0; i < RGFW.text_len; ++i)
        nk_input_unicode(ctx, RGFW.text[i]);

    /* 
        I don't know what the point of this is 
        but it seems to cause a lot of weird issues
    */
    /* optional grabbing behavior */
    /*if (ctx->input.mouse.grab) /* I don't know if this is the intended behavior /
        RGFW_window_showMouse(win, 0); or RGFW_window_holdMouse ?
    else if (ctx->input.mouse.ungrab)
        RGFW_window_showMouse(win, 1);*/

    nk_input_key(ctx, NK_KEY_DEL, RGFW_isPressed(win, RGFW_Delete) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_ENTER, RGFW_isPressed(win, RGFW_Return) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_TAB, RGFW_isPressed(win, RGFW_Tab) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_BACKSPACE, RGFW_isPressed(win, RGFW_BackSpace) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_UP, RGFW_isPressed(win, RGFW_Up) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_DOWN, RGFW_isPressed(win, RGFW_Down) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_TEXT_START, RGFW_isPressed(win, RGFW_Home) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_TEXT_END, RGFW_isPressed(win, RGFW_End) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_SCROLL_START, RGFW_isPressed(win, RGFW_Home) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_SCROLL_END, RGFW_isPressed(win, RGFW_End) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_SCROLL_DOWN, RGFW_isPressed(win, RGFW_PageDown) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_SCROLL_UP, RGFW_isPressed(win, RGFW_PageUp) == RGFW_TRUE);
    nk_input_key(ctx, NK_KEY_SHIFT, RGFW_isPressed(win, RGFW_ShiftL) == RGFW_TRUE||
                                    RGFW_isPressed(win, RGFW_ShiftR) == RGFW_TRUE);

    if (RGFW_isPressed(win, RGFW_ControlL) == RGFW_TRUE ||
        RGFW_isPressed(win, RGFW_ControlR) == RGFW_TRUE) {
        nk_input_key(ctx, NK_KEY_COPY, RGFW_isPressed(win, RGFW_c) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_PASTE, RGFW_isPressed(win, RGFW_v) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_CUT, RGFW_isPressed(win, RGFW_x) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, RGFW_isPressed(win, RGFW_z) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_TEXT_REDO, RGFW_isPressed(win, RGFW_r) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, RGFW_isPressed(win, RGFW_Left) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, RGFW_isPressed(win, RGFW_Right) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, RGFW_isPressed(win, RGFW_b) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, RGFW_isPressed(win, RGFW_e) == RGFW_TRUE);
    } else {
        nk_input_key(ctx, NK_KEY_LEFT, RGFW_isPressed(win, RGFW_Left) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_RIGHT, RGFW_isPressed(win, RGFW_Right) == RGFW_TRUE);
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
    }

    RGFW_point p = RGFW_window_getMousePoint(RGFW.win);
    nk_input_motion(ctx, p.x, p.y);
    if (ctx->input.mouse.grabbed) {
        ctx->input.mouse.prev.x = (float)p.x;
        ctx->input.mouse.prev.y = (float)p.y;

        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }

    nk_input_button(ctx, NK_BUTTON_LEFT, p.x, p.y, RGFW_isMousePressed(win, RGFW_mouseLeft));
    nk_input_button(ctx, NK_BUTTON_MIDDLE, p.x, p.y, RGFW_isMousePressed(win, RGFW_mouseMiddle));
    nk_input_button(ctx, NK_BUTTON_RIGHT, p.x, p.y, RGFW_isMousePressed(win, RGFW_mouseRight));
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)RGFW.double_click_pos.x, (int)RGFW.double_click_pos.y, RGFW.is_double_click_down);
    nk_input_scroll(ctx, RGFW.scroll);
    nk_input_end(&RGFW.ctx);
    RGFW.text_len = 0;
    RGFW.scroll = nk_vec2(0,0);
}

NK_API
void nk_RGFW_shutdown(void)
{
    if (clipboard_string != NULL) {
        free(clipboard_string);
        clipboard_string = NULL;
    }
    
    struct nk_RGFW_device *dev = &RGFW.ogl;
    nk_font_atlas_clear(&RGFW.atlas);
    nk_free(&RGFW.ctx);
    glDeleteTextures(1, &dev->font_tex);
    nk_buffer_free(&dev->cmds);
    memset(&RGFW, 0, sizeof(RGFW));
}

#endif