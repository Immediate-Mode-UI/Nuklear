/* nuklear - public domain */

/* ===============================================================
 *
 *                          CONFIG
 *
 * ===============================================================*/

#ifndef NK_SDL3_RENDERER_CONFIG_H_
#define NK_SDL3_RENDERER_CONFIG_H_

#include <SDL3/SDL.h>

/* optional: sdl3_renderer does not need any of these defines
 * (but some examples might need them, so be careful) */
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_IO

/* note that sdl3_renderer comes with nk_sdl_style_set_debug_font()
 * so you may wish to use that instead of font baking */
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

/* note that sdl3_renderer comes with nk_sdl_allocator()
 * and you probably want to use that allocator instead of the default ones */
/*#define NK_INCLUDE_DEFAULT_ALLOCATOR*/

/* mandatory: sdl3_renderer depends on those defines */
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT


/* We can re-use the types provided by SDL which are extremely portable,
 * so there is no need for Nuklear to detect those on its own */
/*#define NK_INCLUDE_FIXED_TYPES*/
#ifndef NK_INCLUDE_FIXED_TYPES
    #define NK_INT8              Sint8
    #define NK_UINT8             Uint8
    #define NK_INT16             Sint16
    #define NK_UINT16            Uint16
    #define NK_INT32             Sint32
    #define NK_UINT32            Uint32
    /* SDL guarantees 'uintptr_t' typedef */
    #define NK_SIZE_TYPE         uintptr_t
    #define NK_POINTER_TYPE      uintptr_t
#endif

/* We can reuse the `bool` symbol because SDL3 guarantees its existence */
/*#define NK_INCLUDE_STANDARD_BOOL*/
#ifndef NK_INCLUDE_STANDARD_BOOL
    #define NK_BOOL               bool
#endif

/* We can re-use various portable libc functions provided by SDL */
#define NK_ASSERT(condition)      SDL_assert(condition)
#define NK_STATIC_ASSERT(exp)     SDL_COMPILE_TIME_ASSERT(, exp)
#define NK_MEMSET(dst, c, len)    SDL_memset(dst, c, len)
#define NK_MEMCPY(dst, src, len)  SDL_memcpy(dst, src, len)
#define NK_VSNPRINTF(s, n, f, a)  SDL_vsnprintf(s, n, f, a)
#define NK_STRTOD(str, endptr)    SDL_strtod(str, endptr)

/* SDL3 does not provide "dtoa" (only integer versions)
 * but we can emulate it with SDL_snprintf */
SDL_FORCE_INLINE char*
nk_sdl_dtoa(char *str, double d)
{
    NK_ASSERT(str);
    if (!str) return NULL;
    (void)SDL_snprintf(str, 99999, "%.17g", d);
    return str;
}
#define NK_DTOA(str, d) nk_sdl_dtoa(str, d)

/* SDL can also provide us with math functions, but beware that Nuklear's own
 * implementation can be slightly faster at the cost of some precision */
#define NK_INV_SQRT(f)            (1.0f / SDL_sqrtf(f))
#define NK_SIN(f)                 SDL_sinf(f)
#define NK_COS(f)                 SDL_cosf(f)

/* HACK: Nuklear pulls two stb libraries in order to use font baking
 * those libraries pull in some libc headers internally, creating a linkage dependency,
 * so you’ll most likely want to use SDL symbols instead */
#define STBTT_ifloor(x)           ((int)SDL_floor(x))
#define STBTT_iceil(x)            ((int)SDL_ceil(x))
#define STBTT_sqrt(x)             SDL_sqrt(x)
#define STBTT_pow(x,y)            SDL_pow(x,y)
#define STBTT_fmod(x,y)           SDL_fmod(x,y)
#define STBTT_cos(x)              SDL_cosf(x)
#define STBTT_acos(x)             SDL_acos(x)
#define STBTT_fabs(x)             SDL_fabs(x)
#define STBTT_assert(x)           SDL_assert(x)
#define STBTT_strlen(x)           SDL_strlen(x)
#define STBTT_memcpy              SDL_memcpy
#define STBTT_memset              SDL_memset
#define stbtt_uint8               Uint8
#define stbtt_int8                Sint8
#define stbtt_uint16              Uint16
#define stbtt_int16               Sint16
#define stbtt_uint32              Uint32
#define stbtt_int32               Sint32
#define STBRP_SORT                SDL_qsort
#define STBRP_ASSERT              SDL_assert
/* There is no need to define STBTT_malloc/STBTT_free macros
 * Nuklear will define those to user-provided nk_allocator */

#endif /* NK_SDL3_RENDERER_CONFIG_H_ */

