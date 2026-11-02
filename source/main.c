#include "config.h"
#include "graphics.h"
#include "overlay.h"
#include "process.h"
#include "report.h"
#include "window.h"
#include "bloom.h"

static LONG g_drawnCount = -1;

static void RenderIfNeeded(BOOL force)
{
    LONG count = ReportGetWriteCount();
    if (!force && count == g_drawnCount)
    {
        return;
    }

    OverlayRender();
    g_drawnCount = ReportGetWriteCount();
}

static LRESULT CALLBACK OverlayWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_TIMER:
            RenderIfNeeded(FALSE);
            return 0;

        case WM_DISPLAYCHANGE:
        {
            UINT width;
            UINT height;
            WindowFitVirtualScreen(window, &width, &height);
            GraphicsResize(width, height);
            RenderIfNeeded(TRUE);
            return 0;
        }

        case WM_HOTKEY:
            if (wParam == CONFIG_HOTKEY_ID)
            {
                DestroyWindow(window);
                return 0;
            }
            if (wParam == CONFIG_HOTKEY_UP)
            {
                OverlayScrollLines(-1);
            }
            else if (wParam == CONFIG_HOTKEY_DOWN)
            {
                OverlayScrollLines(1);
            }
            else
            {
                return 0;
            }

            RenderIfNeeded(TRUE);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            BeginPaint(window, &paint);
            EndPaint(window, &paint);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR commandLine, int showCommand)
{
    UNREFERENCED_PARAMETER(previousInstance);
    UNREFERENCED_PARAMETER(commandLine);
    UNREFERENCED_PARAMETER(showCommand);

    HRESULT com = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(com))
    {
        return 1;
    }

    if (!ReportOpen())
    {
        CoUninitialize();
        return 1;
    }

    HWND window = WindowCreate(instance, OverlayWindowProc);
    if (window == NULL)
    {
        ReportClose();
        CoUninitialize();
        return 1;
    }

    RECT client;
    GetClientRect(window, &client);
    UINT width = (UINT)(client.right - client.left);
    UINT height = (UINT)(client.bottom - client.top);
    if (!GraphicsStartup(window, width, height))
    {
        MessageBoxW(window, L"Direct3D composition startup failed.", L"D41SY", MB_ICONERROR | MB_OK);
        DestroyWindow(window);
        ReportClose();
        CoUninitialize();
        return 1;
    }

    ReportInfo(L"overlay started");
    OverlayStartup();
    ReportSuccess(L"graphics ready");
    ProcessStartup();
    RenderIfNeeded(TRUE);

    RegisterHotKey(window, CONFIG_HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'Q');
    RegisterHotKey(window, CONFIG_HOTKEY_UP, MOD_CONTROL | MOD_SHIFT, VK_UP);
    RegisterHotKey(window, CONFIG_HOTKEY_DOWN, MOD_CONTROL | MOD_SHIFT, VK_DOWN);
    
    
    DWORD pid = ProcessScan();
    if (pid != 0)
    {
        FlourishProcess(pid);
    }
    
    
    RenderIfNeeded(TRUE);
    SetTimer(window, CONFIG_TIMER_ID, CONFIG_TIMER_MS, NULL);

    MSG message;
    while (GetMessageW(&message, NULL, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    KillTimer(window, CONFIG_TIMER_ID);
    UnregisterHotKey(window, CONFIG_HOTKEY_ID);
    UnregisterHotKey(window, CONFIG_HOTKEY_UP);
    UnregisterHotKey(window, CONFIG_HOTKEY_DOWN);
    ProcessShutdown();
    OverlayShutdown();
    GraphicsShutdown();
    ReportClose();
    CoUninitialize();
    return (int)message.wParam;
}
