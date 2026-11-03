#pragma once

#include <windows.h>
#include <stdbool.h>

#define IDR_LOGO 101
#define IDR_FLOURISH 102
#define IDR_FERTILIZER 103

BOOL FetchResource(WORD resourceId, void** outBytes, DWORD* outSize);