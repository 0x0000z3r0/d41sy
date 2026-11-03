#include <windows.h>
#include <stdio.h>
#include "config.h"
#include "report.h"

int main()
{
    printf("[*] Start\n");

    HANDLE hMutex = OpenMutexW(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, CONFIG_REPORT_MUTEX_NAME);
    if (hMutex == NULL)
    {
        printf("[!] OpenMutexW failed with error code %lu\n", GetLastError());
        return 1;
    }
    printf("[*] Mutex Opened\n");

    HANDLE hMapping = OpenFileMappingW(FILE_MAP_WRITE, FALSE, CONFIG_REPORT_MAPPING_NAME);
    if (hMapping == NULL)
    {
        printf("[!] OpenFileMappingW failed with error code %lu\n", GetLastError());
        CloseHandle(hMutex);
        return 1;
    }
    printf("[*] Mapping Opened\n");

    REPORT_BUFFER* pBuffer = (REPORT_BUFFER*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
    if (pBuffer == NULL)
    {
        printf("Error: MapViewOfFile failed with error code %lu\n", GetLastError());
        CloseHandle(hMapping);
        CloseHandle(hMutex);
        return 1;
    }
    printf("[+] View Mapped\n");
    printf("[-] SYNCHRONIZE = 0x%08X\n", SYNCHRONIZE);
    printf("[-] MUTEX_MODIFY_STATE = 0x%08X\n", MUTEX_MODIFY_STATE);
    printf("[-] FILE_MAP_WRITE = 0x%08X\n", FILE_MAP_WRITE);

    if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
    {
        printf("[!] WaitForSingleObject failed with error code %lu\n", GetLastError());
        UnmapViewOfFile(pBuffer);
        CloseHandle(hMapping);
        CloseHandle(hMutex);
        return 1;
    }
    printf("[*] Mutex Acquired\n");

    LONG index = pBuffer->WriteCount % CONFIG_REPORT_CAPACITY;
    REPORT_ENTRY* entry = &pBuffer->Entries[index];
    
    entry->Level = REPORT_LEVEL_INFO;
    wcscpy_s(entry->Text, CONFIG_REPORT_TEXT_CCH, L"[X] hello from the sandbox");
    
    InterlockedIncrement(&pBuffer->WriteCount);
    printf("[*] Payload Written\n");

    ReleaseMutex(hMutex);
    printf("[*] Done\n");

    UnmapViewOfFile(pBuffer);
    CloseHandle(hMapping);
    CloseHandle(hMutex);

    return 0;
}
