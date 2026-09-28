; Copyright 2012-2014 Neko.
; Recovered guest assembly; source provenance is recorded in README.md.
push bx
push ds
mov bx, 0040
mov ds, bx
cmp ah, 00
jnz $(label_int_1a_cmp_01)
jmp near $(label_int_1a_get_tick)
$(label_int_1a_cmp_01):
cmp ah, 01
jnz $(label_int_1a_cmp_02)
jmp near $(label_int_1a_set_tick)
$(label_int_1a_cmp_02):
cmp ah, 02
jnz $(label_int_1a_cmp_03)
jmp near $(label_int_1a_get_time)
$(label_int_1a_cmp_03):
cmp ah, 03
jnz $(label_int_1a_cmp_04)
jmp near $(label_int_1a_set_time)
$(label_int_1a_cmp_04):
cmp ah, 04
jnz $(label_int_1a_cmp_05)
jmp near $(label_int_1a_get_date)
$(label_int_1a_cmp_05):
cmp ah, 05
jnz $(label_int_1a_cmp_06)
jmp near $(label_int_1a_set_date)
$(label_int_1a_cmp_06):
cmp ah, 06
jnz $(label_int_1a_cmp_def)
jmp near $(label_int_1a_set_alarm)
$(label_int_1a_cmp_def):
jmp near $(label_int_1a_ret)
$(label_int_1a_get_tick):    ; get STD_TIME tick count
mov cx, ds:[006e]
mov dx, ds:[006c]
mov al, ds:[0070]
mov byte ds:[0070], 00
jmp near $(label_int_1a_ret)
$(label_int_1a_set_tick):    ; set STD_TIME tick count
mov ds:[006e], cx
mov ds:[006c], dx
mov byte ds:[0070], 00
jmp near $(label_int_1a_ret)
$(label_int_1a_get_time):    ; get cmos STD_TIME
mov al, 00                   ; read cmos second register
out 70, al
in  al, 71
mov dh, al
mov al, 02                   ; read cmos minute register
out 70, al
in  al, 71
mov cl, al
mov al, 04                   ; read cmos hour register
out 70, al
in  al, 71
mov ch, al
mov al, 0b                   ; read cmos register b
out 70, al
in  al, 71
and al, 01
mov dl, al
clc
jmp near $(label_int_1a_set_flag)
$(label_int_1a_set_time):    ; set cmos STD_TIME
mov al, 00                   ; write cmos second register
out 70, al
mov al, dh
out 71, al
mov al, 02                   ; write cmos minute register
out 70, al
mov al, cl
out 71, al
mov al, 04                   ; write cmos hour register
out 70, al
mov al, ch
out 71, al
mov al, 0b                   ; write cmos register b
out 70, al
in  al, 71
and dl, 01
and al, fe
or  dl, al
mov al, 0b
out 70, al
mov al, dl
out 71, al
clc
jmp near $(label_int_1a_set_flag)
$(label_int_1a_get_date):    ; get cmos date
mov al, 32                   ; read cmos century register
out 70, al
in  al, 71
mov ch, al
mov al, 09                   ; read cmos year register
out 70, al
in  al, 71
mov cl, al
mov al, 08                   ; read cmos month register
out 70, al
in  al, 71
mov dh, al
mov al, 07                   ; read cmos mday register
out 70, al
in  al, 71
mov dl, al
clc
jmp near $(label_int_1a_set_flag)
$(label_int_1a_set_date):    ; set cmos date
mov al, 32                   ; write cmos century register
out 70, al
mov al, ch
out 71, al
mov al, 09                   ; write cmos year register
out 70, al
in  al, 71
mov al, cl
out 71, al
mov al, 08                   ; write cmos month register
out 70, al
mov al, dh
out 71, al
mov al, 07                   ; write cmos mday register
out 70, al
mov al, dl
out 71, al
clc
jmp near $(label_int_1a_set_flag)
$(label_int_1a_set_alarm):   ; set alarm clock
stc                          ; return a fail
jmp near $(label_int_1a_set_flag)
$(label_int_1a_set_flag):
pushf
pop ax
mov bx, sp
and ax, 0001
and word ss:[bx+08], fffe
or  word ss:[bx+08], ax
$(label_int_1a_ret):
pop ds
pop bx
iret
