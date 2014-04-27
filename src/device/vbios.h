/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VBIOS_H
#define NXVM_VBIOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

typedef struct {
	t_nubitcc numPosts;         /* number of POST routines */
	t_strptr  postTable[0x100]; /* table of POST routine string pointers */
	t_strptr  intTable[0x100];  /* table of INT routine string pointers, null if not defined */
} t_bios;

extern t_bios vbios;

#define VBIOS_AREA_START_SEG 0xf000
#define VBIOS_AREA_START_OFF 0x0000
#define VBIOS_AREA_POST_SEG  0xf000
#define VBIOS_AREA_POST_OFF  0xfff0

void vbiosAddPost(t_strptr stmt);
void vbiosAddInt(t_strptr stmt, t_nubit8 intid);

void vbiosRegister();

#define VBIOS_POST_BOOT "             \
$(label_post_boot_start):           \n\
mov bx, 0040                        \n\
mov ds, bx                          \n\
mov dl, ds:[0100] ; select boot disk\n\
mov dh, 00     ; select head 0      \n\
mov ch, 00     ; select cylender 0  \n\
mov cl, 01     ; select sector 1    \n\
mov bx, 0000                        \n\
mov es, bx     ; target es = 0000   \n\
mov bx, 7c00   ; target bx = 7c00   \n\
mov al, 01     ; read 1 sector      \n\
mov ah, 02     ; command read       \n\
int 13                              \n\
pushf                               \n\
pop ax                              \n\
test al, 01                         \n\
jnz $(label_post_boot_fail)         \n\
mov bx, 0000                        \n\
mov ds, bx                          \n\
mov ax, ds:[7dfe]                   \n\
cmp ax, aa55                        \n\
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
test ax, 0040                     \n\
jnz $(label_post_boot_fail_loop)  \n\
mov ah, 00                        \n\
int 16                            \n\
qdx 00  ; special stop            \n\
jmp near $(label_post_boot_start) \n\
\
$(label_post_boot_succ):  \n\
; start operating system  \n\
xor bx, bx                \n\
mov cx, 0001              \n\
xor dx, dx                \n\
mov sp, fffe              \n\
jmp 0000:7c00             \n"

#define VBIOS_INT_SOFT_MISC_11 "\
qdx 02 ; enter isr       \n\
; device test            \n\
push ds                  \n\
push bx                  \n\
mov bx, 0040             \n\
mov ds, bx               \n\
pop bx                   \n\
mov ax, ds:[0010]        \n\
pop ds                   \n\
qdx 03 ; leave isr       \n\
iret                     \n"

#define VBIOS_INT_SOFT_MISC_12 "\
qdx 02 ; enter isr      \n\
; memory test           \n\
push ds                 \n\
push bx                 \n\
mov bx, 0040            \n\
mov ds, bx              \n\
pop bx                  \n\
mov ax, ds:[0013]       \n\
pop ds                  \n\
qdx 03 ; leave isr      \n\
iret                    \n"

#define VBIOS_INT_SOFT_MISC_15 "    \
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
mov bx, 0003                      \n\
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
and ax, 0001              \n\
mov bx, sp                \n\
and word ss:[bx+08], fffe  \n\
or  word ss:[bx+08], ax    \n\
pop bx                    \n\
pop ax                    \n\
\
$(label_int_15_ret): \n\
qdx 03 ; leave isr   \n\
iret                 \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
