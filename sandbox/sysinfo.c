#include <windows.h>
#include <stdio.h>
#include <winternl.h>

typedef LONG (NTAPI *PNtQuerySystemInformation)(
    ULONG                    SystemInformationClass,
    PVOID                    SystemInformation,
    ULONG                    SystemInformationLength,
    PULONG                   ReturnLength
);

typedef struct _SYSTEM_MODULE {
    HANDLE               Section;
    PVOID                MappedBase;
    PVOID                ImageBaseAddress;
    ULONG                ImageSize;
    ULONG                Flags;
    USHORT               LoadOrderIndex;
    USHORT               InitOrderIndex;
    USHORT               LoadCount;
    USHORT               OffsetToFileName;
    UCHAR                Name[256];
} SYSTEM_MODULE, *PSYSTEM_MODULE;

typedef struct _SYSTEM_MODULE_INFORMATION {
    ULONG                ModulesCount;
    SYSTEM_MODULE        Modules[1];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

ULONG64 GetKernelBaseAddress(void)
{
    HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
    if (!hNtDll) {
        printf("[-] Failed to get ntdll.dll handle.\n");
        return 0;
    }

    PNtQuerySystemInformation pNtQuerySystemInformation = 
        (PNtQuerySystemInformation)GetProcAddress(hNtDll, "NtQuerySystemInformation");
        
    if (!pNtQuerySystemInformation) {
        printf("[-] Failed to get NtQuerySystemInformation address.\n");
        return 0;
    }

    static const ULONG SystemModuleInformation = 11;
    ULONG size = 0;
    LONG status = pNtQuerySystemInformation(SystemModuleInformation, NULL, 0, &size);
    
    static const LONG STATUS_INFO_LENGTH_MISMATCH = 0xC0000004;
    if (status != STATUS_INFO_LENGTH_MISMATCH) {
        printf("[-] Initial NtQuerySystemInformation failed with status: 0x%X\n", status);
        return 0;
    }
    
    printf("[+] Size: %lu\n", size);
    printf("[+] SystemModuleInformation: %lu\n", SystemModuleInformation);

    size += 1024;
    PSYSTEM_MODULE_INFORMATION pModInfo = (PSYSTEM_MODULE_INFORMATION)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pModInfo) return 0;

    status = pNtQuerySystemInformation(SystemModuleInformation, pModInfo, size, &size);
    if (status != 0) {
        printf("[-] Failed to get kernel base address. Status: 0x%X\n", status);
        VirtualFree(pModInfo, 0, MEM_RELEASE);
        return 0;
    }

    ULONG64 baseAddress = (ULONG64)pModInfo->Modules[0].ImageBaseAddress;
    printf("[+] Base Address: 0x%llX\n", baseAddress);
    
    char name[256];
    memset(name, 0, sizeof(name));
    memcpy(name, pModInfo->Modules[0].Name, 255);
    printf("[+] Kernel Name: %s\n", name);
    
    VirtualFree(pModInfo, 0, MEM_RELEASE);
    return baseAddress;
}

int wmain(int argc, wchar_t* argv[])
{
    printf("[+] Getting kernel base address...\n");
    ULONG64 kernelBase = GetKernelBaseAddress();
    printf("[+] Kernel Base: 0x%llX\n", kernelBase);
    return 0;
}