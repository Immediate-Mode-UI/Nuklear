#ifndef NK_GDI_WINDOW
#define NK_GDI_WINDOW

#define NK_GDI_WINDOW_CLS L"WNDCLS_NkGdi"

#include <Windows.h>

/* Functin pointer types for window callbacks */
typedef void(*nkgdi_window_func_update)(void);
typedef int(*nkgdi_window_func_close)(void);
typedef int(*nkgdi_window_func_draw)(struct nk_context*);

/* Window container / context */
struct nkgdi_window
{
    /* Properties */
    int allow_sizing;
    int allow_maximize;
    int allow_move;
    int has_titlebar;

    /* Callbacks */
    nkgdi_window_func_update cb_on_update;
    nkgdi_window_func_close cb_on_close;
    nkgdi_window_func_draw cb_on_draw;

    /* Internal Data */
    struct
    {
        // Window handle
        HWND window_handle;

        // Nuklear context
        nk_gdi_ctx nk_gdi_ctx;
        struct nk_context* nk_ctx;

        // Nuklear objects
        GdiFont* gdi_font;
        HDC window_dc;

        // Window runtime features
        int is_open;
        int is_draggin;
        int ws_override;
        int is_maximized;
        POINT drag_offset;
        int width;
        int height;
    }_internal;
};

/* API */
void nkgdi_window_init(void);
void nkgdi_window_shutdown(void);
void nkgdi_window_create(struct nkgdi_window* wnd, unsigned int width, unsigned int height, const char* name, int posX, int posY);
int nkgdi_window_update(struct nkgdi_window* wnd);
void nkgdi_window_destroy(struct nkgdi_window* wnd);

#ifdef NKGDI_IMPLEMENT_WINDOW

LRESULT nkgdi_window_proc_setup(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT nkgdi_window_proc_run(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

void nkgdi_window_init(void)
{
    // Describe class
    WNDCLASSEXW cls;
    cls.cbSize = sizeof(WNDCLASSEXW);
    cls.style = CS_OWNDC | CS_DBLCLKS;
    cls.lpfnWndProc = &nkgdi_window_proc_setup;
    cls.cbClsExtra = 0;
    cls.cbWndExtra = 0;
    cls.hInstance = GetModuleHandle(NULL);
    cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    cls.hCursor = LoadCursor(NULL, IDC_ARROW);
    cls.hbrBackground = NULL;
    cls.lpszMenuName = NULL;
    cls.lpszClassName = NK_GDI_WINDOW_CLS;
    cls.hIconSm = NULL;

    // Register class
    RegisterClassExW(&cls);
}

void nkgdi_window_shutdown(void)
{
    UnregisterClassW(NK_GDI_WINDOW_CLS, GetModuleHandle(NULL));
}

void nkgdi_window_create(struct nkgdi_window* wnd, unsigned int width, unsigned int height, const char* name, int posX, int posY)
{
    DWORD styleEx = WS_EX_WINDOWEDGE;
    DWORD style = WS_POPUP;

    // Compute window size
    RECT cr;
    cr.left = 0;
    cr.top = 0;
    cr.right = width;
    cr.bottom = height;
    AdjustWindowRectEx(&cr, style, FALSE, styleEx);

    // Create the window
    wnd->_internal.window_handle = CreateWindowExW(
        styleEx,
        NK_GDI_WINDOW_CLS,
        L"NkGdi",
        style | WS_VISIBLE,
        posX, posY,
        cr.right - cr.left, cr.bottom - cr.top,
        NULL, NULL,
        GetModuleHandleW(NULL),
        wnd
    );
 
    // Rename window to user picked name
    SetWindowTextA(wnd->_internal.window_handle, name);

    // Get DC
    wnd->_internal.window_dc = GetWindowDC(wnd->_internal.window_handle);

    // Create font
    wnd->_internal.gdi_font = nk_gdifont_create("Arial", 16);
    wnd->_internal.nk_ctx = nk_gdi_init(&wnd->_internal.nk_gdi_ctx, wnd->_internal.gdi_font, wnd->_internal.window_dc, width, height);

    // Setup internal data
    wnd->_internal.is_open = 1;
    wnd->_internal.is_draggin = 0;
    wnd->_internal.ws_override = 0;
    wnd->_internal.is_maximized = 0;
    wnd->_internal.drag_offset.x = 0;
    wnd->_internal.drag_offset.y = 0;
    wnd->_internal.width = width;
    wnd->_internal.height = height;
}

void nkgdi_window_destroy(struct nkgdi_window* wnd)
{
    // Destroy  GDI context
    if (wnd->_internal.nk_gdi_ctx)
    {
        nk_gdi_shutdown(wnd->_internal.nk_gdi_ctx);
    }

    // Destroy font
    if (wnd->_internal.gdi_font)
    {
        nk_gdifont_del(wnd->_internal.gdi_font);
    }

    // Close DC
    if (wnd->_internal.window_dc)
    {
        ReleaseDC(wnd->_internal.window_handle, wnd->_internal.window_dc);
    }

    // Destroy window
    if (wnd->_internal.window_handle)
    {
        CloseWindow(wnd->_internal.window_handle);
        DestroyWindow(wnd->_internal.window_handle);
    }
}

int nkgdi_window_update(struct nkgdi_window* wnd)
{
    // Only process events while window is open
    if (wnd->_internal.is_open)
    {
        // Windows event loop
        MSG msg;
        nk_input_begin(wnd->_internal.nk_ctx);
        while (PeekMessage(&msg, wnd->_internal.window_handle, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        nk_input_end(wnd->_internal.nk_ctx);

        // Get title
        char title[1024];
        GetWindowTextA(wnd->_internal.window_handle, title, 1024);

        // Window flags
        nk_flags window_flags = NK_WINDOW_BORDER;
        if(wnd->has_titlebar)
            window_flags |= NK_WINDOW_CLOSABLE | NK_WINDOW_TITLE;
        if(!wnd->_internal.is_maximized && wnd->allow_sizing)
            window_flags |= NK_WINDOW_SCALABLE;

        // Window body
        if (wnd->_internal.ws_override)
            nk_window_set_bounds(wnd->_internal.nk_ctx, title, nk_rect(0, 0, wnd->_internal.width, wnd->_internal.height));
        if (nk_begin(wnd->_internal.nk_ctx, title, nk_rect(0, 0, wnd->_internal.width, wnd->_internal.height), window_flags))
        {
            if(wnd->cb_on_draw && !wnd->cb_on_draw(wnd->_internal.nk_ctx))
                wnd->_internal.is_open = 0;

            // Update window size
            struct nk_rect bounds = nk_window_get_bounds(wnd->_internal.nk_ctx);
            if(bounds.w != wnd->_internal.width || bounds.h != wnd->_internal.height)
                SetWindowPos(wnd->_internal.window_handle, NULL, 0, 0, bounds.w, bounds.h, SWP_NOMOVE | SWP_NOOWNERZORDER);
        }
        else
        {
            // Handle window closing
            if(!wnd->cb_on_close || wnd->cb_on_close())
                wnd->_internal.is_open = 0;
        }
        nk_end(wnd->_internal.nk_ctx);
        wnd->_internal.ws_override = 0;

        // Final render pass
        nk_gdi_render(wnd->_internal.nk_gdi_ctx, nk_rgb(30, 30, 30));
    }

    return wnd->_internal.is_open;
}

LRESULT nkgdi_window_proc_setup(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Wait for setup message
    if (msg == WM_NCCREATE)
    {
        // Get creation parameters & window pointer
        CREATESTRUCT* ptrCr = (CREATESTRUCT*)lParam;
        struct nkgdi_window* nkgdi_wnd = (struct nkgdi_window*)ptrCr->lpCreateParams;

        // Store pointer and new proc in window
        SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)nkgdi_wnd);
        SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)&nkgdi_window_proc_run);

        // Handled by window
        return nkgdi_window_proc_run(wnd, msg, wParam, lParam);
    }

    // Default handler
    return DefWindowProc(wnd, msg, wParam, lParam);
}

LRESULT nkgdi_window_proc_run(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Get window pointer
    struct nkgdi_window* nkwnd = (struct nkgdi_window*)GetWindowLongPtrW(wnd, GWLP_USERDATA);
    
    // Switch on supplied message code
    switch (msg)
    {
        // Close event
        case WM_CLOSE:
            if(!nkwnd->cb_on_close || nkwnd->cb_on_close())
                nkwnd->_internal.is_open = 0;
            return 0; // Will always be handled internaly
        
        // While sizing
        case WM_SIZING:
            {
            RECT cr;
            GetClientRect(wnd, &cr);
            nkwnd->_internal.width = cr.right - cr.left;
            nkwnd->_internal.height = cr.bottom - cr.top;
            }
            break;

        // When sized
        case WM_SIZE:
            {
            // Adjust maximize properties
            if (wParam == SIZE_MAXIMIZED)
            {
                HMONITOR monitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY);
                MONITORINFO monitorInfo;
                monitorInfo.cbSize = sizeof(MONITORINFO);
                if (GetMonitorInfoW(monitor, &monitorInfo))
                {
                    nkwnd->_internal.height = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
                    nkwnd->_internal.width = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
                    nkwnd->_internal.ws_override = 1;
                    nkwnd->_internal.is_maximized = 1;
                    SetWindowPos(wnd, NULL, 0, 0, nkwnd->_internal.width, nkwnd->_internal.height, SWP_NOMOVE | SWP_NOZORDER);
                }
            }
            else if (wParam == SIZE_RESTORED)
            {
                nkwnd->_internal.is_maximized = 0;
            }

            // Compute new bounds
            RECT cr;
            GetClientRect(wnd, &cr);
            nkwnd->_internal.width = cr.right - cr.left;
            nkwnd->_internal.height = cr.bottom - cr.top;
            }
            break;

        // When mouse start l-press (drag window)
        case WM_LBUTTONDOWN:
            {
            if (HIWORD(lParam) <= 30 && nkwnd->allow_move)
            {
                // Start dragging
                nkwnd->_internal.is_draggin = 1;
                nkwnd->_internal.drag_offset.x = LOWORD(lParam);
                nkwnd->_internal.drag_offset.y = HIWORD(lParam);
            }
            }
            break;

        // When mouse stops l-press (drag window)
        case WM_LBUTTONUP:
            nkwnd->_internal.is_draggin = 0;
            break;

        // Mouse movement (dragging)
        case WM_MOUSEMOVE:
            {
            if (nkwnd->_internal.is_draggin && !nkwnd->_internal.is_maximized)
            {
                // Get mouse postion and substract offset
                POINT cursorPos;
                GetCursorPos(&cursorPos);
                cursorPos.x -= nkwnd->_internal.drag_offset.x;
                cursorPos.y -= nkwnd->_internal.drag_offset.y;
                // Use as new position
                ShowWindow(wnd, SW_RESTORE);
                SetWindowPos(wnd, NULL, cursorPos.x, cursorPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
            }
            break;

        // On mouse doubble click (maximize)
        case WM_LBUTTONDBLCLK:
            {
            if (HIWORD(lParam) <= 30 && nkwnd->allow_maximize)
            {
                if (nkwnd->_internal.is_maximized)
                {
                    ShowWindow(wnd, SW_RESTORE);
                }
                else
                {
                    ShowWindow(wnd, SW_MAXIMIZE);
                }
                nkwnd->_internal.ws_override = 1;
            }
            }
            break;
    }

    // Send to nuklear
    if (nkwnd->_internal.nk_gdi_ctx && nk_gdi_handle_event(nkwnd->_internal.nk_gdi_ctx, wnd, msg, wParam, lParam))
        return 0;

    // In case this is ever reached: Run default behaviour
    return DefWindowProc(wnd, msg, wParam, lParam);
}

#endif
#endif
