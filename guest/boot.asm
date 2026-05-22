BITS 16
ORG 0x7C00

start:
    cli             ; clear interrupt (disable intrrupt) - we don't have IDT (interrupt descriptor table) yet
                    ; if an interrupt fires now, the CPU would triple fault (crashes)

    xor ax, ax      ; ax = 0
    mov ds, ax      ; data segment = 0
    mov es, ax      ; extra segment = 0
    mov ss, ax      ; stack segment = 0
    mov sp, 0x7C00  ; stack pointer just below our code
                    ; stack go downward, so this is safe

; Pad with zeros until byte 510, then write boot signature
times 510 - ($ - $$) db 0
dw 0xAA55