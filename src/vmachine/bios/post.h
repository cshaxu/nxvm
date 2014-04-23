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

#define VBIOS_POST_BOOT "             \
mov bx, 40                          \n\
mov ds, bx                          \n\
mov dl, [0100] ; select boot disk   \n\
mov dh, 00     ; select head 0      \n\
mov ch, 00     ; select cylender 0  \n\
mov cl, 01     ; select sector 1    \n\
mov bx, 00                          \n\
mov es, bx     ; target es = 0000   \n\
mov bx, 7c00   ; target bx = 7c00   \n\
mov al, 01     ; read 1 sector      \n\
mov ah, 02     ; command read       \n\
int 13                              \n\
pushf                               \n\
pop ax                              \n\
test al, 01                         \n\
jnz $(label_post_boot_fail)         \n\
jmp near $(label_post_boot_succ)    \n\
\
$(label_post_boot_fail):          \n\
mov ah, 02                        \n\
mov dh, 05                        \n\
mov dl, 00                        \n\
int 10  ; set cursor position     \n\
mov ah, 0e                        \n\
mov bl, 0f                        \n\
mov bh, 00                        \n\
mov al, 49                        \n\
int 10  ; display char 'I'        \n\
mov al, 6e                        \n\
int 10  ; display char 'n'        \n\
mov al, 76                        \n\
int 10  ; display char 'v'        \n\
mov al, 61                        \n\
int 10  ; display char 'a'        \n\
mov al, 6c                        \n\
int 10  ; display char 'l'        \n\
mov al, 69                        \n\
int 10  ; display char 'i'        \n\
mov al, 64                        \n\
int 10  ; display char 'd'        \n\
mov al, 20                        \n\
int 10  ; display char ' '        \n\
mov al, 62                        \n\
int 10  ; display char 'b'        \n\
mov al, 6f                        \n\
int 10  ; display char 'o'        \n\
mov al, 6f                        \n\
int 10  ; display char 'o'        \n\
mov al, 74                        \n\
int 10  ; display char 't'        \n\
mov al, 20                        \n\
int 10  ; display char ' '        \n\
mov al, 64                        \n\
int 10  ; display char 'd'        \n\
mov al, 69                        \n\
int 10  ; display char 'i'        \n\
mov al, 73                        \n\
int 10  ; display char 's'        \n\
mov al, 6b                        \n\
int 10  ; display char 'k'        \n\
mov al, 0d                        \n\
int 10  ; display new line        \n\
mov al, 0a                        \n\
int 10  ; display new line        \n\
$(label_post_boot_fail_loop):     \n\
mov ah, 11                        \n\
int 16  ; get key press           \n\
pushf   ; if any key pressed,     \n\
pop ax  ; then stop nxvm          \n\
test ax, 40                       \n\
jnz $(label_post_boot_fail_loop)  \n\
mov ah, 00                        \n\
int 16                            \n\
qdx ff  ; special stop            \n\
jmp near $(label_post_boot_fail)  \n\
\
$(label_post_boot_succ):  \n\
mov ax, aa55              \n\
; start operating system  \n\
xor ax, ax                \n\
xor bx, bx                \n\
mov cx, 01                \n\
xor dx, dx                \n\
mov sp, fffe              \n\
jmp 0000:7c00             \n"

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
