#include "window.h"

HWND WindowCreate(HINSTANCE instance, WNDPROC procedure)
{
    WNDCLASSEXW windowClass;
    ZeroMemory(&windowClass, sizeof(windowClass));
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = procedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"D41SY.Overlay";

    if (RegisterClassExW(&windowClass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        return NULL;
    }

    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (width <= 0 || height <= 0)
    {
        return NULL;
    }

    HWND window = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP,
        windowClass.lpszClassName,
        L"",
        WS_POPUP,
        x,
        y,
        width,
        height,
        NULL,
        NULL,
        instance,
        NULL);
    if (window == NULL)
    {
        return NULL;
    }

    SetLayeredWindowAttributes(window, 0, 255, LWA_ALPHA);
    ShowWindow(window, SW_SHOWNOACTIVATE);
    return window;
}

void WindowFitVirtualScreen(HWND window, UINT* outWidth, UINT* outHeight)
{
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (width < 1)
    {
        width = 1;
    }
    if (height < 1)
    {
        height = 1;
    }

    SetWindowPos(window, HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);

    if (outWidth != NULL)
    {
        *outWidth = (UINT)width;
    }
    if (outHeight != NULL)
    {
        *outHeight = (UINT)height;
    }
}
