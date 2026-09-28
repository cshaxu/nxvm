; Copyright 2012-2014 Neko.
; Fixed default-board baseline: 15 MiB above the first MiB.
cmp ah, 24
jnz $(label_int_15_cmp_88)
jmp near $(label_int_15_24)
$(label_int_15_cmp_88):
cmp ah, 88
jnz $(label_int_15_cmp_c0)
jmp near $(label_int_15_88)
$(label_int_15_cmp_c0):
cmp ah, c0
jnz $(label_int_15_cmp_d8)
jmp near $(label_int_15_c0)
$(label_int_15_cmp_d8):
cmp ah, d8
jnz $(label_int_15_default)
jmp near $(label_int_15_d8)
$(label_int_15_default):
mov ah, 86
stc
jmp near $(label_int_15_set_flag)
$(label_int_15_24):
cmp al, 03
jnz $(label_int_15_24_ret)
mov ah, 00
mov bx, 0003
clc
jmp near $(label_int_15_set_flag)
$(label_int_15_24_ret):
jmp near $(label_int_15_ret)
$(label_int_15_88):
mov ax, 3c00
clc
jmp near $(label_int_15_set_flag)
$(label_int_15_c0):
mov bx, f000
mov es, bx
mov bx, e6f5
mov ah, 00
clc
jmp near $(label_int_15_set_flag)
$(label_int_15_d8):
mov ah, 86
stc
jmp near $(label_int_15_set_flag)
$(label_int_15_set_flag):
; set/clear cf
push ax
push bx
pushf
pop ax
and ax, 0001
mov bx, sp
and word ss:[bx+08], fffe
or  word ss:[bx+08], ax
pop bx
pop ax
$(label_int_15_ret):
iret
