#pragma once

namespace NkGdi
{
    // Container for window management
    class WindowClass
    {
        public:
            // Public exposed name
            static const wchar_t* const ClassName;

            WindowClass(const WindowClass&) = delete;
        private:
            // Instance
            static WindowClass ClassInstance;

            // Sigelton
            WindowClass();
    };

    // Window base class
    class Window
    {
        friend class WindowClass;

        public:
            // Default constructs
            Window() = default;
            Window(const Window&) = delete;
            Window(Window&&) = default;
            // Constructor
            Window(unsigned int width, unsigned int height, const char* name, int posX = 100, int posY = 100);
            // Destructor
            ~Window();

            // Processs window events and render the window (returns true as long as window is open)
            bool Update();

            // Properties
            bool AllowSizing = true;
            bool AllowMaximize = true;
            bool AllowMove = true;
            bool HasTitlebar = true;

        public:
            // Called when the core window gets an event
            virtual LRESULT OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

            // Called when the window is created
            inline virtual void OnCreate() {};
            // Called when the window is destroyed
            inline virtual void OnDestroy() {};
            // Called when the windows is beein updated (before events are served)
            inline virtual void OnUpdate() {};
            // Called when the window is beeing closed by the user (return false to abort)
            inline virtual bool OnClose() { return true; };
            // Called when nuklear drawcalls can be issued (return false to close the window)
            inline virtual bool OnDraw(nk_context* ctx) { return true; };

        private:
            // Static window procs
            static LRESULT wndProcSetup(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
            static LRESULT wndProcRun(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);

        private:
            // Window handle
            HWND m_window = NULL;

            // Nuklear context
            nk_gdi_ctx m_nkGdiCtx = nullptr;
            nk_context* m_nkCtx = nullptr;

            // Nuklear objects
            GdiFont* m_gdiFont = nullptr;
            HDC m_windowDc = NULL;

            // Window runtime features
            bool m_isOpen = true;
            bool m_isDraggin = false;
            bool m_wsOverride = false;
            bool m_isMaximized = false;
            POINT m_dragOffset = {};
            int m_width = 0;
            int m_height = 0;
    };
}

#ifdef NKGDI_IMPLEMENT_WINDOW

const wchar_t* const NkGdi::WindowClass::ClassName = L"WNDCLS_NkGdi";
NkGdi::WindowClass NkGdi::WindowClass::ClassInstance;

NkGdi::WindowClass::WindowClass()
{
    // Describe class
    WNDCLASSEXW cls;
    cls.cbSize = sizeof(WNDCLASSEXW);
    cls.style = CS_OWNDC | CS_DBLCLKS;
    cls.lpfnWndProc = &Window::wndProcSetup;
    cls.cbClsExtra = 0;
    cls.cbWndExtra = 0;
    cls.hInstance = GetModuleHandle(NULL);
    cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    cls.hCursor = LoadCursor(NULL, IDC_ARROW);
    cls.hbrBackground = NULL;
    cls.lpszMenuName = nullptr;
    cls.lpszClassName = ClassName;
    cls.hIconSm = NULL;

    // Register class
    RegisterClassExW(&cls);
}

NkGdi::Window::Window(unsigned int width, unsigned int height, const char* name, int posX, int posY)
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
    m_window = CreateWindowExW(
        styleEx,
        WindowClass::ClassName,
        L"NkGdi",
        style | WS_VISIBLE,
        posX, posY,
        cr.right - cr.left, cr.bottom - cr.top,
        NULL, NULL,
        GetModuleHandleW(nullptr),
        this
    );
 
    // Rename window to user picked name
    SetWindowTextA(m_window, name);

    // Get DC
    m_windowDc = GetWindowDC(m_window);

    // Create font
    m_gdiFont = nk_gdifont_create("Arial", 16);
    m_nkCtx = nk_gdi_init(&m_nkGdiCtx, m_gdiFont, m_windowDc, width, height);
}

NkGdi::Window::~Window()
{
    // Destroy  GDI context
    if (m_nkGdiCtx)
    {
        nk_gdi_shutdown(m_nkGdiCtx);
    }

    // Destroy font
    if (m_gdiFont)
    {
        nk_gdifont_del(m_gdiFont);
    }

    // Close DC
    if (m_windowDc)
    {
        ReleaseDC(m_window, m_windowDc);
    }

    // Destroy window
    if (m_window)
    {
        CloseWindow(m_window);
        DestroyWindow(m_window);
    }
}

bool NkGdi::Window::Update()
{
    // Only process events while window is open
    if (m_isOpen)
    {
        // Notify class that event processing has stated
        OnUpdate();

        // Windows event loop
        MSG msg = {};
        nk_input_begin(m_nkCtx);
        while (PeekMessage(&msg, m_window, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        nk_input_end(m_nkCtx);

        // Get title
        char title[1024];
        GetWindowTextA(m_window, title, 1024);

        // Window body
        if (m_wsOverride)
            nk_window_set_bounds(m_nkCtx, title, nk_rect(0, 0, m_width, m_height));
        if (nk_begin(m_nkCtx, title, nk_rect(0, 0, m_width, m_height), 
            NK_WINDOW_BORDER | 
            (HasTitlebar ? NK_WINDOW_CLOSABLE | NK_WINDOW_TITLE : NULL) |
            (m_isMaximized || !AllowSizing ? NULL : NK_WINDOW_SCALABLE) 
        ))
        {
            if(!OnDraw(m_nkCtx))
                m_isOpen = false;

            // Update window size
            struct nk_rect bounds = nk_window_get_bounds(m_nkCtx);
            if(bounds.w != m_width || bounds.h != m_height)
                SetWindowPos(m_window, NULL, 0, 0, bounds.w, bounds.h, SWP_NOMOVE | SWP_NOOWNERZORDER);
        }
        else
        {
            // Handle window closing
            if(OnClose())
                m_isOpen = false;
        }
        nk_end(m_nkCtx);
        m_wsOverride = false;

        // Final render pass
        nk_gdi_render(m_nkGdiCtx, nk_rgb(30, 30, 30));
    }

    return m_isOpen;
}

LRESULT NkGdi::Window::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Switch on supplied message code
    switch (msg)
    {
        // Close event
        case WM_CLOSE:
            if (OnClose())
                m_isOpen = false;
            return 0; // Will always be handled internaly
        
        // While sizing
        case WM_SIZING:
            {
            RECT cr;
            GetClientRect(wnd, &cr);
            m_width = cr.right - cr.left;
            m_height = cr.bottom - cr.top;
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
                    m_height = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
                    m_width = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
                    m_wsOverride = true;
                    m_isMaximized = true;
                    SetWindowPos(wnd, NULL, 0, 0, m_width, m_height, SWP_NOMOVE | SWP_NOZORDER);
                }
            }
            else if (wParam == SIZE_RESTORED)
            {
                m_isMaximized = false;
            }

            // Compute new bounds
            RECT cr;
            GetClientRect(wnd, &cr);
            m_width = cr.right - cr.left;
            m_height = cr.bottom - cr.top;
            }
            break;

        // When mouse start l-press (drag window)
        case WM_LBUTTONDOWN:
            {
            if (HIWORD(lParam) <= 30 && AllowMove)
            {
                // Start dragging
                m_isDraggin = true;
                m_dragOffset.x = LOWORD(lParam);
                m_dragOffset.y = HIWORD(lParam);
            }
            }
            break;

        // When mouse stops l-press (drag window)
        case WM_LBUTTONUP:
            m_isDraggin = false;
            break;

        // Mouse movement (dragging)
        case WM_MOUSEMOVE:
            {
            if (m_isDraggin && !m_isMaximized)
            {
                // Get mouse postion and substract offset
                POINT cursorPos;
                GetCursorPos(&cursorPos);
                cursorPos.x -= m_dragOffset.x;
                cursorPos.y -= m_dragOffset.y;
                // Use as new position
                ShowWindow(m_window, SW_RESTORE);
                SetWindowPos(wnd, NULL, cursorPos.x, cursorPos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
            }
            break;

        // On mouse doubble click (maximize)
        case WM_LBUTTONDBLCLK:
            {
            if (HIWORD(lParam) <= 30 && AllowMaximize)
            {
                if (m_isMaximized)
                {
                    ShowWindow(wnd, SW_RESTORE);
                }
                else
                {
                    ShowWindow(wnd, SW_MAXIMIZE);
                }
                m_wsOverride = true;
            }
            }
            break;
    }

    // Send to nuklear
    if (m_nkGdiCtx && nk_gdi_handle_event(m_nkGdiCtx, wnd, msg, wParam, lParam))
        return 0;

    // In case this is ever reached: Run default behaviour
    return DefWindowProc(wnd, msg, wParam, lParam);
}

LRESULT NkGdi::Window::wndProcSetup(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Wait for setup message
    if (msg == WM_NCCREATE)
    {
        // Get creation parameters & window pointer
        CREATESTRUCT* ptrCr = (CREATESTRUCT*)lParam;
        Window* ptrWindow = (Window*)ptrCr->lpCreateParams;

        // Store pointer and new proc in window
        SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)ptrWindow);
        SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)&wndProcRun);

        // Handled by window
        return ptrWindow->OnWindowMessage(wnd, msg, wParam, lParam);
    }

    // Default handler
    return DefWindowProc(wnd, msg, wParam, lParam);
}

LRESULT NkGdi::Window::wndProcRun(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Get window pointer
    Window* ptrWindow = (Window*)GetWindowLongPtr(wnd, GWLP_USERDATA);
    // Call window
    return ptrWindow->OnWindowMessage(wnd, msg, wParam, lParam);
}

#endif
