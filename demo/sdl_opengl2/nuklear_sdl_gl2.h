/*
 * Nuklear - 1.32.0 - public domain
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
#ifndef NK_SDL_GL2_H_
#define NK_SDL_GL2_H_

#include <SDL2/SDL.h>
NK_API struct nk_context*   nk_sdl_init(SDL_Window *win);
NK_API void                 nk_sdl_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_sdl_font_stash_end(void);
NK_API int                  nk_sdl_handle_event(SDL_Event *evt);
NK_API void                 nk_sdl_render(enum nk_anti_aliasing);
NK_API void                 nk_sdl_shutdown(void);
NK_API void                 nk_sdl_handle_grab(void);

#endif
/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_SDL_GL2_IMPLEMENTATION
#include <string.h>
#include <stdlib.h>

struct nk_sdl_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    GLuint font_tex;
};

struct nk_sdl_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct nk_sdl {
    SDL_Window *win;
    struct nk_sdl_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    Uint64 time_of_last_frame;
} sdl;

NK_INTERN void
nk_sdl_device_upload_atlas(const void *image, int width, int height)
{
    struct nk_sdl_device *dev = &sdl.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void
nk_sdl_render(enum nk_anti_aliasing AA)
{
    /* setup global state */
    struct nk_sdl_device *dev = &sdl.ogl;
    int width, height;
    int display_width, display_height;
    struct nk_vec2 scale;

    Uint64 now = SDL_GetTicks64();
    sdl.ctx.delta_time_seconds = (float)(now - sdl.time_of_last_frame) / 1000;
    sdl.time_of_last_frame = now;

    SDL_GetWindowSize(sdl.win, &width, &height);
    SDL_GL_GetDrawableSize(sdl.win, &display_width, &display_height);
    scale.x = (float)display_width/(float)width;
    scale.y = (float)display_height/(float)height;

    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_TRANSFORM_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* setup viewport/project */
    glViewport(0,0,(GLsizei)display_width,(GLsizei)display_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    {
        GLsizei vs = sizeof(struct nk_sdl_vertex);
        size_t vp = offsetof(struct nk_sdl_vertex, position);
        size_t vt = offsetof(struct nk_sdl_vertex, uv);
        size_t vc = offsetof(struct nk_sdl_vertex, col);

        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        const nk_draw_index *offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* fill converting configuration */
        struct nk_convert_config config;
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_sdl_vertex, position)},
            {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_sdl_vertex, uv)},
            {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_sdl_vertex, col)},
            {NK_VERTEX_LAYOUT_END}
        };
        memset(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(struct nk_sdl_vertex);
        config.vertex_alignment = NK_ALIGNOF(struct nk_sdl_vertex);
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
        nk_convert(&sdl.ctx, &dev->cmds, &vbuf, &ebuf, &config);

        /* setup vertex buffer pointer */
        {const void *vertices = nk_buffer_memory_const(&vbuf);
        glVertexPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vp));
        glTexCoordPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vt));
        glColorPointer(4, GL_UNSIGNED_BYTE, vs, (const void*)((const nk_byte*)vertices + vc));}

        /* iterate over and execute each draw command */
        offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);
        nk_draw_foreach(cmd, &sdl.ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                (GLint)(cmd->clip_rect.x * scale.x),
                (GLint)((height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * scale.y),
                (GLint)(cmd->clip_rect.w * scale.x),
                (GLint)(cmd->clip_rect.h * scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&sdl.ctx);
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

static void
nk_sdl_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = SDL_GetClipboardText();
    if (text) {
        nk_textedit_paste(edit, text, nk_strlen(text));
        SDL_free((void *)text);
    }
    (void)usr;
}

static void
nk_sdl_clipboard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    SDL_SetClipboardText(str);
    free(str);
}

NK_API struct nk_context*
nk_sdl_init(SDL_Window *win)
{
    sdl.win = win;
    nk_init_default(&sdl.ctx, 0);
    sdl.ctx.clip.copy = nk_sdl_clipboard_copy;
    sdl.ctx.clip.paste = nk_sdl_clipboard_paste;
    sdl.ctx.clip.userdata = nk_handle_ptr(0);
    nk_buffer_init_default(&sdl.ogl.cmds);
    sdl.time_of_last_frame = SDL_GetTicks64();
    return &sdl.ctx;
}

NK_API void
nk_sdl_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&sdl.atlas);
    nk_font_atlas_begin(&sdl.atlas);
    *atlas = &sdl.atlas;
}

NK_API void
nk_sdl_font_stash_end(void)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&sdl.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_sdl_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&sdl.atlas, nk_handle_id((int)sdl.ogl.font_tex), &sdl.ogl.tex_null);
    if (sdl.atlas.default_font)
        nk_style_set_font(&sdl.ctx, &sdl.atlas.default_font->handle);
}

NK_API void
nk_sdl_handle_grab(void)
{
    struct nk_context *ctx = &sdl.ctx;
    if (ctx->input.mouse.grab) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    } else if (ctx->input.mouse.ungrab) {
        /* better support for older SDL by setting mode first; causes an extra mouse motion event */
        SDL_SetRelativeMouseMode(SDL_FALSE);
        SDL_WarpMouseInWindow(sdl.win, (int)ctx->input.mouse.prev.x, (int)ctx->input.mouse.prev.y);
    } else if (ctx->input.mouse.grabbed) {
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }
}

NK_API int
nk_sdl_handle_event(SDL_Event *evt)
{
    struct nk_context *ctx = &sdl.ctx;
    int ctrl_down = SDL_GetModState() & (KMOD_LCTRL | KMOD_RCTRL);

    switch(evt->type)
    {
        case SDL_KEYUP: /* KEYUP & KEYDOWN share same routine */
        case SDL_KEYDOWN:
            {
                int down = evt->type == SDL_KEYDOWN;
                switch(evt->key.keysym.sym)
                {
                    case SDLK_RSHIFT: /* RSHIFT & LSHIFT share same routine */
                    case SDLK_LSHIFT:    nk_input_key(ctx, NK_KEY_SHIFT, down); break;
                    case SDLK_DELETE:    nk_input_key(ctx, NK_KEY_DEL, down); break;

                    case SDLK_KP_ENTER:
                    case SDLK_RETURN:    nk_input_key(ctx, NK_KEY_ENTER, down); break;

                    case SDLK_TAB:       nk_input_key(ctx, NK_KEY_TAB, down); break;
                    case SDLK_BACKSPACE: nk_input_key(ctx, NK_KEY_BACKSPACE, down); break;
                    case SDLK_HOME:      nk_input_key(ctx, NK_KEY_TEXT_START, down);
                                         nk_input_key(ctx, NK_KEY_SCROLL_START, down); break;
                    case SDLK_END:       nk_input_key(ctx, NK_KEY_TEXT_END, down);
                                         nk_input_key(ctx, NK_KEY_SCROLL_END, down); break;
                    case SDLK_PAGEDOWN:  nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down); break;
                    case SDLK_PAGEUP:    nk_input_key(ctx, NK_KEY_SCROLL_UP, down); break;
                    case SDLK_z:         nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && ctrl_down); break;
                    case SDLK_r:         nk_input_key(ctx, NK_KEY_TEXT_REDO, down && ctrl_down); break;
                    case SDLK_c:         nk_input_key(ctx, NK_KEY_COPY, down && ctrl_down); break;
                    case SDLK_v:         nk_input_key(ctx, NK_KEY_PASTE, down && ctrl_down); break;
                    case SDLK_x:         nk_input_key(ctx, NK_KEY_CUT, down && ctrl_down); break;
                    case SDLK_b:         nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && ctrl_down); break;
                    case SDLK_e:         nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && ctrl_down); break;
                    case SDLK_UP:        nk_input_key(ctx, NK_KEY_UP, down); break;
                    case SDLK_DOWN:      nk_input_key(ctx, NK_KEY_DOWN, down); break;
                    case SDLK_a:
                        if (ctrl_down)
                            nk_input_key(ctx,NK_KEY_TEXT_SELECT_ALL, down);
                        break;
                    case SDLK_LEFT:
                        if (ctrl_down)
                            nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
                        else nk_input_key(ctx, NK_KEY_LEFT, down);
                        break;
                    case SDLK_RIGHT:
                        if (ctrl_down)
                            nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
                        else nk_input_key(ctx, NK_KEY_RIGHT, down);
                        break;
                }
            }
            return 1;

        case SDL_MOUSEBUTTONUP: /* MOUSEBUTTONUP & MOUSEBUTTONDOWN share same routine */
        case SDL_MOUSEBUTTONDOWN:
            {
                int down = evt->type == SDL_MOUSEBUTTONDOWN;
                const int x = evt->button.x, y = evt->button.y;
                switch(evt->button.button)
                {
                    case SDL_BUTTON_LEFT:
                        if (evt->button.clicks > 1)
                            nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
                        nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down); break;
                    case SDL_BUTTON_MIDDLE: nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down); break;
                    case SDL_BUTTON_RIGHT:  nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down); break;
                }
            }
            return 1;

        case SDL_MOUSEMOTION:
            if (ctx->input.mouse.grabbed) {
                int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
                nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
            }
            else nk_input_motion(ctx, evt->motion.x, evt->motion.y);
            return 1;

        case SDL_TEXTINPUT:
            {
                nk_glyph glyph;
                memcpy(glyph, evt->text.text, NK_UTF_SIZE);
                nk_input_glyph(ctx, glyph);
            }
            return 1;

        case SDL_MOUSEWHEEL:
            nk_input_scroll(ctx,nk_vec2((float)evt->wheel.x,(float)evt->wheel.y));
            return 1;
    }
    return 0;
}

NK_API
void nk_sdl_shutdown(void)
{
    struct nk_sdl_device *dev = &sdl.ogl;
    nk_font_atlas_clear(&sdl.atlas);
    nk_free(&sdl.ctx);
    glDeleteTextures(1, &dev->font_tex);
    nk_buffer_free(&dev->cmds);
    memset(&sdl, 0, sizeof(sdl));
}

#endif
