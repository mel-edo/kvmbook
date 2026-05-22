BITS 16
ORG 0x7C00

start:
    cli             ; clear interrupt (disable interrupts) - we don't have IDT (interrupt descriptor table) yet
                    ; if an interrupt fires now, the CPU would triple fault (crashes)

    xor ax, ax      ; ax = 0
    mov ds, ax      ; data segment = 0
    mov es, ax      ; extra segment = 0
    mov ss, ax      ; stack segment = 0
    mov sp, 0x7C00  ; stack pointer just below our code
                    ; stack grows downward, so this is safe
    lgdt [gdt_descriptor]       ; load the GDT (global descriptor table) register with our table

    hlt

; GDT - Global Descriptor Table
gdt_start:

gdt_null:           ; entry 0 - must always be all zeros (cpu requirement)
    dq 0x0000000000000000

gdt_code:           ; entry 1 - 64-bit code segment
    dw 0xFFFF       ; limit[0:15] = 0xFFFF
    dw 0x0000       ; base[0:15] = 0x0000
    db 0x00         ; base[16:23] = 0x00
    db 10011010b    ; access byte: present, ring 0, code segment, executable, readable
    db 10101111b    ; flags + limit[16:19]: 4k granularity, 64-bit, limit top nibble
    db 0x00         ; base[24:31] = 0x00

gdt_data:           ; entry 2 - data segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b    ; access byte: present, ring 0, data segment, writable
    db 11001111b    ; flags: 4k granularity, 32-bit (data segments don't use the 64-bit flag)
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1      ; size of GDT minus 1 (cpu requirement)
    dd gdt_start                    ; 32-bit address of GDT (global descriptor table)

; Pad with zeros until byte 510, then write boot signature
times 510 - ($ - $$) db 0
dw 0xAA55