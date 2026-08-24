/* Copyright 2012-2026 Neko. */

#ifndef VM_PROFILE_DEFAULT_POST_FIRMWARE_H
#define VM_PROFILE_DEFAULT_POST_FIRMWARE_H

#define VM_PROFILE_DEFAULT_DMA_POST "\
; init vdma      \n\
mov al, 00       \n\
out 08, al ;     \n\
out d0, al ;     \n\
mov al, c0       \n\
out d6, al ;     \n"

#define VM_PROFILE_DEFAULT_PIC_POST "           \
; init pic master           \n\
mov al, 11 ; icw1 0001 0001 \n\
out 20, al                  \n\
mov al, 08 ; icw2 0000 1000 \n\
out 21, al                  \n\
mov al, 04 ; icw3 0000 0100 \n\
out 21, al                  \n\
mov al, 11 ; icw4 0001 0001 \n\
out 21, al                  \n\
\
; init pic slave            \n\
mov al, 11 ; icw1 0001 0001 \n\
out a0, al                  \n\
mov al, 70 ; icw2 0111 0000 \n\
out a1, al                  \n\
mov al, 02 ; icw3 0000 0010 \n\
out a1, al                  \n\
mov al, 01 ; icw4 0000 0001 \n\
out a1, al                  \n"

#define VM_PROFILE_DEFAULT_PIT_POST "                                 \
; init pit                                        \n\
mov al, 36 ; 0011 0110 mode = 3, counter = 0, 16b \n\
out 43, al                                        \n\
mov al, 00                                        \n\
out 40, al ; initial count (0x10000)              \n\
out 40, al                                        \n\
mov al, 54 ; 0101 0100 mode = 2, counter = 1, LSB \n\
out 43, al                                        \n\
mov al, 12                                        \n\
out 41, al ; initial count (0x12)                 \n"

#endif
