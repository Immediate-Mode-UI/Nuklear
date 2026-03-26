/* nuklear - v1.05 - public domain */
struct nk_canvas {
    struct nk_command_buffer *painter;
    struct nk_vec2 item_spacing;
    struct nk_vec2 panel_padding;
    struct nk_style_item window_background;
};

static nk_bool
canvas_begin(struct nk_context *ctx, struct nk_canvas *canvas, nk_flags flags,
    int x, int y, int width, int height, struct nk_color background_color)
{
    /* save style properties which will be overwritten */
    canvas->panel_padding = ctx->style.window.padding;
    canvas->item_spacing = ctx->style.window.spacing;
    canvas->window_background = ctx->style.window.fixed_background;

    /* use the complete window space and set background */
    ctx->style.window.spacing = nk_vec2(0,0);
    ctx->style.window.padding = nk_vec2(0,0);
    ctx->style.window.fixed_background = nk_style_item_color(background_color);

    /* create/update window and set position + size */
    if (!nk_begin(ctx, "Canvas", nk_rect(x, y, width, height), NK_WINDOW_NO_SCROLLBAR|flags))
        return nk_false;

    /* allocate the complete window space for drawing */
    {
        struct nk_rect total_space;
        total_space = nk_window_get_content_region(ctx);
        nk_layout_row_dynamic(ctx, total_space.h, 1);
        nk_widget(&total_space, ctx);
        canvas->painter = nk_window_get_canvas(ctx);
    }

    return nk_true;
}

static void
canvas_end(struct nk_context *ctx, struct nk_canvas *canvas)
{
    nk_end(ctx);
    ctx->style.window.spacing = canvas->panel_padding;
    ctx->style.window.padding = canvas->item_spacing;
    ctx->style.window.fixed_background = canvas->window_background;
}

static void
canvas(struct nk_context *ctx)
{
        struct nk_canvas canvas;
        if (canvas_begin(ctx, &canvas, NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE, 10, 10, 500, 550, nk_rgb(250,250,250)))
        {
            float x = canvas.painter->clip.x, y = canvas.painter->clip.y;

            nk_fill_rect(canvas.painter, nk_rect(x + 15, y + 15, 210, 210), 5, nk_rgb(247, 230, 154));
            nk_fill_rect(canvas.painter, nk_rect(x + 20, y + 20, 200, 200), 5, nk_rgb(188, 174, 118));
            /* nk_draw_text(canvas.painter, nk_rect(x + 30, y + 30, 150, 20), "Text to draw", 12, &font->handle, nk_rgb(188,174,118), nk_rgb(0,0,0)); */
            nk_fill_rect(canvas.painter, nk_rect(x + 250, y + 20, 100, 100), 0, nk_rgb(0,0,255));
            nk_fill_circle(canvas.painter, nk_rect(x + 20, y + 250, 100, 100), nk_rgb(255,0,0));
            nk_fill_triangle(canvas.painter, x + 250, y + 250, x + 350, y + 250, x + 300, y + 350, nk_rgb(0,255,0));
            nk_fill_arc(canvas.painter, x + 300, y + 420, 50, 0, 3.141592654f * 3.0f / 4.0f, nk_rgb(255,255,0));

            {
                float points[12];
                points[0]  = x + 200; points[1]  = y + 250;
                points[2]  = x + 250; points[3]  = y + 350;
                points[4]  = x + 225; points[5]  = y + 350;
                points[6]  = x + 200; points[7]  = y + 300;
                points[8]  = x + 175; points[9]  = y + 350;
                points[10] = x + 150; points[11] = y + 350;
                nk_fill_polygon(canvas.painter, points, 6, nk_rgb(0,0,0));
            }

            {
                float points[12];
                points[0]  = x + 200; points[1]  = y + 370;
                points[2]  = x + 250; points[3]  = y + 470;
                points[4]  = x + 225; points[5]  = y + 470;
                points[6]  = x + 200; points[7]  = y + 420;
                points[8]  = x + 175; points[9]  = y + 470;
                points[10] = x + 150; points[11] = y + 470;
                nk_stroke_polygon(canvas.painter, points, 6, 4, nk_rgb(0,0,0));
            }

            {
                float points[8];
                points[0]  = x + 250; points[1]  = y + 200;
                points[2]  = x + 275; points[3]  = y + 220;
                points[4]  = x + 325; points[5]  = y + 170;
                points[6]  = x + 350; points[7]  = y + 200;
                nk_stroke_polyline(canvas.painter, points, 4, 2, nk_rgb(255,128,0));
            }

            nk_stroke_line(canvas.painter, x + 15, y + 10, x + 200, y + 10, 2.0f, nk_rgb(189,45,75));
            nk_stroke_rect(canvas.painter, nk_rect(x + 370, y + 20, 100, 100), 10, 3, nk_rgb(0,0,255));
            nk_stroke_curve(canvas.painter, x + 380, y + 200, x + 405, y + 270, x + 455, y + 120, x + 480, y + 200, 2, nk_rgb(0,150,220));
            nk_stroke_circle(canvas.painter, nk_rect(x + 20, y + 370, 100, 100), 5, nk_rgb(0,255,120));
            nk_stroke_triangle(canvas.painter, x + 370, y + 250, x + 470, y + 250, x + 420, y + 350, 6, nk_rgb(255,0,143));
            nk_stroke_arc(canvas.painter, x + 420, y + 420, 50, 0, 3.141592654f * 3.0f / 4.0f, 5, nk_rgb(0,255,255));
        }
        canvas_end(ctx, &canvas);
}

