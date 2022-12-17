#include "nuklear.h"
#include "nuklear_internal.h"

#ifdef NK_INCLUDE_FONT_BAKING
/* -------------------------------------------------------------
 *
 *                          RECT PACK
 *
 * --------------------------------------------------------------*/

#include "stb_rect_pack.h"

/*
 * ==============================================================
 *
 *                          TRUETYPE
 *
 * ===============================================================
 */
#define STBTT_MAX_OVERSAMPLE   8
#include "stb_truetype.h"

/* -------------------------------------------------------------
 *
 *                          FONT BAKING
 *
 * --------------------------------------------------------------*/
struct nk_font_bake_data {
    struct stbtt_fontinfo info;
    struct stbrp_rect *rects;
    stbtt_pack_range *ranges;
    nk_rune range_count;
};

struct nk_font_baker {
    struct nk_allocator alloc;
    struct stbtt_pack_context spc;
    struct nk_font_bake_data *build;
    stbtt_packedchar *packed_chars;
    struct stbrp_rect *rects;
    stbtt_pack_range *ranges;
};

NK_GLOBAL const nk_size nk_rect_align = NK_ALIGNOF(struct stbrp_rect);
NK_GLOBAL const nk_size nk_range_align = NK_ALIGNOF(stbtt_pack_range);
NK_GLOBAL const nk_size nk_char_align = NK_ALIGNOF(stbtt_packedchar);
NK_GLOBAL const nk_size nk_build_align = NK_ALIGNOF(struct nk_font_bake_data);
NK_GLOBAL const nk_size nk_baker_align = NK_ALIGNOF(struct nk_font_baker);

NK_INTERN int
nk_range_count(const nk_rune *range)
{
    const nk_rune *iter = range;
    NK_ASSERT(range);
    if (!range) return 0;
    while (*(iter++) != 0);
    return (iter == range) ? 0 : (int)((iter - range)/2);
}
NK_INTERN int
nk_range_glyph_count(const nk_rune *range, int count)
{
    int i = 0;
    int total_glyphs = 0;
    for (i = 0; i < count; ++i) {
        int diff;
        nk_rune f = range[(i*2)+0];
        nk_rune t = range[(i*2)+1];
        NK_ASSERT(t >= f);
        diff = (int)((t - f) + 1);
        total_glyphs += diff;
    }
    return total_glyphs;
}
NK_API const nk_rune*
nk_font_default_glyph_ranges(void)
{
    NK_STORAGE const nk_rune ranges[] = {0x0020, 0x00FF, 0};
    return ranges;
}
NK_API const nk_rune*
nk_font_chinese_glyph_ranges(void)
{
    NK_STORAGE const nk_rune ranges[] = {
        0x0020, 0x00FF,
        0x3000, 0x30FF,
        0x31F0, 0x31FF,
        0xFF00, 0xFFEF,
        0x4E00, 0x9FAF,
        0
    };
    return ranges;
}
NK_API const nk_rune*
nk_font_cyrillic_glyph_ranges(void)
{
    NK_STORAGE const nk_rune ranges[] = {
        0x0020, 0x00FF,
        0x0400, 0x052F,
        0x2DE0, 0x2DFF,
        0xA640, 0xA69F,
        0
    };
    return ranges;
}
NK_API const nk_rune*
nk_font_korean_glyph_ranges(void)
{
    NK_STORAGE const nk_rune ranges[] = {
        0x0020, 0x00FF,
        0x3131, 0x3163,
        0xAC00, 0xD79D,
        0
    };
    return ranges;
}
NK_INTERN void
nk_font_baker_memory(nk_size *temp, int *glyph_count,
    struct nk_font_config *config_list, int count)
{
    int range_count = 0;
    int total_range_count = 0;
    struct nk_font_config *iter, *i;

    NK_ASSERT(config_list);
    NK_ASSERT(glyph_count);
    if (!config_list) {
        *temp = 0;
        *glyph_count = 0;
        return;
    }
    *glyph_count = 0;
    for (iter = config_list; iter; iter = iter->next) {
        i = iter;
        do {if (!i->range) iter->range = nk_font_default_glyph_ranges();
            range_count = nk_range_count(i->range);
            total_range_count += range_count;
            *glyph_count += nk_range_glyph_count(i->range, range_count);
        } while ((i = i->n) != iter);
    }
    *temp = (nk_size)*glyph_count * sizeof(struct stbrp_rect);
    *temp += (nk_size)total_range_count * sizeof(stbtt_pack_range);
    *temp += (nk_size)*glyph_count * sizeof(stbtt_packedchar);
    *temp += (nk_size)count * sizeof(struct nk_font_bake_data);
    *temp += sizeof(struct nk_font_baker);
    *temp += nk_rect_align + nk_range_align + nk_char_align;
    *temp += nk_build_align + nk_baker_align;
}
NK_INTERN struct nk_font_baker*
nk_font_baker(void *memory, int glyph_count, int count, struct nk_allocator *alloc)
{
    struct nk_font_baker *baker;
    if (!memory) return 0;
    /* setup baker inside a memory block  */
    baker = (struct nk_font_baker*)NK_ALIGN_PTR(memory, nk_baker_align);
    baker->build = (struct nk_font_bake_data*)NK_ALIGN_PTR((baker + 1), nk_build_align);
    baker->packed_chars = (stbtt_packedchar*)NK_ALIGN_PTR((baker->build + count), nk_char_align);
    baker->rects = (struct stbrp_rect*)NK_ALIGN_PTR((baker->packed_chars + glyph_count), nk_rect_align);
    baker->ranges = (stbtt_pack_range*)NK_ALIGN_PTR((baker->rects + glyph_count), nk_range_align);
    baker->alloc = *alloc;
    return baker;
}
NK_INTERN int
nk_font_bake_pack(struct nk_font_baker *baker,
    nk_size *image_memory, int *width, int *height, struct nk_recti *custom,
    const struct nk_font_config *config_list, int count,
    struct nk_allocator *alloc)
{
    NK_STORAGE const nk_size max_height = 1024 * 32;
    const struct nk_font_config *config_iter, *it;
    int total_glyph_count = 0;
    int total_range_count = 0;
    int range_count = 0;
    int i = 0;

    NK_ASSERT(image_memory);
    NK_ASSERT(width);
    NK_ASSERT(height);
    NK_ASSERT(config_list);
    NK_ASSERT(count);
    NK_ASSERT(alloc);

    if (!image_memory || !width || !height || !config_list || !count) return nk_false;
    for (config_iter = config_list; config_iter; config_iter = config_iter->next) {
        it = config_iter;
        do {range_count = nk_range_count(it->range);
            total_range_count += range_count;
            total_glyph_count += nk_range_glyph_count(it->range, range_count);
        } while ((it = it->n) != config_iter);
    }
    /* setup font baker from temporary memory */
    for (config_iter = config_list; config_iter; config_iter = config_iter->next) {
        it = config_iter;
        do {
            struct stbtt_fontinfo *font_info = &baker->build[i++].info;
            font_info->userdata = alloc;

            if (!stbtt_InitFont(font_info, (const unsigned char*)it->ttf_blob, stbtt_GetFontOffsetForIndex((const unsigned char*)it->ttf_blob, 0)))
                return nk_false;
        } while ((it = it->n) != config_iter);
    }
    *height = 0;
    *width = (total_glyph_count > 1000) ? 1024 : 512;
    stbtt_PackBegin(&baker->spc, 0, (int)*width, (int)max_height, 0, 1, alloc);
    {
        int input_i = 0;
        int range_n = 0;
        int rect_n = 0;
        int char_n = 0;

        if (custom) {
            /* pack custom user data first so it will be in the upper left corner*/
            struct stbrp_rect custom_space;
            nk_zero(&custom_space, sizeof(custom_space));
            custom_space.w = (stbrp_coord)(custom->w);
            custom_space.h = (stbrp_coord)(custom->h);

            stbtt_PackSetOversampling(&baker->spc, 1, 1);
            stbrp_pack_rects((struct stbrp_context*)baker->spc.pack_info, &custom_space, 1);
            *height = NK_MAX(*height, (int)(custom_space.y + custom_space.h));

            custom->x = (short)custom_space.x;
            custom->y = (short)custom_space.y;
            custom->w = (short)custom_space.w;
            custom->h = (short)custom_space.h;
        }

        /* first font pass: pack all glyphs */
        for (input_i = 0, config_iter = config_list; input_i < count && config_iter;
            config_iter = config_iter->next) {
            it = config_iter;
            do {int n = 0;
                int glyph_count;
                const nk_rune *in_range;
                const struct nk_font_config *cfg = it;
                struct nk_font_bake_data *tmp = &baker->build[input_i++];

                /* count glyphs + ranges in current font */
                glyph_count = 0; range_count = 0;
                for (in_range = cfg->range; in_range[0] && in_range[1]; in_range += 2) {
                    glyph_count += (int)(in_range[1] - in_range[0]) + 1;
                    range_count++;
                }

                /* setup ranges  */
                tmp->ranges = baker->ranges + range_n;
                tmp->range_count = (nk_rune)range_count;
                range_n += range_count;
                for (i = 0; i < range_count; ++i) {
                    in_range = &cfg->range[i * 2];
                    tmp->ranges[i].font_size = cfg->size;
                    tmp->ranges[i].first_unicode_codepoint_in_range = (int)in_range[0];
                    tmp->ranges[i].num_chars = (int)(in_range[1]- in_range[0]) + 1;
                    tmp->ranges[i].chardata_for_range = baker->packed_chars + char_n;
                    char_n += tmp->ranges[i].num_chars;
                }

                /* pack */
                tmp->rects = baker->rects + rect_n;
                rect_n += glyph_count;
                stbtt_PackSetOversampling(&baker->spc, cfg->oversample_h, cfg->oversample_v);
                n = stbtt_PackFontRangesGatherRects(&baker->spc, &tmp->info,
                    tmp->ranges, (int)tmp->range_count, tmp->rects);
                stbrp_pack_rects((struct stbrp_context*)baker->spc.pack_info, tmp->rects, (int)n);

                /* texture height */
                for (i = 0; i < n; ++i) {
                    if (tmp->rects[i].was_packed)
                        *height = NK_MAX(*height, tmp->rects[i].y + tmp->rects[i].h);
                }
            } while ((it = it->n) != config_iter);
        }
        NK_ASSERT(rect_n == total_glyph_count);
        NK_ASSERT(char_n == total_glyph_count);
        NK_ASSERT(range_n == total_range_count);
    }
    *height = (int)nk_round_up_pow2((nk_uint)*height);
    *image_memory = (nk_size)(*width) * (nk_size)(*height);
    return nk_true;
}
NK_INTERN void
nk_font_bake(struct nk_font_baker *baker, void *image_memory, int width, int height,
    struct nk_font_glyph *glyphs, int glyphs_count,
    const struct nk_font_config *config_list, int font_count)
{
    int input_i = 0;
    nk_rune glyph_n = 0;
    const struct nk_font_config *config_iter;
    const struct nk_font_config *it;

    NK_ASSERT(image_memory);
    NK_ASSERT(width);
    NK_ASSERT(height);
    NK_ASSERT(config_list);
    NK_ASSERT(baker);
    NK_ASSERT(font_count);
    NK_ASSERT(glyphs_count);
    if (!image_memory || !width || !height || !config_list ||
        !font_count || !glyphs || !glyphs_count)
        return;

    /* second font pass: render glyphs */
    nk_zero(image_memory, (nk_size)((nk_size)width * (nk_size)height));
    baker->spc.pixels = (unsigned char*)image_memory;
    baker->spc.height = (int)height;
    for (input_i = 0, config_iter = config_list; input_i < font_count && config_iter;
        config_iter = config_iter->next) {
        it = config_iter;
        do {const struct nk_font_config *cfg = it;
            struct nk_font_bake_data *tmp = &baker->build[input_i++];
            stbtt_PackSetOversampling(&baker->spc, cfg->oversample_h, cfg->oversample_v);
            stbtt_PackFontRangesRenderIntoRects(&baker->spc, &tmp->info, tmp->ranges, (int)tmp->range_count, tmp->rects);
        } while ((it = it->n) != config_iter);
    } stbtt_PackEnd(&baker->spc);

    /* third pass: setup font and glyphs */
    for (input_i = 0, config_iter = config_list; input_i < font_count && config_iter;
        config_iter = config_iter->next) {
        it = config_iter;
        do {nk_size i = 0;
            int char_idx = 0;
            nk_rune glyph_count = 0;
            const struct nk_font_config *cfg = it;
            struct nk_font_bake_data *tmp = &baker->build[input_i++];
            struct nk_baked_font *dst_font = cfg->font;

            float font_scale = stbtt_ScaleForPixelHeight(&tmp->info, cfg->size);
            int unscaled_ascent, unscaled_descent, unscaled_line_gap;
            stbtt_GetFontVMetrics(&tmp->info, &unscaled_ascent, &unscaled_descent,
                                    &unscaled_line_gap);

            /* fill baked font */
            if (!cfg->merge_mode) {
                dst_font->ranges = cfg->range;
                dst_font->height = cfg->size;
                dst_font->ascent = ((float)unscaled_ascent * font_scale);
                dst_font->descent = ((float)unscaled_descent * font_scale);
                dst_font->glyph_offset = glyph_n;
                /*
                    Need to zero this, or it will carry over from a previous
                    bake, and cause a segfault when accessing glyphs[].
                */
                dst_font->glyph_count = 0;
            }

            /* fill own baked font glyph array */
            for (i = 0; i < tmp->range_count; ++i) {
                stbtt_pack_range *range = &tmp->ranges[i];
                for (char_idx = 0; char_idx < range->num_chars; char_idx++)
                {
                    nk_rune codepoint = 0;
                    float dummy_x = 0, dummy_y = 0;
                    stbtt_aligned_quad q;
                    struct nk_font_glyph *glyph;

                    /* query glyph bounds from stb_truetype */
                    const stbtt_packedchar *pc = &range->chardata_for_range[char_idx];
                    if (!pc->x0 && !pc->x1 && !pc->y0 && !pc->y1) continue;
                    codepoint = (nk_rune)(range->first_unicode_codepoint_in_range + char_idx);
                    stbtt_GetPackedQuad(range->chardata_for_range, (int)width,
                        (int)height, char_idx, &dummy_x, &dummy_y, &q, 0);

                    /* fill own glyph type with data */
                    glyph = &glyphs[dst_font->glyph_offset + dst_font->glyph_count + (unsigned int)glyph_count];
                    glyph->codepoint = codepoint;
                    glyph->x0 = q.x0; glyph->y0 = q.y0;
                    glyph->x1 = q.x1; glyph->y1 = q.y1;
                    glyph->y0 += (dst_font->ascent + 0.5f);
                    glyph->y1 += (dst_font->ascent + 0.5f);
                    glyph->w = glyph->x1 - glyph->x0 + 0.5f;
                    glyph->h = glyph->y1 - glyph->y0;

                    if (cfg->coord_type == NK_COORD_PIXEL) {
                        glyph->u0 = q.s0 * (float)width;
                        glyph->v0 = q.t0 * (float)height;
                        glyph->u1 = q.s1 * (float)width;
                        glyph->v1 = q.t1 * (float)height;
                    } else {
                        glyph->u0 = q.s0;
                        glyph->v0 = q.t0;
                        glyph->u1 = q.s1;
                        glyph->v1 = q.t1;
                    }
                    glyph->xadvance = (pc->xadvance + cfg->spacing.x);
                    if (cfg->pixel_snap)
                        glyph->xadvance = (float)(int)(glyph->xadvance + 0.5f);
                    glyph_count++;
                }
            }
            dst_font->glyph_count += glyph_count;
            glyph_n += glyph_count;
        } while ((it = it->n) != config_iter);
    }
}
NK_INTERN void
nk_font_bake_custom_data(void *img_memory, int img_width, int img_height,
    struct nk_recti img_dst, const char *texture_data_mask, int tex_width,
    int tex_height, char white, char black)
{
    nk_byte *pixels;
    int y = 0;
    int x = 0;
    int n = 0;

    NK_ASSERT(img_memory);
    NK_ASSERT(img_width);
    NK_ASSERT(img_height);
    NK_ASSERT(texture_data_mask);
    NK_UNUSED(tex_height);
    if (!img_memory || !img_width || !img_height || !texture_data_mask)
        return;

    pixels = (nk_byte*)img_memory;
    for (y = 0, n = 0; y < tex_height; ++y) {
        for (x = 0; x < tex_width; ++x, ++n) {
            const int off0 = ((img_dst.x + x) + (img_dst.y + y) * img_width);
            const int off1 = off0 + 1 + tex_width;
            pixels[off0] = (texture_data_mask[n] == white) ? 0xFF : 0x00;
            pixels[off1] = (texture_data_mask[n] == black) ? 0xFF : 0x00;
        }
    }
}
NK_INTERN void
nk_font_bake_convert(void *out_memory, int img_width, int img_height,
    const void *in_memory)
{
    int n = 0;
    nk_rune *dst;
    const nk_byte *src;

    NK_ASSERT(out_memory);
    NK_ASSERT(in_memory);
    NK_ASSERT(img_width);
    NK_ASSERT(img_height);
    if (!out_memory || !in_memory || !img_height || !img_width) return;

    dst = (nk_rune*)out_memory;
    src = (const nk_byte*)in_memory;
    for (n = (int)(img_width * img_height); n > 0; n--)
        *dst++ = ((nk_rune)(*src++) << 24) | 0x00FFFFFF;
}

/* -------------------------------------------------------------
 *
 *                          FONT
 *
 * --------------------------------------------------------------*/
NK_INTERN float
nk_font_text_width(nk_handle handle, float height, const char *text, int len)
{
    nk_rune unicode;
    int text_len  = 0;
    float text_width = 0;
    int glyph_len = 0;
    float scale = 0;

    struct nk_font *font = (struct nk_font*)handle.ptr;
    NK_ASSERT(font);
    NK_ASSERT(font->glyphs);
    if (!font || !text || !len)
        return 0;

    scale = height/font->info.height;
    glyph_len = text_len = nk_utf_decode(text, &unicode, (int)len);
    if (!glyph_len) return 0;
    while (text_len <= (int)len && glyph_len) {
        const struct nk_font_glyph *g;
        if (unicode == NK_UTF_INVALID) break;

        /* query currently drawn glyph information */
        g = nk_font_find_glyph(font, unicode);
        text_width += g->xadvance * scale;

        /* offset next glyph */
        glyph_len = nk_utf_decode(text + text_len, &unicode, (int)len - text_len);
        text_len += glyph_len;
    }
    return text_width;
}
#ifdef NK_INCLUDE_VERTEX_BUFFER_OUTPUT
NK_INTERN void
nk_font_query_font_glyph(nk_handle handle, float height,
    struct nk_user_font_glyph *glyph, nk_rune codepoint, nk_rune next_codepoint)
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
#endif
NK_API const struct nk_font_glyph*
nk_font_find_glyph(struct nk_font *font, nk_rune unicode)
{
    int i = 0;
    int count;
    int total_glyphs = 0;
    const struct nk_font_glyph *glyph = 0;
    const struct nk_font_config *iter = 0;

    NK_ASSERT(font);
    NK_ASSERT(font->glyphs);
    NK_ASSERT(font->info.ranges);
    if (!font || !font->glyphs) return 0;

    glyph = font->fallback;
    iter = font->config;
    do {count = nk_range_count(iter->range);
        for (i = 0; i < count; ++i) {
            nk_rune f = iter->range[(i*2)+0];
            nk_rune t = iter->range[(i*2)+1];
            int diff = (int)((t - f) + 1);
            if (unicode >= f && unicode <= t)
                return &font->glyphs[((nk_rune)total_glyphs + (unicode - f))];
            total_glyphs += diff;
        }
    } while ((iter = iter->n) != font->config);
    return glyph;
}
NK_INTERN void
nk_font_init(struct nk_font *font, float pixel_height,
    nk_rune fallback_codepoint, struct nk_font_glyph *glyphs,
    const struct nk_baked_font *baked_font, nk_handle atlas)
{
    struct nk_baked_font baked;
    NK_ASSERT(font);
    NK_ASSERT(glyphs);
    NK_ASSERT(baked_font);
    if (!font || !glyphs || !baked_font)
        return;

    baked = *baked_font;
    font->fallback = 0;
    font->info = baked;
    font->scale = (float)pixel_height / (float)font->info.height;
    font->glyphs = &glyphs[baked_font->glyph_offset];
    font->texture = atlas;
    font->fallback_codepoint = fallback_codepoint;
    font->fallback = nk_font_find_glyph(font, fallback_codepoint);

    font->handle.height = font->info.height * font->scale;
    font->handle.width = nk_font_text_width;
    font->handle.userdata.ptr = font;
#ifdef NK_INCLUDE_VERTEX_BUFFER_OUTPUT
    font->handle.query = nk_font_query_font_glyph;
    font->handle.texture = font->texture;
#endif
}

/* ---------------------------------------------------------------------------
 *
 *                          DEFAULT FONT
 *
 * ProggyClean.ttf
 * Copyright (c) 2004, 2005 Tristan Grimmer
 * MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
 * Download and more information at http://upperbounds.net
 *-----------------------------------------------------------------------------*/
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverlength-strings"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif

#ifdef NK_INCLUDE_DEFAULT_FONT

NK_GLOBAL const char nk_proggy_clean_ttf_compressed_data_base85[11980+1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

#endif /* NK_INCLUDE_DEFAULT_FONT */

#define NK_CURSOR_DATA_W 90
#define NK_CURSOR_DATA_H 27
NK_GLOBAL const char nk_custom_cursor_data[NK_CURSOR_DATA_W * NK_CURSOR_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX"
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X"
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X"
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X"
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X"
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X"
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX"
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        "
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         "
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          "
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           "
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            "
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           "
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          "
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       ------------------------------------"
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           "
    "------------        -    X    -           X           -X.....................X-           "
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -           "
    "                                                      -  X..X           X..X  -           "
    "                                                      -   X.X           X.X   -           "
    "                                                      -    XX           XX    -           "
};

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

NK_GLOBAL unsigned char *nk__barrier;
NK_GLOBAL unsigned char *nk__barrier2;
NK_GLOBAL unsigned char *nk__barrier3;
NK_GLOBAL unsigned char *nk__barrier4;
NK_GLOBAL unsigned char *nk__dout;

NK_INTERN unsigned int
nk_decompress_length(unsigned char *input)
{
    return (unsigned int)((input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11]);
}
NK_INTERN void
nk__match(unsigned char *data, unsigned int length)
{
    /* INVERSE of memmove... write each byte before copying the next...*/
    NK_ASSERT (nk__dout + length <= nk__barrier);
    if (nk__dout + length > nk__barrier) { nk__dout += length; return; }
    if (data < nk__barrier4) { nk__dout = nk__barrier+1; return; }
    while (length--) *nk__dout++ = *data++;
}
NK_INTERN void
nk__lit(unsigned char *data, unsigned int length)
{
    NK_ASSERT (nk__dout + length <= nk__barrier);
    if (nk__dout + length > nk__barrier) { nk__dout += length; return; }
    if (data < nk__barrier2) { nk__dout = nk__barrier+1; return; }
    NK_MEMCPY(nk__dout, data, length);
    nk__dout += length;
}
NK_INTERN unsigned char*
nk_decompress_token(unsigned char *i)
{
    #define nk__in2(x)   ((i[x] << 8) + i[(x)+1])
    #define nk__in3(x)   ((i[x] << 16) + nk__in2((x)+1))
    #define nk__in4(x)   ((i[x] << 24) + nk__in3((x)+1))

    if (*i >= 0x20) { /* use fewer if's for cases that expand small */
        if (*i >= 0x80)       nk__match(nk__dout-i[1]-1, (unsigned int)i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  nk__match(nk__dout-(nk__in2(0) - 0x4000 + 1), (unsigned int)i[2]+1), i += 3;
        else /* *i >= 0x20 */ nk__lit(i+1, (unsigned int)i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { /* more ifs for cases that expand large, since overhead is amortized */
        if (*i >= 0x18)       nk__match(nk__dout-(unsigned int)(nk__in3(0) - 0x180000 + 1), (unsigned int)i[3]+1), i += 4;
        else if (*i >= 0x10)  nk__match(nk__dout-(unsigned int)(nk__in3(0) - 0x100000 + 1), (unsigned int)nk__in2(3)+1), i += 5;
        else if (*i >= 0x08)  nk__lit(i+2, (unsigned int)nk__in2(0) - 0x0800 + 1), i += 2 + (nk__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  nk__lit(i+3, (unsigned int)nk__in2(1) + 1), i += 3 + (nk__in2(1) + 1);
        else if (*i == 0x06)  nk__match(nk__dout-(unsigned int)(nk__in3(1)+1), i[4]+1u), i += 5;
        else if (*i == 0x04)  nk__match(nk__dout-(unsigned int)(nk__in3(1)+1), (unsigned int)nk__in2(4)+1u), i += 6;
    }
    return i;
}
NK_INTERN unsigned int
nk_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0]; s2 += s1;
            s1 += buffer[1]; s2 += s1;
            s1 += buffer[2]; s2 += s1;
            s1 += buffer[3]; s2 += s1;
            s1 += buffer[4]; s2 += s1;
            s1 += buffer[5]; s2 += s1;
            s1 += buffer[6]; s2 += s1;
            s1 += buffer[7]; s2 += s1;
            buffer += 8;
        }
        for (; i < blocklen; ++i) {
            s1 += *buffer++; s2 += s1;
        }

        s1 %= ADLER_MOD; s2 %= ADLER_MOD;
        buflen -= (unsigned int)blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}
NK_INTERN unsigned int
nk_decompress(unsigned char *output, unsigned char *i, unsigned int length)
{
    unsigned int olen;
    if (nk__in4(0) != 0x57bC0000) return 0;
    if (nk__in4(4) != 0)          return 0; /* error! stream is > 4GB */
    olen = nk_decompress_length(i);
    nk__barrier2 = i;
    nk__barrier3 = i+length;
    nk__barrier = output + olen;
    nk__barrier4 = output;
    i += 16;

    nk__dout = output;
    for (;;) {
        unsigned char *old_i = i;
        i = nk_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                NK_ASSERT(nk__dout == output + olen);
                if (nk__dout != output + olen) return 0;
                if (nk_adler32(1, output, olen) != (unsigned int) nk__in4(2))
                    return 0;
                return olen;
            } else {
                NK_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        NK_ASSERT(nk__dout <= output + olen);
        if (nk__dout > output + olen)
            return 0;
    }
}
NK_INTERN unsigned int
nk_decode_85_byte(char c)
{
    return (unsigned int)((c >= '\\') ? c-36 : c-35);
}
NK_INTERN void
nk_decode_85(unsigned char* dst, const unsigned char* src)
{
    while (*src)
    {
        unsigned int tmp =
            nk_decode_85_byte((char)src[0]) +
            85 * (nk_decode_85_byte((char)src[1]) +
            85 * (nk_decode_85_byte((char)src[2]) +
            85 * (nk_decode_85_byte((char)src[3]) +
            85 * nk_decode_85_byte((char)src[4]))));

        /* we can't assume little-endianess. */
        dst[0] = (unsigned char)((tmp >> 0) & 0xFF);
        dst[1] = (unsigned char)((tmp >> 8) & 0xFF);
        dst[2] = (unsigned char)((tmp >> 16) & 0xFF);
        dst[3] = (unsigned char)((tmp >> 24) & 0xFF);

        src += 5;
        dst += 4;
    }
}

/* -------------------------------------------------------------
 *
 *                          FONT ATLAS
 *
 * --------------------------------------------------------------*/
NK_API struct nk_font_config
nk_font_config(float pixel_height)
{
    struct nk_font_config cfg;
    nk_zero_struct(cfg);
    cfg.ttf_blob = 0;
    cfg.ttf_size = 0;
    cfg.ttf_data_owned_by_atlas = 0;
    cfg.size = pixel_height;
    cfg.oversample_h = 3;
    cfg.oversample_v = 1;
    cfg.pixel_snap = 0;
    cfg.coord_type = NK_COORD_UV;
    cfg.spacing = nk_vec2(0,0);
    cfg.range = nk_font_default_glyph_ranges();
    cfg.merge_mode = 0;
    cfg.fallback_glyph = '?';
    cfg.font = 0;
    cfg.n = 0;
    return cfg;
}
#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
NK_API void
nk_font_atlas_init_default(struct nk_font_atlas *atlas)
{
    NK_ASSERT(atlas);
    if (!atlas) return;
    nk_zero_struct(*atlas);
    atlas->temporary.userdata.ptr = 0;
    atlas->temporary.alloc = nk_malloc;
    atlas->temporary.free = nk_mfree;
    atlas->permanent.userdata.ptr = 0;
    atlas->permanent.alloc = nk_malloc;
    atlas->permanent.free = nk_mfree;
}
#endif
NK_API void
nk_font_atlas_init(struct nk_font_atlas *atlas, struct nk_allocator *alloc)
{
    NK_ASSERT(atlas);
    NK_ASSERT(alloc);
    if (!atlas || !alloc) return;
    nk_zero_struct(*atlas);
    atlas->permanent = *alloc;
    atlas->temporary = *alloc;
}
NK_API void
nk_font_atlas_init_custom(struct nk_font_atlas *atlas,
    struct nk_allocator *permanent, struct nk_allocator *temporary)
{
    NK_ASSERT(atlas);
    NK_ASSERT(permanent);
    NK_ASSERT(temporary);
    if (!atlas || !permanent || !temporary) return;
    nk_zero_struct(*atlas);
    atlas->permanent = *permanent;
    atlas->temporary = *temporary;
}
NK_API void
nk_font_atlas_begin(struct nk_font_atlas *atlas)
{
    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc && atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc && atlas->permanent.free);
    if (!atlas || !atlas->permanent.alloc || !atlas->permanent.free ||
        !atlas->temporary.alloc || !atlas->temporary.free) return;
    if (atlas->glyphs) {
        atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
        atlas->glyphs = 0;
    }
    if (atlas->pixel) {
        atlas->permanent.free(atlas->permanent.userdata, atlas->pixel);
        atlas->pixel = 0;
    }
}
NK_API struct nk_font*
nk_font_atlas_add(struct nk_font_atlas *atlas, const struct nk_font_config *config)
{
    struct nk_font *font = 0;
    struct nk_font_config *cfg;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);

    NK_ASSERT(config);
    NK_ASSERT(config->ttf_blob);
    NK_ASSERT(config->ttf_size);
    NK_ASSERT(config->size > 0.0f);

    if (!atlas || !config || !config->ttf_blob || !config->ttf_size || config->size <= 0.0f||
        !atlas->permanent.alloc || !atlas->permanent.free ||
        !atlas->temporary.alloc || !atlas->temporary.free)
        return 0;

    /* allocate font config  */
    cfg = (struct nk_font_config*)
        atlas->permanent.alloc(atlas->permanent.userdata,0, sizeof(struct nk_font_config));
    NK_MEMCPY(cfg, config, sizeof(*config));
    cfg->n = cfg;
    cfg->p = cfg;

    if (!config->merge_mode) {
        /* insert font config into list */
        if (!atlas->config) {
            atlas->config = cfg;
            cfg->next = 0;
        } else {
            struct nk_font_config *i = atlas->config;
            while (i->next) i = i->next;
            i->next = cfg;
            cfg->next = 0;
        }
        /* allocate new font */
        font = (struct nk_font*)
            atlas->permanent.alloc(atlas->permanent.userdata,0, sizeof(struct nk_font));
        NK_ASSERT(font);
        nk_zero(font, sizeof(*font));
        if (!font) return 0;
        font->config = cfg;

        /* insert font into list */
        if (!atlas->fonts) {
            atlas->fonts = font;
            font->next = 0;
        } else {
            struct nk_font *i = atlas->fonts;
            while (i->next) i = i->next;
            i->next = font;
            font->next = 0;
        }
        cfg->font = &font->info;
    } else {
        /* extend previously added font */
        struct nk_font *f = 0;
        struct nk_font_config *c = 0;
        NK_ASSERT(atlas->font_num);
        f = atlas->fonts;
        c = f->config;
        cfg->font = &f->info;

        cfg->n = c;
        cfg->p = c->p;
        c->p->n = cfg;
        c->p = cfg;
    }
    /* create own copy of .TTF font blob */
    if (!config->ttf_data_owned_by_atlas) {
        cfg->ttf_blob = atlas->permanent.alloc(atlas->permanent.userdata,0, cfg->ttf_size);
        NK_ASSERT(cfg->ttf_blob);
        if (!cfg->ttf_blob) {
            atlas->font_num++;
            return 0;
        }
        NK_MEMCPY(cfg->ttf_blob, config->ttf_blob, cfg->ttf_size);
        cfg->ttf_data_owned_by_atlas = 1;
    }
    atlas->font_num++;
    return font;
}
NK_API struct nk_font*
nk_font_atlas_add_from_memory(struct nk_font_atlas *atlas, void *memory,
    nk_size size, float height, const struct nk_font_config *config)
{
    struct nk_font_config cfg;
    NK_ASSERT(memory);
    NK_ASSERT(size);

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    if (!atlas || !atlas->temporary.alloc || !atlas->temporary.free || !memory || !size ||
        !atlas->permanent.alloc || !atlas->permanent.free)
        return 0;

    cfg = (config) ? *config: nk_font_config(height);
    cfg.ttf_blob = memory;
    cfg.ttf_size = size;
    cfg.size = height;
    cfg.ttf_data_owned_by_atlas = 0;
    return nk_font_atlas_add(atlas, &cfg);
}
#ifdef NK_INCLUDE_STANDARD_IO
NK_API struct nk_font*
nk_font_atlas_add_from_file(struct nk_font_atlas *atlas, const char *file_path,
    float height, const struct nk_font_config *config)
{
    nk_size size;
    char *memory;
    struct nk_font_config cfg;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);

    if (!atlas || !file_path) return 0;
    memory = nk_file_load(file_path, &size, &atlas->permanent);
    if (!memory) return 0;

    cfg = (config) ? *config: nk_font_config(height);
    cfg.ttf_blob = memory;
    cfg.ttf_size = size;
    cfg.size = height;
    cfg.ttf_data_owned_by_atlas = 1;
    return nk_font_atlas_add(atlas, &cfg);
}
#endif
NK_API struct nk_font*
nk_font_atlas_add_compressed(struct nk_font_atlas *atlas,
    void *compressed_data, nk_size compressed_size, float height,
    const struct nk_font_config *config)
{
    unsigned int decompressed_size;
    void *decompressed_data;
    struct nk_font_config cfg;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);

    NK_ASSERT(compressed_data);
    NK_ASSERT(compressed_size);
    if (!atlas || !compressed_data || !atlas->temporary.alloc || !atlas->temporary.free ||
        !atlas->permanent.alloc || !atlas->permanent.free)
        return 0;

    decompressed_size = nk_decompress_length((unsigned char*)compressed_data);
    decompressed_data = atlas->permanent.alloc(atlas->permanent.userdata,0,decompressed_size);
    NK_ASSERT(decompressed_data);
    if (!decompressed_data) return 0;
    nk_decompress((unsigned char*)decompressed_data, (unsigned char*)compressed_data,
        (unsigned int)compressed_size);

    cfg = (config) ? *config: nk_font_config(height);
    cfg.ttf_blob = decompressed_data;
    cfg.ttf_size = decompressed_size;
    cfg.size = height;
    cfg.ttf_data_owned_by_atlas = 1;
    return nk_font_atlas_add(atlas, &cfg);
}
NK_API struct nk_font*
nk_font_atlas_add_compressed_base85(struct nk_font_atlas *atlas,
    const char *data_base85, float height, const struct nk_font_config *config)
{
    int compressed_size;
    void *compressed_data;
    struct nk_font *font;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);

    NK_ASSERT(data_base85);
    if (!atlas || !data_base85 || !atlas->temporary.alloc || !atlas->temporary.free ||
        !atlas->permanent.alloc || !atlas->permanent.free)
        return 0;

    compressed_size = (((int)nk_strlen(data_base85) + 4) / 5) * 4;
    compressed_data = atlas->temporary.alloc(atlas->temporary.userdata,0, (nk_size)compressed_size);
    NK_ASSERT(compressed_data);
    if (!compressed_data) return 0;
    nk_decode_85((unsigned char*)compressed_data, (const unsigned char*)data_base85);
    font = nk_font_atlas_add_compressed(atlas, compressed_data,
                    (nk_size)compressed_size, height, config);
    atlas->temporary.free(atlas->temporary.userdata, compressed_data);
    return font;
}

#ifdef NK_INCLUDE_DEFAULT_FONT
NK_API struct nk_font*
nk_font_atlas_add_default(struct nk_font_atlas *atlas,
    float pixel_height, const struct nk_font_config *config)
{
    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    return nk_font_atlas_add_compressed_base85(atlas,
        nk_proggy_clean_ttf_compressed_data_base85, pixel_height, config);
}
#endif
NK_API const void*
nk_font_atlas_bake(struct nk_font_atlas *atlas, int *width, int *height,
    enum nk_font_atlas_format fmt)
{
    int i = 0;
    void *tmp = 0;
    nk_size tmp_size, img_size;
    struct nk_font *font_iter;
    struct nk_font_baker *baker;

    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);

    NK_ASSERT(width);
    NK_ASSERT(height);
    if (!atlas || !width || !height ||
        !atlas->temporary.alloc || !atlas->temporary.free ||
        !atlas->permanent.alloc || !atlas->permanent.free)
        return 0;

#ifdef NK_INCLUDE_DEFAULT_FONT
    /* no font added so just use default font */
    if (!atlas->font_num)
        atlas->default_font = nk_font_atlas_add_default(atlas, 13.0f, 0);
#endif
    NK_ASSERT(atlas->font_num);
    if (!atlas->font_num) return 0;

    /* allocate temporary baker memory required for the baking process */
    nk_font_baker_memory(&tmp_size, &atlas->glyph_count, atlas->config, atlas->font_num);
    tmp = atlas->temporary.alloc(atlas->temporary.userdata,0, tmp_size);
    NK_ASSERT(tmp);
    if (!tmp) goto failed;
    NK_MEMSET(tmp,0,tmp_size);

    /* allocate glyph memory for all fonts */
    baker = nk_font_baker(tmp, atlas->glyph_count, atlas->font_num, &atlas->temporary);
    atlas->glyphs = (struct nk_font_glyph*)atlas->permanent.alloc(
        atlas->permanent.userdata,0, sizeof(struct nk_font_glyph)*(nk_size)atlas->glyph_count);
    NK_ASSERT(atlas->glyphs);
    if (!atlas->glyphs)
        goto failed;

    /* pack all glyphs into a tight fit space */
    atlas->custom.w = (NK_CURSOR_DATA_W*2)+1;
    atlas->custom.h = NK_CURSOR_DATA_H + 1;
    if (!nk_font_bake_pack(baker, &img_size, width, height, &atlas->custom,
        atlas->config, atlas->font_num, &atlas->temporary))
        goto failed;

    /* allocate memory for the baked image font atlas */
    atlas->pixel = atlas->temporary.alloc(atlas->temporary.userdata,0, img_size);
    NK_ASSERT(atlas->pixel);
    if (!atlas->pixel)
        goto failed;

    /* bake glyphs and custom white pixel into image */
    nk_font_bake(baker, atlas->pixel, *width, *height,
        atlas->glyphs, atlas->glyph_count, atlas->config, atlas->font_num);
    nk_font_bake_custom_data(atlas->pixel, *width, *height, atlas->custom,
            nk_custom_cursor_data, NK_CURSOR_DATA_W, NK_CURSOR_DATA_H, '.', 'X');

    if (fmt == NK_FONT_ATLAS_RGBA32) {
        /* convert alpha8 image into rgba32 image */
        void *img_rgba = atlas->temporary.alloc(atlas->temporary.userdata,0,
                            (nk_size)(*width * *height * 4));
        NK_ASSERT(img_rgba);
        if (!img_rgba) goto failed;
        nk_font_bake_convert(img_rgba, *width, *height, atlas->pixel);
        atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
        atlas->pixel = img_rgba;
    }
    atlas->tex_width = *width;
    atlas->tex_height = *height;

    /* initialize each font */
    for (font_iter = atlas->fonts; font_iter; font_iter = font_iter->next) {
        struct nk_font *font = font_iter;
        struct nk_font_config *config = font->config;
        nk_font_init(font, config->size, config->fallback_glyph, atlas->glyphs,
            config->font, nk_handle_ptr(0));
    }

    /* initialize each cursor */
    {NK_STORAGE const struct nk_vec2 nk_cursor_data[NK_CURSOR_COUNT][3] = {
        /* Pos      Size        Offset */
        {{ 0, 3},   {12,19},    { 0, 0}},
        {{13, 0},   { 7,16},    { 4, 8}},
        {{31, 0},   {23,23},    {11,11}},
        {{21, 0},   { 9, 23},   { 5,11}},
        {{55,18},   {23, 9},    {11, 5}},
        {{73, 0},   {17,17},    { 9, 9}},
        {{55, 0},   {17,17},    { 9, 9}}
    };
    for (i = 0; i < NK_CURSOR_COUNT; ++i) {
        struct nk_cursor *cursor = &atlas->cursors[i];
        cursor->img.w = (unsigned short)*width;
        cursor->img.h = (unsigned short)*height;
        cursor->img.region[0] = (unsigned short)(atlas->custom.x + nk_cursor_data[i][0].x);
        cursor->img.region[1] = (unsigned short)(atlas->custom.y + nk_cursor_data[i][0].y);
        cursor->img.region[2] = (unsigned short)nk_cursor_data[i][1].x;
        cursor->img.region[3] = (unsigned short)nk_cursor_data[i][1].y;
        cursor->size = nk_cursor_data[i][1];
        cursor->offset = nk_cursor_data[i][2];
    }}
    /* free temporary memory */
    atlas->temporary.free(atlas->temporary.userdata, tmp);
    return atlas->pixel;

failed:
    /* error so cleanup all memory */
    if (tmp) atlas->temporary.free(atlas->temporary.userdata, tmp);
    if (atlas->glyphs) {
        atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
        atlas->glyphs = 0;
    }
    if (atlas->pixel) {
        atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
        atlas->pixel = 0;
    }
    return 0;
}
NK_API void
nk_font_atlas_end(struct nk_font_atlas *atlas, nk_handle texture,
    struct nk_draw_null_texture *tex_null)
{
    int i = 0;
    struct nk_font *font_iter;
    NK_ASSERT(atlas);
    if (!atlas) {
        if (!tex_null) return;
        tex_null->texture = texture;
        tex_null->uv = nk_vec2(0.5f,0.5f);
    }
    if (tex_null) {
        tex_null->texture = texture;
        tex_null->uv.x = (atlas->custom.x + 0.5f)/(float)atlas->tex_width;
        tex_null->uv.y = (atlas->custom.y + 0.5f)/(float)atlas->tex_height;
    }
    for (font_iter = atlas->fonts; font_iter; font_iter = font_iter->next) {
        font_iter->texture = texture;
#ifdef NK_INCLUDE_VERTEX_BUFFER_OUTPUT
        font_iter->handle.texture = texture;
#endif
    }
    for (i = 0; i < NK_CURSOR_COUNT; ++i)
        atlas->cursors[i].img.handle = texture;

    atlas->temporary.free(atlas->temporary.userdata, atlas->pixel);
    atlas->pixel = 0;
    atlas->tex_width = 0;
    atlas->tex_height = 0;
    atlas->custom.x = 0;
    atlas->custom.y = 0;
    atlas->custom.w = 0;
    atlas->custom.h = 0;
}
NK_API void
nk_font_atlas_cleanup(struct nk_font_atlas *atlas)
{
    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    if (!atlas || !atlas->permanent.alloc || !atlas->permanent.free) return;
    if (atlas->config) {
        struct nk_font_config *iter;
        for (iter = atlas->config; iter; iter = iter->next) {
            struct nk_font_config *i;
            for (i = iter->n; i != iter; i = i->n) {
                atlas->permanent.free(atlas->permanent.userdata, i->ttf_blob);
                i->ttf_blob = 0;
            }
            atlas->permanent.free(atlas->permanent.userdata, iter->ttf_blob);
            iter->ttf_blob = 0;
        }
    }
}
NK_API void
nk_font_atlas_clear(struct nk_font_atlas *atlas)
{
    NK_ASSERT(atlas);
    NK_ASSERT(atlas->temporary.alloc);
    NK_ASSERT(atlas->temporary.free);
    NK_ASSERT(atlas->permanent.alloc);
    NK_ASSERT(atlas->permanent.free);
    if (!atlas || !atlas->permanent.alloc || !atlas->permanent.free) return;

    if (atlas->config) {
        struct nk_font_config *iter, *next;
        for (iter = atlas->config; iter; iter = next) {
            struct nk_font_config *i, *n;
            for (i = iter->n; i != iter; i = n) {
                n = i->n;
                if (i->ttf_blob)
                    atlas->permanent.free(atlas->permanent.userdata, i->ttf_blob);
                atlas->permanent.free(atlas->permanent.userdata, i);
            }
            next = iter->next;
            if (i->ttf_blob)
                atlas->permanent.free(atlas->permanent.userdata, iter->ttf_blob);
            atlas->permanent.free(atlas->permanent.userdata, iter);
        }
        atlas->config = 0;
    }
    if (atlas->fonts) {
        struct nk_font *iter, *next;
        for (iter = atlas->fonts; iter; iter = next) {
            next = iter->next;
            atlas->permanent.free(atlas->permanent.userdata, iter);
        }
        atlas->fonts = 0;
    }
    if (atlas->glyphs)
        atlas->permanent.free(atlas->permanent.userdata, atlas->glyphs);
    nk_zero_struct(*atlas);
}
#endif

