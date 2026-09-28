; Copyright 2012-2014 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
push ax
push bx
push ds
push si
in al, 60
mov bl, al
xor bh, bh
mov ax, 0040
mov ds, ax
mov al, ds:[00b9]
or al, al
jz $(keyboard_09_pause_start)
dec al
mov ds:[00b9], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_pause_start):
cmp bx, 00e1
jnz $(keyboard_09_prefix)
mov al, 05
mov ds:[00b9], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_prefix):
cmp bx, 00e0
jnz $(keyboard_09_shift_left_make)
mov al, ds:[0018]
or al, 04
mov ds:[0018], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_shift_left_make):
cmp bx, 002a
jnz $(keyboard_09_shift_right_make)
mov al, ds:[0017]
or al, 02
mov ds:[0017], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_shift_right_make):
cmp bx, 0036
jnz $(keyboard_09_shift_left_break)
mov al, ds:[0017]
or al, 01
mov ds:[0017], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_shift_left_break):
cmp bx, 00aa
jnz $(keyboard_09_shift_right_break)
mov al, ds:[0017]
and al, fd
mov ds:[0017], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_shift_right_break):
cmp bx, 00b6
jnz $(keyboard_09_ctrl_make)
mov al, ds:[0017]
and al, fe
mov ds:[0017], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_ctrl_make):
cmp bx, 001d
jnz $(keyboard_09_ctrl_break)
mov al, ds:[0017]
or al, 04
mov ds:[0017], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_ctrl_break):
cmp bx, 009d
jnz $(keyboard_09_alt_make)
mov al, ds:[0017]
and al, fb
mov ds:[0017], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_alt_make):
cmp bx, 0038
jnz $(keyboard_09_alt_break)
mov al, ds:[0017]
or al, 08
mov ds:[0017], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_alt_break):
cmp bx, 00b8
jnz $(keyboard_09_extended)
mov al, ds:[0017]
and al, f7
mov ds:[0017], al
jmp near $(keyboard_09_eoi)
$(keyboard_09_extended):
mov al, ds:[0018]
test al, 04
jz $(keyboard_09_regular)
and al, fb
mov ds:[0018], al
cmp bx, 0080
jb $(keyboard_09_extended_make)
jmp near $(keyboard_09_eoi)
$(keyboard_09_extended_make):
xor al, al
jmp near $(keyboard_09_mapped)
$(keyboard_09_regular):
cmp bx, 0080
jb $(keyboard_09_normal)
jmp near $(keyboard_09_eoi)
$(keyboard_09_normal):
mov al, ds:[0017]
test al, 08
jz $(keyboard_09_not_alt)
xor al, al
jmp near $(keyboard_09_mapped)
$(keyboard_09_not_alt):
and al, 03
jnz $(keyboard_09_shift)
mov al, cs:[bx+e000]
jmp near $(keyboard_09_mapped)
$(keyboard_09_shift):
mov al, cs:[bx+e080]
$(keyboard_09_mapped):
mov ah, bl
mov bx, ds:[001c]
mov si, bx
add si, 0002
cmp si, 043e
jb $(keyboard_09_tail_ready)
mov si, 041e
$(keyboard_09_tail_ready):
cmp si, ds:[001a]
jz $(keyboard_09_eoi)
push ds
push ax
xor ax, ax
mov ds, ax
pop ax
mov ds:[bx], ax
pop ds
mov ds:[001c], si
$(keyboard_09_eoi):
mov al, 20
out 20, al
pop si
pop ds
pop bx
pop ax
iret
