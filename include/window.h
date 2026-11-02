#pragma once

#include <windows.h>

HWND WindowCreate(HINSTANCE instance, WNDPROC procedure);
void WindowFitVirtualScreen(HWND window, UINT* outWidth, UINT* outHeight);
