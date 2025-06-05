# Nuklear

[![](https://github.com/Immediate-Mode-UI/Nuklear/workflows/C%2FC++%20CI/badge.svg )](https://github.com/Immediate-Mode-UI/Nuklear/actions)

This is a minimal-state, immediate-mode graphical user interface toolkit
written in ANSI C and licensed under public domain. It was designed as a simple
embeddable user interface for application and does not have any dependencies,
a default render backend or OS window/input handling but instead provides a
highly modular, library-based approach, with simple input state for input and
draw commands describing primitive shapes as output. So instead of providing a
layered library that tries to abstract over a number of platform and
render backends, it focuses only on the actual UI.

## Features

- Immediate-mode graphical user interface toolkit
- Single-header library
- Written in C89 (ANSI C)
- Small codebase (~18kLOC)
- Focus on portability, efficiency and simplicity
- No dependencies (not even the standard library if not wanted)
- Fully skinnable and customizable
- Low memory footprint with total control of memory usage if needed / wanted
- UTF-8 support
- No global or hidden state
- Customizable library modules (you can compile and use only what you need)
- Optional font baker and vertex buffer output
- [Documentation](https://Immediate-Mode-UI.github.io/Nuklear/)

## Building

This library is self-contained in one single header file and can be used either
in header-only mode or in implementation mode. The header-only mode is used
by default when included and allows including this header in other headers
and does not contain the actual implementation.

The implementation mode requires defining the preprocessor macro
`NK_IMPLEMENTATION` in *one* .c/.cpp file before `#include`ing this file, e.g.:
```c
#define NK_IMPLEMENTATION
#include "nuklear.h"
```
IMPORTANT: Every time you include "nuklear.h" you have to define the same optional flags.
This is very important; not doing it either leads to compiler errors, or even worse, stack corruptions.

## Gallery

![screenshot](https://cloud.githubusercontent.com/assets/8057201/11761525/ae06f0ca-a0c6-11e5-819d-5610b25f6ef4.gif)
![screen](https://cloud.githubusercontent.com/assets/8057201/13538240/acd96876-e249-11e5-9547-5ac0b19667a0.png)
![screen2](https://cloud.githubusercontent.com/assets/8057201/13538243/b04acd4c-e249-11e5-8fd2-ad7744a5b446.png)
![node](https://cloud.githubusercontent.com/assets/8057201/9976995/e81ac04a-5ef7-11e5-872b-acd54fbeee03.gif)
![skinning](https://cloud.githubusercontent.com/assets/8057201/15991632/76494854-30b8-11e6-9555-a69840d0d50b.png)
![gamepad](https://cloud.githubusercontent.com/assets/8057201/14902576/339926a8-0d9c-11e6-9fee-a8b73af04473.png)

## Example

```c
/* init gui state */
struct nk_context ctx;
nk_init_fixed(&ctx, calloc(1, MAX_MEMORY), MAX_MEMORY, &font);

enum {EASY, HARD};
static int op = EASY;
static float value = 0.6f;
static int i =  20;

if (nk_begin(&ctx, "Show", nk_rect(50, 50, 220, 220),
    NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
    /* fixed widget pixel width */
    nk_layout_row_static(&ctx, 30, 80, 1);
    if (nk_button_label(&ctx, "button")) {
        /* event handling */
    }

    /* fixed widget window ratio width */
    nk_layout_row_dynamic(&ctx, 30, 2);
    if (nk_option_label(&ctx, "easy", op == EASY)) op = EASY;
    if (nk_option_label(&ctx, "hard", op == HARD)) op = HARD;

    /* custom widget pixel width */
    nk_layout_row_begin(&ctx, NK_STATIC, 30, 2);
    {
        nk_layout_row_push(&ctx, 50);
        nk_label(&ctx, "Volume:", NK_TEXT_LEFT);
        nk_layout_row_push(&ctx, 110);
        nk_slider_float(&ctx, 0, &value, 1.0f, 0.1f);
    }
    nk_layout_row_end(&ctx);
}
nk_end(&ctx);
```
![example](https://cloud.githubusercontent.com/assets/8057201/10187981/584ecd68-675c-11e5-897c-822ef534a876.png)

## Bindings
There are a number of nuklear bindings for different languages created by other authors.
I cannot attest for their quality since I am not necessarily proficient in any of these
languages. Furthermore there are no guarantee that all bindings will always be kept up to date:

- [Java](https://github.com/glegris/nuklear4j) by Guillaume Legris
- [D](https://github.com/Timu5/bindbc-nuklear) by Mateusz Muszyński
- [Golang](https://github.com/golang-ui/nuklear) by golang-ui@github.com
- [Rust](https://github.com/snuk182/nuklear-rust) by snuk182@github.com
- [Chicken](https://github.com/wasamasa/nuklear) by wasamasa@github.com
- [Nim](https://github.com/zacharycarter/nuklear-nim) by zacharycarter@github.com
- Lua
  - [LÖVE-Nuklear](https://github.com/keharriso/love-nuklear) by Kevin Harrison
  - [MoonNuklear](https://github.com/stetre/moonnuklear) by Stefano Trettel
- Python
  - [pyNuklear](https://github.com/billsix/pyNuklear) by William Emerison Six (ctypes-based wrapper)
  - [pynk](https://github.com/nathanrw/nuklear-cffi) by nathanrw@github.com (cffi binding)
- [CSharp/.NET](https://github.com/cartman300/NuklearDotNet) by cartman300@github.com
- [V](https://github.com/nsauzede/vnk) by Nicolas Sauzede

## Credits
Developed by Micha Mettke and every direct or indirect contributor to the GitHub.


Embeds `stb_texedit`, `stb_truetype` and `stb_rectpack` by Sean Barrett (public domain)
Embeds `ProggyClean.ttf` font by Tristan Grimmer (MIT license).


Big thank you to Omar Cornut (ocornut@github) for his [imgui](https://github.com/ocornut/imgui) library and
giving me the inspiration for this library, Casey Muratori for handmade hero
and his original immediate-mode graphical user interface idea and Sean
Barrett for his amazing single-header [libraries](https://github.com/nothings/stb) which restored my faith
in libraries and brought me to create some of my own. Finally Apoorva Joshi for his single-header [file packer](http://apoorvaj.io/single-header-packer.html).

## License
Nuklear is avaliable under either the MIT License or public domain.
See [LICENSE](LICENSE) for more info.

## Reviewers guide

When reviewing pull request there are common things a reviewer should keep
in mind.

Reviewing changes to `src/*` and `nuklear.h`:

* Ensure C89 compatibility.
* The code should work for several backends to an acceptable degree.
* Check no other parts of `nuklear.h` are related to the PR and thus nothing is missing.
* Recommend simple optimizations.
  * Pass small structs by value instead of by pointer.
  * Use local buffers over heap allocation when possible.
* Check that the coding style is consistent with code around it.
  * Variable/function name casing.
  * Indentation.
  * Curly bracket (`{}`) placement.
* Ensure that the contributor has bumped the appropriate version in
  [clib.json](https://github.com/Immediate-Mode-UI/Nuklear/blob/master/clib.json)
  and added their changes to the
  [CHANGELOG](https://github.com/Immediate-Mode-UI/Nuklear/blob/master/src/CHANGELOG).
* Have at least one other person review the changes before merging.

Reviewing changes to `demo/*`, `example/*` and other files in the repo:

* Focus on getting working code merged.
  * We want to make it easy for people to get started with Nuklear, and any
    `demo` and `example` improvements helps in this regard.
* Use of newer C features, or even other languages is not discouraged.
  * If another language is used, ensure that the build process is easy to figure out.
* Messy or less efficient code can be merged so long as these outliers are pointed out
  and easy to find.
* Version shouldn't be bumped for these changes.
* Changes that improves code to be more inline with `nuklear.h` are ofc always welcome.

