.code
ALIGN 16

flourish PROC
    jmp entry

szGetProcAddress      db "GetProcAddress", 0
szOpenMutexW          db "OpenMutexW", 0
szOpenFileMappingW    db "OpenFileMappingW", 0
szMapViewOfFile       db "MapViewOfFile", 0
szWaitForSingleObject db "WaitForSingleObject", 0
szReleaseMutex        db "ReleaseMutex", 0

ALIGN 8
wszMutex   dw 'L','o','c','a','l','\','D','4','1','S','Y','.','R','e','p','o','r','t','.'
           dw 'M','u','t','e','x', 0
wszMapping dw 'L','o','c','a','l','\','D','4','1','S','Y','.','R','e','p','o','r','t', 0
wszMsgStart dw '[', 273Fh, ']',' ','s','t','a','r','t','e','d',' ','t','o',' ','t','h','r','i','v','e', 0
wszMsgEnd   dw '[', 273Fh, ']',' ','w','i','l','t','i','n','g','.','.','.', 0

ALIGN 16
entry:
    push rbp
    mov rbp, rsp
    and rsp, -16            ; Align stack to 16 bytes (crucial for Windows x64 ABI)
    
    ; Allocate 120h bytes (100h for locals/shadow space, 20h for saving registers)
    sub rsp, 120h
    mov [rsp+100h], rsi
    mov [rsp+108h], rdi
    mov [rsp+110h], rbx

    ; 1. Get Kernel32 Base via TEB->PEB->LDR
    mov rax, gs:[60h]       ; PEB
    mov rax, [rax + 18h]    ; PEB_LDR_DATA
    mov rax, [rax + 20h]    ; InMemoryOrderModuleList
    mov rax, [rax]          ; 2nd entry (ntdll)
    mov rax, [rax]          ; 3rd entry (kernel32)
    mov rbx, [rax + 20h]    ; DllBase
    mov [rbp-8], rbx        ; Save kernel32 base

    ; 2. Find GetProcAddress
    mov rcx, rbx
    lea rdx, [szGetProcAddress]
    call GetExportAddress
    mov [rbp-16], rax       ; Save GetProcAddress

    ; 3. Resolve APIs
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

    ; 4. Open Mutex
    ; OpenMutexW(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, wszMutex)
    mov rcx, 00100001h      ; SYNCHRONIZE | MUTEX_MODIFY_STATE
    xor rdx, rdx
    lea r8, [wszMutex]
    mov rax, [rbp-24]
    call rax
    mov [rbp-88], rax       ; hMutex
    test rax, rax
    jz Cleanup

    ; 5. Open File Mapping
    mov rcx, 2              ; FILE_MAP_WRITE
    xor rdx, rdx
    lea r8, [wszMapping]
    mov rax, [rbp-32]
    call rax
    mov [rbp-96], rax       ; hMapping
    test rax, rax
    jz Cleanup

    ; 6. Map View Of File
    mov rcx, [rbp-96]
    mov rdx, 2              ; FILE_MAP_WRITE
    xor r8, r8
    xor r9, r9
    mov qword ptr [rsp + 20h], 0 ; 5th arg
    mov rax, [rbp-40]
    call rax
    mov [rbp-104], rax       ; pBuffer
    test rax, rax
    jz Cleanup

    ; 7. Write First Message (REPORT_LEVEL_SUCCESS = 1)
    mov rcx, 1
    lea rdx, [wszMsgStart]
    mov r8, [rbp-88]
    mov r9, [rbp-104]
    mov rax, [rbp-48]
    mov [rsp+20h], rax
    mov rax, [rbp-56]
    mov [rsp+28h], rax
    call ReportMessage

    ; 8. Write Second Message (REPORT_LEVEL_INFO = 0)
    mov rcx, 0
    lea rdx, [wszMsgEnd]
    mov r8, [rbp-88]
    mov r9, [rbp-104]
    mov rax, [rbp-48]
    mov [rsp+20h], rax
    mov rax, [rbp-56]
    mov [rsp+28h], rax
    call ReportMessage

Cleanup:
    mov rsi, [rsp+100h]
    mov rdi, [rsp+108h]
    mov rbx, [rsp+110h]
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