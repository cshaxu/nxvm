/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_FIRMWARE_HDC_H
#define VM_PROFILE_DEFAULT_FIRMWARE_HDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#define VM_PROFILE_DEFAULT_DEVICE_HDC "Unknown Hard Drive Controller"

#define VHDC_INT_SOFT_HDD_13 "\
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
mov ah, 01                  \n\
stc                         \n\
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
or ah, ah                   \n\
jnz $(label_int_13_01_fail) \n\
clc                         \n\
jmp near $(label_int_13_end)\n\
$(label_int_13_01_fail):    \n\
stc                         \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_02):         \n\
; read AL CHS sectors through the primary ATA PIO channel \n\
or al, al                   \n\
jz $(label_int_13_02_fail)  \n\
push ax                     \n\
push bx                     \n\
push cx                     \n\
push dx                     \n\
push si                     \n\
push di                     \n\
push es                     \n\
mov si, ax                  \n\
mov di, bx                  \n\
mov dx, 01f2                \n\
mov ax, si                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, cl                  \n\
and al, 3f                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, ch                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, cl                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, dh                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, 20                  \n\
out dx, al                  \n\
in al, dx                   \n\
test al, 01                 \n\
jnz $(label_int_13_02_pop_fail) \n\
test al, 08                 \n\
jz $(label_int_13_02_pop_fail) \n\
mov cx, si                  \n\
and cx, 00ff                \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
mov dx, 01f0                \n\
$(label_int_13_02_data):    \n\
in ax, dx                   \n\
stosw                       \n\
loop $(label_int_13_02_data)\n\
mov dx, 01f7                \n\
in al, dx                   \n\
pop es                      \n\
pop di                      \n\
pop si                      \n\
pop dx                      \n\
pop cx                      \n\
pop bx                      \n\
pop ax                      \n\
mov ah, 00                  \n\
clc                         \n\
jmp near $(label_int_13_end)\n\
$(label_int_13_02_pop_fail):\n\
pop es                      \n\
pop di                      \n\
pop si                      \n\
pop dx                      \n\
pop cx                      \n\
pop bx                      \n\
pop ax                      \n\
$(label_int_13_02_fail):    \n\
mov ah, 04                  \n\
stc                         \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_03):         \n\
; write AL CHS sectors through the primary ATA PIO channel \n\
or al, al                   \n\
jz $(label_int_13_03_fail)  \n\
push ax                     \n\
push bx                     \n\
push cx                     \n\
push dx                     \n\
push si                     \n\
push di                     \n\
push ds                     \n\
push es                     \n\
mov di, ax                  \n\
mov si, bx                  \n\
mov ax, es                  \n\
mov ds, ax                  \n\
mov dx, 01f2                \n\
mov ax, di                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, cl                  \n\
and al, 3f                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, ch                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, cl                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
shr al, 01                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, dh                  \n\
out dx, al                  \n\
inc dx                      \n\
mov al, 30                  \n\
out dx, al                  \n\
in al, dx                   \n\
test al, 01                 \n\
jnz $(label_int_13_03_pop_fail) \n\
test al, 08                 \n\
jz $(label_int_13_03_pop_fail) \n\
mov cx, di                  \n\
and cx, 00ff                \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
shl cx, 01                  \n\
mov dx, 01f0                \n\
$(label_int_13_03_data):    \n\
lodsw                       \n\
out dx, ax                  \n\
loop $(label_int_13_03_data)\n\
mov dx, 01f7                \n\
in al, dx                   \n\
pop es                      \n\
pop ds                      \n\
pop di                      \n\
pop si                      \n\
pop dx                      \n\
pop cx                      \n\
pop bx                      \n\
pop ax                      \n\
mov ah, 00                  \n\
clc                         \n\
jmp near $(label_int_13_end)\n\
$(label_int_13_03_pop_fail):\n\
pop es                      \n\
pop ds                      \n\
pop di                      \n\
pop si                      \n\
pop dx                      \n\
pop cx                      \n\
pop bx                      \n\
pop ax                      \n\
$(label_int_13_03_fail):    \n\
mov ah, 04                  \n\
stc                         \n\
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
push ds                      \n\
pop es                       \n\
mov di, bx                   \n\
mov cx, ds:[bx+00]          \n\
dec cx          ; ncyl - 1  \n\
xchg ch, cl                 \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
shl cl, 01                  \n\
mov al, ds:[bx+0e] ; nsector\n\
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
mov bl, 2f                  \n\
mov al, cl                  \n\
and al, 3f                  \n\
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
mov al, ds:[bx+0e] ; nsector\n\
mov dh, ds:[bx+02] ; nhead  \n\
mov ah, 00                  \n\
mul dh ; nhead * nsector    \n\
mul cx ; total size         \n\
mov cx, dx ; size high 16   \n\
mov dx, ax ; size low  16   \n\
pop ds                      \n\
pop bx                      \n\
mov ah, 03                  \n\
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
iret                        \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
