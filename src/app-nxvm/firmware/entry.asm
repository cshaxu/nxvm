; Copyright 2026 Neko.
; Guest initialization reconstructed from the owner-provided default ROM.
mov ax, f000
mov ds, ax
xor ax, ax
mov es, ax
mov si, f800
xor di, di
mov cx, 0200
cld
rep:
movsw
mov si, fc00
mov di, 0400
mov cx, 0100
rep:
movsw
mov ax, 0003
int 10
