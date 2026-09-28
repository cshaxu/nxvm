; Copyright 2012-2014 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
cmp ah, 00
jz $(keyboard_16_read)
cmp ah, 10
jz $(keyboard_16_read)
cmp ah, 01
jz $(keyboard_16_status)
cmp ah, 11
jz $(keyboard_16_status)
cmp ah, 02
jz $(keyboard_16_flags)
cmp ah, 05
jz $(keyboard_16_store)
iret
$(keyboard_16_read):
push bx
push ds
mov ax, 0040
mov ds, ax
$(keyboard_16_wait):
mov bx, ds:[001a]
cmp bx, ds:[001c]
jnz $(keyboard_16_read_ready)
sti
hlt
cli
jmp near $(keyboard_16_wait)
$(keyboard_16_read_ready):
push ds
xor ax, ax
mov ds, ax
mov ax, ds:[bx]
pop ds
add bx, 0002
cmp bx, 043e
jb $(keyboard_16_head_ready)
mov bx, 041e
$(keyboard_16_head_ready):
mov ds:[001a], bx
pop ds
pop bx
iret
$(keyboard_16_status):
push bx
push ds
mov ax, 0040
mov ds, ax
mov bx, ds:[001a]
cmp bx, ds:[001c]
jnz $(keyboard_16_status_ready)
mov bx, sp
or word ss:[bx+08], 0040
jmp near $(keyboard_16_status_done)
$(keyboard_16_status_ready):
push ds
xor ax, ax
mov ds, ax
mov ax, ds:[bx]
pop ds
mov bx, sp
and word ss:[bx+08], ffbf
$(keyboard_16_status_done):
pop ds
pop bx
iret
$(keyboard_16_flags):
push bx
push ds
mov bx, 0040
mov ds, bx
mov al, ds:[0017]
pop ds
pop bx
iret
$(keyboard_16_store):
push bx
push ds
push si
mov ax, 0040
mov ds, ax
mov bx, ds:[001c]
mov si, bx
add si, 0002
cmp si, 043e
jb $(keyboard_16_store_tail_ready)
mov si, 041e
$(keyboard_16_store_tail_ready):
cmp si, ds:[001a]
jz $(keyboard_16_store_full)
mov ax, cx
push ds
push ax
xor ax, ax
mov ds, ax
pop ax
mov ds:[bx], ax
pop ds
mov ds:[001c], si
xor al, al
jmp near $(keyboard_16_store_done)
$(keyboard_16_store_full):
mov al, 01
$(keyboard_16_store_done):
pop si
pop ds
pop bx
iret
