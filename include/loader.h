#pragma once

#include <windows.h>

BOOL LoadDriver(LPCWSTR serviceName, LPCWSTR driverPath);
BOOL UnloadDriver(LPCWSTR serviceName);