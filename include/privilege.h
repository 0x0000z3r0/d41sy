#pragma once

#include <windows.h>
#include <stdbool.h>

BOOL EnablePrivilege(LPCWSTR privilegeName);
BOOL ContainsPrivilege(DWORD processId, LPCWSTR privilegeName);