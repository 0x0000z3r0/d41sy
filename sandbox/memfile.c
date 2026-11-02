#include <windows.h>
#include <stdio.h>

int main()
{
    LPCWSTR sharedMemName = L"Local\\MyRAMFileDemo";
    SIZE_T memSize = 4096; // 4 KB

    printf("[*] Creating RAM-backed memory mapped file...\n");
    
    HANDLE hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,    // Use paging file (RAM)
        NULL,                    // Default security
        PAGE_READWRITE,          // Read/write access
        0,                       // Maximum object size (high-order DWORD)
        (DWORD)memSize,          // Maximum object size (low-order DWORD)
        sharedMemName            // Name of mapping object
    );

    if (hMapFile == NULL)
    {
        printf("[-] Could not create file mapping object (%lu).\n", GetLastError());
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        printf("[!] Warning: File mapping object already existed.\n");
    }
    else
    {
        printf("[+] File mapping object created successfully.\n");
    }

    LPVOID pBuf = MapViewOfFile(
        hMapFile,            // Handle to map object
        FILE_MAP_ALL_ACCESS, // Read/write permission
        0,                   // Offset (high-order DWORD)
        0,                   // Offset (low-order DWORD)
        memSize              // Number of bytes to map
    );

    if (pBuf == NULL)
    {
        printf("[-] Could not map view of file (%lu).\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    const char* message = "[*] RAM-backed memory mapped file...";
    printf("[*] Writing to memory: \"%s\"\n", message);
    CopyMemory((PVOID)pBuf, message, strlen(message) + 1);

    printf("[+] Read from memory (Process A): \"%s\"\n\n", (char*)pBuf);

    // Simulate a SECOND process opening the shared memory by its name
    printf("[*] Simulating Process B opening the shared memory by name...\n");
    
    HANDLE hMapFile2 = OpenFileMappingW(
        FILE_MAP_READ,   // Read access only
        FALSE,           // Do not inherit the name
        sharedMemName    // Name of mapping object to open
    );

    if (hMapFile2 != NULL)
    {
        LPVOID pBuf2 = MapViewOfFile(hMapFile2, FILE_MAP_READ, 0, 0, memSize);
        if (pBuf2 != NULL)
        {
            printf("[+] Process B successfully read: \"%s\"\n", (char*)pBuf2);
            
            // Unmap Process B's view
            UnmapViewOfFile(pBuf2);
        }
        else
        {
            printf("[-] Process B failed to map view (%lu).\n", GetLastError());
        }
        
        CloseHandle(hMapFile2);
    }
    else
    {
        printf("[-] Process B failed to open file mapping (%lu).\n", GetLastError());
    }

    printf("\n[*] Cleaning up...\n");
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    printf("[+] Done.\n");

    return 0;
}
