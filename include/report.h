#pragma once

#include "config.h"

#include <windows.h>

typedef enum REPORT_LEVEL
{
    REPORT_LEVEL_INFO = 0,
    REPORT_LEVEL_SUCCESS = 1,
    REPORT_LEVEL_FAIL = 2
} REPORT_LEVEL;

typedef struct REPORT_ENTRY
{
    LONG Level;
    WCHAR Text[CONFIG_REPORT_TEXT_CCH];
} REPORT_ENTRY;

typedef struct REPORT_BUFFER
{
    volatile LONG WriteCount;
    REPORT_ENTRY Entries[CONFIG_REPORT_CAPACITY];
} REPORT_BUFFER;

typedef struct REPORT_LINE
{
    LONG Level;
    WCHAR Text[CONFIG_REPORT_TEXT_CCH];
} REPORT_LINE;

BOOL ReportOpen(void);
void ReportClose(void);
LONG ReportGetWriteCount(void);
BOOL ReportSnapshot(REPORT_LINE* lines, LONG capacity, LONG* outCount);

void ReportInfo(PCWSTR format, ...);
void ReportSuccess(PCWSTR format, ...);
void ReportFail(PCWSTR format, ...);
