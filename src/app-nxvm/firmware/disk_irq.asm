; Copyright 2026 Neko.
; IRQ14: release the ATA interrupt source, then both cascaded PIC service bits.
; Command/result interpretation remains in the polling INT 13h service.
push ax
push dx
mov dx, 01f7
in al, dx
mov al, 20
out a0, al
out 20, al
pop dx
pop ax
iret
