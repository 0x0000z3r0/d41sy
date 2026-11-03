#include "privilege.h"
#include "report.h"

BOOL EnablePrivilege(LPCWSTR privilegeName)
{
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        ReportFail(L"OpenProcessToken failed: %lu", GetLastError());
        return FALSE;
    }

    LUID luid;
    if (!LookupPrivilegeValueW(NULL, privilegeName, &luid))
    {
        ReportFail(L"LookupPrivilegeValueW failed for %ws: %lu", privilegeName, GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
        ReportFail(L"AdjustTokenPrivileges failed: %lu", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        ReportFail(L"the privilege %ws is not held by the current token (try running as Administrator).", privilegeName);
        CloseHandle(hToken);
        return FALSE;
    }

    ReportSuccess(L"Successfully enabled privilege: %ws", privilegeName);
    CloseHandle(hToken);
    return TRUE;
}

BOOL ContainsPrivilege(DWORD processId, LPCWSTR privilegeName)
{
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (process == NULL)
    {
        return FALSE;
    }

    HANDLE token = NULL;
    if (!OpenProcessToken(process, TOKEN_QUERY, &token))
    {
        CloseHandle(process);
        return FALSE;
    }

    DWORD bytes = 0;
    GetTokenInformation(token, TokenPrivileges, NULL, 0, &bytes);
    TOKEN_PRIVILEGES* tokenPrivs = NULL;
    if (bytes > 0)
    {
        tokenPrivs = (TOKEN_PRIVILEGES*)HeapAlloc(GetProcessHeap(), 0, bytes);
    }

    BOOL found = FALSE;
    if (tokenPrivs != NULL && GetTokenInformation(token, TokenPrivileges, tokenPrivs, bytes, &bytes))
    {
        LUID luid;
        if (LookupPrivilegeValueW(NULL, privilegeName, &luid))
        {
            for (DWORD j = 0; j < tokenPrivs->PrivilegeCount; j++)
            {
                if (tokenPrivs->Privileges[j].Luid.LowPart == luid.LowPart &&
                    tokenPrivs->Privileges[j].Luid.HighPart == luid.HighPart)
                {
                    found = TRUE;
                    break;
                }
            }
        }
    }

    if (tokenPrivs != NULL)
    {
        HeapFree(GetProcessHeap(), 0, tokenPrivs);
    }

    CloseHandle(token);
    CloseHandle(process);
    return found;
}