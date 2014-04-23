/* This file is a part of NXVM project. */

#ifndef NXVM_QDMISC_H
#define NXVM_QDMISC_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#define SOFT_MISC_INT_11 "\
qdx 02 ; enter isr \n\
; device test           \n\
push ds                 \n\
push bx                 \n\
mov bx, 40              \n\
mov ds, bx              \n\
pop bx                  \n\
mov ax, [0010]          \n\
pop ds                  \n\
qdx 03 ; leave isr \n\
iret                    \n"

#define SOFT_MISC_INT_12 "\
qdx 02 ; enter isr \n\
; memory test           \n\
push ds                 \n\
push bx                 \n\
mov bx, 40              \n\
mov ds, bx              \n\
pop bx                  \n\
mov ax, [0013]          \n\
pop ds                  \n\
qdx 03 ; leave isr \n\
iret                    \n"

#define SOFT_MISC_INT_15 "          \
qdx 02 ; enter isr \n\
cmp ah, 24                        \n\
jnz $(label_int_15_cmp_88)        \n\
jmp near $(label_int_15_24)       \n\
$(label_int_15_cmp_88):           \n\
cmp ah, 88                        \n\
jnz $(label_int_15_cmp_c0)        \n\
jmp near $(label_int_15_88)       \n\
$(label_int_15_cmp_c0):           \n\
cmp ah, c0                        \n\
jnz $(label_int_15_cmp_d8)        \n\
jmp near $(label_int_15_c0)       \n\
$(label_int_15_cmp_d8):           \n\
cmp ah, d8                        \n\
jnz $(label_int_15_default)       \n\
jmp near $(label_int_15_d8)       \n\
$(label_int_15_default):          \n\
jmp near $(label_int_15_ret)      \n\
\
$(label_int_15_24):               \n\
cmp al, 03                        \n\
jnz $(label_int_15_24_ret)        \n\
mov ah, 00                        \n\
mov bx, 03                        \n\
clc                               \n\
jmp near $(label_int_15_set_flag) \n\
$(label_int_15_24_ret):           \n\
jmp near $(label_int_15_ret)      \n\
\
$(label_int_15_88):               \n\
mov ax, 0800 ; 2048 KB            \n\
; if memory size > 16 M, ret 3c00 \n\
sub ax, 0400                      \n\
clc                               \n\
jmp near $(label_int_15_set_flag) \n\
\
$(label_int_15_c0):               \n\
mov bx, f000                      \n\
mov es, bx                        \n\
mov bx, e6f5                      \n\
mov ah, 00                        \n\
clc                               \n\
jmp near $(label_int_15_set_flag) \n\
\
$(label_int_15_d8):               \n\
mov ah, 86                        \n\
stc                               \n\
jmp near $(label_int_15_set_flag) \n\
\
$(label_int_15_set_flag): \n\
; set/clear cf            \n\
push ax                   \n\
push bx                   \n\
pushf                     \n\
pop ax                    \n\
and ax, 01                \n\
mov bx, sp                \n\
ss:                       \n\
and word [bx+8], fffe     \n\
ss:                       \n\
or  word [bx+8], ax       \n\
pop bx                    \n\
pop ax                    \n\
\
$(label_int_15_ret): \n\
qdx 03 ; leave isr \n\
iret                 \n"

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
