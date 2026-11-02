#include <windows.h>
#include <stdio.h>

BOOL LoadDriver(LPCWSTR serviceName, LPCWSTR driverPath)
{
    SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager)
    {
        printf("[-] OpenSCManager failed: %lu\n", GetLastError());
        return FALSE;
    }

    printf("[*] Creating service '%ws' for '%ws'...\n", serviceName, driverPath);
    SC_HANDLE hService = CreateServiceW(
        hSCManager,
        serviceName,
        serviceName,
        SERVICE_START | DELETE | SERVICE_STOP,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        driverPath,
        NULL, NULL, NULL, NULL, NULL
    );

    if (!hService)
    {
        if (GetLastError() == ERROR_SERVICE_EXISTS)
        {
            printf("[*] Service already exists, opening it...\n");
            hService = OpenServiceW(hSCManager, serviceName, SERVICE_START | DELETE | SERVICE_STOP);
        }
        else
        {
            printf("[-] CreateService failed: %lu\n", GetLastError());
            CloseServiceHandle(hSCManager);
            return FALSE;
        }
    }

    if (hService)
    {
        printf("[*] Starting service...\n");
        if (!StartServiceW(hService, 0, NULL))
        {
            if (GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
            {
                printf("[-] StartService failed: %lu\n", GetLastError());
                CloseServiceHandle(hService);
                CloseServiceHandle(hSCManager);
                return FALSE;
            }
            printf("[+] Service is already running.\n");
        }
        else
        {
            printf("[+] Driver loaded successfully.\n");
        }
        CloseServiceHandle(hService);
    }

    CloseServiceHandle(hSCManager);
    return TRUE;
}

BOOL UnloadDriver(LPCWSTR serviceName)
{
    SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager)
    {
        printf("[-] OpenSCManager failed: %lu\n", GetLastError());
        return FALSE;
    }

    SC_HANDLE hService = OpenServiceW(hSCManager, serviceName, SERVICE_STOP | DELETE);
    if (!hService)
    {
        printf("[-] OpenService failed: %lu\n", GetLastError());
        CloseServiceHandle(hSCManager);
        return FALSE;
    }

    printf("[*] Stopping service...\n");
    SERVICE_STATUS status;
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &status))
    {
        if (GetLastError() != ERROR_SERVICE_NOT_ACTIVE)
        {
            printf("[-] ControlService (STOP) failed: %lu\n", GetLastError());
        }
        else
        {
            printf("[*] Service is not active.\n");
        }
    }
    else
    {
        printf("[+] Service stopped.\n");
    }

    printf("[*] Deleting service...\n");
    if (!DeleteService(hService))
    {
        printf("[-] DeleteService failed: %lu\n", GetLastError());
    }
    else
    {
        printf("[+] Service deleted successfully.\n");
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return TRUE;
}

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

        LoadDriver(argv[2], absPath);
    }
    else if (_wcsicmp(argv[1], L"unload") == 0)
    {
        UnloadDriver(argv[2]);
    }
    else
    {
        printf("[-] Unknown command: %ws\n", argv[1]);
        return 1;
    }

    return 0;
}
