#pragma once

#include <windows.h>

BOOL OverlayStartup(void);
void OverlayShutdown(void);
void OverlayRender(void);
void OverlayScrollLines(LONG deltaLines);
