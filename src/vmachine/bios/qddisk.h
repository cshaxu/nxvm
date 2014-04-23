/* This file is a part of NXVM project. */

#ifndef NXVM_QDDISK_H
#define NXVM_QDDISK_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#define HARD_FDD_INT_0E "  \
push ax                  \n\
push dx                  \n\
mov dx, 03f4             \n\
in  al, dx               \n\
and al, c0               \n\
cmp al, 80 ; write ready \n\
jnz $(label_int_0e_ret)  \n\
\
mov dx, 03f5             \n\
mov al, 08               \n\
out dx, al ; sense int   \n\
in  al, dx ; st0 0x20    \n\
in  al, dx ; cyl 0x00    \n\
\
$(label_int_0e_ret):     \n\
mov al, 20               \n\
out 20, al ; eoi cmd     \n\
pop dx                   \n\
pop ax                   \n"

#define SOFT_DISK_INT_13 "    \
test dl, 80                 \n\
jnz $(label_int_13_hdd)     \n\
int 40                      \n\
iret                        \n\
$(label_int_13_hdd):        \n\
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
jmp near $(label_int_13_end)\n\
\
$(label_int_13_00):         \n\
;qdx a0 \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_01):         \n\
;qdx a1 \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_02):         \n\
;qdx a2 \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_03):         \n\
;qdx a3 \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_08):         \n\
;qdx a8 \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_15):         \n\
;qdx b5 \n\
jmp near $(label_int_13_end)\n\
\
$(label_int_13_end):        \n\
; set fdd status byte       \n\
push bx                     \n\
push ds                     \n\
mov bx, 40                  \n\
mov ds, bx                  \n\
mov [0041], ah              \n\
pop ds                      \n\
pop bx                      \n\
; set/clear cf              \n\
push ax                     \n\
push bx                     \n\
pushf                       \n\
pop ax                      \n\
and ax, 01                  \n\
mov bx, sp                  \n\
ss:                         \n\
and word [bx+8], fffe       \n\
ss:                         \n\
or  word [bx+8], ax         \n\
pop bx                      \n\
pop ax                      \n\
iret                        \n"

#define SOFT_DISK_INT_40 "    \
test dl, 80                 \n\
jz $(label_int_40_fdd)      \n\
iret ; invalid drive id     \n\
$(label_int_40_fdd):        \n\
cmp ah, 00                  \n\
jnz $(label_int_40_cmp_01)  \n\
jmp near $(label_int_40_00) \n\
$(label_int_40_cmp_01):     \n\
cmp ah, 01                  \n\
jnz $(label_int_40_cmp_02)  \n\
jmp near $(label_int_40_01) \n\
$(label_int_40_cmp_02):     \n\
cmp ah, 02                  \n\
jnz $(label_int_40_cmp_03)  \n\
jmp near $(label_int_40_02) \n\
$(label_int_40_cmp_03):     \n\
cmp ah, 03                  \n\
jnz $(label_int_40_cmp_08)  \n\
jmp near $(label_int_40_03) \n\
$(label_int_40_cmp_08):     \n\
cmp ah, 08                  \n\
jnz $(label_int_40_cmp_15)  \n\
jmp near $(label_int_40_08) \n\
$(label_int_40_cmp_15):     \n\
cmp ah, 15                  \n\
jnz $(label_int_40_cmp_def) \n\
jmp near $(label_int_40_15) \n\
$(label_int_40_cmp_def):    \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_00):         \n\
; reset drive               \n\
cmp dl, 00                  \n\
jnz $(label_int_40_00_x)    \n\
mov ah, 00                  \n\
clc                         \n\
jmp near $(label_int_40_end)\n\
$(label_int_40_00_x):       \n\
mov ah, 0c                  \n\
stc                         \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_01):         \n\
; get fdd status byte       \n\
push bx                     \n\
push ds                     \n\
mov bx, 40                  \n\
mov ds, bx                  \n\
mov ah, [0041]              \n\
pop ds                      \n\
pop bx                      \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_02):         \n\
qdx a2 \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_03):         \n\
qdx a3 \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_08):         \n\
qdx a8 \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_15):         \n\
qdx b5 \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_end):        \n\
; set fdd status byte       \n\
push bx                     \n\
push ds                     \n\
mov bx, 40                  \n\
mov ds, bx                  \n\
mov [0041], ah              \n\
pop ds                      \n\
pop bx                      \n\
; set/clear cf              \n\
push ax                     \n\
push bx                     \n\
pushf                       \n\
pop ax                      \n\
and ax, 01                  \n\
mov bx, sp                  \n\
ss:                         \n\
and word [bx+8], fffe       \n\
ss:                         \n\
or  word [bx+8], ax         \n\
pop bx                      \n\
pop ax                      \n\
iret                        \n"

// TODO: IMPLEMENT INT 40 FDD SOFT SERVICE 08/15/02/03

void qddiskReset();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
