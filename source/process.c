#include "process.h"

#include "report.h"
#include "privilege.h"

#include <strsafe.h>
#include <tlhelp32.h>

static const PCWSTR g_whitelist[] = {
    L"Taskmgr.exe",
    L"cmd.exe",
    L"explorer.exe",
    L"firefox.exe",
    L"chrome.exe",
    L"brave.exe",
    L"opera.exe",
    L"vivaldi.exe",
    L"edge.exe",
    L"msedge.exe",
};

static const PCWSTR g_blacklist[] = {
    L"dwm.exe",
    L"smss.exe",
    L"csrss.exe",
    L"wininit.exe",
    L"services.exe",
    L"lsass.exe",
    L"winlogon.exe",
    L"svchost.exe",
    L"spoolsv.exe",
    L"LogonUI.exe", 
    L"sihost.exe",
    L"fontdrvhost.exe",
    L"Memory Compression",
    L"Registry"
};

static const PCWSTR g_requiredPrivileges[] = {
    L"SeDebugPrivilege",
    L"SeLoadDriverPrivilege"
};

static BOOL ProcessNameEquals(PCWSTR left, PCWSTR right)
{
    return CompareStringOrdinal(left, -1, right, -1, TRUE) == CSTR_EQUAL;
}

static BOOL ProcessInList(PCWSTR name, const PCWSTR* names, LONG count)
{
    for (LONG i = 0; i < count; i++)
    {
        if (ProcessNameEquals(name, names[i]))
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void ProcessReportNames(const PCWSTR* names, LONG count, BOOL blacklist)
{
    for (LONG i = 0; i < count; i++)
    {
        if (blacklist)
        {
            ReportFail(L"blacklist %s", names[i]);
        }
        else
        {
            ReportSuccess(L"whitelist %s", names[i]);
        }
    }
}

static void ProcessAppendPrivilege(WCHAR* text, DWORD textCount, PCWSTR name)
{
    if (text[0] == L'\0')
    {
        StringCchCopyW(text, textCount, name);
        return;
    }

    if (wcslen(text) + wcslen(name) + 4 >= textCount)
    {
        StringCchCatW(text, textCount, L",...");
        return;
    }

    StringCchCatW(text, textCount, L",");
    StringCchCatW(text, textCount, name);
}

static void ProcessFormatPrivileges(DWORD processId, WCHAR* text, DWORD textCount)
{
    StringCchCopyW(text, textCount, L"unavailable");

    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (process == NULL)
    {
        return;
    }

    HANDLE token = NULL;
    if (!OpenProcessToken(process, TOKEN_QUERY, &token))
    {
        CloseHandle(process);
        return;
    }

    DWORD bytes = 0;
    GetTokenInformation(token, TokenPrivileges, NULL, 0, &bytes);
    TOKEN_PRIVILEGES* privileges = NULL;
    if (bytes > 0)
    {
        privileges = (TOKEN_PRIVILEGES*)HeapAlloc(GetProcessHeap(), 0, bytes);
    }

    text[0] = L'\0';
    if (privileges != NULL && GetTokenInformation(token, TokenPrivileges, privileges, bytes, &bytes))
    {
        for (DWORD i = 0; i < privileges->PrivilegeCount; i++)
        {
            if ((privileges->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) == 0)
            {
                continue;
            }

            WCHAR name[64];
            DWORD nameCount = 64;
            if (!LookupPrivilegeNameW(NULL, &privileges->Privileges[i].Luid, name, &nameCount))
            {
                continue;
            }

            if (wcsstr(text, L"...") != NULL)
            {
                break;
            }

            ProcessAppendPrivilege(text, textCount, name);
        }
    }

    if (privileges != NULL)
    {
        HeapFree(GetProcessHeap(), 0, privileges);
    }

    if (text[0] == L'\0')
    {
        StringCchCopyW(text, textCount, L"none");
    }

    CloseHandle(token);
    CloseHandle(process);
}

static BOOL ProcessHasPrivileges(DWORD processId, const PCWSTR* privs, LONG count)
{
    for (LONG i = 0; i < count; i++)
    {
        if (!ContainsPrivilege(processId, privs[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}

DWORD ProcessScan(void)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        ReportFail(L"process snapshot failed error=%lu", GetLastError());
        return 0;
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);
    if (!Process32FirstW(snapshot, &entry))
    {
        ReportFail(L"process snapshot empty error=%lu", GetLastError());
        CloseHandle(snapshot);
        return 0;
    }

    DWORD selected = 0;

    LONG whiteCount = (LONG)(sizeof(g_whitelist) / sizeof(g_whitelist[0]));
    LONG blackCount = (LONG)(sizeof(g_blacklist) / sizeof(g_blacklist[0]));
    LONG reqCount = (LONG)(sizeof(g_requiredPrivileges) / sizeof(g_requiredPrivileges[0]));
    ULONG reported = 0;
    do
    {
        BOOL blacklisted = ProcessInList(entry.szExeFile, g_blacklist, blackCount);
        BOOL whitelisted = ProcessInList(entry.szExeFile, g_whitelist, whiteCount);
        WCHAR privileges[160];
        ProcessFormatPrivileges(entry.th32ProcessID, privileges, 160);

        BOOL hasReq = ProcessHasPrivileges(entry.th32ProcessID, g_requiredPrivileges, reqCount);

        if (blacklisted)
        {
            ReportFail(L"%s pid=%lu priv=%s list=blacklist", entry.szExeFile, entry.th32ProcessID, privileges);
        }
        else if (whitelisted)
        {
            if (hasReq)
            {
                ReportSuccess(L"%s pid=%lu priv=%s list=whitelist", entry.szExeFile, entry.th32ProcessID, privileges);
                if (selected == 0)
                {
                    selected = entry.th32ProcessID;
                }
            }
            else
            {
                ReportFail(L"%s pid=%lu lacks required privileges", entry.szExeFile, entry.th32ProcessID);
            }
        }
        else
        {
            ReportInfo(L"%s pid=%lu priv=%s list=none", entry.szExeFile, entry.th32ProcessID, privileges);
        }

        reported += 1;
        entry.dwSize = sizeof(entry);
    } while (Process32NextW(snapshot, &entry));

    CloseHandle(snapshot);
    ReportInfo(L"process scan finished count=%lu", reported);

    if (selected == 0)
    {
        ReportFail(L"no process selected");
    }

    ReportSuccess(L"selected process pid=%lu", selected);
    return selected;
}

BOOL ProcessStartup(void)
{
    ReportInfo(L"process module started");
    EnablePrivilege(L"SeDebugPrivilege");
    ProcessReportNames(g_whitelist, (LONG)(sizeof(g_whitelist) / sizeof(g_whitelist[0])), FALSE);
    ProcessReportNames(g_blacklist, (LONG)(sizeof(g_blacklist) / sizeof(g_blacklist[0])), TRUE);
    return TRUE;
}

void ProcessShutdown(void)
{
    ReportInfo(L"process module stopped");
}
