; Copyright 2012-2014 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
cli
push ds
push ax
pushf
mov ax, 0040
mov ds, ax
add word ds:[006c], 01 ; increase tick count
adc word ds:[006e], 00
cmp word ds:[006c], 00b0 ; test timer rollover
jnz $(label_int_08_1)
cmp word ds:[006e], 0018
jnz $(label_int_08_1)
mov word ds:[006c], 0000 ; execute timer rollover
mov word ds:[006e], 0000
mov byte ds:[0070], 01
$(label_int_08_1):
popf
pop ax
pop ds
int 1c              ; call int 1c
push ax
push dx
mov al, 20          ; send eoi command
mov dx, 0020
out dx, al
pop dx
pop ax
sti
iret
