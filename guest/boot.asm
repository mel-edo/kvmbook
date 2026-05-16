BITS 16
ORG 0x0

start:
    hlt

; Pad with zeros until byte 510, then write boot signature
times 510 - ($ - $$) db 0
dw 0xAA55