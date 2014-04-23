/* This file is a part of NXVM project. */

#ifndef NXVM_QDDISK_H
#define NXVM_QDDISK_H

#ifdef __cplusplus
extern "C" {
#endif

#define HARD_FDD_INT_0E "  \
qdx 02 ; enter isr \n\
cli                      \n\
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
pop ax                   \n\
sti                      \n\
qdx 03 ; leave isr \n\
iret                     \n"

#define SOFT_DISK_INT_13 "    \
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

#define SOFT_DISK_INT_40 "    \
qdx 02 ; enter isr          \n\
test dl, 80                 \n\
jz $(label_int_40_fdd)      \n\
mov ah, 01                  \n\
stc                         \n\
jmp near $(label_int_40_end)\n\
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
clc                         \n\
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
mov bx, 0040                \n\
mov ds, bx                  \n\
mov ah, ds:[0041]           \n\
pop ds                      \n\
pop bx                      \n\
clc                         \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_02):         \n\
; read sector               \n\
cmp dl, 00                  \n\
ja $(label_int_40_02_bad)   \n\
cmp dh, 01                  \n\
ja $(label_int_40_02_bad)   \n\
cmp cl, 12                  \n\
ja $(label_int_40_02_bad)   \n\
cmp ch, 4f                  \n\
ja $(label_int_40_02_bad)   \n\
jmp near $(label_int_40_02_work) \n\
$(label_int_40_02_bad):     \n\
mov ah, 04                  \n\
stc                         \n\
jmp near $(label_int_40_end)\n\
$(label_int_40_02_work):    \n\
; set dma                        \n\
push bx                          \n\
push cx                          \n\
push dx                          \n\
push ax                          \n\
mov al, 86 ; block, inc, w, chn2 \n\
out 0b, al                       \n\
mov cx, bx                       \n\
mov dx, es                       \n\
shr cx, 01                       \n\
shr cx, 01                       \n\
shr cx, 01                       \n\
shr cx, 01                       \n\
add dx, cx                       \n\
mov ax, dx                       \n\
shl ax, 01                       \n\
shl ax, 01                       \n\
shl ax, 01                       \n\
shl ax, 01                       \n\
mov cx, bx                       \n\
and cx, 000f                     \n\
or  ax, cx ; calc base addr      \n\
mov cl, 0c                       \n\
shr dx, cl ; calc page register  \n\
out 04, al ; addr low byte       \n\
mov al, ah                       \n\
out 04, al ; addr high byte      \n\
mov al, dl                       \n\
out 81, al ; page register       \n\
pop ax                           \n\
pop dx                           \n\
mov ah, 00 ; clear ah for wc     \n\
mov cl, 09                       \n\
shl ax, cl ; al * 512            \n\
dec ax                           \n\
pop cx                           \n\
pop bx                           \n\
out 05, al ; wc low byte         \n\
mov al, ah                       \n\
out 05, al ; wc high byte        \n\
mov al, 02                       \n\
out 0a, al ; unmask dma1.ch2     \n\
mov al, 00                       \n\
out d4, al ; unmask dma2.ch0     \n\
; set fdc                        \n\
push bx                          \n\
mov bx, dx                       \n\
mov dx, 03f5                     \n\
mov al, e6                       \n\
out dx, al ; read command        \n\
shl bh, 01                       \n\
shl bh, 01                       \n\
or  bl, bh                       \n\
shr bh, 01                       \n\
shr bh, 01 ; calc hds byte       \n\
mov al, bl                       \n\
out dx, al ; set stdi hds        \n\
mov al, ch                       \n\
out dx, al ; set stdi cyl        \n\
mov al, bh                       \n\
out dx, al ; set stdi head       \n\
mov al, cl                       \n\
out dx, al ; set stdi sec start  \n\
mov al, 02                       \n\
out dx, al ; set stdi sec size   \n\
mov al, 12                       \n\
out dx, al ; set stdi sec end    \n\
mov al, 1b                       \n\
out dx, al ; set stdi gap len    \n\
mov al, ff                       \n\
out dx, al ; set stdi cus secsz  \n\
pop bx                           \n\
; vdma drq generated, wait       \n\
sti                              \n\
mov dx, 03f4                     \n\
$(label_int_40_02_l2):           \n\
in  al, dx                       \n\
and al, c0                       \n\
cmp al, c0                       \n\
jnz $(label_int_40_02_l2)        \n\
mov dx, 03f5                     \n\
in  al, dx ; get stdo st0        \n\
in  al, dx ; get stdo st1        \n\
in  al, dx ; get stdo st2        \n\
in  al, dx ; get stdo cyl        \n\
in  al, dx ; get stdo head       \n\
in  al, dx ; get stdo sec        \n\
in  al, dx ; get stdo sec size   \n\
mov ah, 00                       \n\
clc                              \n\
jmp near $(label_int_40_end)     \n\
\
$(label_int_40_03):         \n\
; write sector              \n\
cmp dl, 00                  \n\
ja $(label_int_40_03_bad)   \n\
cmp dh, 01                  \n\
ja $(label_int_40_03_bad)   \n\
cmp cl, 12                  \n\
ja $(label_int_40_03_bad)   \n\
cmp ch, 4f                  \n\
ja $(label_int_40_03_bad)   \n\
jmp near $(label_int_40_03_work) \n\
$(label_int_40_03_bad):     \n\
mov ah, 04                  \n\
stc                         \n\
jmp near $(label_int_40_end)\n\
$(label_int_40_03_work):         \n\
; set dma                        \n\
push bx                          \n\
push cx                          \n\
push dx                          \n\
push ax                          \n\
mov al, 8a ; block, inc, r, chn2 \n\
out 0b, al                       \n\
mov cx, bx                       \n\
mov dx, es                       \n\
shr cx, 01                       \n\
shr cx, 01                       \n\
shr cx, 01                       \n\
shr cx, 01                       \n\
add dx, cx                       \n\
mov ax, dx                       \n\
shl ax, 01                       \n\
shl ax, 01                       \n\
shl ax, 01                       \n\
shl ax, 01                       \n\
mov cx, bx                       \n\
and cx, 000f                     \n\
or  ax, cx ; calc base addr      \n\
mov cl, 0c                       \n\
shr dx, cl ; calc page register  \n\
out 04, al ; addr low byte       \n\
mov al, ah                       \n\
out 04, al ; addr high byte      \n\
mov al, dl                       \n\
out 81, al ; page register       \n\
pop ax                           \n\
pop dx                           \n\
mov ah, 00 ; clear ah for wc     \n\
mov cl, 09                       \n\
shl ax, cl ; al * 512            \n\
dec ax                           \n\
pop cx                           \n\
pop bx                           \n\
out 05, al ; wc low byte         \n\
mov al, ah                       \n\
out 05, al ; wc high byte        \n\
mov al, 02                       \n\
out 0a, al ; unmask dma1.ch2     \n\
mov al, 00                       \n\
out d4, al ; unmask dma2.ch0     \n\
; set fdc                        \n\
push bx                          \n\
mov bx, dx                       \n\
mov dx, 03f5                     \n\
mov al, c5                       \n\
out dx, al ; write command       \n\
shl bh, 01                       \n\
shl bh, 01                       \n\
or  bl, bh                       \n\
shr bh, 01                       \n\
shr bh, 01 ; calc hds byte       \n\
mov al, bl                       \n\
out dx, al ; set stdi hds        \n\
mov al, ch                       \n\
out dx, al ; set stdi cyl        \n\
mov al, bh                       \n\
out dx, al ; set stdi head       \n\
mov al, cl                       \n\
out dx, al ; set stdi sec start  \n\
mov al, 02                       \n\
out dx, al ; set stdi sec size   \n\
mov al, 12                       \n\
out dx, al ; set stdi sec end    \n\
mov al, 1b                       \n\
out dx, al ; set stdi gap len    \n\
mov al, ff                       \n\
out dx, al ; set stdi cus secsz  \n\
pop bx                           \n\
; vdma drq generated, wait       \n\
sti                              \n\
mov dx, 03f4                     \n\
$(label_int_40_03_l2):           \n\
in  al, dx                       \n\
and al, c0                       \n\
cmp al, c0                       \n\
jnz $(label_int_40_03_l2)        \n\
mov dx, 03f5                     \n\
in  al, dx ; get stdo st0        \n\
in  al, dx ; get stdo st1        \n\
in  al, dx ; get stdo st2        \n\
in  al, dx ; get stdo cyl        \n\
in  al, dx ; get stdo head       \n\
in  al, dx ; get stdo sec        \n\
in  al, dx ; get stdo sec size   \n\
mov ah, 00                       \n\
clc                              \n\
jmp near $(label_int_40_end)     \n\
\
$(label_int_40_08):         \n\
; get fdd parameters        \n\
mov bx, 0004                \n\
mov cx, 4f12                \n\
mov dx, 0102                \n\
push ds                     \n\
mov ax, 0000                \n\
mov ds, ax                  \n\
mov di, ds:[0078]           \n\
mov ax, ds:[007a]           \n\
mov es, ax                  \n\
pop ds                      \n\
mov ax, 0000                \n\
clc                         \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_15):         \n\
; get drive type            \n\
mov ah, dl                  \n\
and ah, 02                  \n\
shr ah, 01                  \n\
not ah                      \n\
and ah, 01                  \n\
clc                         \n\
jmp near $(label_int_40_end)\n\
\
$(label_int_40_end):        \n\
; set fdd status byte       \n\
push bx                     \n\
push ds                     \n\
mov bx, 0040                \n\
mov ds, bx                  \n\
mov ds:[0041], ah           \n\
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
qdx 03 ; leave isr \n\
iret                        \n"

void qddiskReset();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
