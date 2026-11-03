#include "filemap.h"
#include "report.h"

HANDLE FileMapCreate(LPCWSTR name, const void* contentBuffer, SIZE_T contentSize)
{
    if (!contentBuffer || contentSize == 0)
    {
        ReportFail(L"FileMapCreate: Invalid buffer or size.");
        return NULL;
    }

    HANDLE hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        (DWORD)(contentSize >> 32),
        (DWORD)(contentSize & 0xFFFFFFFF),
        name
    );
    if (hMapFile == NULL)
    {
        ReportFail(L"CreateFileMappingW failed: %lu", GetLastError());
        return NULL;
    }

    LPVOID pBuf = MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        contentSize
    );
    if (pBuf == NULL)
    {
        ReportFail(L"MapViewOfFile failed: %lu", GetLastError());
        CloseHandle(hMapFile);
        return NULL;
    }

    CopyMemory(pBuf, contentBuffer, contentSize);
    UnmapViewOfFile(pBuf);

    ReportSuccess(L"created RAM file mapping '%ws' (%zu bytes).", name, contentSize);
    return hMapFile;
}
