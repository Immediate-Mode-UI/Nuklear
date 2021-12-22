#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                              TEXT
 *
 * ===============================================================*/
NK_LIB void
nk_widget_text(struct nk_command_buffer *o, struct nk_rect b,
    struct nk_slice string, const struct nk_text *t,
    nk_flags a, const struct nk_user_font *f)
{
    struct nk_rect label;
    float text_width;

    NK_ASSERT(o);
    NK_ASSERT(t);
    if (!o || !t) return;

    b.h = NK_MAX(b.h, 2 * t->padding.y);
    label.x = 0; label.w = 0;
    label.y = b.y + t->padding.y;
    label.h = NK_MIN(f->height, b.h - 2 * t->padding.y);

    text_width = f->width(f->userdata, f->height, string);
    text_width += (2.0f * t->padding.x);

    /* align in x-axis */
    if (a & NK_TEXT_ALIGN_LEFT) {
        label.x = b.x + t->padding.x;
        label.w = NK_MAX(0, b.w - 2 * t->padding.x);
    } else if (a & NK_TEXT_ALIGN_CENTERED) {
        label.w = NK_MAX(1, 2 * t->padding.x + (float)text_width);
        label.x = (b.x + t->padding.x + ((b.w - 2 * t->padding.x) - label.w) / 2);
        label.x = NK_MAX(b.x + t->padding.x, label.x);
        label.w = NK_MIN(b.x + b.w, label.x + label.w);
        if (label.w >= label.x) label.w -= label.x;
    } else if (a & NK_TEXT_ALIGN_RIGHT) {
        label.x = NK_MAX(b.x + t->padding.x, (b.x + b.w) - (2 * t->padding.x + (float)text_width));
        label.w = (float)text_width + 2 * t->padding.x;
    } else return;

    /* align in y-axis */
    if (a & NK_TEXT_ALIGN_MIDDLE) {
        label.y = b.y + b.h/2.0f - (float)f->height/2.0f;
        label.h = NK_MAX(b.h/2.0f, b.h - (b.h/2.0f + f->height/2.0f));
    } else if (a & NK_TEXT_ALIGN_BOTTOM) {
        label.y = b.y + b.h - f->height;
        label.h = f->height;
    }
    nk_draw_text(o, label, string, f, t->background, t->text);
}
NK_LIB void
nk_widget_text_wrap(struct nk_command_buffer *o, struct nk_rect b,
    struct nk_slice string, const struct nk_text *t,
    const struct nk_user_font *f)
{
    float width;
    int glyphs = 0;
    int fitting = 0;
    nk_size done = 0;
    struct nk_rect line;
    struct nk_text text;
    NK_INTERN nk_rune seperator[] = {' '};

    NK_ASSERT(o);
    NK_ASSERT(t);
    if (!o || !t) return;

    text.padding = nk_vec2(0,0);
    text.background = t->background;
    text.text = t->text;

    b.w = NK_MAX(b.w, 2 * t->padding.x);
    b.h = NK_MAX(b.h, 2 * t->padding.y);
    b.h = b.h - 2 * t->padding.y;

    line.x = b.x + t->padding.x;
    line.y = b.y + t->padding.y;
    line.w = b.w - 2 * t->padding.x;
    line.h = 2 * t->padding.y + f->height;

    fitting = nk_text_clamp(f, string, line.w, &glyphs, &width, seperator,NK_LEN(seperator));
    while (done < string.len) {
        if (!fitting || line.y + line.h >= (b.y + b.h)) break;
        nk_widget_text(o, line, nk_substr(string, done, done + fitting), &text, NK_TEXT_LEFT, f);
        done += fitting;
        line.y += f->height + 2 * t->padding.y;
        fitting = nk_text_clamp(f, nk_substr(string, done, string.len), line.w, &glyphs, &width, seperator,NK_LEN(seperator));
    }
}
NK_API void
nk_label_colored(struct nk_context *ctx, struct nk_slice str,
    nk_flags alignment, struct nk_color color)
{
    struct nk_window *win;
    const struct nk_style *style;

    struct nk_vec2 item_padding;
    struct nk_rect bounds;
    struct nk_text text;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    style = &ctx->style;
    nk_panel_alloc_space(&bounds, ctx);
    item_padding = style->text.padding;

    text.padding.x = item_padding.x;
    text.padding.y = item_padding.y;
    text.background = style->window.background;
    text.text = color;
    nk_widget_text(&win->buffer, bounds, str, &text, alignment, style->font);
}
NK_API void
nk_label_wrap_colored(struct nk_context *ctx, struct nk_slice str,
    struct nk_color color)
{
    struct nk_window *win;
    const struct nk_style *style;

    struct nk_vec2 item_padding;
    struct nk_rect bounds;
    struct nk_text text;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout) return;

    win = ctx->current;
    style = &ctx->style;
    nk_panel_alloc_space(&bounds, ctx);
    item_padding = style->text.padding;

    text.padding.x = item_padding.x;
    text.padding.y = item_padding.y;
    text.background = style->window.background;
    text.text = color;
    nk_widget_text_wrap(&win->buffer, bounds, str, &text, style->font);
}
#ifdef NK_INCLUDE_STANDARD_VARARGS
NK_API void
nk_labelf_colored(struct nk_context *ctx, nk_flags flags,
    struct nk_color color, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nk_labelfv_colored(ctx, flags, color, fmt, args);
    va_end(args);
}
NK_API void
nk_labelf_colored_wrap(struct nk_context *ctx, struct nk_color color,
    const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nk_labelfv_colored_wrap(ctx, color, fmt, args);
    va_end(args);
}
NK_API void
nk_labelf(struct nk_context *ctx, nk_flags flags, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    nk_labelfv(ctx, flags, fmt, args);
    va_end(args);
}
NK_API void
nk_labelf_wrap(struct nk_context *ctx, const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);
    nk_labelfv_wrap(ctx, fmt, args);
    va_end(args);
}
NK_API void
nk_labelfv_colored(struct nk_context *ctx, nk_flags flags,
    struct nk_color color, const char *fmt, va_list args)
{
    nk_size len;
    char buf[256];
    len = nk_strfmt(buf, NK_LEN(buf), fmt, args);
    nk_label_colored(ctx, nk_slice(buf, len), flags, color);
}

NK_API void
nk_labelfv_colored_wrap(struct nk_context *ctx, struct nk_color color,
    const char *fmt, va_list args)
{
    nk_size len;
    char buf[256];
    len = nk_strfmt(buf, NK_LEN(buf), fmt, args);
    nk_label_wrap_colored(ctx, nk_slice(buf, len), color);
}

NK_API void
nk_labelfv(struct nk_context *ctx, nk_flags flags, const char *fmt, va_list args)
{
    nk_size len;
    char buf[256];
    len = nk_strfmt(buf, NK_LEN(buf), fmt, args);
    nk_label(ctx, nk_slice(buf, len), flags);
}

NK_API void
nk_labelfv_wrap(struct nk_context *ctx, const char *fmt, va_list args)
{
    nk_size len;
    char buf[256];
    len = nk_strfmt(buf, NK_LEN(buf), fmt, args);
    nk_label_wrap(ctx, nk_slice(buf, len));
}

NK_API void
nk_value_bool(struct nk_context *ctx, struct nk_slice prefix, int value)
{
    nk_labelf(ctx, NK_TEXT_LEFT, "%.*s: %s", (int)prefix.len, prefix.ptr, ((value) ? "true": "false"));
}
NK_API void
nk_value_int(struct nk_context *ctx, struct nk_slice prefix, int value)
{
    nk_labelf(ctx, NK_TEXT_LEFT, "%.*s: %d", (int)prefix.len, prefix.ptr, value);
}
NK_API void
nk_value_uint(struct nk_context *ctx, struct nk_slice prefix, unsigned int value)
{
    nk_labelf(ctx, NK_TEXT_LEFT, "%.*s: %u", (int)prefix.len, prefix.ptr, value);
}
NK_API void
nk_value_float(struct nk_context *ctx, struct nk_slice prefix, float value)
{
    double double_value = (double)value;
    nk_labelf(ctx, NK_TEXT_LEFT, "%.*s: %.3f", (int)prefix.len, prefix.ptr, double_value);
}
NK_API void
nk_value_color_byte(struct nk_context *ctx, struct nk_slice p, struct nk_color c)
{
    nk_labelf(ctx, NK_TEXT_LEFT, "%.*s: (%d, %d, %d, %d)", (int)p.len, p.ptr, c.r, c.g, c.b, c.a);
}
NK_API void
nk_value_color_float(struct nk_context *ctx, struct nk_slice p, struct nk_color color)
{
    double c[4]; nk_color_dv(c, color);
    nk_labelf(ctx, NK_TEXT_LEFT, "%.*s: (%.2f, %.2f, %.2f, %.2f)",
        (int)p.len, p.ptr, c[0], c[1], c[2], c[3]);
}
NK_API void
nk_value_color_hex(struct nk_context *ctx, struct nk_slice prefix, struct nk_color color)
{
    char hex[16];
    nk_color_hex_rgba(hex, color);
    nk_labelf(ctx, NK_TEXT_LEFT, "%.*s: %s", (int)prefix.len, prefix.ptr, hex);
}
#endif
NK_API void
nk_label(struct nk_context *ctx, struct nk_slice str, nk_flags alignment)
{
    NK_ASSERT(ctx);
    if (!ctx) return;
    nk_label_colored(ctx, str, alignment, ctx->style.text.color);
}
NK_API void
nk_label_wrap(struct nk_context *ctx, struct nk_slice str)
{
    NK_ASSERT(ctx);
    if (!ctx) return;
    nk_label_wrap_colored(ctx, str, ctx->style.text.color);
}

