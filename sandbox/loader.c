#include "loader.h"
#include <stdio.h>

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("  Load:   loader.exe load <ServiceName> <DriverPath>\n");
        printf("  Unload: loader.exe unload <ServiceName>\n");
        return 1;
    }

    if (_wcsicmp(argv[1], L"load") == 0)
    {
        if (argc < 4)
        {
            printf("[-] Missing driver path for load command.\n");
            return 1;
        }

        // Get full absolute path for the driver (SCM requires absolute paths)
        WCHAR absPath[MAX_PATH];
        if (!GetFullPathNameW(argv[3], MAX_PATH, absPath, NULL))
        {
            printf("[-] Failed to get absolute path for driver.\n");
            return 1;
        }
        printf("[+] Getting absolute path for driver...\n");

        if (!LoadDriver(argv[2], absPath))
        {
            printf("[-] Failed to load driver.\n");
            return 1;
        }
        printf("[+] Loaded driver successfully.\n");
    }
    else if (_wcsicmp(argv[1], L"unload") == 0)
    {
        if (!UnloadDriver(argv[2]))
        {
            printf("[-] Failed to unload driver.\n");
            return 1;
        }
    }
    else
    {
        printf("[-] Unknown command: %ws\n", argv[1]);
        return 1;
    }

    printf("[+] Successfully loaded/unloaded driver.\n");
    return 0;
}
