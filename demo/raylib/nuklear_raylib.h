/*
 * Nuklear - 1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 * 
 * Fork: https://github.com/tcfunk/raylib-nuklear
 * Actualizacion por Jhoson Ozuna(slam) - hbiblia@g
 */

#ifndef NUKLEAR_RAYLIB_H
#define NUKLEAR_RAYLIB_H
#include <math.h>
#include "raylib.h"

NK_API struct nk_context *nk_raylib_init(const char *file_name);
NK_API struct nk_user_font *nk_raylib_create_font(const char *file_name);
NK_API nk_raylib_handle_input(void);
NK_API nk_raylib_render(void);
#endif

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_RAYLIB_IMPLEMENTATION
static struct nk_context ctx;
static float prev_mouse_x;
static float prev_mouse_y;

static float
nk_raylib_font_get_text_width(nk_handle handle, float height, const char *text, int len)
{
    if (!text){ return 0; }

    char tmp_buffer[len + 1];
    strncpy((char *)&tmp_buffer, text, len);
    tmp_buffer[len] = '\0';

    return (float)MeasureText(tmp_buffer, (int)handle.ptr);
}

NK_API struct nk_user_font *nk_raylib_create_font(const char *file_name)
{
    SpriteFont sprite_font;
    sprite_font = (strcmp(file_name, "default") == 0 ? GetFontDefault() : LoadFont(file_name));

    int fontSize = sprite_font.baseSize;

    struct nk_user_font *font = calloc(1, sizeof(struct nk_user_font*));
    font->height = sprite_font.recs->height;
    font->width = nk_raylib_font_get_text_width;
    font->userdata = nk_handle_ptr(&fontSize);
    return font;
}

NK_API struct nk_context *
nk_raylib_init(const char *file_name)
{
    struct nk_user_font *font = nk_raylib_create_font(file_name);

    nk_init_default(&ctx, font);
    return &ctx;
}

static Color
nk_raylib_color_parse(struct nk_color color)
{
    return (Color){color.r, color.g, color.b, color.a};
}

NK_API nk_raylib_render(void)
{
    const struct nk_command *cmd;

    nk_foreach(cmd, &ctx)
    {
        Color color;
        switch (cmd->type)
        {
        case NK_COMMAND_NOP:
            break;
        case NK_COMMAND_SCISSOR:
        {
            const struct nk_command_scissor *s = (const struct nk_command_scissor *)cmd;
            BeginScissorMode((int)s->x, (int)s->y, (int)s->w, (int)s->h);
            // EndScissorMode();
        }
        break;

        case NK_COMMAND_LINE:
        {
            const struct nk_command_line *l = (const struct nk_command_line *)cmd;
            color = nk_raylib_color_parse(l->color);
            DrawLineEx((Vector2){l->begin.x, l->begin.y}, (Vector2){l->end.x, l->end.y}, l->line_thickness, color);
        }
        break;

        case NK_COMMAND_RECT:
        {
            const struct nk_command_rect *r = (const struct nk_command_rect *)cmd;
            color = nk_raylib_color_parse(r->color);
            float rounding = (r->rounding * 0.06f);
            DrawRectangleRoundedLines((Rectangle){r->x, r->y, r->w, r->h}, rounding, 4, r->line_thickness, color);
        }
        break;

        case NK_COMMAND_RECT_FILLED:
        {
            const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled *)cmd;
            color = nk_raylib_color_parse(r->color);
            float rounding = (r->rounding * 0.06f);
            DrawRectangleRounded((Rectangle){r->x, r->y, r->w, r->h}, rounding, 4, color);
        }
        break;

        case NK_COMMAND_CIRCLE:
        {
            const struct nk_command_circle *c = (const struct nk_command_circle *)cmd;
            color = nk_raylib_color_parse(c->color);
            float xr = (float)(c->w / 2);
            float yr = (float)(c->h / 2);
            float r = (xr + yr) / 2;
            DrawCircleLines((c->x + xr), (c->y + yr), r, color);
        }
        break;

        case NK_COMMAND_CIRCLE_FILLED:
        {
            const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;
            color = nk_raylib_color_parse(c->color);
            float xr = (float)(c->w / 2);
            float yr = (float)(c->h / 2);
            float r = (xr + yr) / 2;
            DrawCircle((c->x + xr), (c->y + yr), r, color);
        }
        break;

        case NK_COMMAND_TRIANGLE:
        {
            const struct nk_command_triangle *t = (const struct nk_command_triangle *)cmd;
            color = nk_raylib_color_parse(t->color);
            Vector2 a = {t->a.x, t->a.y};
            Vector2 b = {t->c.x, t->c.y};
            Vector2 c = {t->b.x, t->b.y};
            DrawTriangleLines(a, b, c, color);
        }
        break;

        case NK_COMMAND_TRIANGLE_FILLED:
        {
            const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled *)cmd;
            color = nk_raylib_color_parse(t->color);
            Vector2 a = {t->a.x, t->a.y};
            Vector2 b = {t->c.x, t->c.y};
            Vector2 c = {t->b.x, t->b.y};
            DrawTriangle(a, b, c, color);
        }
        break;

        case NK_COMMAND_POLYGON:
        {
            const struct nk_command_polygon *p = (const struct nk_command_polygon *)cmd;
            color = nk_raylib_color_parse(p->color);
            DrawLineStrip(p->points, p->point_count, color);
        }
        break;

        case NK_COMMAND_POLYGON_FILLED:
        {
            const struct nk_command_polygon_filled *p = (const struct nk_command_polygon_filled *)cmd;
            color = nk_raylib_color_parse(p->color);
            DrawLineStrip(p->points, p->point_count, color);
        }
        break;

        case NK_COMMAND_TEXT:
        {
            const struct nk_command_text *t = (const struct nk_command_text *)cmd;
            color = nk_raylib_color_parse(t->foreground);
            struct nk_user_font *font = (struct nk_user_font *)t->font;
            DrawText((const char *)t->string, t->x, t->y, font->userdata.ptr, color);
        }
        break;
        case NK_COMMAND_CURVE:
        {
            const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;
            color = nk_raylib_color_parse(q->color);
            DrawLineBezier((Vector2){(float)q->begin.x, (float)q->begin.y}, (Vector2){(float)q->end.x, (float)q->end.y}, (float)q->line_thickness, RED);
        }
        break;
        case NK_COMMAND_ARC:
        {
            const struct nk_command_arc *a = (const struct nk_command_arc *)cmd;
            color = nk_raylib_color_parse(a->color);
            DrawCircleSectorLines((Vector2){a->cx, a->cy}, (float)a->r, (int)a->a[0], (int)a->a[1], 22, color);
        }
        break;
        case NK_COMMAND_ARC_FILLED:
        {
            const struct nk_command_arc *a = (const struct nk_command_arc *)cmd;
            color = nk_raylib_color_parse(a->color);
            DrawCircleSector((Vector2){a->cx, a->cy}, (float)a->r, (int)a->a[0], (int)a->a[1], 22, color);
        }
        break;
        case NK_COMMAND_RECT_MULTI_COLOR:
        {
            const struct nk_command_rect_multi_color *a = (const struct nk_command_rect_multi_color *)cmd;
            Color c_d[4];
            c_d[0] = nk_raylib_color_parse(a->left);
            c_d[1] = nk_raylib_color_parse(a->top);
            c_d[2] = nk_raylib_color_parse(a->bottom);
            c_d[3] = nk_raylib_color_parse(a->right);
            DrawRectangleGradientEx((Rectangle){a->x, a->y, a->w, a->h}, c_d[0], c_d[2], c_d[3], c_d[4]);
        }
        break;
        case NK_COMMAND_IMAGE:
        {
            const struct nk_command_image *i = (const struct nk_command_image *)cmd;
            DrawTexture(*(Texture2D *)i->img.handle.ptr, i->x, i->y, nk_raylib_color_parse(i->col));
        }
        break;
        }
    }

    nk_clear(&ctx);
}

NK_API nk_raylib_handle_input(void)
{
    nk_input_begin(&ctx);
    {

        int button = -1;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            button = MOUSE_LEFT_BUTTON;
        }
        else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) || IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
        {
            button = MOUSE_RIGHT_BUTTON;
        }
        else if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON) || IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON))
        {
            button = MOUSE_MIDDLE_BUTTON;
        }

        if (button != -1)
        {
            int down = IsMouseButtonDown(button);
            if (button == MOUSE_RIGHT_BUTTON)
            {
                button = MOUSE_MIDDLE_BUTTON;
            }
            else if (button == MOUSE_MIDDLE_BUTTON)
            {
                button = MOUSE_RIGHT_BUTTON;
            }
            nk_input_button(&ctx, button, GetMouseX(), GetMouseY(), down);
        }

        float mouse_x = GetMouseX();
        float mouse_y = GetMouseY();

        if (mouse_x != prev_mouse_x || mouse_y != prev_mouse_y)
        {
            nk_input_motion(&ctx, mouse_x, mouse_y);
        }

        prev_mouse_x = mouse_x;
        prev_mouse_y = mouse_y;
    }
    nk_input_end(&ctx);
}
#endif
