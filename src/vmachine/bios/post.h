/* This file is a part of NXVM project. */

#ifndef NXVM_POST_H
#define NXVM_POST_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#define VBIOS_POST_VPIC "     \
; init vpic1                \n\
mov al, 11 ; icw1 0001 0001 \n\
out 20, al                  \n\
mov al, 08 ; icw2 0000 1000 \n\
out 21, al                  \n\
mov al, 04 ; icw3 0000 0100 \n\
out 21, al                  \n\
mov al, 11 ; icw4 0001 0001 \n\
out 21, al                  \n\
\
; init vpic2                \n\
mov al, 11 ; icw1 0001 0001 \n\
out a0, al                  \n\
mov al, 70 ; icw2 0111 0000 \n\
out a1, al                  \n\
mov al, 02 ; icw3 0000 0010 \n\
out a1, al                  \n\
mov al, 01 ; icw4 0000 0001 \n\
out a1, al                  \n"

#define VBIOS_POST_VCMOS "    \
; init vcmos                \n\
mov al, 0b ; select reg b   \n\
out 70, al                  \n\
mov al, 01 ; 24 hour mode   \n\
out 71, al                  \n"

#define VBIOS_POST_VDMA "     \
; init vdma                 \n\
mov al, 00                  \n\
out 08, al ;                \n\
out d0, al ;                \n\
mov al, c0                  \n\
out d6, al ;                \n"

#define VBIOS_POST_VFDC "     \
; init vfdc                 \n\
mov al, 00                  \n\
mov dx, 03f2                \n\
out dx, al                  \n\
mov al, 0c                  \n\
mov dx, 03f2                \n\
out dx, al                  \n\
mov al, 03                  \n\
mov dx, 03f5                \n\
out dx, al ; cmd specify    \n\
mov al, af                  \n\
mov dx, 03f5                \n\
out dx, al                  \n\
mov al, 02                  \n\
mov dx, 03f5                \n\
out dx, al                  \n"

#define VBIOS_POST_VPIT "                           \
; init vpit                                       \n\
mov al, 36 ; 0011 0110 mode = 3, counter = 0, 16b \n\
out 43, al                                        \n\
mov al, 00                                        \n\
out 40, al ; initial count (0x10000)              \n\
out 40, al                                        \n\
mov al, 54 ; 0101 0100 mode = 2, counter = 1, LSB \n\
out 43, al                                        \n\
mov al, 12                                        \n\
out 41, al ; initial count (0x12)                 \n"

#define VBIOS_POST_BOOT "  \
; start operating system \n\
xor ax, ax               \n\
xor bx, bx               \n\
xor cx, cx               \n\
xor dx, dx               \n\
jmp 0000:7c00            \n"

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
