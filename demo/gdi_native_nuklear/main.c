#include <windows.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

/* Includes the default nuklear implementation
 * Includes the modified GDI backend (No more global state to allow multiple hwnd's)
 */ 
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_GDI_IMPLEMENTATION
#include "../../nuklear.h"
#include "nuklear_gdi.h"

/* Include the window framework (the new fancy code of this demo) */
#define NKGDI_IMPLEMENT_WINDOW
#include "window.h"

/* This callback will be called when the window is draw 
 * You will NOT need to call nk_begin(...) and nk_end(...)
 * begin and end are handled by the parent code calling this
 * callback
 */
int drawCallback(struct nk_context* ctx)
{
    /* Code is from ../calculator.c */
    static int set = 0, prev = 0, op = 0;
    static const char numbers[] = "789456123";
    static const char ops[] = "+-*/";
    static double a = 0, b = 0;
    static double *current = &a;

    size_t i = 0;
    int solve = 0;
    {int len; char buffer[256];
    nk_layout_row_dynamic(ctx, 35, 1);
    len = snprintf(buffer, 256, "%.2f", *current);
    nk_edit_string(ctx, NK_EDIT_SIMPLE, buffer, &len, 255, nk_filter_float);
    buffer[len] = 0;
    *current = atof(buffer);}

    nk_layout_row_dynamic(ctx, 35, 4);
    for (i = 0; i < 16; ++i) {
        if (i >= 12 && i < 15) {
            if (i > 12) continue;
            if (nk_button_label(ctx, "C")) {
                a = b = op = 0; current = &a; set = 0;
            } if (nk_button_label(ctx, "0")) {
                *current = *current*10.0f; set = 0;
            } if (nk_button_label(ctx, "=")) {
                solve = 1; prev = op; op = 0;
            }
        } else if (((i+1) % 4)) {
            if (nk_button_text(ctx, &numbers[(i/4)*3+i%4], 1)) {
                *current = *current * 10.0f + numbers[(i/4)*3+i%4] - '0';
                set = 0;
            }
        } else if (nk_button_text(ctx, &ops[i/4], 1)) {
            if (!set) {
                if (current != &b) {
                    current = &b;
                } else {
                    prev = op;
                    solve = 1;
                }
            }
            op = ops[i/4];
            set = 1;
        }
    }
    if (solve) {
        if (prev == '+') a = a + b;
        if (prev == '-') a = a - b;
        if (prev == '*') a = a * b;
        if (prev == '/') a = a / b;
        current = &a;
        if (set) current = &b;
        b = 0; set = 0;
    }
    return 1;
}

/* Main entry point - wWinMain used for UNICODE
 * (You can also use _tWinMain(...) to automaticaly use the ASCII or WIDE char entry point base on your build)  
 */
INT WINAPI wWinMain(HINSTANCE _In_ hInstance, HINSTANCE _In_opt_ hPrevInstance, PWSTR _In_ cmdArgs, INT _In_ cmdShow)
{
    /* Call this first to setup all required prerequisites */
    nkgdi_window_init();
	
    /* Preparing two window contexts */
    struct nkgdi_window w1, w2;
    memset(&w1, 0x0, sizeof(struct nkgdi_window));
    memset(&w2, 0x0, sizeof(struct nkgdi_window));
	
    /* Configure and create window 1. 
     * Note: You can allways change the direct accesible parameters later as well! 
     */
    w1.allow_sizing = 0;
    w1.allow_maximize = 0;
    w1.allow_move = 0;
    w1.has_titlebar = 0;
    w1.cb_on_draw = &drawCallback;
    nkgdi_window_create(&w1, 500, 500, "F1", 10, 10);

    /* Configure and create window 2 */
    w2.allow_sizing = 1;
    w2.allow_maximize = 1;
    w2.allow_move = 1;
    w2.has_titlebar = 1;
    w2.cb_on_draw = &drawCallback;
    nkgdi_window_create(&w2, 500, 500, "F2", 520, 10);
    
    /* As long as both windows are valid (nkgdi_window_update returning 1) */
    while (nkgdi_window_update(&w1) && nkgdi_window_update(&w2)) Sleep(20);

    /* Destroy both windows context */
    nkgdi_window_destroy(&w1);
    nkgdi_window_destroy(&w2);

    /* Call nkgdi_window_shutdown to properly shutdown the gdi window framework */
    nkgdi_window_shutdown();
    return 0;
}
