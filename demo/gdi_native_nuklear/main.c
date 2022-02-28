#include <Windows.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_GDI_IMPLEMENTATION
#include "../../nuklear.h"
#include "nuklear_gdi.h"

#define NKGDI_IMPLEMENT_WINDOW
#include "window.h"

int drawCallback(struct nk_context* ctx)
{
    nk_label(ctx, "Label", NK_TEXT_ALIGN_CENTERED);
    nk_button_label(ctx, "Test 1234");
    return 1;
}

INT WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, PWSTR _In_ cmdArgs, INT _In_ cmdShow)
{
    nkgdi_window_init();
    struct nkgdi_window w1, w2;
    memset(&w1, 0x0, sizeof(struct nkgdi_window));
    memset(&w2, 0x0, sizeof(struct nkgdi_window));

    w1.allow_sizing = 0;
    w1.allow_maximize = 0;
    w1.allow_move = 0;
    w1.has_titlebar = 0;
    w1.cb_on_draw = &drawCallback;
    nkgdi_window_create(&w1, 500, 500, "F1", 10, 10);

    w2.allow_sizing = 1;
    w2.allow_maximize = 1;
    w2.allow_move = 1;
    w2.has_titlebar = 1;
    w2.cb_on_draw = &drawCallback;
    nkgdi_window_create(&w2, 500, 500, "F2", 520, 10);
    
    while (nkgdi_window_update(&w1) && nkgdi_window_update(&w2)) Sleep(20);

    nkgdi_window_destroy(&w1);
    nkgdi_window_destroy(&w2);

    nkgdi_window_shutdown();
    return 0;
}
