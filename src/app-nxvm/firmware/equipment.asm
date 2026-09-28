; Copyright 2012-2014 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
; device test
push ds
push bx
mov bx, 0040
mov ds, bx
pop bx
mov ax, ds:[0010]
pop ds
iret
