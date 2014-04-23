/* This file is a part of NXVM project. */

#ifndef NXVM_QDRTC_H
#define NXVM_QDRTC_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#define HARD_RTC_INT_08 "                   \
qdx 02 ; enter isr \n\
cli                                       \n\
push ds                                   \n\
push ax                                   \n\
pushf                                     \n\
mov ax, 40                                \n\
mov ds, ax                                \n\
add word [006c], 1  ; increase tick count \n\
adc word [006e], 0                        \n\
cmp word [006c], b2 ; test rtc rollover   \n\
jnz $(label_int_08_1)                     \n\
cmp word [006e], 18                       \n\
jnz $(label_int_08_1)                     \n\
mov word [006c], 0  ; exec rtc rollover   \n\
mov word [006e], 0                        \n\
mov byte [0070], 1                        \n\
$(label_int_08_1):                        \n\
popf                                      \n\
pop ax                                    \n\
pop ds                                    \n\
int 1c              ; call int 1c         \n\
push ax                                   \n\
push dx                                   \n\
mov al, 20          ; send eoi command    \n\
mov dx, 20                                \n\
out dx, al                                \n\
pop dx                                    \n\
pop ax                                    \n\
sti                                       \n\
qdx 03 ; leave isr \n\
iret                                      \n"

#define SOFT_RTC_INT_1A "\
qdx 02 ; enter isr \n\
push bx                \n\
push ds                \n\
mov bx, 40             \n\
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
$(label_int_1a_get_tick):    ; get time tick count        \n\
mov cx, [006e]                                            \n\
mov dx, [006c]                                            \n\
mov al, [0070]                                            \n\
mov byte [0070], 00                                       \n\
jmp near $(label_int_1a_ret)                              \n\
\
$(label_int_1a_set_tick):    ; set time tick count        \n\
mov [006e], cx                                            \n\
mov [006c], dx                                            \n\
mov byte [0070], 00                                       \n\
jmp near $(label_int_1a_ret)                              \n\
\
$(label_int_1a_get_time):    ; get cmos time              \n\
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
$(label_int_1a_set_time):    ; set cmos time              \n\
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
and ax, 01                       \n\
ss:                              \n\
and word [bx+08], fffe           \n\
ss:                              \n\
or  word [bx+08], ax             \n\
\
$(label_int_1a_ret): \n\
pop ds               \n\
pop bx               \n\
qdx 03 ; leave isr \n\
iret                 \n"

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
