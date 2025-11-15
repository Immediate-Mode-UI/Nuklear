/* nuklear - public domain */

/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */

#ifndef NK_SDL3_RENDERER_H_
#define NK_SDL3_RENDERER_H_

#if SDL_MAJOR_VERSION < 3
    #error "nk_sdl3_renderer requires at least SDL 3.0.0"
#endif
#ifndef NK_INCLUDE_COMMAND_USERDATA
    #error "nk_sdl3_renderer requires the NK_INCLUDE_COMMAND_USERDATA define"
#endif
#ifndef NK_INCLUDE_VERTEX_BUFFER_OUTPUT
    #error "nk_sdl3_renderer requires the NK_INCLUDE_VERTEX_BUFFER_OUTPUT define"
#endif

/* We have to redefine it because demos do not include any headers
 * This is the same default value as the one from "src/nuklear_internal.h" */
#ifndef NK_BUFFER_DEFAULT_INITIAL_SIZE
    #define NK_BUFFER_DEFAULT_INITIAL_SIZE (4*1024)
#endif

NK_API struct nk_context*   nk_sdl_init(SDL_Window *win, SDL_Renderer *renderer, struct nk_allocator allocator);
#ifdef NK_INCLUDE_FONT_BAKING
NK_API struct nk_font_atlas* nk_sdl_font_stash_begin(struct nk_context* ctx);
NK_API void                 nk_sdl_font_stash_end(struct nk_context* ctx);
#endif
NK_API int                  nk_sdl_handle_event(struct nk_context* ctx, SDL_Event *evt);
NK_API void                 nk_sdl_render(struct nk_context* ctx, enum nk_anti_aliasing);
NK_API void                 nk_sdl_update_TextInput(struct nk_context* ctx);
NK_API void                 nk_sdl_shutdown(struct nk_context* ctx);
NK_API nk_handle            nk_sdl_get_userdata(struct nk_context* ctx);
NK_API void                 nk_sdl_set_userdata(struct nk_context* ctx, nk_handle userdata);
NK_API void                 nk_sdl_style_set_debug_font(struct nk_context* ctx);
NK_API struct nk_allocator  nk_sdl_allocator(void);

#endif /* NK_SDL3_RENDERER_H_ */

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_SDL3_RENDERER_IMPLEMENTATION
#ifndef NK_SDL3_RENDERER_IMPLEMENTATION_ONCE
#define NK_SDL3_RENDERER_IMPLEMENTATION_ONCE

#ifndef NK_SDL_DOUBLE_CLICK_LO
#define NK_SDL_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_SDL_DOUBLE_CLICK_HI
#define NK_SDL_DOUBLE_CLICK_HI 0.2
#endif

struct nk_sdl_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    SDL_Texture *font_tex;
};

struct nk_sdl_vertex {
    float position[2];
    float uv[2];
    float col[4];
};

struct nk_sdl {
    SDL_Window *win;
    SDL_Renderer *renderer;
    struct nk_user_font* debug_font;
    struct nk_sdl_device ogl;
    struct nk_context ctx;
#ifdef NK_INCLUDE_FONT_BAKING
    struct nk_font_atlas atlas;
#endif
    struct nk_allocator allocator;
    nk_handle userdata;
    Uint64 last_left_click;
    Uint64 last_render;
    bool insert_toggle;
    bool edit_was_active;
};

NK_API nk_handle
nk_sdl_get_userdata(struct nk_context* ctx) {
    struct nk_sdl* sdl;
    NK_ASSERT(ctx);
    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);
    return sdl->userdata;
}

NK_API void
nk_sdl_set_userdata(struct nk_context* ctx, nk_handle userdata) {
    struct nk_sdl* sdl;
    NK_ASSERT(ctx);
    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);
    sdl->userdata = userdata;
}

NK_INTERN void *
nk_sdl_alloc(nk_handle user, void *old, nk_size size)
{
    NK_UNUSED(user);
    /* FIXME: nk_sdl_alloc should use SDL_realloc here, not SDL_malloc
     * but this could cause a double-free due to bug within Nuklear, see:
     * https://github.com/Immediate-Mode-UI/Nuklear/issues/768
     * */
#if 0
    return SDL_realloc(old, size);
#else
    NK_UNUSED(old);
    return SDL_malloc(size);
#endif
}

NK_INTERN void
nk_sdl_free(nk_handle user, void *old)
{
    NK_UNUSED(user);
    SDL_free(old);
}

NK_API struct nk_allocator
nk_sdl_allocator()
{
    struct nk_allocator allocator;
    allocator.userdata.ptr = 0;
    allocator.alloc = nk_sdl_alloc;
    allocator.free = nk_sdl_free;
    return allocator;
}

NK_INTERN void
nk_sdl_device_upload_atlas(struct nk_context* ctx, const void *image, int width, int height)
{
    struct nk_sdl* sdl;
    NK_ASSERT(ctx);
    NK_ASSERT(image);
    NK_ASSERT(width > 0);
    NK_ASSERT(height > 0);

    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);

    /* Clean up if the texture already exists. */
    if (sdl->ogl.font_tex != NULL) {
        SDL_DestroyTexture(sdl->ogl.font_tex);
        sdl->ogl.font_tex = NULL;
    }

    sdl->ogl.font_tex = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
    NK_ASSERT(sdl->ogl.font_tex);
    SDL_UpdateTexture(sdl->ogl.font_tex, NULL, image, 4 * width);
    SDL_SetTextureBlendMode(sdl->ogl.font_tex, SDL_BLENDMODE_BLEND);
}

NK_API void
nk_sdl_update_TextInput(struct nk_context* ctx)
{
    struct nk_sdl* sdl;
    bool active;
    NK_ASSERT(ctx);
    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);

    /* Determine if Nuklear is using any top-level "edit" widget.
     * Popups take higher priority because they block any incomming input.
     * This will not work, if the widget is not updating context state properly. */
    if (!ctx->active)
        active = false;
    else if (ctx->active->popup.win)
        active = ctx->active->popup.win->edit.active;
    else
        active = ctx->active->edit.active;

    /* decide, if TextInputActive should be unchanged/stoped/started
     * and change its state accordingly for owned SDL Window */
    if (active != sdl->edit_was_active)
    {
        const bool window_edit_active = SDL_TextInputActive(sdl->win);

        /* If you ever hit this check, it means that the demo and your app
         * (or something else) are all trying to manage TextInputActive state.
         * This can cause subtle bugs where the state won't be what you expect.
         * You can safely remove this assert and the demo will keep working,
         * but make sure it does not cause any issues for you */
        NK_ASSERT(window_edit_active == sdl->edit_was_active && "something else changed TextInputActive state for this Window");

        if (!window_edit_active && !sdl->edit_was_active && active)
            SDL_StartTextInput(sdl->win);
        else if (window_edit_active && sdl->edit_was_active && !active)
            SDL_StopTextInput(sdl->win);
        sdl->edit_was_active = active;
    }

    /* FIXME:
     * for full SDL3 integration, you also need to find current edit widget
     * bounds and the text cursor offset, and pass this data into SDL_SetTextInputArea.
     * This is currently not possible to do safely as Nuklear does not support it.
     * https://wiki.libsdl.org/SDL3/SDL_SetTextInputArea
     * https://github.com/Immediate-Mode-UI/Nuklear/pull/857
     */
}

NK_API void
nk_sdl_render(struct nk_context* ctx, enum nk_anti_aliasing AA)
{
    /* setup global state */
    struct nk_sdl* sdl;
    NK_ASSERT(ctx);
    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);

    { /* setup internal delta time that Nuklear needs for animations */
        const Uint64 ticks = SDL_GetTicks();
        ctx->delta_time_seconds = (float)(ticks - sdl->last_render) / 1000.0f;
        sdl->last_render = ticks;
    }

    {
        SDL_Rect saved_clip;
        bool clipping_enabled;
        int vs = sizeof(struct nk_sdl_vertex);
        size_t vp = NK_OFFSETOF(struct nk_sdl_vertex, position);
        size_t vt = NK_OFFSETOF(struct nk_sdl_vertex, uv);
        size_t vc = NK_OFFSETOF(struct nk_sdl_vertex, col);

        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        const nk_draw_index *offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* fill converting configuration */
        struct nk_convert_config config;
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            {NK_VERTEX_POSITION,    NK_FORMAT_FLOAT,                NK_OFFSETOF(struct nk_sdl_vertex, position)},
            {NK_VERTEX_TEXCOORD,    NK_FORMAT_FLOAT,                NK_OFFSETOF(struct nk_sdl_vertex, uv)},
            {NK_VERTEX_COLOR,       NK_FORMAT_R32G32B32A32_FLOAT,   NK_OFFSETOF(struct nk_sdl_vertex, col)},
            {NK_VERTEX_LAYOUT_END}
        };
        NK_MEMSET(&config, 0, sizeof(config));
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(struct nk_sdl_vertex);
        config.vertex_alignment = NK_ALIGNOF(struct nk_sdl_vertex);
        config.tex_null = sdl->ogl.tex_null;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.global_alpha = 1.0f;
        config.shape_AA = AA;
        config.line_AA = AA;

        /* convert shapes into vertexes */
        nk_buffer_init(&vbuf, &sdl->allocator, NK_BUFFER_DEFAULT_INITIAL_SIZE);
        nk_buffer_init(&ebuf, &sdl->allocator, NK_BUFFER_DEFAULT_INITIAL_SIZE);
        nk_convert(&sdl->ctx, &sdl->ogl.cmds, &vbuf, &ebuf, &config);

        /* iterate over and execute each draw command */
        offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);

        clipping_enabled = SDL_RenderClipEnabled(sdl->renderer);
        SDL_GetRenderClipRect(sdl->renderer, &saved_clip);

        nk_draw_foreach(cmd, &sdl->ctx, &sdl->ogl.cmds)
        {
            if (!cmd->elem_count) continue;

            {
                SDL_Rect r;
                r.x = cmd->clip_rect.x;
                r.y = cmd->clip_rect.y;
                r.w = cmd->clip_rect.w;
                r.h = cmd->clip_rect.h;
                SDL_SetRenderClipRect(sdl->renderer, &r);
            }

            {
                const void *vertices = nk_buffer_memory_const(&vbuf);

                SDL_RenderGeometryRaw(
                        sdl->renderer,
                        (SDL_Texture *)cmd->texture.ptr,
                        (const float*)((const nk_byte*)vertices + vp), vs,
                        (const SDL_FColor*)((const nk_byte*)vertices + vc), vs,
                        (const float*)((const nk_byte*)vertices + vt), vs,
                        (vbuf.needed / vs),
                        (void *) offset, cmd->elem_count, 2);

                offset += cmd->elem_count;
            }
        }

        SDL_SetRenderClipRect(sdl->renderer, &saved_clip);
        if (!clipping_enabled) {
            SDL_SetRenderClipRect(sdl->renderer, NULL);
        }

        nk_clear(&sdl->ctx);
        nk_buffer_clear(&sdl->ogl.cmds);
        nk_buffer_free(&vbuf);
        nk_buffer_free(&ebuf);
    }
}

NK_INTERN void
nk_sdl_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    char *text;
    int len;
    NK_UNUSED(usr);

    /* this function returns empty string on failure, not NULL */
    text = SDL_GetClipboardText();
    NK_ASSERT(text);

    if (text[0] != '\0') {
        /* FIXME: there is a bug in Nuklear that affects UTF8 clipboard handling
         * "len" should be a buffer length, but due to bug it must be a glyph count
         * see: https://github.com/Immediate-Mode-UI/Nuklear/pull/841 */
#if 0
        len = nk_strlen(text);
#else
        len = SDL_utf8strlen(text);
#endif
        nk_textedit_paste(edit, text, len);
    }
    SDL_free(text);
}

NK_INTERN void
nk_sdl_clipboard_copy(nk_handle usr, const char *text, int len)
{
    const char *ptext;
    char *str;
    size_t buflen;
    int i;
    struct nk_sdl* sdl = (struct nk_sdl*)usr.ptr;
    NK_ASSERT(sdl);
    if (len <= 0 || text == NULL) return;

    /* FIXME: there is a bug in Nuklear that affects UTF8 clipboard handling
     * "len" is expected to be a buffer length, but due to bug it actually is a glyph count
     * see: https://github.com/Immediate-Mode-UI/Nuklear/pull/841 */
#if 0
    buflen = len + 1;
    NK_UNUSED(ptext);
#else
    ptext = text;
    for (i = len; i > 0; i--)
        (void)SDL_StepUTF8(&ptext, NULL);
    buflen = (size_t)(ptext - text) + 1;
#endif

    str = sdl->allocator.alloc(sdl->allocator.userdata, 0, buflen);
    if (!str) return;
    SDL_strlcpy(str, text, buflen);
    SDL_SetClipboardText(str);
    sdl->allocator.free(sdl->allocator.userdata, str);
}

NK_API struct nk_context*
nk_sdl_init(SDL_Window *win, SDL_Renderer *renderer, struct nk_allocator allocator)
{
    struct nk_sdl* sdl;
    NK_ASSERT(win);
    NK_ASSERT(renderer);
    NK_ASSERT(allocator.alloc);
    NK_ASSERT(allocator.free);
    sdl = allocator.alloc(allocator.userdata, 0, sizeof(*sdl));
    NK_ASSERT(sdl);
    SDL_zerop(sdl);
    sdl->allocator.userdata = allocator.userdata;
    sdl->allocator.alloc = allocator.alloc;
    sdl->allocator.free = allocator.free;
    sdl->win = win;
    sdl->renderer = renderer;
    nk_init(&sdl->ctx, &sdl->allocator, 0);
    sdl->ctx.userdata = nk_handle_ptr((void*)sdl);
    sdl->ctx.clip.copy = nk_sdl_clipboard_copy;
    sdl->ctx.clip.paste = nk_sdl_clipboard_paste;
    sdl->ctx.clip.userdata = nk_handle_ptr((void*)sdl);
    nk_buffer_init(&sdl->ogl.cmds, &sdl->allocator, NK_BUFFER_DEFAULT_INITIAL_SIZE);
    sdl->last_left_click = 0;
    sdl->edit_was_active = false;
    sdl->insert_toggle = false;
    return &sdl->ctx;
}

#ifdef NK_INCLUDE_FONT_BAKING
NK_API struct nk_font_atlas*
nk_sdl_font_stash_begin(struct nk_context* ctx)
{
    struct nk_sdl* sdl;
    NK_ASSERT(ctx);
    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);
    nk_font_atlas_init(&sdl->atlas, &sdl->allocator);
    nk_font_atlas_begin(&sdl->atlas);
    return &sdl->atlas;
}
#endif

#ifdef NK_INCLUDE_FONT_BAKING
NK_API void
nk_sdl_font_stash_end(struct nk_context* ctx)
{
    struct nk_sdl* sdl;
    const void *image; int w, h;
    NK_ASSERT(ctx);
    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);
    image = nk_font_atlas_bake(&sdl->atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    NK_ASSERT(image);
    nk_sdl_device_upload_atlas(&sdl->ctx, image, w, h);
    nk_font_atlas_end(&sdl->atlas, nk_handle_ptr(sdl->ogl.font_tex), &sdl->ogl.tex_null);
    if (sdl->atlas.default_font) {
        nk_style_set_font(&sdl->ctx, &sdl->atlas.default_font->handle);
    }
}
#endif

NK_API int
nk_sdl_handle_event(struct nk_context* ctx, SDL_Event *evt)
{
    struct nk_sdl* sdl;

    NK_ASSERT(ctx);
    NK_ASSERT(evt);

    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);

    /* We only care about Window currently used by Nuklear */
    if (sdl->win != SDL_GetWindowFromEvent(evt)) {
        return 0;
    }

    switch(evt->type)
    {
        case SDL_EVENT_KEY_UP: /* KEYUP & KEYDOWN share same routine */
        case SDL_EVENT_KEY_DOWN:
            {
                int down = evt->type == SDL_EVENT_KEY_DOWN;
                int ctrl_down = evt->key.mod & (SDL_KMOD_LCTRL | SDL_KMOD_RCTRL);

                /* In 99% of the time, you want to use scancodes, not real key codes,
                 * see: https://wiki.libsdl.org/SDL3/BestKeyboardPractices */
                switch(evt->key.scancode)
                {
                    case SDL_SCANCODE_RSHIFT: /* RSHIFT & LSHIFT share same routine */
                    case SDL_SCANCODE_LSHIFT:    nk_input_key(ctx, NK_KEY_SHIFT, down); break;
                    case SDL_SCANCODE_DELETE:    nk_input_key(ctx, NK_KEY_DEL, down); break;
                    case SDL_SCANCODE_RETURN:    nk_input_key(ctx, NK_KEY_ENTER, down); break;
                    case SDL_SCANCODE_TAB:       nk_input_key(ctx, NK_KEY_TAB, down); break;
                    case SDL_SCANCODE_BACKSPACE: nk_input_key(ctx, NK_KEY_BACKSPACE, down); break;
                    case SDL_SCANCODE_HOME:      nk_input_key(ctx, NK_KEY_TEXT_START, down);
                                                 nk_input_key(ctx, NK_KEY_SCROLL_START, down); break;
                    case SDL_SCANCODE_END:       nk_input_key(ctx, NK_KEY_TEXT_END, down);
                                                 nk_input_key(ctx, NK_KEY_SCROLL_END, down); break;
                    case SDL_SCANCODE_PAGEDOWN:  nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down); break;
                    case SDL_SCANCODE_PAGEUP:    nk_input_key(ctx, NK_KEY_SCROLL_UP, down); break;
                    case SDL_SCANCODE_A:         nk_input_key(ctx, NK_KEY_TEXT_SELECT_ALL, down && ctrl_down); break;
                    case SDL_SCANCODE_Z:         nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && ctrl_down); break;
                    case SDL_SCANCODE_R:         nk_input_key(ctx, NK_KEY_TEXT_REDO, down && ctrl_down); break;
                    case SDL_SCANCODE_C:         nk_input_key(ctx, NK_KEY_COPY, down && ctrl_down); break;
                    case SDL_SCANCODE_V:         nk_input_key(ctx, NK_KEY_PASTE, down && ctrl_down); break;
                    case SDL_SCANCODE_X:         nk_input_key(ctx, NK_KEY_CUT, down && ctrl_down); break;
                    case SDL_SCANCODE_B:         nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && ctrl_down); break;
                    case SDL_SCANCODE_E:         nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && ctrl_down); break;
                    case SDL_SCANCODE_UP:        nk_input_key(ctx, NK_KEY_UP, down); break;
                    case SDL_SCANCODE_DOWN:      nk_input_key(ctx, NK_KEY_DOWN, down); break;
                    case SDL_SCANCODE_ESCAPE:    nk_input_key(ctx, NK_KEY_TEXT_RESET_MODE, down); break;
                    case SDL_SCANCODE_INSERT:
                        if (down) sdl->insert_toggle = !sdl->insert_toggle;
                        if (sdl->insert_toggle) {
                            nk_input_key(ctx, NK_KEY_TEXT_INSERT_MODE, down);
                        } else {
                            nk_input_key(ctx, NK_KEY_TEXT_REPLACE_MODE, down);
                        }
                        break;
                    case SDL_SCANCODE_LEFT:
                        if (ctrl_down)
                            nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
                        else
                            nk_input_key(ctx, NK_KEY_LEFT, down);
                        break;
                    case SDL_SCANCODE_RIGHT:
                        if (ctrl_down)
                            nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
                        else
                            nk_input_key(ctx, NK_KEY_RIGHT, down);
                        break;
                    default:
                        return 0;
                }
                return 1;
            }

        case SDL_EVENT_MOUSE_BUTTON_UP: /* MOUSEBUTTONUP & MOUSEBUTTONDOWN share same routine */
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                const int x = evt->button.x, y = evt->button.y;
                const int down = evt->button.down;
                const double dt = (double)(evt->button.timestamp - sdl->last_left_click) / 1000000000.0;
                switch(evt->button.button)
                {
                    case SDL_BUTTON_LEFT:
                        nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
                        nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y,
                                down && dt > NK_SDL_DOUBLE_CLICK_LO && dt < NK_SDL_DOUBLE_CLICK_HI);
                        sdl->last_left_click = evt->button.timestamp;
                        break;
                    case SDL_BUTTON_MIDDLE: nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down); break;
                    case SDL_BUTTON_RIGHT:  nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down); break;
                    default:
                        return 0;
                }
            }
            return 1;

        case SDL_EVENT_MOUSE_MOTION:
            ctx->input.mouse.pos.x = evt->motion.x;
            ctx->input.mouse.pos.y = evt->motion.y;
            ctx->input.mouse.delta.x = ctx->input.mouse.pos.x - ctx->input.mouse.prev.x;
            ctx->input.mouse.delta.y = ctx->input.mouse.pos.y - ctx->input.mouse.prev.y;
            return 1;

        case SDL_EVENT_TEXT_INPUT:
            {
                nk_glyph glyph;
                nk_size len;
                NK_ASSERT(evt->text.text);
                len = SDL_strlen(evt->text.text);
                NK_ASSERT(len <= NK_UTF_SIZE);
                NK_MEMCPY(glyph, evt->text.text, len);
                nk_input_glyph(ctx, glyph);
            }
            return 1;

        case SDL_EVENT_MOUSE_WHEEL:
            nk_input_scroll(ctx, nk_vec2(evt->wheel.x, evt->wheel.y));
            return 1;
    }
    return 0;
}

NK_API
void nk_sdl_shutdown(struct nk_context* ctx)
{
    struct nk_sdl* sdl;
    NK_ASSERT(ctx);
    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);

#ifdef NK_INCLUDE_FONT_BAKING
    if (sdl->atlas.font_num > 0)
        nk_font_atlas_clear(&sdl->atlas);
#endif

    nk_buffer_free(&sdl->ogl.cmds);

    if (sdl->ogl.font_tex != NULL) {
        SDL_DestroyTexture(sdl->ogl.font_tex);
        sdl->ogl.font_tex = NULL;
    }

    nk_free(ctx);
    sdl->allocator.free(sdl->allocator.userdata, sdl->debug_font);
    sdl->allocator.free(sdl->allocator.userdata, sdl);
}

/* Debug Font Width/Height of internal texture atlas
 * This is a result of: ceil(sqrt('~' - ' '))
 * There is a sanity check for this value in nk_sdl_style_set_debug_font */
#define NK_SDL_DFWH (10)

NK_INTERN float
nk_sdl_query_debug_font_width(nk_handle handle, float height,
                              const char *text, int len)
{
    NK_UNUSED(handle);
    return nk_utf_len(text, len) * height;
}

NK_INTERN void
nk_sdl_query_debug_font_glypth(nk_handle handle, float height,
                               struct nk_user_font_glyph *glyph,
                               nk_rune codepoint, nk_rune next_codepoint)
{
    char ascii;
    int idx, x, y;
    NK_UNUSED(next_codepoint);
    NK_UNUSED(handle);

    /* replace non-ASCII characters with question mark */
    ascii = (codepoint < (nk_rune)' ' || codepoint > (nk_rune)'~')
            ? '?' : (char)codepoint;
    NK_ASSERT(ascii >= ' ' && ascii <= '~');

    idx = (int)(ascii - ' ');
    x = idx / NK_SDL_DFWH;
    y = idx % NK_SDL_DFWH;
    NK_ASSERT(x >= 0 && x < NK_SDL_DFWH);
    NK_ASSERT(y >= 0 && y < NK_SDL_DFWH);

    glyph->height = height;
    glyph->width = height;
    glyph->xadvance = height;
    glyph->uv[0].x = (float)(x + 0) / NK_SDL_DFWH;
    glyph->uv[0].y = (float)(y + 0) / NK_SDL_DFWH;
    glyph->uv[1].x = (float)(x + 1) / NK_SDL_DFWH;
    glyph->uv[1].y = (float)(y + 1) / NK_SDL_DFWH;
    glyph->offset.x = 0.0f;
    glyph->offset.y = 0.0f;
}

NK_API void
nk_sdl_style_set_debug_font(struct nk_context* ctx)
{
    struct nk_user_font* font;
    struct nk_sdl* sdl;
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    char buf[2];
    int x, y;
    bool success;
    NK_ASSERT(ctx);

    sdl = (struct nk_sdl*)ctx->userdata.ptr;
    NK_ASSERT(sdl);

    if (sdl->debug_font) {
        sdl->allocator.free(sdl->allocator.userdata, sdl->debug_font);
        sdl->debug_font = 0;
    }

    /* sanity check: formal proof of NK_SDL_DFWH value (which is 10) */
    NK_ASSERT(SDL_ceil(SDL_sqrt('~' - ' ')) == NK_SDL_DFWH);

    /* We use another Software Renderer just to make sure
     * that we won't mutate any state in the main Renderer. */
    surface = SDL_CreateSurface(
            NK_SDL_DFWH * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE,
            NK_SDL_DFWH * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE,
            SDL_PIXELFORMAT_RGBA32);
    NK_ASSERT(surface);
    renderer = SDL_CreateSoftwareRenderer(surface);
    NK_ASSERT(renderer);
    success = SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    NK_ASSERT(success);

    /* SPACE is the first printable ASCII character */
    NK_MEMCPY(buf, " ", sizeof(buf));
    for (x = 0; x < NK_SDL_DFWH; x++)
    {
        for (y = 0; y < NK_SDL_DFWH; y++)
        {
            success = SDL_RenderDebugText(
                    renderer,
                    (float)(x * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE),
                    (float)(y * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE),
                    buf);
            NK_ASSERT(success);
            buf[0]++;

            /* TILDE is the last printable ASCII character */
            if (buf[0] > '~')
                break;
        }
    }
    success = SDL_RenderPresent(renderer);
    NK_ASSERT(success);

    font = sdl->allocator.alloc(sdl->allocator.userdata, 0, sizeof(*font));
    NK_ASSERT(font);
    font->userdata.ptr = sdl;
    font->height = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
    font->width = &nk_sdl_query_debug_font_width;
    font->query = &nk_sdl_query_debug_font_glypth;

    /* HACK: nk_sdl_device_upload_atlas turns pixels into SDL_Texture
     *       and sets said Texture into sdl->ogl.font_tex
     *       then nk_sdl_render expects same Texture at font->texture */
    nk_sdl_device_upload_atlas(ctx, surface->pixels, surface->w, surface->h);
    font->texture.ptr = sdl->ogl.font_tex;

    sdl->debug_font = font;
    nk_style_set_font(ctx, font);

    SDL_DestroyRenderer(renderer);
    SDL_DestroySurface(surface);
}

#endif /* NK_SDL3_RENDERER_IMPLEMENTATION_ONCE */
#endif /* NK_SDL3_RENDERER_IMPLEMENTATION */

