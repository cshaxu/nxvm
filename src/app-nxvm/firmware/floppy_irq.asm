; Copyright 2012-2026 Neko.
; Notify only. INT 40h alone consumes SIS and command result bytes.
push ax
push ds
mov ax, 0040
mov ds, ax
or byte ds:[003e], 80
mov al, 20
out 20, al
pop ds
pop ax
iret
