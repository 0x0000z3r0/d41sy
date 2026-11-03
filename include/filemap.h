#pragma once

#include <windows.h>
#include <stdbool.h>

HANDLE FileMapCreate(LPCWSTR name, const void* contentBuffer, SIZE_T contentSize);
