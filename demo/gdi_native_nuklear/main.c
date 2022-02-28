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

INT WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, PWSTR _In_ cmdArgs, INT _In_ cmdShow)
{
    struct nkgdi_window w1, w2;

    w1.allow_sizing = 0;
    w1.allow_maximize = 0;
    w1.allow_move = 0;
    w1.has_titlebar = 0;
    nkgdi_window_create(&w1, 500, 500, "F1", 10, 10);

    w2.allow_sizing = 1;
    w2.allow_maximize = 1;
    w2.allow_move = 1;
    w2.has_titlebar = 1;
    nkgdi_window_create(&w2, 500, 500, "F2", 520, 10);
    
    while (nkgdi_window_update(&w1) && nkgdi_window_update(&w2)) Sleep(20);

    return 0;
}
