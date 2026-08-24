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

#define VCMOS_INT_SOFT_TIMER_1A(CHUNK) \
    CHUNK("push bx                \npush ds                \nmov bx, 0040           \nmov ds, bx             \ncmp ah, 00                         \njnz $(label_int_1a_cmp_01)         \njmp near $(label_int_1a_get_tick)  \n$(label_int_1a_cmp_01):            \ncmp ah, 01                         \njnz $(label_int_1a_cmp_02)         \njmp near $(label_int_1a_set_tick)  \n$(label_int_1a_cmp_02):            \ncmp ah, 02                         \njnz $(label_int_1a_cmp_03)         \njmp near $(label_int_1a_get_time)  \n$(label_int_1a_cmp_03):            \ncmp ah, 03                         \njnz $(label_int_1a_cmp_04)         \njmp near $(label_int_1a_set_time)  \n$(label_int_1a_cmp_04):            \ncmp ah, 04                         \njnz $(label_int_1a_cmp_05)         \njmp near $(label_int_1a_get_date)  \n$(label_int_1a_cmp_05):            \ncmp ah, 05                         \njnz $(label_int_1a_cmp_06)         \njmp near $(label_int_1a_set_date)  \n$(label_int_1a_cmp_06):            \ncmp ah, 06                         \njnz $(label_int_1a_cmp_def)        \njmp near $(label_int_1a_set_alarm) \n$(label_int_1a_cmp_def):           \njmp near $(label_int_1a_ret)       \n$(label_int_1a_get_tick):    ; get STD_TIME tick count        \nmov cx, ds:[006e]                                         \nmov dx, ds:[006c]                                         \nmov al, ds:[0070]                                         \nmov byte ds:[0070], 00                                    \njmp near $(label_int_1a_ret)                              \n$(label_int_1a_set_tick):    ; set STD_TIME tick count        \nmov ds:[006e], cx                                         \nmov ds:[006c], dx                                         \nmov byte ds:[0070], 00                                    \njmp near $(label_int_1a_ret)                              \n$(label_int_1a_get_time):    ; get cmos STD_TIME              \nmov al, 00                   ; read cmos second register  \nout 70, al                                                \nin  al, 71                                                \nmov dh, al                                                \nmov al, 02                   ; read cmos minute register  \nout 70, al                                                \nin  al, 71                                                \nmov cl, al                                                \nmov al, 04                   ; read cmos hour register    \nout 70, al                                                \nin  al, 71                                                \nmov ch, al                                                \nmov al, 0b                   ; read cmos register b       \nout 70, al                                                \nin  al, 71                                                \nand al, 01                                                \nmov dl, al                                                \nclc                                                       \njmp near $(label_int_1a_set_flag)                         \n$(label_int_1a_set_time):    ; set cmos STD_TIME              \nmov al, 00                   ; write cmos second register \nout 70, al                                                \nmov al, dh                                                \nout 71, al                                                \nmov al, 02                   ; write cmos minute register \nout 70, al                                                \n") \
    CHUNK("mov al, cl                                                \nout 71, al                                                \nmov al, 04                   ; write cmos hour register   \nout 70, al                                                \nmov al, ch                                                \nout 71, al                                                \nmov al, 0b                   ; write cmos register b      \nout 70, al                                                \nin  al, 71                                                \nand dl, 01                                                \nand al, fe                                                \nor  dl, al                                                \nmov al, 0b                                                \nout 70, al                                                \nmov al, dl                                                \nout 71, al                                                \nclc                                                       \njmp near $(label_int_1a_set_flag)                         \n$(label_int_1a_get_date):    ; get cmos date              \nmov al, 32                   ; read cmos century register \nout 70, al                                                \nin  al, 71                                                \nmov ch, al                                                \nmov al, 09                   ; read cmos year register    \nout 70, al                                                \nin  al, 71                                                \nmov cl, al                                                \nmov al, 08                   ; read cmos month register   \nout 70, al                                                \nin  al, 71                                                \nmov dh, al                                                \nmov al, 07                   ; read cmos mday register    \nout 70, al                                                \nin  al, 71                                                \nmov dl, al                                                \nclc                                                       \njmp near $(label_int_1a_set_flag)                         \n$(label_int_1a_set_date):    ; set cmos date              \nmov al, 32                   ; write cmos century register\nout 70, al                                                \nmov al, ch                                                \nout 71, al                                                \nmov al, 09                   ; write cmos year register   \nout 70, al                                                \nin  al, 71                                                \nmov al, cl                                                \nout 71, al                                                \nmov al, 08                   ; write cmos month register  \nout 70, al                                                \nmov al, dh                                                \nout 71, al                                                \nmov al, 07                   ; write cmos mday register   \nout 70, al                                                \nmov al, dl                                                \nout 71, al                                                \nclc                                                       \njmp near $(label_int_1a_set_flag)                         \n$(label_int_1a_set_alarm):   ; set alarm clock \n") \
    CHUNK("stc                          ; return a fail   \njmp near $(label_int_1a_set_flag)              \n$(label_int_1a_set_flag):        \npushf                            \npop ax                           \nmov bx, sp                       \nand ax, 0001                     \nand word ss:[bx+08], fffe        \nor  word ss:[bx+08], ax          \n$(label_int_1a_ret): \npop ds               \npop bx               \niret                 \n")


#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
