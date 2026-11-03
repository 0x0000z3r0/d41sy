#include "loader.h"
#include "report.h"

BOOL LoadDriver(LPCWSTR serviceName, LPCWSTR driverPath)
{
    SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager)
    {
        ReportFail(L"OpenSCManager failed: %lu", GetLastError());
        return FALSE;
    }

    ReportInfo(L"Creating service '%ws' for '%ws'...", serviceName, driverPath);
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
            ReportInfo(L"service already exists, opening it...");
            hService = OpenServiceW(hSCManager, serviceName, SERVICE_START | DELETE | SERVICE_STOP);
        }
        else
        {
            ReportFail(L"CreateService failed: %lu", GetLastError());
            CloseServiceHandle(hSCManager);
            return FALSE;
        }
    }

    if (hService)
    {
        ReportInfo(L"starting service...");
        if (!StartServiceW(hService, 0, NULL))
        {
            if (GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
            {
                ReportFail(L"StartService failed: %lu", GetLastError());
                CloseServiceHandle(hService);
                CloseServiceHandle(hSCManager);
                return FALSE;
            }
            ReportSuccess(L"service is already running.");
        }
        else
        {
            ReportSuccess(L"driver loaded successfully.");
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
        ReportFail(L"OpenSCManager failed: %lu", GetLastError());
        return FALSE;
    }

    SC_HANDLE hService = OpenServiceW(hSCManager, serviceName, SERVICE_STOP | DELETE);
    if (!hService)
    {
        ReportFail(L"OpenService failed: %lu", GetLastError());
        CloseServiceHandle(hSCManager);
        return FALSE;
    }

    ReportInfo(L"stopping service...");
    SERVICE_STATUS status;
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &status))
    {
        if (GetLastError() != ERROR_SERVICE_NOT_ACTIVE)
        {
            ReportFail(L"ControlService (STOP) failed: %lu", GetLastError());
        }
        else
        {
            ReportInfo(L"service is not active.");
        }
    }
    else
    {
        ReportSuccess(L"service stopped.");
    }

    ReportInfo(L"deleting service...");
    if (!DeleteService(hService))
    {
        ReportFail(L"DeleteService failed: %lu", GetLastError());
    }
    else
    {
        ReportSuccess(L"service deleted successfully.");
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return TRUE;
}