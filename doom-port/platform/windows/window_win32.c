#include "../platform_window.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

static HWND g_hwnd;
static HDC g_hdc;
static HBITMAP g_dib_section;
static void* g_framebuffer;
static int g_width;
static int g_height;
static platform_event_callback_t g_event_callback;

static int MapKey(WPARAM wParam) {
    switch (wParam) {
        case VK_LEFT:   return 0xac; // KEY_LEFTARROW
        case VK_RIGHT:  return 0xae; // KEY_RIGHTARROW
        case VK_UP:     return 0xad; // KEY_UPARROW
        case VK_DOWN:   return 0xaf; // KEY_DOWNARROW
        case VK_ESCAPE: return 27;   // KEY_ESCAPE
        case VK_RETURN: return 13;   // KEY_ENTER
        case VK_TAB:    return 9;    // KEY_TAB
        case VK_F1:     return (0x80+0x3b);
        case VK_F2:     return (0x80+0x3c);
        case VK_F3:     return (0x80+0x3d);
        case VK_F4:     return (0x80+0x3e);
        case VK_F5:     return (0x80+0x3f);
        case VK_F6:     return (0x80+0x40);
        case VK_F7:     return (0x80+0x41);
        case VK_F8:     return (0x80+0x42);
        case VK_F9:     return (0x80+0x43);
        case VK_F10:    return (0x80+0x44);
        case VK_F11:    return (0x80+0x57);
        case VK_F12:    return (0x80+0x58);
        case VK_BACK:   return 127;  // KEY_BACKSPACE
        case VK_PAUSE:  return 0xff; // KEY_PAUSE
        case VK_CONTROL: return (0x80+0x1d); // KEY_RCTRL
        case VK_SHIFT:   return (0x80+0x36); // KEY_RSHIFT
        case VK_MENU:    return (0x80+0x38); // KEY_RALT
        case VK_OEM_PLUS: return 0x3d; // KEY_EQUALS
        case VK_OEM_MINUS: return 0x2d; // KEY_MINUS
        default:
            if (wParam >= 'A' && wParam <= 'Z')
                return (int)wParam + ('a' - 'A'); // To lowercase
            if (wParam >= '0' && wParam <= '9')
                return (int)wParam;
            return (int)wParam;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    platform_event_t ev;
    static int last_x = -1;
    static int last_y = -1;

    switch (uMsg) {
        case WM_KEYDOWN:
            if (g_event_callback) {
                ev.type = PLATFORM_EVENT_KEYDOWN;
                ev.data1 = MapKey(wParam);
                g_event_callback(&ev);
            }
            return 0;
        case WM_KEYUP:
            if (g_event_callback) {
                ev.type = PLATFORM_EVENT_KEYUP;
                ev.data1 = MapKey(wParam);
                g_event_callback(&ev);
            }
            return 0;
        case WM_MOUSEMOVE:
            if (g_event_callback) {
                int x = (int)(short)LOWORD(lParam);
                int y = (int)(short)HIWORD(lParam);
                if (last_x != -1) {
                    ev.type = PLATFORM_EVENT_MOUSEMOVE;
                    ev.data1 = 0; // buttons
                    ev.data2 = (x - last_x) * 2; // sensitivity
                    ev.data3 = (last_y - y) * 2;
                    if (ev.data2 != 0 || ev.data3 != 0)
                        g_event_callback(&ev);
                }
                last_x = x;
                last_y = y;
            }
            return 0;
        case WM_CLOSE:
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Window_SetEventCallback(platform_event_callback_t callback) {
    g_event_callback = callback;
}

void Window_Init(int width, int height, const char* title) {
    g_width = width;
    g_height = height;

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "DoomPortClass";
    RegisterClass(&wc);

    // Calculate window size based on client area
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    g_hwnd = CreateWindowEx(
        0, "DoomPortClass", title,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, wc.hInstance, NULL
    );

    g_hdc = GetDC(g_hwnd);

    // Create DIB section for the 8-bit paletted rendering
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // We'll manually expand 8-bit to 32-bit before blitting for simplicity right now
    bmi.bmiHeader.biCompression = BI_RGB;

    g_dib_section = CreateDIBSection(g_hdc, &bmi, DIB_RGB_COLORS, &g_framebuffer, NULL, 0);
}

void Window_Shutdown(void) {
    if (g_dib_section) DeleteObject(g_dib_section);
    if (g_hdc && g_hwnd) ReleaseDC(g_hwnd, g_hdc);
    if (g_hwnd) DestroyWindow(g_hwnd);
}

void Window_ProcessEvents(void) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            exit(0); // Quick exit for now
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

#include <stdio.h>

LONG WINAPI CrashHandler(EXCEPTION_POINTERS *ExceptionInfo) {
    char buf[512];
    sprintf(buf, "Unhandled Exception: 0x%08X at address %p", 
            ExceptionInfo->ExceptionRecord->ExceptionCode,
            ExceptionInfo->ExceptionRecord->ExceptionAddress);
    fprintf(stderr, "CRASH: %s\n", buf);
    fflush(stderr);
    MessageBoxA(NULL, buf, "DOOM Crash", MB_OK | MB_ICONERROR);
    return EXCEPTION_EXECUTE_HANDLER;
}

void Window_InstallCrashHandler(void) {
    SetUnhandledExceptionFilter(CrashHandler);
}

void Window_ShowError(const char* error) {
    MessageBoxA(NULL, error, "DOOM Fatal Error", MB_OK | MB_ICONERROR);
}

void Window_Present(const void* pixels, int width, int height) {
    // For now, we will handle the palette mapping in i_video.c and pass 32-bit pixels here.
    // Or we can pass 8-bit pixels here and do the palette mapping here. Let's assume 32-bit pixels passed here for simplicity of the Win32 API.
    memcpy(g_framebuffer, pixels, width * height * 4);

    HDC memDC = CreateCompatibleDC(g_hdc);
    HGDIOBJ oldBMP = SelectObject(memDC, g_dib_section);
    
    // Nearest neighbor scaling to window size
    RECT clientRect;
    GetClientRect(g_hwnd, &clientRect);
    StretchBlt(g_hdc, 0, 0, clientRect.right, clientRect.bottom,
               memDC, 0, 0, width, height, SRCCOPY);

    SelectObject(memDC, oldBMP);
    DeleteDC(memDC);
}
