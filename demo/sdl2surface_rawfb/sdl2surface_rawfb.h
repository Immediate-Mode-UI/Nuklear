/*
 * MIT License
 *
 * Copyright (c) 2021 BieHDC
 *
 * Based on:
 * Copyright (c) 2016-2017 Patrick Rudolph <siro@das-labor.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
 /* Adapted from nulear_rawfb.h for use with SDL_Surface by Martijn Versteegh*/
#ifndef NK_SDLSURFACE_H_
#define NK_SDLSURFACE_H_
/* The Preprocessor is unable to evaluate on SDL_PIXELFORMAT_X directly */
#define NK_SDL_USE_RGBX (1)
#define NK_SDL_USE_XRGB (2)

#include <SDL.h>
#include <SDL_surface.h>

/* Exported Functions */
NK_API struct sdlsurface_context*   nk_sdlsurface_init(int w, int h);
NK_API void                         nk_sdlsurface_render(const struct sdlsurface_context *sdlsurface, const struct nk_color clear, const unsigned char enable_clear);
NK_API void                         nk_sdlsurface_update(struct sdlsurface_context* sdlsurface, int w, int h);
NK_API void                         nk_sdlsurface_shutdown(struct sdlsurface_context *sdlsurface);
NK_API int                          nk_sdl_handle_event(struct nk_context *ctx, SDL_Event *evt);
NK_API SDL_Surface*                 nk_sdlsurface_convert_surface(SDL_Surface* src); /* Use this to convert any SDL_Surface in the target format for blitting */
NK_API void                         nk_sdl_font_stash_begin(struct sdlsurface_context* sdlsurface);
NK_API void                         nk_sdl_font_stash_end(struct sdlsurface_context* sdlsurface);

#endif
/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_SDLSURFACE_IMPLEMENTATION
/* You dont have to change that unless you have a reason to */
/* TODO: Figure out which one is preferred on Windows */
//#define NK_SDL_USE_FORMAT NK_SDL_USE_RGBX
#define NK_SDL_USE_FORMAT NK_SDL_USE_XRGB /* This one is faster on Linux */

#if NK_SDL_USE_FORMAT==NK_SDL_USE_RGBX
    #define NK_SDL_PIXELFORMAT SDL_PIXELFORMAT_RGBX8888
    #define NK_SDL_PIXELFORMAT_A SDL_PIXELFORMAT_RGBA8888
#elif NK_SDL_USE_FORMAT==NK_SDL_USE_XRGB
    #define NK_SDL_PIXELFORMAT SDL_PIXELFORMAT_XRGB8888
    #define NK_SDL_PIXELFORMAT_A SDL_PIXELFORMAT_ARGB8888
#else
    #error Unsupported Pixelformat
#endif

struct sdlsurface_context {
    struct nk_context ctx;
    struct nk_rect scissors;
    struct SDL_Surface* fb;
    struct SDL_Surface* font_tex;
    struct nk_font_atlas atlas;
};


#if NK_SDL_USE_FORMAT==NK_SDL_USE_RGBX
    #define NK_SDLSURFACE_SETPIXEL(i, col)     {\
        i = 0;                                  \
        i |= col.r << 24;                       \
        i |= col.g << 16;                       \
        i |= col.b << 8;                        \
        i |= col.a;                             }
#elif NK_SDL_USE_FORMAT==NK_SDL_USE_XRGB
    #define NK_SDLSURFACE_SETPIXEL(i, col)     {\
        i = 0;                                  \
        i |= col.a << 24;                       \
        i |= col.r << 16;                       \
        i |= col.g << 8;                        \
        i |= col.b;                             }
#else
    #error NK_SDLSURFACE_SETPIXEL: Unsupported pixel layout.
#endif


#if NK_SDL_USE_FORMAT==NK_SDL_USE_RGBX
    #define NK_SDLSURFACE_GETPIXEL(col, i)     {\
        col.r = (i >> 24) & 0xff;               \
        col.g = (i >> 16) & 0xff;               \
        col.b = (i >> 8) & 0xff;                \
        col.a = i & 0xff;                       }
#elif NK_SDL_USE_FORMAT==NK_SDL_USE_XRGB
    #define NK_SDLSURFACE_GETPIXEL(col, i)     {\
        col.a = (i >> 24) & 0xff;               \
        col.r = (i >> 16) & 0xff;               \
        col.g = (i >> 8) & 0xff;                \
        col.b = i & 0xff;                       }
#else
    #error NK_SDLSURFACE_GETPIXEL: Unsupported pixel layout.
#endif

static void
nk_sdlsurface_scissor(struct sdlsurface_context *sdlsurface, const short x, const short y, const unsigned short w, const unsigned short h)
{
    sdlsurface->scissors.x = NK_CLAMP(0,  x,      sdlsurface->fb->w);
    sdlsurface->scissors.y = NK_CLAMP(0,  y,      sdlsurface->fb->h);
    sdlsurface->scissors.w = NK_CLAMP(0, (x + w), sdlsurface->fb->w);
    sdlsurface->scissors.h = NK_CLAMP(0, (y + h), sdlsurface->fb->h);
}

/* The caller has to check the bounds */
static void
nk_sdlsurface_img_blendpixel_unsafe(const struct sdlsurface_context* sdlsurface,
    const int x0, const int y0, struct nk_color col)
{
    unsigned int tmp;
    struct nk_color col2 = {0};
    unsigned char inv_a;

    if (col.a == 0)
        return;
    
    inv_a = 0xff - col.a;

    /* No need to blend if we dont have transparency */
    if (inv_a == 0) {
        NK_SDLSURFACE_SETPIXEL(tmp, col);
        *(((unsigned int *)(sdlsurface->fb->pixels + (sdlsurface->fb->pitch * y0))) + x0) = tmp; 
    } else {
        tmp = *(((unsigned int *)(sdlsurface->fb->pixels + (sdlsurface->fb->pitch * y0))) + x0);
        NK_SDLSURFACE_GETPIXEL(col2, tmp);

        col.r = (col.r * col.a + col2.r * inv_a) >> 8;
        col.g = (col.g * col.a + col2.g * inv_a) >> 8;
        col.b = (col.b * col.a + col2.b * inv_a) >> 8;

        NK_SDLSURFACE_SETPIXEL(tmp, col);
        *(((unsigned int *)(sdlsurface->fb->pixels + (sdlsurface->fb->pitch * y0))) + x0) = tmp; 
    }
}

/* The caller has to check the bounds */
static void
nk_sdlsurface_rect_fast(const struct sdlsurface_context* sdlsurface,
    const short x, const short y, const short w, const short h, const struct nk_color col)
{
    unsigned int pixel;
    struct nk_color col1 = col;
    unsigned char inv_a;

    /* Its transparent, lets return */
    if (col.a == 0)
        return;

    inv_a = 0xff - col.a;

    #define PXBUFSIZE 16 /* 8 seems slightly faster, but 16 more stable */
    unsigned int xi, n;
    struct nk_color pixel_in[PXBUFSIZE];
    unsigned int pixel_out[PXBUFSIZE];
    unsigned char *pixels;
    unsigned int *ptr;

    /* No need to blend if we dont have transparency */
    if (inv_a == 0) {
        NK_SDLSURFACE_SETPIXEL(pixel, col);
        for (xi = 0; xi < sizeof(pixel_out) / sizeof(pixel_out[0]); xi++)
            pixel_out[xi] = pixel;

        for (short yi = y; yi < h; yi++) {
            pixels = sdlsurface->fb->pixels + sdlsurface->fb->pitch * yi;
            ptr = (unsigned int *)pixels + x;

            n = w - x;
            while (n > PXBUFSIZE) {
                memcpy((void *)ptr, pixel_out, sizeof(pixel_out));
                n -= PXBUFSIZE; ptr += PXBUFSIZE;
            }
            for (xi = 0; xi < n; xi++)
                ptr[xi] = pixel_out[xi];
        }

    } else {
        for (short yi = y; yi < h; yi++) {
            pixels = sdlsurface->fb->pixels + sdlsurface->fb->pitch * yi;
            ptr = (unsigned int *)pixels + x;

            n = w - x;
            while (n > PXBUFSIZE) {
                for (xi = 0; xi < sizeof(pixel_in) / sizeof(pixel_in[0]); xi++) {
                    NK_SDLSURFACE_GETPIXEL(pixel_in[xi], ptr[xi]);
                }

                for (xi = 0; xi < sizeof(pixel_in) / sizeof(pixel_in[0]); xi++) {
                    col1.r = (col.r * col.a + pixel_in[xi].r * inv_a) >> 8;
                    col1.g = (col.g * col.a + pixel_in[xi].g * inv_a) >> 8;
                    col1.b = (col.b * col.a + pixel_in[xi].b * inv_a) >> 8;
                    NK_SDLSURFACE_SETPIXEL(pixel_out[xi], col1);
                }

                memcpy((void *)ptr, pixel_out, sizeof(pixel_out));
                n -= PXBUFSIZE;
                ptr += PXBUFSIZE;
            }

            /* Do the rest */
            for (xi = 0; xi < n; xi++) {
                NK_SDLSURFACE_GETPIXEL(pixel_in[xi], ptr[xi]);
            }

            for (xi = 0; xi < n; xi++) {
                col1.r = (col.r * col.a + pixel_in[xi].r * inv_a) >> 8;
                col1.g = (col.g * col.a + pixel_in[xi].g * inv_a) >> 8;
                col1.b = (col.b * col.a + pixel_in[xi].b * inv_a) >> 8;
                NK_SDLSURFACE_SETPIXEL(pixel_out[xi], col1);
            }

            for (xi = 0; xi < n; xi++) {
                ptr[xi] = pixel_out[xi];
            }
        }
    }
}

static void
nk_sdlsurface_fill_polygon(const struct sdlsurface_context* sdlsurface,
    const struct nk_vec2i *pnts, int count, const struct nk_color col)
{
    /* Maybe we should make MAX_POINTS dynamic and grow the buffer if needed, likely with a max cap, freeing on close */
    int i = 0;
    #define MAX_POINTS 128
    int left = 10000, top = 10000, bottom = 0, right = 0;
    int nodes, nodeX[MAX_POINTS], pixelX, pixelY, j, swap ;

    if (count == 0) return;
    if (count > MAX_POINTS)
        count = MAX_POINTS;

    /* Get polygon dimensions */
    for (i = 0; i < count; i++) {
        if (left > pnts[i].x) 
            left = pnts[i].x;
        if (right < pnts[i].x)
            right = pnts[i].x;
        if (top > pnts[i].y)
            top = pnts[i].y;
        if (bottom < pnts[i].y)
            bottom = pnts[i].y;
    } 
    bottom++; right++;

    /* Snip away out of bounds */
    left =   left   > sdlsurface->scissors.x ? left   : sdlsurface->scissors.x;
    right =  right  < sdlsurface->scissors.w ? right  : sdlsurface->scissors.w;
    top =    top    > sdlsurface->scissors.y ? top    : sdlsurface->scissors.y;
    bottom = bottom < sdlsurface->scissors.h ? bottom : sdlsurface->scissors.h;

    /* Polygon scanline algorithm released under public-domain by Darel Rex Finley, 2007 */
    /*  Loop through the rows of the image. */
    for (pixelY = top; pixelY < bottom; pixelY++) {
        nodes = 0; /*  Build a list of nodes. */
        j = count - 1;
        for (i = 0; i < count; i++) {
            if (((pnts[i].y < pixelY) && (pnts[j].y >= pixelY)) ||
                ((pnts[j].y < pixelY) && (pnts[i].y >= pixelY))) {
                nodeX[nodes++] = (int)((float)pnts[i].x
                     + ((float)pixelY - (float)pnts[i].y) / ((float)pnts[j].y - (float)pnts[i].y)
                     * ((float)pnts[j].x - (float)pnts[i].x));
            }
            j = i;
        }

        /*  Sort the nodes, via a simple “Bubble” sort. */
        i = 0;
        while (i < nodes - 1) {
            if (nodeX[i] > nodeX[i+1]) {
                swap = nodeX[i];
                nodeX[i] = nodeX[i+1];
                nodeX[i+1] = swap;
                if (i)
                    i--;
            }
            else
                i++;
        }
        /*  Fill the pixels between node pairs. */
        for (i = 0; i < nodes; i += 2) {
            if (nodeX[i+0] >= right)
                break;
            if (nodeX[i+1] > left) {
                if (nodeX[i+0] < left)
                    nodeX[i+0] = left;
                if (nodeX[i+1] > right)
                    nodeX[i+1] = right;

                nk_sdlsurface_rect_fast(sdlsurface, nodeX[i], pixelY, nodeX[i+1], pixelY+1, col);
            }
        }
    }
    #undef MAX_POINTS
}

static void
nk_sdlsurface_stroke_line(const struct sdlsurface_context *sdlsurface,
    const short x0, const short y0, const short x1, const short y1,
    const unsigned int line_thickness, const struct nk_color col)
{
    const float len = hypotf(x1 - x0, y1 - y0);
    const float tx = 0.5f * line_thickness *  (y1 - y0) / len;
    const float ty = 0.5f * line_thickness * -(x1 - x0) / len;

    if (len == 0)
        return;

    struct nk_vec2i pnts[4];
    pnts[0].x = x0 + tx;
    pnts[0].y = y0 + ty;
    pnts[1].x = x0 - tx;
    pnts[1].y = y0 - ty;
    pnts[2].x = x1 - tx;
    pnts[2].y = y1 - ty;
    pnts[3].x = x1 + tx;
    pnts[3].y = y1 + ty;

    nk_sdlsurface_fill_polygon(sdlsurface, pnts, 4, col);
}

static void
nk_sdlsurface_fill_arc(const struct sdlsurface_context *sdlsurface, short x0, short y0,
    short w, short h, const short s, const struct nk_color col)
{
    /* Bresenham's ellipses - modified to fill one quarter */
    const int a2 = (w * w) / 4;
    const int b2 = (h * h) / 4;
    const int fa2 = 4 * a2, fb2 = 4 * b2;
    int x, y, sigma;
    struct nk_vec2i pnts[3];
    if (w < 1 || h < 1)
        return;
    if (s != 0 && s != 90 && s != 180 && s != 270)
        return;

    /* Convert upper left to center */
    h = (h + 1) / 2;
    w = (w + 1) / 2;
    x0 += w;
    y0 += h;

    pnts[0].x = x0;
    pnts[0].y = y0;
    pnts[2].x = x0;
    pnts[2].y = y0;

    /* First half */
    for (x = 0, y = h, sigma = 2*b2+a2*(1-2*h); b2*x <= a2*y; x++) {
        if (s == 180) {
            pnts[1].x = x0 + x; pnts[1].y = y0 + y;
        } else if (s == 270) {
            pnts[1].x = x0 - x; pnts[1].y = y0 + y;
        } else if (s == 0) {
            pnts[1].x = x0 + x; pnts[1].y = y0 - y;
        } else if (s == 90) {
            pnts[1].x = x0 - x; pnts[1].y = y0 - y;
        }
        nk_sdlsurface_fill_polygon(sdlsurface, pnts, 3, col);
        pnts[2] = pnts[1];
        if (sigma >= 0) {
            sigma += fa2 * (1 - y);
            y--;
        }
        sigma += b2 * ((4 * x) + 6);
    }

    /* Second half */
    for (x = w, y = 0, sigma = 2*a2+b2*(1-2*w); a2*y <= b2*x; y++) {
        if (s == 180) {
            pnts[1].x = x0 + x; pnts[1].y = y0 + y;
        } else if (s == 270) {
            pnts[1].x = x0 - x; pnts[1].y = y0 + y;
        } else if (s == 0) {
            pnts[1].x = x0 + x; pnts[1].y = y0 - y;
        } else if (s == 90) {
            pnts[1].x = x0 - x; pnts[1].y = y0 - y;
        }
        nk_sdlsurface_fill_polygon(sdlsurface, pnts, 3, col);
        pnts[2] = pnts[1];
        if (sigma >= 0) {
            sigma += fb2 * (1 - x);
            x--;
        }
        sigma += a2 * ((4 * y) + 6);
    }
}

/* Thank you Allegro5 for kindly donating this function
 * Adapted to work on nk_vec2i
 */
void
nk_sdlsurface_calculate_arc(struct nk_vec2i* dest, const int stride,
    const float cx, const float cy, const float rx, const float ry,
    const float start_theta, const float delta_theta, const float thickness,
    const int num_points)
{
    float x, y, t;
    int ii;

    SDL_assert(dest);
    SDL_assert(num_points > 1);
    SDL_assert(rx >= 0);
    SDL_assert(ry >= 0);

    const float theta = delta_theta / ((float)(num_points) - 1);
    const float c = cosf(theta);
    const float s = sinf(theta);

    if (thickness > 0.0f) {
        x = cosf(start_theta);
        y = sinf(start_theta);

        if (rx == ry) {
            /*
            The circle case is particularly simple
            */
            float r1 = rx - thickness / 2.0f;
            float r2 = rx + thickness / 2.0f;
            for (ii = 0; ii < num_points; ii ++) {
                dest->x = r2 * x + cx;
                dest->y = r2 * y + cy;
                dest = (struct nk_vec2i*)(((char*)dest) + stride);
                dest->x = r1 * x + cx;
                dest->y = r1 * y + cy;
                dest = (struct nk_vec2i*)(((char*)dest) + stride);

                t = x;
                x = c * x - s * y;
                y = s * t + c * y;
            }
        } else {
            if (rx != 0 && ry != 0) {
                for (ii = 0; ii < num_points; ii++) {
                    float denom = hypotf(ry * x, rx * y);
                    float nx = thickness / 2 * ry * x / denom;
                    float ny = thickness / 2 * rx * y / denom;

                    dest->x = rx * x + cx + nx;
                    dest->y = ry * y + cy + ny;
                    dest = (struct nk_vec2i*)(((char*)dest) + stride);
                    dest->x = rx * x + cx - nx;
                    dest->y = ry * y + cy - ny;
                    dest = (struct nk_vec2i*)(((char*)dest) + stride);

                    t = x;
                    x = c * x - s * y;
                    y = s * t + c * y;
                }
            }
        }
    } else {
        x = cosf(start_theta);
        y = sinf(start_theta);

        for (ii = 0; ii < num_points; ii++) {
            dest->x = rx * x + cx;
            dest->y = ry * y + cy;
            dest = (struct nk_vec2i*)(((char*)dest) + stride);

            t = x;
            x = c * x - s * y;
            y = s * t + c * y;
        }
    }
}

void
nk_sdlsurface_draw_arc(const struct sdlsurface_context *sdlsurface, 
    const short cx, const short cy, const unsigned short rx, const unsigned short ry, 
    float start_theta, float delta_theta, const unsigned short line_thickness, const struct nk_color color)
{
    #define ARC_QUALITY 24 /* seems like an appropitate number, otherwise nk_sdlsurface_fill_polygon surrenders */
    unsigned long int num_segments;

    SDL_assert(rx >= 0 && ry >= 0);

    /* Just makes things a bit easier */
    if(delta_theta < 0) {
        delta_theta = -delta_theta;
        start_theta -= delta_theta;
    }

    if (line_thickness <= 0) {

        num_segments = fabs(delta_theta / (2 * NK_PI) * ARC_QUALITY * sqrtf((rx + ry) / 2.0f));

        if (num_segments < 3)
            num_segments = 3;

        struct nk_vec2i* points = malloc((num_segments)*sizeof(*points));

        if (!points) {
            SDL_assert(points == NULL && "Malloc for ARC failed. You can ignore this, but it wont be rendered");
            return;
        }

        nk_sdlsurface_calculate_arc((points+1), sizeof(*points), cx, cy, rx, ry, start_theta, delta_theta, 0, num_segments-1);
        points->x = cx;
        points->y = cy;

        nk_sdlsurface_fill_polygon(sdlsurface, points, num_segments, color);

        free(points);
    } else {
        float half_thickness = (float)line_thickness / 2.0f;
        float outer_radius_rx = rx + half_thickness;
        float outer_radius_ry = ry + half_thickness;
        float inner_radius_rx = rx - half_thickness;
        float inner_radius_ry = ry - half_thickness;

        /* If its too small */
        if (outer_radius_rx < 0 || outer_radius_ry < 0 || inner_radius_rx < 0 || inner_radius_ry < 0) {
            SDL_assert(0 != 0 && "Your thick line circle is too small");
            return;
        }

        /*
                2
                V
                ----...
                |       \
                |--..    \
               ^^^   \   |
                4    |   |
                 3-> ----- <- 1

            we get 1 .. 2 from the outer calc and
            we get 4 .. 3 from the inner calc,
            we do that in reverse to have 3 connect to 2 at the right spot,
            thats why we pass in negative sizeof as stride,
            basically reversing the algorithm and then            
            we feed it into draw polygon fill and we are done            
        */

        int num_segments_outer = fabs(delta_theta / (2 * NK_PI) * ARC_QUALITY * sqrtf((outer_radius_rx + outer_radius_ry) / 2.0f));
        int num_segments_inner = fabs(delta_theta / (2 * NK_PI) * ARC_QUALITY * sqrtf((inner_radius_rx + inner_radius_ry) / 2.0f));

        if (num_segments_outer < 3)
            num_segments_outer = 3;

        if (num_segments_inner < 3)
            num_segments_inner = 3;

        struct nk_vec2i* points = malloc((num_segments_outer+num_segments_inner)*sizeof(*points));

        if (!points) {
            SDL_assert(points == NULL && "Malloc for ARC failed. You can ignore this, but it wont be rendered");
            return;
        }

        nk_sdlsurface_calculate_arc(points,                                                 sizeof(*points), cx, cy, outer_radius_rx, outer_radius_ry, start_theta, delta_theta, 0, num_segments_outer);
        nk_sdlsurface_calculate_arc((points+num_segments_outer+num_segments_inner-1), -(int)sizeof(*points), cx, cy, inner_radius_rx, inner_radius_ry, start_theta, delta_theta, 0, num_segments_inner);

        nk_sdlsurface_fill_polygon(sdlsurface, points, num_segments_outer+num_segments_inner, color);
        /* // Debugpoints, leaving that one for porters
        for (int i = 0; i < num_segments_outer+num_segments_inner; i++)
            nk_sdlsurface_img_blendpixel(sdlsurface->fb, (points+i)->x, (points+i)->y, (struct nk_color){255,0,0,255});
        */
        free(points);
    }
    #undef ARC_QUALITY
}

static void
nk_sdlsurface_stroke_rect(const struct sdlsurface_context *sdlsurface,
    const short x, const short y, const short w, const short h,
    const short r, const short line_thickness, const struct nk_color col)
{
    if (r == 0) {
        const short x_s = NK_MAX(x, sdlsurface->scissors.x);
        const short y_s = NK_MAX(y, sdlsurface->scissors.y);

        const short xpw_s =   NK_CLAMP(x_s, (x + w),                  sdlsurface->scissors.w);
        const short xpwmt_s = NK_CLAMP(x_s, (x + w - line_thickness), sdlsurface->scissors.w);
        const short xpt_s =   NK_CLAMP(x_s, (x +     line_thickness), sdlsurface->scissors.w);

        const short yph_s =   NK_CLAMP(y_s, (y + h),                  sdlsurface->scissors.h);
        const short yphmt_s = NK_CLAMP(y_s, (y + h - line_thickness), sdlsurface->scissors.h);
        const short ypt_s =   NK_CLAMP(y_s, (y +     line_thickness), sdlsurface->scissors.h);

        nk_sdlsurface_rect_fast(sdlsurface, x_s,     y_s,     xpw_s, ypt_s,   col);
        nk_sdlsurface_rect_fast(sdlsurface, x_s,     yphmt_s, xpw_s, yph_s,   col);
        nk_sdlsurface_rect_fast(sdlsurface, x_s,     ypt_s,   xpt_s, yphmt_s, col);
        nk_sdlsurface_rect_fast(sdlsurface, xpwmt_s, ypt_s,   xpw_s, yphmt_s, col);
    } else {
        const short xc = x + r;
        const short yc = y + r;
        const short wc = (short)(w - 2 * r);
        const short hc = (short)(h - 2 * r);

        /* Using fast rect here yields bad results */
        nk_sdlsurface_stroke_line(sdlsurface, xc,    y,     xc + wc, y,       line_thickness, col);
        nk_sdlsurface_stroke_line(sdlsurface, x + w, yc,    x + w,   yc + hc, line_thickness, col);
        nk_sdlsurface_stroke_line(sdlsurface, xc,    y + h, xc + wc, y + h,   line_thickness, col);
        nk_sdlsurface_stroke_line(sdlsurface, x,     yc,    x,       yc + hc, line_thickness, col);
        
        nk_sdlsurface_draw_arc(sdlsurface, x + w - r, y + h - r,  r, r, 0,         NK_PI/2,   line_thickness, col);
        nk_sdlsurface_draw_arc(sdlsurface, x + r,     y + h - r,  r, r, NK_PI/2,   NK_PI/2,   line_thickness, col);
        nk_sdlsurface_draw_arc(sdlsurface, x + r,     y + r,      r, r, NK_PI-0.01,NK_PI/1.9, line_thickness, col); /* Blame it on floating point imprecicion */
        nk_sdlsurface_draw_arc(sdlsurface, x + w - r, y + r,      r, r, NK_PI*1.5, NK_PI/2,   line_thickness, col);
    }
}

static void
nk_sdlsurface_fill_rect(const struct sdlsurface_context *sdlsurface,
    const short x, const short y, const short w, const short h,
    const short r, const struct nk_color col)
{
    int i;
    if (r == 0) {
        nk_sdlsurface_rect_fast(sdlsurface, NK_CLAMP(sdlsurface->scissors.x,  x,      sdlsurface->scissors.w),       
                                            NK_CLAMP(sdlsurface->scissors.y,  y,      sdlsurface->scissors.h),       
                                            NK_CLAMP(sdlsurface->scissors.x, (x + w), sdlsurface->scissors.w),                  
                                            NK_CLAMP(sdlsurface->scissors.y, (y + h), sdlsurface->scissors.h), col);
    } else {
        const short x_s = NK_MAX(x, sdlsurface->scissors.x);
        const short y_s = NK_MAX(y, sdlsurface->scissors.y);

        const short xpw_s =   NK_CLAMP(x_s, (x + w        ), sdlsurface->scissors.w);
        const short xpr_s =   NK_CLAMP(x_s, (x     + r    ), sdlsurface->scissors.w);
        const short xpwmr_s = NK_CLAMP(x_s, (x + w - r    ), sdlsurface->scissors.w);

        const short yph_s =   NK_CLAMP(y_s, (y + h     + 1), sdlsurface->scissors.h); /* +1 to work around imprecicion */
        const short ypr_s =   NK_CLAMP(y_s, (y     + r    ), sdlsurface->scissors.h);
        const short yphmr_s = NK_CLAMP(y_s, (y + h - r + 1), sdlsurface->scissors.h); /* +1 to work around imprecicion */

        nk_sdlsurface_rect_fast(sdlsurface, xpr_s, y_s,     xpwmr_s, ypr_s,   col);
        nk_sdlsurface_rect_fast(sdlsurface, x_s,   ypr_s,   xpw_s,   yphmr_s, col);
        nk_sdlsurface_rect_fast(sdlsurface, xpr_s, yphmr_s, xpwmr_s, yph_s,   col);

        const short xc = x + r;
        const short yc = y + r;
        const short wc = (short)(w - 2 * r);
        const short hc = (short)(h - 2 * r);

        nk_sdlsurface_fill_arc(sdlsurface, xc + wc - r, y,           (unsigned)r*2, (unsigned)r*2, 0,   col);
        nk_sdlsurface_fill_arc(sdlsurface, x,           y,           (unsigned)r*2, (unsigned)r*2, 90,  col);
        nk_sdlsurface_fill_arc(sdlsurface, x,           yc + hc - r, (unsigned)r*2, (unsigned)r*2, 270, col);
        nk_sdlsurface_fill_arc(sdlsurface, xc + wc - r, yc + hc - r, (unsigned)r*2, (unsigned)r*2, 180, col);
    }
}

void
nk_sdlsurface_draw_rect_multi_color(const struct sdlsurface_context *sdlsurface,
    const short x, const short y, const short w, const short h, const struct nk_color tl,
    const struct nk_color tr, const struct nk_color br, const struct nk_color bl)
{
    int i, j;
    struct nk_color *edge_buf;
    struct nk_color *edge_t;
    struct nk_color *edge_b;
    struct nk_color *edge_l;
    struct nk_color *edge_r;
    struct nk_color pixel;

    edge_buf = malloc(((2*w) + (2*h)) * sizeof(struct nk_color));
    if (!edge_buf)
        return;

    edge_t = edge_buf;
    edge_b = edge_buf + w;
    edge_l = edge_buf + (w*2);
    edge_r = edge_buf + (w*2) + h;

    /* Top and bottom edge gradients */
    for (i = 0; i < w; i++)
    {
        edge_t[i].r = (((((float)tr.r - tl.r)/(w-1))*i) + 0.5) + tl.r;
        edge_t[i].g = (((((float)tr.g - tl.g)/(w-1))*i) + 0.5) + tl.g;
        edge_t[i].b = (((((float)tr.b - tl.b)/(w-1))*i) + 0.5) + tl.b;
        edge_t[i].a = (((((float)tr.a - tl.a)/(w-1))*i) + 0.5) + tl.a;

        edge_b[i].r = (((((float)br.r - bl.r)/(w-1))*i) + 0.5) + bl.r;
        edge_b[i].g = (((((float)br.g - bl.g)/(w-1))*i) + 0.5) + bl.g;
        edge_b[i].b = (((((float)br.b - bl.b)/(w-1))*i) + 0.5) + bl.b;
        edge_b[i].a = (((((float)br.a - bl.a)/(w-1))*i) + 0.5) + bl.a;
    }

    /* Left and right edge gradients */
    for (i = 0; i < h; i++)
    {
        edge_l[i].r = (((((float)bl.r - tl.r)/(h-1))*i) + 0.5) + tl.r;
        edge_l[i].g = (((((float)bl.g - tl.g)/(h-1))*i) + 0.5) + tl.g;
        edge_l[i].b = (((((float)bl.b - tl.b)/(h-1))*i) + 0.5) + tl.b;
        edge_l[i].a = (((((float)bl.a - tl.a)/(h-1))*i) + 0.5) + tl.a;

        edge_r[i].r = (((((float)br.r - tr.r)/(h-1))*i) + 0.5) + tr.r;
        edge_r[i].g = (((((float)br.g - tr.g)/(h-1))*i) + 0.5) + tr.g;
        edge_r[i].b = (((((float)br.b - tr.b)/(h-1))*i) + 0.5) + tr.b;
        edge_r[i].a = (((((float)br.a - tr.a)/(h-1))*i) + 0.5) + tr.a;
    }

    for (i = 0; i < h; i++) {
        if (y + i < sdlsurface->scissors.y)
            continue;
        if (y + i > sdlsurface->scissors.h)
            goto done;
        
        for (j = 0; j < w; j++) {
            if (x + j < sdlsurface->scissors.x)
                continue;
            if (x + j > sdlsurface->scissors.w)
                goto nextxx;
            if (i == 0)
            {
                nk_sdlsurface_img_blendpixel_unsafe(sdlsurface, x+j, y+i, edge_t[j]);
            }
            else if (i==h-1)
            {
                nk_sdlsurface_img_blendpixel_unsafe(sdlsurface, x+j, y+i, edge_b[j]);
            }
            else
            {
                if (j==0)
                {
                    nk_sdlsurface_img_blendpixel_unsafe(sdlsurface, x+j, y+i, edge_l[i]);
                }
                else if (j==w-1)
                {
                    nk_sdlsurface_img_blendpixel_unsafe(sdlsurface, x+j, y+i, edge_r[i]);
                }
                else
                {
                    pixel.r = (((((float)edge_r[i].r - edge_l[i].r)/(w-1))*j) + 0.5) + edge_l[i].r;
                    pixel.g = (((((float)edge_r[i].g - edge_l[i].g)/(w-1))*j) + 0.5) + edge_l[i].g;
                    pixel.b = (((((float)edge_r[i].b - edge_l[i].b)/(w-1))*j) + 0.5) + edge_l[i].b;
                    pixel.a = (((((float)edge_r[i].a - edge_l[i].a)/(w-1))*j) + 0.5) + edge_l[i].a;
                    nk_sdlsurface_img_blendpixel_unsafe(sdlsurface, x+j, y+i, pixel);
                }
            }
        }
        nextxx:;
    }
    done:
    free(edge_buf);
}

static void
nk_sdlsurface_fill_triangle(const struct sdlsurface_context *sdlsurface,
    const short x0, const short y0, const short x1, const short y1,
    const short x2, const short y2, const struct nk_color col)
{
    /* Fixme/Workaround, i added the -1 there because the scaler would end up one pixel too low */
    struct nk_vec2i pnts[3];
    pnts[0].x = x0;
    pnts[0].y = y0-1;
    pnts[1].x = x1;
    pnts[1].y = y1-1;
    pnts[2].x = x2;
    pnts[2].y = y2-1;
    nk_sdlsurface_fill_polygon(sdlsurface, pnts, 3, col);
}

static void
nk_sdlsurface_stroke_triangle(const struct sdlsurface_context *sdlsurface,
    const short x0, const short y0, const short x1, const short y1,
    const short x2, const short y2, const unsigned short line_thickness,
    const struct nk_color col)
{
    /* Suboptimal, but good enough for me right now! Feel free to fix the corners */
    nk_sdlsurface_stroke_line(sdlsurface, x0, y0, x1, y1, line_thickness, col);
    nk_sdlsurface_stroke_line(sdlsurface, x1, y1, x2, y2, line_thickness, col);
    nk_sdlsurface_stroke_line(sdlsurface, x2, y2, x0, y0, line_thickness, col);
}

static void
nk_sdlsurface_stroke_polygon(const struct sdlsurface_context *sdlsurface,
    const struct nk_vec2i *pnts, const int count,
    const unsigned short line_thickness, const struct nk_color col)
{
    int i;
    for (i = 1; i < count; ++i)
        nk_sdlsurface_stroke_line(sdlsurface, pnts[i-1].x, pnts[i-1].y, pnts[i].x,
                pnts[i].y, line_thickness, col);
    nk_sdlsurface_stroke_line(sdlsurface, pnts[count-1].x, pnts[count-1].y,
            pnts[0].x, pnts[0].y, line_thickness, col);
}

static void
nk_sdlsurface_stroke_polyline(const struct sdlsurface_context *sdlsurface,
    const struct nk_vec2i *pnts, const int count,
    const unsigned short line_thickness, const struct nk_color col)
{
    int i;
    for (i = 0; i < count-1; ++i)
        nk_sdlsurface_stroke_line(sdlsurface, pnts[i].x, pnts[i].y,
                 pnts[i+1].x, pnts[i+1].y, line_thickness, col);
}

static void
nk_sdlsurface_fill_circle(const struct sdlsurface_context *sdlsurface,
    short x0, short y0, short w, short h, const struct nk_color col)
{
    /* Bresenham's ellipses */
    const int a2 = (w * w) / 4;
    const int b2 = (h * h) / 4;
    const int fa2 = 4 * a2, fb2 = 4 * b2;
    int x, y, sigma;

    /* Convert upper left to center */
    h = (h + 1) / 2;
    w = (w + 1) / 2;
    x0 += w;
    y0 += h;

    /* First half */
    for (x = 0, y = h, sigma = 2*b2+a2*(1-2*h); b2*x <= a2*y; x++) {
        const short x0mx_s = NK_CLAMP(sdlsurface->scissors.x, (x0 - x), sdlsurface->scissors.w);
        const short x0px_s = NK_CLAMP(sdlsurface->scissors.x, (x0 + x), sdlsurface->scissors.w);
        const short y0py_s =   NK_CLAMP(sdlsurface->scissors.y, (y0 + y),     sdlsurface->scissors.h);
        const short y0my_s =   NK_CLAMP(sdlsurface->scissors.y, (y0 - y),     sdlsurface->scissors.h);
        const short y0pyp1_s = NK_CLAMP(sdlsurface->scissors.y, (y0 + y + 1), sdlsurface->scissors.h);
        const short y0myp1_s = NK_CLAMP(sdlsurface->scissors.y, (y0 - y + 1), sdlsurface->scissors.h);

        nk_sdlsurface_rect_fast(sdlsurface, x0mx_s, y0py_s, x0px_s, y0pyp1_s, col);
        nk_sdlsurface_rect_fast(sdlsurface, x0mx_s, y0my_s, x0px_s, y0myp1_s, col);
        if (sigma >= 0) {
            sigma += fa2 * (1 - y);
            y--;
        }
        sigma += b2 * ((4 * x) + 6);
    }
    /* Second half */
    for (x = w, y = 0, sigma = 2*a2+b2*(1-2*w); a2*y <= b2*x; y++) {
        const short x0mx_s = NK_CLAMP(sdlsurface->scissors.x, (x0 - x), sdlsurface->scissors.w);
        const short x0px_s = NK_CLAMP(sdlsurface->scissors.x, (x0 + x), sdlsurface->scissors.w);
        const short y0py_s =   NK_CLAMP(sdlsurface->scissors.y, (y0 + y),     sdlsurface->scissors.h);
        const short y0my_s =   NK_CLAMP(sdlsurface->scissors.y, (y0 - y),     sdlsurface->scissors.h);
        const short y0pyp1_s = NK_CLAMP(sdlsurface->scissors.y, (y0 + y + 1), sdlsurface->scissors.h);
        const short y0myp1_s = NK_CLAMP(sdlsurface->scissors.y, (y0 - y + 1), sdlsurface->scissors.h);

        nk_sdlsurface_rect_fast(sdlsurface, x0mx_s, y0py_s, x0px_s, y0pyp1_s, col);
        nk_sdlsurface_rect_fast(sdlsurface, x0mx_s, y0my_s, x0px_s, y0myp1_s, col);
        if (sigma >= 0) {
            sigma += fb2 * (1 - x);
            x--;
        }
        sigma += a2 * ((4 * y) + 6);
    }
}

static void
nk_sdlsurface_stroke_circle(const struct sdlsurface_context *sdlsurface,
    const short x0, const short y0, const short w, const short h, const short line_thickness,
    const struct nk_color col)
{
    const short xn = x0 + (w / 2);
    const short yn = y0 + (h / 2);
    const short wn = w / 2;
    const short hn = h / 2;

    nk_sdlsurface_draw_arc(sdlsurface, xn, yn, wn, hn, 0,         NK_PI/2,  line_thickness, col);
    nk_sdlsurface_draw_arc(sdlsurface, xn, yn, wn, hn, NK_PI/2,   NK_PI/2,  line_thickness, col);
    nk_sdlsurface_draw_arc(sdlsurface, xn, yn, wn, hn, NK_PI,     NK_PI/2,  line_thickness, col);
    nk_sdlsurface_draw_arc(sdlsurface, xn, yn, wn, hn, NK_PI*1.5, NK_PI/2,  line_thickness, col);
}

static void
nk_sdlsurface_stroke_curve(const struct sdlsurface_context *sdlsurface,
    const struct nk_vec2i p1, const struct nk_vec2i p2,
    const struct nk_vec2i p3, const struct nk_vec2i p4,
    const unsigned int num_segments, const unsigned short line_thickness,
    const struct nk_color col)
{
    unsigned int i_step;
    struct nk_vec2i last = p1;

    const unsigned int segments = NK_MAX(num_segments, 1);
    const float t_step = 1.0f/(float)segments;

    for (i_step = 1; i_step <= segments; ++i_step) {
        float t = t_step * (float)i_step;
        float u = 1.0f - t;
        float w1 = u*u*u;
        float w2 = 3*u*u*t;
        float w3 = 3*u*t*t;
        float w4 = t * t *t;
        float x = w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x;
        float y = w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y;

        nk_sdlsurface_stroke_line(sdlsurface, last.x, last.y,
                (short)x, (short)y, line_thickness, col);
        last.x = (short)x;
        last.y = (short)y;
    }
}

static void
nk_sdlsurface_clear(const struct sdlsurface_context *sdlsurface, const struct nk_color col)
{
    nk_sdlsurface_rect_fast(sdlsurface, 0, 0, sdlsurface->fb->w, sdlsurface->fb->h, col);
}

static void
nk_sdlsurface_stretch_image(const struct SDL_Surface *dst,
    const struct SDL_Surface *src, const struct nk_rect *dst_rect,
    const struct nk_rect *src_rect, const struct nk_rect *dst_scissors,
    const struct nk_color *fg)
{
    short i, j;
    struct nk_color col;
    float xinc = src_rect->w / dst_rect->w;
    float yinc = src_rect->h / dst_rect->h;
    float xoff = src_rect->x, yoff = src_rect->y;

    struct nk_color col2 = {0};
    unsigned char inv_a;

    if (!dst_scissors) {
        SDL_Log("This Function requires the dst_scissors argument, exiting");
        return;
    }

    /* Simple nearest filtering rescaling */
    /* TODO: use bilinear filter */

    for (j = 0; j < (short)dst_rect->h; j++) {
        if (j + (int)(dst_rect->y + 0.5f) < dst_scissors->y)
            continue;
        if (j + (int)(dst_rect->y + 0.5f) >= dst_scissors->h)
            goto done;

        //fixme optimisation room
        for (i = 0; i < (short)dst_rect->w; i++) {
            if (i + (int)(dst_rect->x + 0.5f) < dst_scissors->x)
                continue;
            if (i + (int)(dst_rect->x + 0.5f) >= dst_scissors->w)
                goto maxwidth;

            unsigned int tmp = *(((unsigned int *)(src->pixels + (src->pitch * (int)yoff))) + (int)xoff);
            NK_SDLSURFACE_GETPIXEL(col, tmp);

            if (col.a == 0) {
                xoff += xinc;
                continue;
            }
            
            tmp = *(((unsigned int *)(dst->pixels + (dst->pitch * (j + (int)(dst_rect->y + 0.5f))))) + (i + (int)(dst_rect->x + 0.5f)));
            NK_SDLSURFACE_GETPIXEL(col2, tmp);

            if (fg && (col.r || col.g || col.b))
            {
                col.r = fg->r;
                col.g = fg->g;
                col.b = fg->b;
            }
            
            inv_a = 0xff - col.a;
            col.r = (col.r * col.a + col2.r * inv_a) >> 8;
            col.g = (col.g * col.a + col2.g * inv_a) >> 8;
            col.b = (col.b * col.a + col2.b * inv_a) >> 8;

            NK_SDLSURFACE_SETPIXEL(tmp, col);
            *(((unsigned int *)(dst->pixels + (dst->pitch * (j + (int)(dst_rect->y + 0.5f))))) + (i + (int)(dst_rect->x + 0.5f))) = tmp; 

            xoff += xinc;
        }
        maxwidth:
        xoff = src_rect->x;
        yoff += yinc;
    }
    done:
    return;
}

static void
nk_sdlsurface_font_query_font_glyph(nk_handle handle, const float height,
    struct nk_user_font_glyph *glyph, const nk_rune codepoint,
    const nk_rune next_codepoint)
{
    float scale;
    const struct nk_font_glyph *g;
    struct nk_font *font;
    NK_ASSERT(glyph);
    NK_UNUSED(next_codepoint);

    font = (struct nk_font*)handle.ptr;
    NK_ASSERT(font);
    NK_ASSERT(font->glyphs);
    if (!font || !glyph)
        return;

    scale = height/font->info.height;
    g = nk_font_find_glyph(font, codepoint);
    glyph->width = (g->x1 - g->x0) * scale;
    glyph->height = (g->y1 - g->y0) * scale;
    glyph->offset = nk_vec2(g->x0 * scale, g->y0 * scale);
    glyph->xadvance = (g->xadvance * scale);
    glyph->uv[0] = nk_vec2(g->u0, g->v0);
    glyph->uv[1] = nk_vec2(g->u1, g->v1);
}

void
nk_sdlsurface_draw_text(const struct sdlsurface_context *sdlsurface,
    const struct nk_user_font *font, const struct nk_rect rect,
    const char *text, const int len, const float font_height,
    const struct nk_color fg)
{
    float x = 0;
    int text_len = 0;
    nk_rune unicode = 0;
    nk_rune next = 0;
    int glyph_len = 0;
    int next_glyph_len = 0;
    struct nk_user_font_glyph g;
    if (!len || !text)
        return;

    x = 0;
    glyph_len = nk_utf_decode(text, &unicode, len);
    if (!glyph_len) return;

    /* draw every glyph image */
    while (text_len < len && glyph_len) {
        struct nk_rect src_rect;
        struct nk_rect dst_rect;
        float char_width = 0;
        if (unicode == NK_UTF_INVALID)
            break;

        /* query currently drawn glyph information */
        next_glyph_len = nk_utf_decode(text + text_len + glyph_len, &next, (int)len - text_len);
        nk_sdlsurface_font_query_font_glyph(font->userdata, font_height, &g, unicode,
                    (next == NK_UTF_INVALID) ? '\0' : next);

        /* calculate and draw glyph drawing rectangle and image */
        char_width = g.xadvance;
        src_rect.x = g.uv[0].x * sdlsurface->font_tex->w;
        src_rect.y = g.uv[0].y * sdlsurface->font_tex->h;
        src_rect.w = g.uv[1].x * sdlsurface->font_tex->w - g.uv[0].x * sdlsurface->font_tex->w;
        src_rect.h = g.uv[1].y * sdlsurface->font_tex->h - g.uv[0].y * sdlsurface->font_tex->h;

        dst_rect.x = x + g.offset.x + rect.x;
        dst_rect.y = g.offset.y + rect.y;
        dst_rect.w = ceilf(g.width);
        dst_rect.h = ceilf(g.height);

        nk_sdlsurface_stretch_image(sdlsurface->fb, sdlsurface->font_tex, &dst_rect, &src_rect, &sdlsurface->scissors, &fg);

        /* offset next glyph */
        text_len += glyph_len;
        x += char_width;
        glyph_len = next_glyph_len;
        unicode = next;
    }
}

void
nk_sdlsurface_drawimage(const struct sdlsurface_context *sdlsurface,
    const int x, const int y, const int w, const int h,
    const struct nk_image *img, const struct nk_color *col)
{
    struct nk_rect src_rect;
    struct nk_rect dst_rect;

    src_rect.x = img->region[0];
    src_rect.y = img->region[1];
    src_rect.w = img->region[2];
    src_rect.h = img->region[3];

    dst_rect.x = x;
    dst_rect.y = y;
    dst_rect.w = w;
    dst_rect.h = h;

    if (img->handle.ptr) {
        /* If we have a texture */
        nk_sdlsurface_stretch_image(sdlsurface->fb, ((const SDL_Surface*)(img->handle.ptr)), &dst_rect, &src_rect, &sdlsurface->scissors, NULL);
    }
    else {
        /* If we have a font glyph */
        nk_sdlsurface_stretch_image(sdlsurface->fb, sdlsurface->font_tex, &dst_rect, &src_rect, &sdlsurface->scissors, col);
    }
}

NK_API SDL_Surface*
nk_sdlsurface_convert_surface(SDL_Surface* src) {
    return SDL_ConvertSurfaceFormat(src, NK_SDL_PIXELFORMAT_A, 0);
}

void
nk_sdlsurface_upload_atlas(struct sdlsurface_context* sdlsurface, const void* image, int w, int h) 
{
    sdlsurface->font_tex = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, sdlsurface->fb->format->format);

    if (!sdlsurface->font_tex) {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        SDL_assert(0);
    }

    memcpy(sdlsurface->font_tex->pixels, image, w * h * 4);

    if (sdlsurface->fb->format->format == SDL_PIXELFORMAT_RGBX8888)
    {
        SDL_assert(sdlsurface->font_tex->pitch == sdlsurface->font_tex->w * 4);
        uint32_t *fontPixels = (uint32_t *)sdlsurface->font_tex->pixels;
        for (int i = 0; i < sdlsurface->font_tex->w * sdlsurface->font_tex->h; i++)
        {
            uint32_t col = fontPixels[i];
            fontPixels[i] &= 0xFFFF00;
            fontPixels[i] |= ((col & 0xFF000000) >> 24);
            fontPixels[i] |= ((col & 0xFF) << 24);
        }
    }
}

NK_API void
nk_sdl_font_stash_begin(struct sdlsurface_context* sdlsurface)
{
    nk_font_atlas_init_default(&(sdlsurface->atlas));
    nk_font_atlas_begin(&(sdlsurface->atlas));
}

NK_API void
nk_sdl_font_stash_end(struct sdlsurface_context* sdlsurface)
{
    const void *image;
    int w, h;
    image = nk_font_atlas_bake(&(sdlsurface->atlas), &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_sdlsurface_upload_atlas(sdlsurface, image, w, h);
    nk_font_atlas_end(&(sdlsurface->atlas), nk_handle_ptr(sdlsurface->font_tex), NULL);
    if (sdlsurface->atlas.default_font)
        nk_style_set_font(&(sdlsurface->ctx), &(sdlsurface->atlas.default_font->handle));
}

static void
nk_sdlsurface_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = SDL_GetClipboardText();
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

static void
nk_sdlsurface_clipboard_copy(nk_handle usr, const char *text, int len)
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

NK_API struct sdlsurface_context*
nk_sdlsurface_init(const int w, const int h)
{
    struct sdlsurface_context *sdlsurface;

    sdlsurface = malloc(sizeof(struct sdlsurface_context));
    if (!sdlsurface)
        return NULL;

    NK_MEMSET(sdlsurface, 0, sizeof(struct sdlsurface_context));

    sdlsurface->fb = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, NK_SDL_PIXELFORMAT);
    if (!sdlsurface->fb)
    {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        SDL_assert(0);
    }

    SDL_SetSurfaceBlendMode(sdlsurface->fb, SDL_BLENDMODE_NONE);

    if (!nk_init_default(&(sdlsurface->ctx), 0)) {
        free(sdlsurface);
        return NULL;
    }

    nk_sdlsurface_scissor(sdlsurface, 0, 0, sdlsurface->fb->w, sdlsurface->fb->h);

    sdlsurface->ctx.clip.copy = nk_sdlsurface_clipboard_copy;
    sdlsurface->ctx.clip.paste = nk_sdlsurface_clipboard_paste;
    sdlsurface->ctx.clip.userdata = nk_handle_ptr(0);

    return sdlsurface;
}

NK_API void
nk_sdlsurface_update(struct sdlsurface_context* sdlsurface, const int w, const int h)
{
    SDL_Surface* oldsurface = sdlsurface->fb;
    sdlsurface->fb = NULL;

    sdlsurface->fb = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, NK_SDL_PIXELFORMAT);
    if (!sdlsurface->fb) {
        SDL_Log("Failed to create new surface");
        SDL_assert(0);
    }
    if (oldsurface)
        SDL_FreeSurface(oldsurface);

    SDL_SetSurfaceBlendMode(sdlsurface->fb, SDL_BLENDMODE_NONE);

    nk_sdlsurface_scissor(sdlsurface, 0, 0, sdlsurface->fb->w, sdlsurface->fb->h);
}

NK_API void
nk_sdlsurface_shutdown(struct sdlsurface_context *sdlsurface)
{
    if (sdlsurface) {
        nk_font_atlas_clear(&sdlsurface->atlas);
        SDL_FreeSurface(sdlsurface->fb);
        SDL_FreeSurface(sdlsurface->font_tex);
        nk_free(&sdlsurface->ctx);
        NK_MEMSET(sdlsurface, 0, sizeof(struct sdlsurface_context));
        free(sdlsurface);
    }
}


NK_API void
nk_sdlsurface_render(const struct sdlsurface_context *sdlsurface,
                const struct nk_color clear,
                const unsigned char enable_clear)
{
    const struct nk_command *cmd;
    if (enable_clear)
        nk_sdlsurface_clear(sdlsurface, clear);

    nk_foreach(cmd, (struct nk_context*)&sdlsurface->ctx) {
        switch (cmd->type) {
        case NK_COMMAND_NOP: break;
        case NK_COMMAND_SCISSOR: {
            const struct nk_command_scissor *s =(const struct nk_command_scissor*)cmd;
            nk_sdlsurface_scissor((struct sdlsurface_context *)sdlsurface, s->x, s->y, s->w, s->h);
        } break;
        case NK_COMMAND_LINE: {
            const struct nk_command_line *l = (const struct nk_command_line *)cmd;
            nk_sdlsurface_stroke_line(sdlsurface, l->begin.x, l->begin.y, l->end.x,
                l->end.y, l->line_thickness, l->color);
        } break;
        case NK_COMMAND_RECT: {
            const struct nk_command_rect *r = (const struct nk_command_rect *)cmd;
            nk_sdlsurface_stroke_rect(sdlsurface, r->x, r->y, r->w, r->h,
                (unsigned short)r->rounding, r->line_thickness, r->color);
        } break;
        case NK_COMMAND_RECT_FILLED: {
            const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled *)cmd;
            nk_sdlsurface_fill_rect(sdlsurface, r->x, r->y, r->w, r->h,
                (unsigned short)r->rounding, r->color);
        } break;
        case NK_COMMAND_CIRCLE: {
            const struct nk_command_circle *c = (const struct nk_command_circle *)cmd;
            nk_sdlsurface_stroke_circle(sdlsurface, c->x, c->y, c->w, c->h, c->line_thickness, c->color);
        } break;
        case NK_COMMAND_CIRCLE_FILLED: {
            const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;
            nk_sdlsurface_fill_circle(sdlsurface, c->x, c->y, c->w, c->h, c->color);
        } break;
        case NK_COMMAND_TRIANGLE: {
            const struct nk_command_triangle*t = (const struct nk_command_triangle*)cmd;
            nk_sdlsurface_stroke_triangle(sdlsurface, t->a.x, t->a.y, t->b.x, t->b.y,
                t->c.x, t->c.y, t->line_thickness, t->color);
        } break;
        case NK_COMMAND_TRIANGLE_FILLED: {
            const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled *)cmd;
            nk_sdlsurface_fill_triangle(sdlsurface, t->a.x, t->a.y, t->b.x, t->b.y,
                t->c.x, t->c.y, t->color);
        } break;
        case NK_COMMAND_POLYGON: {
            const struct nk_command_polygon *p =(const struct nk_command_polygon*)cmd;
            nk_sdlsurface_stroke_polygon(sdlsurface, p->points, p->point_count, p->line_thickness,p->color);
        } break;
        case NK_COMMAND_POLYGON_FILLED: {
            const struct nk_command_polygon_filled *p = (const struct nk_command_polygon_filled *)cmd;
            nk_sdlsurface_fill_polygon(sdlsurface, p->points, p->point_count, p->color);
        } break;
        case NK_COMMAND_POLYLINE: {
            const struct nk_command_polyline *p = (const struct nk_command_polyline *)cmd;
            nk_sdlsurface_stroke_polyline(sdlsurface, p->points, p->point_count, p->line_thickness, p->color);
        } break;
        case NK_COMMAND_TEXT: {
            const struct nk_command_text *t = (const struct nk_command_text*)cmd;
            nk_sdlsurface_draw_text(sdlsurface, t->font, nk_rect(t->x, t->y, t->w, t->h),
                t->string, t->length, t->height, t->foreground);
        } break;
        case NK_COMMAND_CURVE: {
            const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;
            nk_sdlsurface_stroke_curve(sdlsurface, q->begin, q->ctrl[0], q->ctrl[1],
                q->end, 22, q->line_thickness, q->color);
        } break;
        case NK_COMMAND_RECT_MULTI_COLOR: {
            const struct nk_command_rect_multi_color *q = (const struct nk_command_rect_multi_color *)cmd;
            nk_sdlsurface_draw_rect_multi_color(sdlsurface, q->x, q->y, q->w, q->h, q->left, q->top, q->right, q->bottom);
        } break;
        case NK_COMMAND_IMAGE: {
            const struct nk_command_image *q = (const struct nk_command_image *)cmd;
            nk_sdlsurface_drawimage(sdlsurface, q->x, q->y, q->w, q->h, &q->img, &q->col);
        } break;
        case NK_COMMAND_ARC: {
            const struct nk_command_arc *a = (const struct nk_command_arc *)cmd;
            nk_sdlsurface_draw_arc(sdlsurface, a->cx, a->cy, a->r, a->r, a->a[0],
                a->a[1], a->line_thickness, a->color);
        } break;
        case NK_COMMAND_ARC_FILLED: {
            /* If you are porting this, you are looking for "pieslice"/sector, not "segment" */
            const struct nk_command_arc_filled *a = (const struct nk_command_arc_filled *)cmd;
            nk_sdlsurface_draw_arc(sdlsurface, a->cx, a->cy, a->r, a->r, a->a[0],
                a->a[1], 0, a->color);
        } break;
        default: break;
        }
    }
    nk_clear((struct nk_context*)&sdlsurface->ctx);
}


NK_API int
nk_sdlsurface_handle_event(struct nk_context *ctx, SDL_Event *evt)
{
    /* optional grabbing behavior */
    /* broken, fix this at some point */
    /*
    if (ctx->input.mouse.grab) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        ctx->input.mouse.grab = 0;
    } else if (ctx->input.mouse.ungrab) {
        int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        SDL_WarpMouseGlobal(x, y);
        ctx->input.mouse.ungrab = 0;
    }
    */
    if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN) {
        /* key events */
        int down = evt->type == SDL_KEYDOWN;
        const Uint8* state = SDL_GetKeyboardState(0);
        SDL_Keycode sym = evt->key.keysym.sym;
        if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
            nk_input_key(ctx, NK_KEY_SHIFT, down);
        else if (sym == SDLK_DELETE)
            nk_input_key(ctx, NK_KEY_DEL, down);
        else if (sym == SDLK_RETURN)
            nk_input_key(ctx, NK_KEY_ENTER, down);
        else if (sym == SDLK_TAB)
            nk_input_key(ctx, NK_KEY_TAB, down);
        else if (sym == SDLK_BACKSPACE)
            nk_input_key(ctx, NK_KEY_BACKSPACE, down);
        else if (sym == SDLK_HOME) {
            nk_input_key(ctx, NK_KEY_TEXT_START, down);
            nk_input_key(ctx, NK_KEY_SCROLL_START, down);
        } else if (sym == SDLK_END) {
            nk_input_key(ctx, NK_KEY_TEXT_END, down);
            nk_input_key(ctx, NK_KEY_SCROLL_END, down);
        } else if (sym == SDLK_PAGEDOWN) {
            nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
        } else if (sym == SDLK_PAGEUP) {
            nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
        } else if (sym == SDLK_z)
            nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_r)
            nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_c)
            nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_v)
            nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_x)
            nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_b)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_e)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_UP)
            nk_input_key(ctx, NK_KEY_UP, down);
        else if (sym == SDLK_DOWN)
            nk_input_key(ctx, NK_KEY_DOWN, down);
        else if (sym == SDLK_LEFT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
            else nk_input_key(ctx, NK_KEY_LEFT, down);
        } else if (sym == SDLK_RIGHT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
            else nk_input_key(ctx, NK_KEY_RIGHT, down);
        } else return 0;
        return 1;
    } else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP) {
        /* mouse button */
        int down = evt->type == SDL_MOUSEBUTTONDOWN;
        const int x = evt->button.x, y = evt->button.y;
        if (evt->button.button == SDL_BUTTON_LEFT) {
            if (evt->button.clicks > 1)
                nk_input_button(ctx, NK_BUTTON_DOUBLE, x, y, down);
            nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
        } else if (evt->button.button == SDL_BUTTON_MIDDLE)
            nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
        else if (evt->button.button == SDL_BUTTON_RIGHT)
            nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
        return 1;
    } else if (evt->type == SDL_MOUSEMOTION) {
        /* mouse motion */
        if (ctx->input.mouse.grabbed) {
            int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
            nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
        } else nk_input_motion(ctx, evt->motion.x, evt->motion.y);
        return 1;
    } else if (evt->type == SDL_TEXTINPUT) {
        /* text input */
        nk_glyph glyph;
        memcpy(glyph, evt->text.text, NK_UTF_SIZE);
        nk_input_glyph(ctx, glyph);
        return 1;
    } else if (evt->type == SDL_MOUSEWHEEL) {
        /* mouse wheel */
        nk_input_scroll(ctx,nk_vec2((float)evt->wheel.x,(float)evt->wheel.y));
        return 1;
    }
    return 0;
}

#endif

