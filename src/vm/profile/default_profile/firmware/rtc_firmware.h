/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_RTC_FIRMWARE_H
#define VM_PROFILE_DEFAULT_RTC_FIRMWARE_H

#ifdef __cplusplus
extern "C" {
#endif


#define VCMOS_POST "            \
; init cmos                   \n\
mov al, 0b ; select reg b     \n\
out 70, al                    \n\
 mov al, 02 ; 24 hour mode     \n\
out 71, al                    \n\
\
; init vrtc                   \n\
mov ah, 02 ; ch,cl,dh         \n\
int 1a     ; get cmos STD_TIME    \n\
\
mov bh, ch ; convert ch       \n\
and bh, 0f                    \n\
shr ch, 01                    \n\
shr ch, 01                    \n\
shr ch, 01                    \n\
shr ch, 01                    \n\
mov al, ch                    \n\
mov ch, 0a                    \n\
mul ch                        \n\
add bh, al                    \n\
mov ch, bh ; ch is hex now    \n\
\
mov bh, cl ; convert cl       \n\
and bh, 0f                    \n\
shr cl, 01                    \n\
shr cl, 01                    \n\
shr cl, 01                    \n\
shr cl, 01                    \n\
mov al, cl                    \n\
mov cl, 0a                    \n\
mul cl                        \n\
add bh, al                    \n\
mov cl, bh ; cl is hex now    \n\
\
mov bh, dh ; convert dh       \n\
and bh, 0f                    \n\
shr dh, 01                    \n\
shr dh, 01                    \n\
shr dh, 01                    \n\
shr dh, 01                    \n\
mov al, dh                    \n\
mov dh, 0a                    \n\
mul dh                        \n\
add bh, al                    \n\
mov dh, bh ; dh is hex now    \n\
\
mov al, ch ; x = hour         \n\
mov bl, 3c                    \n\
mul bl     ; x *= 60          \n\
mov ch, 00                    \n\
add ax, cx ; x += min         \n\
xor cx, cx                    \n\
mov cl, dh                    \n\
mov bx, 003c                  \n\
mul bx     ; x *= 60          \n\
add ax, cx ; x += second      \n\
mov bx, 0040                  \n\
mov ds, bx                    \n\
mov cx, dx                    \n\
mov bx, 0012                  \n\
mul bx      ; x *= 18         \n\
mov ds:[006c], ax             \n\
mov ds:[006e], dx             \n\
mov ax, cx                    \n\
mul bx                        \n\
add ds:[006e], ax             \n"

#define VCMOS_INT_HARD_TIMER_08 "           \
cli                                       \n\
push ds                                   \n\
push ax                                   \n\
pushf                                     \n\
mov ax, 0040                              \n\
mov ds, ax                                \n\
add word ds:[006c], 01 ; increase tick count \n\
adc word ds:[006e], 00                       \n\
cmp word ds:[006c], 00b0 ; test timer rollover \n\
jnz $(label_int_08_1)                     \n\
cmp word ds:[006e], 0018                  \n\
jnz $(label_int_08_1)                     \n\
mov word ds:[006c], 0000 ; execute timer rollover \n\
mov word ds:[006e], 0000                     \n\
mov byte ds:[0070], 01                       \n\
$(label_int_08_1):                        \n\
popf                                      \n\
pop ax                                    \n\
pop ds                                    \n\
int 1c              ; call int 1c         \n\
push ax                                   \n\
push dx                                   \n\
mov al, 20          ; send eoi command    \n\
mov dx, 0020                              \n\
out dx, al                                \n\
pop dx                                    \n\
pop ax                                    \n\
sti                                       \n\
iret                                      \n"

#define VCMOS_INT_SOFT_TIMER_1A "\
push bx                \n\
push ds                \n\
mov bx, 0040           \n\
mov ds, bx             \n\
\
cmp ah, 00                         \n\
jnz $(label_int_1a_cmp_01)         \n\
jmp near $(label_int_1a_get_tick)  \n\
$(label_int_1a_cmp_01):            \n\
cmp ah, 01                         \n\
jnz $(label_int_1a_cmp_02)         \n\
jmp near $(label_int_1a_set_tick)  \n\
$(label_int_1a_cmp_02):            \n\
cmp ah, 02                         \n\
jnz $(label_int_1a_cmp_03)         \n\
jmp near $(label_int_1a_get_time)  \n\
$(label_int_1a_cmp_03):            \n\
cmp ah, 03                         \n\
jnz $(label_int_1a_cmp_04)         \n\
jmp near $(label_int_1a_set_time)  \n\
$(label_int_1a_cmp_04):            \n\
cmp ah, 04                         \n\
jnz $(label_int_1a_cmp_05)         \n\
jmp near $(label_int_1a_get_date)  \n\
$(label_int_1a_cmp_05):            \n\
cmp ah, 05                         \n\
jnz $(label_int_1a_cmp_06)         \n\
jmp near $(label_int_1a_set_date)  \n\
$(label_int_1a_cmp_06):            \n\
cmp ah, 06                         \n\
jnz $(label_int_1a_cmp_def)        \n\
jmp near $(label_int_1a_set_alarm) \n\
$(label_int_1a_cmp_def):           \n\
jmp near $(label_int_1a_ret)       \n\
\
$(label_int_1a_get_tick):    ; get STD_TIME tick count        \n\
mov cx, ds:[006e]                                         \n\
mov dx, ds:[006c]                                         \n\
mov al, ds:[0070]                                         \n\
mov byte ds:[0070], 00                                    \n\
jmp near $(label_int_1a_ret)                              \n\
\
$(label_int_1a_set_tick):    ; set STD_TIME tick count        \n\
mov ds:[006e], cx                                         \n\
mov ds:[006c], dx                                         \n\
mov byte ds:[0070], 00                                    \n\
jmp near $(label_int_1a_ret)                              \n\
\
$(label_int_1a_get_time):    ; get cmos STD_TIME              \n\
mov al, 00                   ; read cmos second register  \n\
out 70, al                                                \n\
in  al, 71                                                \n\
mov dh, al                                                \n\
mov al, 02                   ; read cmos minute register  \n\
out 70, al                                                \n\
in  al, 71                                                \n\
mov cl, al                                                \n\
mov al, 04                   ; read cmos hour register    \n\
out 70, al                                                \n\
in  al, 71                                                \n\
mov ch, al                                                \n\
mov al, 0b                   ; read cmos register b       \n\
out 70, al                                                \n\
in  al, 71                                                \n\
and al, 01                                                \n\
mov dl, al                                                \n\
clc                                                       \n\
jmp near $(label_int_1a_set_flag)                         \n\
\
$(label_int_1a_set_time):    ; set cmos STD_TIME              \n\
mov al, 00                   ; write cmos second register \n\
out 70, al                                                \n\
mov al, dh                                                \n\
out 71, al                                                \n\
mov al, 02                   ; write cmos minute register \n\
out 70, al                                                \n\
mov al, cl                                                \n\
out 71, al                                                \n\
mov al, 04                   ; write cmos hour register   \n\
out 70, al                                                \n\
mov al, ch                                                \n\
out 71, al                                                \n\
mov al, 0b                   ; write cmos register b      \n\
out 70, al                                                \n\
in  al, 71                                                \n\
and dl, 01                                                \n\
and al, fe                                                \n\
or  dl, al                                                \n\
mov al, 0b                                                \n\
out 70, al                                                \n\
mov al, dl                                                \n\
out 71, al                                                \n\
clc                                                       \n\
jmp near $(label_int_1a_set_flag)                         \n\
\
$(label_int_1a_get_date):    ; get cmos date              \n\
mov al, 32                   ; read cmos century register \n\
out 70, al                                                \n\
in  al, 71                                                \n\
mov ch, al                                                \n\
mov al, 09                   ; read cmos year register    \n\
out 70, al                                                \n\
in  al, 71                                                \n\
mov cl, al                                                \n\
mov al, 08                   ; read cmos month register   \n\
out 70, al                                                \n\
in  al, 71                                                \n\
mov dh, al                                                \n\
mov al, 07                   ; read cmos mday register    \n\
out 70, al                                                \n\
in  al, 71                                                \n\
mov dl, al                                                \n\
clc                                                       \n\
jmp near $(label_int_1a_set_flag)                         \n\
\
$(label_int_1a_set_date):    ; set cmos date              \n\
mov al, 32                   ; write cmos century register\n\
out 70, al                                                \n\
mov al, ch                                                \n\
out 71, al                                                \n\
mov al, 09                   ; write cmos year register   \n\
out 70, al                                                \n\
in  al, 71                                                \n\
mov al, cl                                                \n\
out 71, al                                                \n\
mov al, 08                   ; write cmos month register  \n\
out 70, al                                                \n\
mov al, dh                                                \n\
out 71, al                                                \n\
mov al, 07                   ; write cmos mday register   \n\
out 70, al                                                \n\
mov al, dl                                                \n\
out 71, al                                                \n\
clc                                                       \n\
jmp near $(label_int_1a_set_flag)                         \n\
\
$(label_int_1a_set_alarm):   ; set alarm clock \n\
stc                          ; return a fail   \n\
jmp near $(label_int_1a_set_flag)              \n\
\
$(label_int_1a_set_flag):        \n\
pushf                            \n\
pop ax                           \n\
mov bx, sp                       \n\
and ax, 0001                     \n\
and word ss:[bx+08], fffe        \n\
or  word ss:[bx+08], ax          \n\
\
$(label_int_1a_ret): \n\
pop ds               \n\
pop bx               \n\
iret                 \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
