/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VHDC_H
#define NXVM_VHDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_HDC "Unknown"

void vhdcRegister();

#define VHDC_INT_SOFT_HDD_13 "\
qdx 02 ; leave isr          \n\
test dl, 80                 \n\
jnz $(label_int_13_hdd)     \n\
int 40                      \n\
jmp near $(label_int_13_end)\n\
$(label_int_13_hdd):        \n\
$(label_int_13_cmp_00):     \n\
cmp ah, 00                  \n\
jnz $(label_int_13_cmp_01)  \n\
jmp near $(label_int_13_00) \n\
$(label_int_13_cmp_01):     \n\
cmp ah, 01                  \n\
jnz $(label_int_13_cmp_02)  \n\
jmp near $(label_int_13_01) \n\
$(label_int_13_cmp_02):     \n\
cmp ah, 02                  \n\
jnz $(label_int_13_cmp_03)  \n\
jmp near $(label_int_13_02) \n\
$(label_int_13_cmp_03):     \n\
cmp ah, 03                  \n\
jnz $(label_int_13_cmp_08)  \n\
jmp near $(label_int_13_03) \n\
$(label_int_13_cmp_08):     \n\
cmp ah, 08                  \n\
jnz $(label_int_13_cmp_15)  \n\
jmp near $(label_int_13_08) \n\
$(label_int_13_cmp_15):     \n\
cmp ah, 15                  \n\
jnz $(label_int_13_cmp_def) \n\
jmp near $(label_int_13_15) \n\
$(label_int_13_cmp_def):    \n\
clc                         \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_00):         \n\
; reset drive               \n\
cmp dl, 80                  \n\
jnz $(label_int_13_00_x)    \n\
mov ah, 00                  \n\
clc                         \n\
jmp near $(label_int_13_end)\n\
$(label_int_13_00_x):       \n\
mov ah, 0c                  \n\
stc                         \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_01):         \n\
; get hdd status byte       \n\
push bx                     \n\
push ds                     \n\
mov bx, 0040                \n\
mov ds, bx                  \n\
mov ah, ds:[0074]           \n\
pop ds                      \n\
pop bx                      \n\
clc                         \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_02):         \n\
qdx a2 \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_03):         \n\
qdx a3 \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_08):         \n\
; get hdd parameters        \n\
push ax                     \n\
push bx                     \n\
push ds                     \n\
mov ax, 0000                \n\
mov ds, ax                  \n\
mov bx, ds:[0104]           \n\
mov ax, ds:[0106]           \n\
mov ds, ax                  \n\
mov cx, ds:[bx+00]          \n\
dec cx          ; ncyl - 1  \n\
xchg ch, cl                 \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
mov al, ds:[bx+04] ; nsector\n\
or  cl, al ; (ncyl>>2)&0xc0 \n\
           ; | nsector      \n\
mov dh, ds:[bx+02]          \n\
dec dh          ; nhead - 1 \n\
mov ax, 0040                \n\
mov ds, ax                  \n\
mov dl, ds:[0075]           \n\
pop ds                      \n\
pop bx                      \n\
pop ax                      \n\
mov ah, 00                  \n\
clc                         \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_15):         \n\
; get drive type            \n\
; count=(ncyl-1)*nhead*nsec \n\
push bx                     \n\
push ds                     \n\
mov ax, 0000                \n\
mov ds, ax                  \n\
mov bx, ds:[0104]           \n\
mov ax, ds:[0106]           \n\
mov ds, ax                  \n\
mov cx, ds:[bx+00]          \n\
dec cx          ; ncyl - 1  \n\
mov al, ds:[bx+04] ; nsector\n\
mov dh, ds:[bx+02] ; nhead  \n\
mov ah, 00                  \n\
mul dh ; nhead * nsector    \n\
mul cx ; total size         \n\
mov cx, dx ; size high 16   \n\
mov dx, ax ; size low  16   \n\
pop ds                      \n\
pop bx                      \n\
mov ax, 0003                \n\
clc                         \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_end):        \n\
; set hdd status byte       \n\
push bx                     \n\
push ds                     \n\
mov bx, 0040                \n\
mov ds, bx                  \n\
mov ds:[0074], ah           \n\
pop ds                      \n\
pop bx                      \n\
; set/clear cf              \n\
push ax                     \n\
push bx                     \n\
pushf                       \n\
pop ax                      \n\
and ax, 0001                \n\
mov bx, sp                  \n\
and word ss:[bx+08], fffe   \n\
or  word ss:[bx+08], ax     \n\
pop bx                      \n\
pop ax                      \n\
qdx 03 ; leave isr          \n\
iret                        \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
