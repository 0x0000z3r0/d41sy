#include "report.h"

#include <stdarg.h>
#include <strsafe.h>

static HANDLE g_mapping = NULL;
static HANDLE g_mutex = NULL;
static REPORT_BUFFER* g_buffer = NULL;

static void ReportWrite(LONG level, PCWSTR tag, PCWSTR message)
{
    if (g_buffer == NULL || g_mutex == NULL)
    {
        return;
    }

    if (message == NULL)
    {
        message = L"";
    }

    if (WaitForSingleObject(g_mutex, INFINITE) != WAIT_OBJECT_0)
    {
        return;
    }

    LONG index = g_buffer->WriteCount % CONFIG_REPORT_CAPACITY;
    REPORT_ENTRY* entry = &g_buffer->Entries[index];
    entry->Level = level;
    StringCchCopyW(entry->Text, CONFIG_REPORT_TEXT_CCH, tag);
    StringCchCatW(entry->Text, CONFIG_REPORT_TEXT_CCH, message);
    InterlockedIncrement(&g_buffer->WriteCount);

    ReleaseMutex(g_mutex);
}

static void ReportWriteV(LONG level, PCWSTR tag, PCWSTR format, va_list args)
{
    if (format == NULL)
    {
        format = L"";
    }

    WCHAR body[CONFIG_REPORT_TEXT_CCH];
    StringCchVPrintfW(body, CONFIG_REPORT_TEXT_CCH, format, args);
    ReportWrite(level, tag, body);
}

BOOL ReportOpen(void)
{
    g_mutex = CreateMutexW(NULL, FALSE, CONFIG_REPORT_MUTEX_NAME);
    if (g_mutex == NULL)
    {
        ReportClose();
        return FALSE;
    }

    g_mapping = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(REPORT_BUFFER),
        CONFIG_REPORT_MAPPING_NAME);
    if (g_mapping == NULL)
    {
        ReportClose();
        return FALSE;
    }

    BOOL created = (GetLastError() != ERROR_ALREADY_EXISTS);
    g_buffer = (REPORT_BUFFER*)MapViewOfFile(g_mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(REPORT_BUFFER));
    if (g_buffer == NULL)
    {
        ReportClose();
        return FALSE;
    }

    if (created)
    {
        if (WaitForSingleObject(g_mutex, INFINITE) == WAIT_OBJECT_0)
        {
            ZeroMemory(g_buffer, sizeof(REPORT_BUFFER));
            ReleaseMutex(g_mutex);
        }
    }

    return TRUE;
}

void ReportClose(void)
{
    if (g_buffer != NULL)
    {
        UnmapViewOfFile(g_buffer);
        g_buffer = NULL;
    }

    if (g_mapping != NULL)
    {
        CloseHandle(g_mapping);
        g_mapping = NULL;
    }

    if (g_mutex != NULL)
    {
        CloseHandle(g_mutex);
        g_mutex = NULL;
    }
}

LONG ReportGetWriteCount(void)
{
    if (g_buffer == NULL)
    {
        return 0;
    }

    return InterlockedCompareExchange(&g_buffer->WriteCount, 0, 0);
}

BOOL ReportSnapshot(REPORT_LINE* lines, LONG capacity, LONG* outCount)
{
    if (outCount != NULL)
    {
        *outCount = 0;
    }

    if (g_buffer == NULL || g_mutex == NULL || lines == NULL || capacity <= 0)
    {
        return FALSE;
    }

    if (WaitForSingleObject(g_mutex, 20) != WAIT_OBJECT_0)
    {
        return FALSE;
    }

    LONG count = g_buffer->WriteCount;
    LONG available = count;
    if (available > CONFIG_REPORT_CAPACITY)
    {
        available = CONFIG_REPORT_CAPACITY;
    }
    if (available > capacity)
    {
        available = capacity;
    }

    LONG start = count - available;
    for (LONG i = 0; i < available; i++)
    {
        LONG index = (start + i) % CONFIG_REPORT_CAPACITY;
        lines[i].Level = g_buffer->Entries[index].Level;
        StringCchCopyW(lines[i].Text, CONFIG_REPORT_TEXT_CCH, g_buffer->Entries[index].Text);
    }

    ReleaseMutex(g_mutex);

    if (outCount != NULL)
    {
        *outCount = available;
    }

    return TRUE;
}

void ReportInfo(PCWSTR format, ...)
{
    va_list args;
    va_start(args, format);
    ReportWriteV(REPORT_LEVEL_INFO, L"[*] ", format, args);
    va_end(args);
}

void ReportSuccess(PCWSTR format, ...)
{
    va_list args;
    va_start(args, format);
    ReportWriteV(REPORT_LEVEL_SUCCESS, L"[+] ", format, args);
    va_end(args);
}

void ReportFail(PCWSTR format, ...)
{
    va_list args;
    va_start(args, format);
    ReportWriteV(REPORT_LEVEL_FAIL, L"[!] ", format, args);
    va_end(args);
}
