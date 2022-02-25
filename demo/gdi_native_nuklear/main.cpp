#include <Windows.h>
#include <iostream>

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
    NkGdi::Window w1(500, 500, "F1", 10, 10);
    w1.AllowSizing = false;
    w1.AllowMaximize = false;
    w1.AllowMove = false;
    w1.HasTitlebar = false;

    NkGdi::Window w2(500, 500, "F2", 520, 10);
    w2.AllowSizing = true;
    w2.AllowMaximize = true;
    w2.AllowMove = true;
    w2.HasTitlebar = true;
    
    while (w1.Update() && w2.Update()) Sleep(20);

    return 0;
}
