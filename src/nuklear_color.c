#include "nuklear.h"
#include "nuklear_internal.h"

/* ==============================================================
 *
 *                          COLOR
 *
 * ===============================================================*/
NK_INTERN int
nk_parse_hex(const char *p, int length)
{
    int i = 0;
    int len = 0;
    while (len < length) {
        i <<= 4;
        if (p[len] >= 'a' && p[len] <= 'f')
            i += ((p[len] - 'a') + 10);
        else if (p[len] >= 'A' && p[len] <= 'F')
            i += ((p[len] - 'A') + 10);
        else i += (p[len] - '0');
        len++;
    }
    return i;
}
NK_API struct nk_color
nk_rgb_factor(struct nk_color col, const float factor)
{
    if (factor == 1.0f)
        return col;
    col.r = (nk_byte)(col.r * factor);
    col.g = (nk_byte)(col.g * factor);
    col.b = (nk_byte)(col.b * factor);
    return col;
}
NK_API struct nk_color
nk_rgba(int r, int g, int b, int a)
{
    struct nk_color ret;
    ret.r = (nk_byte)NK_CLAMP(0, r, 255);
    ret.g = (nk_byte)NK_CLAMP(0, g, 255);
    ret.b = (nk_byte)NK_CLAMP(0, b, 255);
    ret.a = (nk_byte)NK_CLAMP(0, a, 255);
    return ret;
}
NK_API struct nk_color
nk_rgb_hex(const char *rgb)
{
    struct nk_color col;
    const char *c = rgb;
    if (*c == '#') c++;
    col.r = (nk_byte)nk_parse_hex(c, 2);
    col.g = (nk_byte)nk_parse_hex(c+2, 2);
    col.b = (nk_byte)nk_parse_hex(c+4, 2);
    col.a = 255;
    return col;
}
NK_API struct nk_color
nk_rgba_hex(const char *rgb)
{
    struct nk_color col;
    const char *c = rgb;
    if (*c == '#') c++;
    col.r = (nk_byte)nk_parse_hex(c, 2);
    col.g = (nk_byte)nk_parse_hex(c+2, 2);
    col.b = (nk_byte)nk_parse_hex(c+4, 2);
    col.a = (nk_byte)nk_parse_hex(c+6, 2);
    return col;
}
NK_API void
nk_color_hex_rgba(char *output, struct nk_color col)
{
    #define NK_TO_HEX(i) ((i) <= 9 ? '0' + (i): 'A' - 10 + (i))
    output[0] = (char)NK_TO_HEX((col.r & 0xF0) >> 4);
    output[1] = (char)NK_TO_HEX((col.r & 0x0F));
    output[2] = (char)NK_TO_HEX((col.g & 0xF0) >> 4);
    output[3] = (char)NK_TO_HEX((col.g & 0x0F));
    output[4] = (char)NK_TO_HEX((col.b & 0xF0) >> 4);
    output[5] = (char)NK_TO_HEX((col.b & 0x0F));
    output[6] = (char)NK_TO_HEX((col.a & 0xF0) >> 4);
    output[7] = (char)NK_TO_HEX((col.a & 0x0F));
    output[8] = '\0';
    #undef NK_TO_HEX
}
NK_API void
nk_color_hex_rgb(char *output, struct nk_color col)
{
    #define NK_TO_HEX(i) ((i) <= 9 ? '0' + (i): 'A' - 10 + (i))
    output[0] = (char)NK_TO_HEX((col.r & 0xF0) >> 4);
    output[1] = (char)NK_TO_HEX((col.r & 0x0F));
    output[2] = (char)NK_TO_HEX((col.g & 0xF0) >> 4);
    output[3] = (char)NK_TO_HEX((col.g & 0x0F));
    output[4] = (char)NK_TO_HEX((col.b & 0xF0) >> 4);
    output[5] = (char)NK_TO_HEX((col.b & 0x0F));
    output[6] = '\0';
    #undef NK_TO_HEX
}
NK_API struct nk_color
nk_rgba_iv(const int *c)
{
    return nk_rgba(c[0], c[1], c[2], c[3]);
}
NK_API struct nk_color
nk_rgba_bv(const nk_byte *c)
{
    return nk_rgba(c[0], c[1], c[2], c[3]);
}
NK_API struct nk_color
nk_rgb(int r, int g, int b)
{
    struct nk_color ret;
    ret.r = (nk_byte)NK_CLAMP(0, r, 255);
    ret.g = (nk_byte)NK_CLAMP(0, g, 255);
    ret.b = (nk_byte)NK_CLAMP(0, b, 255);
    ret.a = (nk_byte)255;
    return ret;
}
NK_API struct nk_color
nk_rgb_iv(const int *c)
{
    return nk_rgb(c[0], c[1], c[2]);
}
NK_API struct nk_color
nk_rgb_bv(const nk_byte* c)
{
    return nk_rgb(c[0], c[1], c[2]);
}
NK_API struct nk_color
nk_rgba_u32(nk_uint in)
{
    struct nk_color ret;
    ret.r = (in & 0xFF);
    ret.g = ((in >> 8) & 0xFF);
    ret.b = ((in >> 16) & 0xFF);
    ret.a = (nk_byte)((in >> 24) & 0xFF);
    return ret;
}
NK_API struct nk_color
nk_rgba_f(float r, float g, float b, float a)
{
    struct nk_color ret;
    ret.r = (nk_byte)(NK_SATURATE(r) * 255.0f);
    ret.g = (nk_byte)(NK_SATURATE(g) * 255.0f);
    ret.b = (nk_byte)(NK_SATURATE(b) * 255.0f);
    ret.a = (nk_byte)(NK_SATURATE(a) * 255.0f);
    return ret;
}
NK_API struct nk_color
nk_rgba_fv(const float *c)
{
    return nk_rgba_f(c[0], c[1], c[2], c[3]);
}
NK_API struct nk_color
nk_rgba_cf(struct nk_colorf c)
{
    return nk_rgba_f(c.r, c.g, c.b, c.a);
}
NK_API struct nk_color
nk_rgb_f(float r, float g, float b)
{
    struct nk_color ret;
    ret.r = (nk_byte)(NK_SATURATE(r) * 255.0f);
    ret.g = (nk_byte)(NK_SATURATE(g) * 255.0f);
    ret.b = (nk_byte)(NK_SATURATE(b) * 255.0f);
    ret.a = 255;
    return ret;
}
NK_API struct nk_color
nk_rgb_fv(const float *c)
{
    return nk_rgb_f(c[0], c[1], c[2]);
}
NK_API struct nk_color
nk_rgb_cf(struct nk_colorf c)
{
    return nk_rgb_f(c.r, c.g, c.b);
}
NK_API struct nk_color
nk_hsv(int h, int s, int v)
{
    return nk_hsva(h, s, v, 255);
}
NK_API struct nk_color
nk_hsv_iv(const int *c)
{
    return nk_hsv(c[0], c[1], c[2]);
}
NK_API struct nk_color
nk_hsv_bv(const nk_byte *c)
{
    return nk_hsv(c[0], c[1], c[2]);
}
NK_API struct nk_color
nk_hsv_f(float h, float s, float v)
{
    return nk_hsva_f(h, s, v, 1.0f);
}
NK_API struct nk_color
nk_hsv_fv(const float *c)
{
    return nk_hsv_f(c[0], c[1], c[2]);
}
NK_API struct nk_color
nk_hsva(int h, int s, int v, int a)
{
    float hf = ((float)NK_CLAMP(0, h, 255)) / 255.0f;
    float sf = ((float)NK_CLAMP(0, s, 255)) / 255.0f;
    float vf = ((float)NK_CLAMP(0, v, 255)) / 255.0f;
    float af = ((float)NK_CLAMP(0, a, 255)) / 255.0f;
    return nk_hsva_f(hf, sf, vf, af);
}
NK_API struct nk_color
nk_hsva_iv(const int *c)
{
    return nk_hsva(c[0], c[1], c[2], c[3]);
}
NK_API struct nk_color
nk_hsva_bv(const nk_byte *c)
{
    return nk_hsva(c[0], c[1], c[2], c[3]);
}
NK_API struct nk_colorf
nk_hsva_colorf(float h, float s, float v, float a)
{
    int i;
    float p, q, t, f;
    struct nk_colorf out = {0,0,0,0};
    if (s <= 0.0f) {
        out.r = v; out.g = v; out.b = v; out.a = a;
        return out;
    }
    h = h / (60.0f/360.0f);
    i = (int)h;
    f = h - (float)i;
    p = v * (1.0f - s);
    q = v * (1.0f - (s * f));
    t = v * (1.0f - s * (1.0f - f));

    switch (i) {
    case 0: default: out.r = v; out.g = t; out.b = p; break;
    case 1: out.r = q; out.g = v; out.b = p; break;
    case 2: out.r = p; out.g = v; out.b = t; break;
    case 3: out.r = p; out.g = q; out.b = v; break;
    case 4: out.r = t; out.g = p; out.b = v; break;
    case 5: out.r = v; out.g = p; out.b = q; break;}
    out.a = a;
    return out;
}
NK_API struct nk_colorf
nk_hsva_colorfv(float *c)
{
    return nk_hsva_colorf(c[0], c[1], c[2], c[3]);
}
NK_API struct nk_color
nk_hsva_f(float h, float s, float v, float a)
{
    struct nk_colorf c = nk_hsva_colorf(h, s, v, a);
    return nk_rgba_f(c.r, c.g, c.b, c.a);
}
NK_API struct nk_color
nk_hsva_fv(const float *c)
{
    return nk_hsva_f(c[0], c[1], c[2], c[3]);
}
NK_API nk_uint
nk_color_u32(struct nk_color in)
{
    nk_uint out = (nk_uint)in.r;
    out |= ((nk_uint)in.g << 8);
    out |= ((nk_uint)in.b << 16);
    out |= ((nk_uint)in.a << 24);
    return out;
}
NK_API void
nk_color_f(float *r, float *g, float *b, float *a, struct nk_color in)
{
    NK_STORAGE const float s = 1.0f/255.0f;
    *r = (float)in.r * s;
    *g = (float)in.g * s;
    *b = (float)in.b * s;
    *a = (float)in.a * s;
}
NK_API void
nk_color_fv(float *c, struct nk_color in)
{
    nk_color_f(&c[0], &c[1], &c[2], &c[3], in);
}
NK_API struct nk_colorf
nk_color_cf(struct nk_color in)
{
    struct nk_colorf o;
    nk_color_f(&o.r, &o.g, &o.b, &o.a, in);
    return o;
}
NK_API void
nk_color_d(double *r, double *g, double *b, double *a, struct nk_color in)
{
    NK_STORAGE const double s = 1.0/255.0;
    *r = (double)in.r * s;
    *g = (double)in.g * s;
    *b = (double)in.b * s;
    *a = (double)in.a * s;
}
NK_API void
nk_color_dv(double *c, struct nk_color in)
{
    nk_color_d(&c[0], &c[1], &c[2], &c[3], in);
}
NK_API void
nk_color_hsv_f(float *out_h, float *out_s, float *out_v, struct nk_color in)
{
    float a;
    nk_color_hsva_f(out_h, out_s, out_v, &a, in);
}
NK_API void
nk_color_hsv_fv(float *out, struct nk_color in)
{
    float a;
    nk_color_hsva_f(&out[0], &out[1], &out[2], &a, in);
}
NK_API void
nk_colorf_hsva_f(float *out_h, float *out_s,
    float *out_v, float *out_a, struct nk_colorf in)
{
    float chroma;
    float K = 0.0f;
    if (in.g < in.b) {
        const float t = in.g; in.g = in.b; in.b = t;
        K = -1.f;
    }
    if (in.r < in.g) {
        const float t = in.r; in.r = in.g; in.g = t;
        K = -2.f/6.0f - K;
    }
    chroma = in.r - ((in.g < in.b) ? in.g: in.b);
    *out_h = NK_ABS(K + (in.g - in.b)/(6.0f * chroma + 1e-20f));
    *out_s = chroma / (in.r + 1e-20f);
    *out_v = in.r;
    *out_a = in.a;

}
NK_API void
nk_colorf_hsva_fv(float *hsva, struct nk_colorf in)
{
    nk_colorf_hsva_f(&hsva[0], &hsva[1], &hsva[2], &hsva[3], in);
}
NK_API void
nk_color_hsva_f(float *out_h, float *out_s,
    float *out_v, float *out_a, struct nk_color in)
{
    struct nk_colorf col;
    nk_color_f(&col.r,&col.g,&col.b,&col.a, in);
    nk_colorf_hsva_f(out_h, out_s, out_v, out_a, col);
}
NK_API void
nk_color_hsva_fv(float *out, struct nk_color in)
{
    nk_color_hsva_f(&out[0], &out[1], &out[2], &out[3], in);
}
NK_API void
nk_color_hsva_i(int *out_h, int *out_s, int *out_v,
                int *out_a, struct nk_color in)
{
    float h,s,v,a;
    nk_color_hsva_f(&h, &s, &v, &a, in);
    *out_h = (nk_byte)(h * 255.0f);
    *out_s = (nk_byte)(s * 255.0f);
    *out_v = (nk_byte)(v * 255.0f);
    *out_a = (nk_byte)(a * 255.0f);
}
NK_API void
nk_color_hsva_iv(int *out, struct nk_color in)
{
    nk_color_hsva_i(&out[0], &out[1], &out[2], &out[3], in);
}
NK_API void
nk_color_hsva_bv(nk_byte *out, struct nk_color in)
{
    int tmp[4];
    nk_color_hsva_i(&tmp[0], &tmp[1], &tmp[2], &tmp[3], in);
    out[0] = (nk_byte)tmp[0];
    out[1] = (nk_byte)tmp[1];
    out[2] = (nk_byte)tmp[2];
    out[3] = (nk_byte)tmp[3];
}
NK_API void
nk_color_hsva_b(nk_byte *h, nk_byte *s, nk_byte *v, nk_byte *a, struct nk_color in)
{
    int tmp[4];
    nk_color_hsva_i(&tmp[0], &tmp[1], &tmp[2], &tmp[3], in);
    *h = (nk_byte)tmp[0];
    *s = (nk_byte)tmp[1];
    *v = (nk_byte)tmp[2];
    *a = (nk_byte)tmp[3];
}
NK_API void
nk_color_hsv_i(int *out_h, int *out_s, int *out_v, struct nk_color in)
{
    int a;
    nk_color_hsva_i(out_h, out_s, out_v, &a, in);
}
NK_API void
nk_color_hsv_b(nk_byte *out_h, nk_byte *out_s, nk_byte *out_v, struct nk_color in)
{
    int tmp[4];
    nk_color_hsva_i(&tmp[0], &tmp[1], &tmp[2], &tmp[3], in);
    *out_h = (nk_byte)tmp[0];
    *out_s = (nk_byte)tmp[1];
    *out_v = (nk_byte)tmp[2];
}
NK_API void
nk_color_hsv_iv(int *out, struct nk_color in)
{
    nk_color_hsv_i(&out[0], &out[1], &out[2], in);
}
NK_API void
nk_color_hsv_bv(nk_byte *out, struct nk_color in)
{
    int tmp[4];
    nk_color_hsv_i(&tmp[0], &tmp[1], &tmp[2], in);
    out[0] = (nk_byte)tmp[0];
    out[1] = (nk_byte)tmp[1];
    out[2] = (nk_byte)tmp[2];
}

NK_API void nk_name_color_init(struct nk_name_color *cn, const char *n, struct nk_color c)
{
    int len;

    NK_ASSERT(cn);
    NK_ASSERT(n);

    if (!cn || !n)
        return;

    len = NK_MIN(nk_strlen(n), NK_NAME_COLOR_MAX_NAME - 1);
    cn->name = nk_murmur_hash(n, len, NK_COLOR_INLINE_TAG);
    NK_MEMCPY(cn->name_string, n, len);
    cn->name_string[len] = '\0';
    cn->color = c;
}

#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
NK_API void nk_map_name_color_init_default(struct nk_map_name_color *c)
{
    struct nk_allocator alloc;

    NK_ASSERT(c);

    if (!c)
        return;

    alloc.userdata.ptr = 0;
    alloc.alloc = nk_malloc;
    alloc.free = nk_mfree;
    nk_buffer_init(&c->buffer, &alloc, NK_DEFAULT_MAP_NAME_COLOR_BUFFER_SIZE);
    c->count = 0;
}

#endif

NK_API void nk_map_name_color_init(struct nk_map_name_color *c, const struct nk_allocator *a, const struct nk_name_color *cv, int cc)
{
    nk_size size;

    NK_ASSERT(c);
    NK_ASSERT(a);

    if (!c || !a)
        return;

    if (cc == 0) {
        nk_buffer_init(&c->buffer, a, NK_DEFAULT_MAP_NAME_COLOR_BUFFER_SIZE);
        c->count = 0;
        return;
    }

    NK_ASSERT(cv);
    if (!cv)
        return;

    size = sizeof(struct nk_name_color) * cc;
    nk_buffer_init(&c->buffer, a, size);
    nk_buffer_push(&c->buffer, NK_BUFFER_FRONT, cv, size, sizeof(nk_hash));
    c->count = cc;
}

NK_API void nk_map_name_color_init_colors(struct nk_map_name_color *c, const struct nk_allocator *a, const char **nv, struct nk_color *cv, int cc)
{
    nk_size size;
    struct nk_name_color *m;
    int i;

    NK_ASSERT(c);
    NK_ASSERT(a);

    if (!c || !a)
        return;

    if (cc == 0) {
        nk_buffer_init(&c->buffer, a, NK_DEFAULT_MAP_NAME_COLOR_BUFFER_SIZE);
        c->count = 0;
        return;
    }

    NK_ASSERT(nv);
    NK_ASSERT(cv);
    if (!nv || !cv)
        return;

    size = sizeof(struct nk_name_color) * cc;
    nk_buffer_init(&c->buffer, a, size);
    nk_buffer_alloc(&c->buffer, NK_BUFFER_FRONT, size, sizeof(nk_hash));
    m = (struct nk_name_color *)c->buffer.memory.ptr;
    for (i = 0; i < cc; ++i)
        nk_name_color_init(&m[i], nv[i], cv[i]);
    c->count = cc;
}

NK_API void nk_map_name_color_init_map_name_color(struct nk_map_name_color *c0, const struct nk_allocator *a, const struct nk_map_name_color *c1, const char **filter_out, int count)
{
    nk_size size;
    struct nk_name_color *cv1;
    int i, j, len, hashes_count;
    nk_bool filtered;
    nk_hash hashes[32];

    NK_ASSERT(c0);
    NK_ASSERT(a);
    NK_ASSERT(c1);

    if (!c0 || !a || !c1)
        return;

    if (c1->count == 0) {
        nk_buffer_init(&c0->buffer, a, NK_DEFAULT_MAP_NAME_COLOR_BUFFER_SIZE);
        c0->count = 0;
        return;
    }

    size = sizeof(struct nk_name_color) * c1->count;
    cv1 = (struct nk_name_color *)c1->buffer.memory.ptr;

    if (count == 0) {
        nk_buffer_init(&c0->buffer, a, size);
        nk_buffer_push(&c0->buffer, NK_BUFFER_FRONT, cv1, size, sizeof(nk_hash));
        c0->count = c1->count;
        return;
    }

    NK_ASSERT(filter_out);
    if (!filter_out)
        return;

    nk_buffer_init(&c0->buffer, a, size);

    c0->count = 0;
    hashes_count = NK_MIN((int)NK_LEN(hashes), count);
    for (j = 0; j < hashes_count; ++j) {
        len = NK_MIN(nk_strlen(filter_out[j]), NK_NAME_COLOR_MAX_NAME - 1);
        hashes[j] = nk_murmur_hash(filter_out[j], len, NK_COLOR_INLINE_TAG);
    }

    for (i = 0; i < c1->count; ++i) {
        filtered = 0;
        for (j = 0; j < hashes_count; ++j) {
            if (hashes[j] == cv1[i].name) {
                if (nk_stricmpn(cv1[i].name_string, filter_out[j], NK_NAME_COLOR_MAX_NAME - 1) == 0) {
                    filtered = 1;
                    break;
                }
            }
        }
        if (!filtered) {
            nk_buffer_push(&c0->buffer, NK_BUFFER_FRONT, &cv1[i], sizeof(struct nk_name_color), sizeof(nk_hash));
            ++c0->count;
        }
    }

    if (count > hashes_count)
        nk_map_name_color_delete(c0, filter_out + hashes_count, count - hashes_count);
}

NK_API void nk_map_name_color_init_fixed(struct nk_map_name_color *c, struct nk_name_color *cv, int count, int capacity)
{
    NK_ASSERT(c);
    NK_ASSERT(cv);
    NK_ASSERT(count <= capacity);
    NK_ASSERT(capacity != 0);

    if (!c || !cv || count > capacity || capacity == 0)
        return;

    nk_buffer_init_fixed(&c->buffer, cv, capacity * sizeof(struct nk_name_color));
    c->buffer.allocated = count * sizeof(struct nk_name_color);
    c->count = count;
}

NK_API void nk_map_name_color_free(struct nk_map_name_color *c)
{
    NK_ASSERT(c);

    if (!c)
        return;

    nk_buffer_free(&c->buffer);
    c->count = 0;
}

NK_API void nk_map_name_color_push(struct nk_map_name_color *c, const struct nk_name_color *cv, int cc)
{
    nk_size size;
    void *mem;

    NK_ASSERT(c);

    if (!c)
        return;

    if (cc == 0)
        return;
    NK_ASSERT(cv);
    if (!cv)
        return;

    size = cc * sizeof(struct nk_name_color);
    mem = nk_buffer_alloc(&c->buffer, NK_BUFFER_FRONT, size, sizeof(nk_hash));
    if (!mem)
        return;
    NK_MEMCPY(mem, cv, size);
    c->count += cc;
}

NK_API void nk_map_name_color_push_colors(struct nk_map_name_color *c, const char **nv, struct nk_color *cv, int cc)
{
    nk_size size;
    void *mem;
    struct nk_name_color *m;
    int i;

    NK_ASSERT(c);

    if (!c)
        return;

    if (cc == 0)
        return;
    NK_ASSERT(nv);
    NK_ASSERT(cv);
    if (!nv || !cv)
        return;

    size = sizeof(struct nk_name_color) * cc;
    mem = nk_buffer_alloc(&c->buffer, NK_BUFFER_FRONT, size, sizeof(nk_hash));
    if (!mem)
        return;
    m = (struct nk_name_color *)mem;
    for (i = 0; i < cc; ++i)
        nk_name_color_init(&m[i], nv[i], cv[i]);
    c->count += cc;
}

NK_API void nk_map_name_color_push_map_name_color(struct nk_map_name_color *c0, const struct nk_map_name_color *c1)
{
    nk_size size;
    void *mem;

    NK_ASSERT(c0);
    NK_ASSERT(c1);

    if (!c0 || !c1)
        return;

    size = c1->count * sizeof(struct nk_name_color);
    mem = nk_buffer_alloc(&c0->buffer, NK_BUFFER_FRONT, size, sizeof(nk_hash));
    if (!mem)
        return;
    NK_MEMCPY(mem, c1->buffer.memory.ptr, size);
    c0->count += c1->count;
}

NK_API void nk_map_name_color_delete(struct nk_map_name_color *c, const char **filter_out, int count)
{
    nk_size size;
    int hashes_count, out_count = 0;
    nk_bool filtered;
    int out_begin, out_end, i, j, len, out_state;
    struct nk_name_color *cv;
    nk_hash hashes[32];

    NK_ASSERT(c);

    if (!c || c->count == 0)
        return;

    if (count == 0)
        return;
    NK_ASSERT(filter_out);
    if (!filter_out)
        return;

    cv = (struct nk_name_color *)c->buffer.memory.ptr;
    while (count > 0) {
        hashes_count = NK_MIN((int)NK_LEN(hashes), count);

        for (j = 0; j < hashes_count; ++j) {
            len = NK_MIN(nk_strlen(filter_out[j]), NK_NAME_COLOR_MAX_NAME - 1);
            hashes[j] = nk_murmur_hash(filter_out[j], len, NK_COLOR_INLINE_TAG);
        }

        out_state = -1;
        for (i = 0; i < c->count; ++i) {
            filtered = 0;
            for (j = 0; j < hashes_count; ++j) {
                if (hashes[j] == cv[i].name) {
                    if (nk_stricmpn(cv[i].name_string, filter_out[j], NK_NAME_COLOR_MAX_NAME - 1) == 0) {
                        filtered = 1;
                        break;
                    }
                }
            }
            if (filtered) {
                if (out_state == 1) {
                    /* memmove */
                    size = (i - out_end) * sizeof(struct nk_name_color);
                    NK_MEMCPY(&cv[out_begin], &cv[out_end], size);
                    out_count += out_end - out_begin;
                }
                if (out_state != 0) {
                    out_state = 0;
                    out_begin = i;
                }
            } else {
                if (out_state == 0) {
                    out_state = 1;
                    out_end = i;
                }
            }
        }
        /* copy final stretch if needed */
        if (out_state == 1) {
            size = (i - out_end) * sizeof(struct nk_name_color);
            NK_MEMCPY(&cv[out_begin], &cv[out_end], size);
            out_count += out_end - out_begin;
        } else if (out_state == 0) {
            out_count += i - out_begin;
        }

        c->buffer.allocated -= out_count * sizeof(struct nk_name_color);
        c->count -= out_count;

        filter_out += hashes_count;
        count -= hashes_count;
    }
}

NK_API void nk_map_name_color_clear(struct nk_map_name_color *c)
{
    NK_ASSERT(c);

    if (!c)
        return;

    nk_buffer_clear(&c->buffer);
    c->count = 0;
}
