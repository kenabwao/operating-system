InitSegments:
    ;setup the segment registers
    mov ax, 0x07C0
    mov ds, ax                          ;set data segment to 0x07C0
    mov es, ax                          ;set extra segment to 0x07C0
    mov fs, ax                          ;set general purpose segment to 0x07C0
    mov gs, ax                          ;set general purpose segment to 0x07C0
    mov ss, ax                          ;set stacK segment to 0x07C0
    mov sp, 0xFFFF                      ;set pointer top of stack
    mov bp, 0xF000                      ;set pointer bottom of stack
ret
;____________________________________________________________________
;
;                       VIDEO FUNCTIONS
;____________________________________________________________________
ClearScreen:
    xor ah, ah
    mov al, 0x03
    int 0x10
ret

PrintText:
    mov ah, 0x0E
    xor bx, bx
    ._print_loop:
        lodsb
        cmp al, 0
        je ._done
        int 0x10
        jmp ._print_loop
    ._done:
ret

KeyPress:
    ._check_if_keypressed:
        mov ah, 0x01
        int 0x16
        jz ._check_if_keypressed
    ._get_keypressed:
        xor ah, ah
        int 0x16
ret
;____________________________________________________________________
;
;                   DISK FUNCTIONS
;____________________________________________________________________

LoadSectorsHDD:
;====================================================================
;       EAX:     first linear block addres (LBA)
;       BL:     Drive number
;       CL:     Number of sectors to read  
;       RET:    AL
;====================================================================
    ;select drive, select LBA mode & set LBA (24-27 bits)
    mov dx, 0x01F6
    push eax
    shr eax, 24
    and al, 0x0F                                    ;clear upper nibble
    cmp bl, 1
    je ._use_disk_1
        or al, 0xE0                                     ;set bit 6 for LBA mode, bit 7 & 5 always high, bit 4 to select drive 0 or 1, lower nibble are bit 24-27 of LBA
        out dx, al
        jmp ._load_LBA
    ._use_disk_1:
        or al, 0xF0                                 ;set bit 6 for LBA mode, bit 7 & 5 always high, bit 4 to select drive 0 or 1, lower nibble are bit 24-27 of LBA
        out dx, al

    ._load_LBA:
        pop eax
        ;set LBA (0-23 bits)
        mov dx, 0x01F3
        out dx, al                                    ; LBA bits 0-7
        mov dx, 0x01F4
        shr eax, 8                                    ; LBA bits 8-15
        out dx, al
        mov dx, 0x01F5
        shr eax, 8                                   ; LBA bits 16-23
        out dx, al

    ; Set sector to read
    mov dx, 0x1F2
    mov al, cl
    out dx, al

    ; Send the read command (0x20 for PIO read)
    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    ._data_not_ready:
        mov dx, 0x1F7
        in al, dx
        test al, 0x08                    ; Sector buffer requires servicing
        jz ._data_not_ready

    ._read_data:
        mov dx, 0x01F0
        push cx
        mov cx, 128                     ;sector size in words    
        rep insd                        ;in to [ES:(E)DI]
        pop cx
        loop ._data_not_ready

    ; Check for errors
    mov dx, 0x01F7
    in al, dx
    test al, 0x01              ; Check Error bit
    jz ._exit_ok                 ; Jump to error handling

    ._exit_fail:
        mov al, 1
    ret

    ._exit_ok:
        xor al, al
ret

FatalError:
    call PrintText                              ;call function to print string
    call KeyPress
    jmp 0xFFFF:0x0000                           ;warm boot

;____________________________________________________________________
;
;                   FILE SYSTEM FUNCTIONS
;____________________________________________________________________
LoadFIS:
    xor eax, eax
    mov al, byte[reserved_sectors]                          ;start of FIS on disk
    mov bl, DRIVE_NO
    mov cl, 64
    call LoadSectorsHDD
    cmp al, 0
    je ._exit
    mov si, disk_err
    call FatalError
    ._exit:
ret
    
LoadDirectory:
    mov eax,FILE_DATA_SECTOR
    mov bl, DRIVE_NO
    mov cl, byte[sector_per_cluster]
    call LoadSectorsHDD  
    cmp al, 0
    je ._exit
    mov si, disk_err
    call FatalError
    ._exit:
ret

;====================================================================
;       SI:     file name to compare
;       RET:    EAX
;====================================================================
FindFile:
    xor di, di
    mov cx, 64                                  ; entries in cluster (4096/64)
    ._find_file:
        push cx
        push si
        push di
        mov cx, FILE_NAME_SIZE
        rep cmpsb
        pop di
        pop si
        pop cx
        jz ._file_found
        add di, DIRECTORY_ENTRY_SIZE
        loop ._find_file
    .file_not_found:
        mov si, no_file
        call FatalError
    ._file_found:
        add di, START_CLUSTER_OFF
        mov eax, dword[es:di]
ret

LoadFile:
    push eax
    mov ax, es
    mov fs, ax
    pop eax

    ._start:
        push eax                                    ;save pointer to next cluster
        ;calculate location of data
        shl eax, 3                                  ;multiply by 8 (sectors per cluster)
        add eax, FILE_DATA_SECTOR                   ;calculate location of data

        push eax                                    ;save LBA of file
        mov ax, LOADER_ADD
        mov es, ax
        mov di, LOADER_OFF  

        pop eax                                     ;restor LBA of file
    ._load_cluster:
        mov bl, DRIVE_NO
        mov cl, byte[sector_per_cluster]
        call LoadSectorsHDD
    ._check_for_next_cluster:
        pop eax                                 ;restore pointer to next cluster
        push edi
        shl eax, 2                              ;multiply by 4 as each enty is 4 bytes
        mov edi, eax
        mov eax, dword[fs:edi]
        pop edi
        cmp eax, END_OF_FILE
        jb ._start
    ._end_of_file:
ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;DEBUG FUNCTIONS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;    PrintChar:
;    mov ah, 0x0E
;    xor bx, bx
;    int 0x10
;    ret

;    PrintHex:
;    mov cx, 8
;    .next:
;        push eax
;        and eax, 0xF0000000
;        shr eax, 4
;        cmp eax, 0x09000000
;        ja  .hex
;            add eax, 0x30000000
;            jmp .print
;        .hex:
;            add eax, 0x37000000
;        .print:
;            shr eax, 24
;            call PrintChar
;            pop eax
;            shl eax, 4
;            loop .next
;        .done:
;    ret