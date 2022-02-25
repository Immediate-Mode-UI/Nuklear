#include "nuklear.h"
#include "nuklear_internal.h"

/* ===============================================================
 *
 *                              UTF-8
 *
 * ===============================================================*/
NK_GLOBAL const nk_byte nk_utfbyte[NK_UTF_SIZE+1] = {0x80, 0, 0xC0, 0xE0, 0xF0};
NK_GLOBAL const nk_byte nk_utfmask[NK_UTF_SIZE+1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
NK_GLOBAL const nk_uint nk_utfmin[NK_UTF_SIZE+1] = {0, 0, 0x80, 0x800, 0x10000};
NK_GLOBAL const nk_uint nk_utfmax[NK_UTF_SIZE+1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};

NK_INTERN int
nk_utf_validate(nk_rune *u, int i)
{
    NK_ASSERT(u);
    if (!u) return 0;
    if (!NK_BETWEEN(*u, nk_utfmin[i], nk_utfmax[i]) ||
         NK_BETWEEN(*u, 0xD800, 0xDFFF))
            *u = NK_UTF_INVALID;
    for (i = 1; *u > nk_utfmax[i]; ++i);
    return i;
}
NK_INTERN nk_rune
nk_utf_decode_byte(char c, nk_size *i)
{
    NK_ASSERT(i);
    if (!i) return 0;
    for(*i = 0; *i < (int)NK_LEN(nk_utfmask); ++(*i)) {
        if (((nk_byte)c & nk_utfmask[*i]) == nk_utfbyte[*i])
            return (nk_byte)(c & ~nk_utfmask[*i]);
    }
    return 0;
}
NK_API int
nk_utf_decode(struct nk_slice str, nk_rune *u)
{
    nk_size i, j, len, type=0;
    nk_rune udecoded;

    NK_ASSERT(str.ptr);
    NK_ASSERT(u);

    if (!str.ptr || !u) return 0;
    if (!str.len) return 0;
    *u = NK_UTF_INVALID;

    udecoded = nk_utf_decode_byte(str.ptr[0], &len);
    if (!NK_BETWEEN(len, 1, NK_UTF_SIZE))
        return 1;

    for (i = 1, j = 1; i < str.len && j < len; ++i, ++j) {
        udecoded = (udecoded << 6) | nk_utf_decode_byte(str.ptr[i], &type);
        if (type != 0)
            return j;
    }
    if (j < len)
        return 0;
    *u = udecoded;
    nk_utf_validate(u, len);
    return len;
}
NK_INTERN char
nk_utf_encode_byte(nk_rune u, int i)
{
    return (char)((nk_utfbyte[i]) | ((nk_byte)u & ~nk_utfmask[i]));
}
NK_API int
nk_utf_encode(nk_rune u, char *c, int clen)
{
    int len, i;
    len = nk_utf_validate(&u, 0);
    if (clen < len || !len || len > NK_UTF_SIZE)
        return 0;

    for (i = len - 1; i != 0; --i) {
        c[i] = nk_utf_encode_byte(u, 0);
        u >>= 6;
    }
    c[0] = nk_utf_encode_byte(u, len);
    return len;
}
NK_API int
nk_utf_len(struct nk_slice str)
{
    int glyphs = 0;
    int glyph_len;
    nk_size src_len = 0;
    nk_rune unicode;

    NK_ASSERT(str.ptr);
    if (!str.ptr || !str.len) return 0;

    glyph_len = nk_utf_decode(str, &unicode);
    while (glyph_len && src_len < str.len) {
        glyphs++;
        src_len = src_len + glyph_len;
        glyph_len = nk_utf_decode(nk_substr(str, src_len, str.len), &unicode);
    }
    return glyphs;
}
NK_API struct nk_slice
nk_utf_at(struct nk_slice buffer, int index,
    nk_rune *unicode)
{
    int i = 0;
    int src_len = 0;
    int glyph_len = 0;

    NK_ASSERT(buffer.ptr);
    NK_ASSERT(unicode);

    if (!buffer.ptr || !unicode) return nk_slicez("");
    if (index < 0) {
        *unicode = NK_UTF_INVALID;
        return nk_slicez("");
    }

    glyph_len = nk_utf_decode(buffer, unicode);
    while (glyph_len && i != index) {
        i++;
        src_len = src_len + glyph_len;
        glyph_len = nk_utf_decode(nk_substr(buffer, src_len, buffer.len), unicode);
    }
    if (i != index) return nk_slicez("");
    return nk_substr(buffer, src_len, src_len + glyph_len);
}

