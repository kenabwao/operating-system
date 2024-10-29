bits 16
org 0x0000

    jmp 0x07C0:_start
    volume_author       db  'FIRECORE'
    volume_label        db  'FIRE DRIVE '
    volume_serial       dd  0x00000000
    sector_size         dw  0x0200
    sector_per_cluster  db  0x08
    reserved_sectors    db  0x01
    total_sectors       dq  1981440
    sectors_per_track   dw  0
    total_heads         dw  0
    drive_number        db  0
    media_descriptor    db  0xF8

    %include "functions.inc"
    %include "variables.inc"
    %include "constants.inc"

_start:
    ;call ClearScreen
    call InitSegments

    ;load root directory after boot sector
    mov ax, 0x07E0
    mov es, ax
    xor edi, edi
    call LoadDirectory
    mov si, file_name
    call FindFile

    ;load file index sector after boot sector
    push eax                            ;save first cluster FIS
    mov ax, 0x07E0
    mov es, ax
    xor edi, edi
    call LoadFIS
    pop eax
    call LoadFile

    jmp LOADER_ADD:LOADER_OFF                          

times 510-($-$$) db 0               ; file size must be 512. Pad file to 510 bytes
dw 0xAA55