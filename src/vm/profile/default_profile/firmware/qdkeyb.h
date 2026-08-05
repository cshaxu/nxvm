/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_QDKEYB_H
#define VM_PROFILE_DEFAULT_QDKEYB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/machine/keyboard_interface.h"

#define QDKEYB_VBIOS_ADDR_KEYB_FLAG0         0x0417
#define QDKEYB_VBIOS_ADDR_KEYB_FLAG1         0x0418
#define QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD      0x041a
#define QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL      0x041c
#define QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START  0x041e
#define QDKEYB_VBIOS_ADDR_KEYB_BUFFER_END    0x043d

#define QDKEYB_FLAG0_A_INSERT  0x80
#define QDKEYB_FLAG0_A_CAPLCK  0x40
#define QDKEYB_FLAG0_A_NUMLCK  0x20
#define QDKEYB_FLAG0_A_SCRLCK  0x10
#define QDKEYB_FLAG0_D_ALT     0x08
#define QDKEYB_FLAG0_D_CTRL    0x04
#define QDKEYB_FLAG0_D_LSHIFT  0x02
#define QDKEYB_FLAG0_D_RSHIFT  0x01

#define QDKEYB_FLAG1_D_INSERT  0x80
#define QDKEYB_FLAG1_D_CAPLCK  0x40
#define QDKEYB_FLAG1_D_NUMLCK  0x20
#define QDKEYB_FLAG1_D_SCRLCK  0x10
#define QDKEYB_FLAG1_D_PAUSE   0x08
#define QDKEYB_FLAG1_D_SYSRQ   0x04
#define QDKEYB_FLAG1_D_LALT    0x02
#define QDKEYB_FLAG1_D_LCTRL   0x01

typedef struct vm_profile_default_context vm_profile_default_context;
C_VOID vm_profile_default_keyboard_reset(vm_profile_default_context *profile);
const core_machine_keyboard_provider *vm_profile_default_keyboard_provider(C_VOID);

#define VBIOS_INT_HARD_KEYBOARD_09 "\
push ax                              \n\
push bx                              \n\
push ds                              \n\
push si                              \n\
in al, 60                            \n\
mov bl, al                           \n\
xor bh, bh                           \n\
mov ax, 0040                         \n\
mov ds, ax                           \n\
mov al, ds:[0017]                    \n\
and al, 03                           \n\
jnz $(keyboard_09_shift)             \n\
mov al, cs:[bx+e000]                 \n\
jmp near $(keyboard_09_mapped)       \n\
$(keyboard_09_shift):                \n\
mov al, cs:[bx+e080]                 \n\
$(keyboard_09_mapped):               \n\
or al, al                            \n\
jz $(keyboard_09_eoi)                \n\
mov ah, bl                           \n\
mov bx, ds:[001c]                    \n\
mov si, bx                           \n\
add si, 0002                         \n\
cmp si, 043e                         \n\
jb $(keyboard_09_tail_ready)         \n\
mov si, 041e                         \n\
$(keyboard_09_tail_ready):           \n\
cmp si, ds:[001a]                    \n\
jz $(keyboard_09_eoi)                \n\
push ds                              \n\
push ax                              \n\
xor ax, ax                           \n\
mov ds, ax                           \n\
pop ax                               \n\
mov ds:[bx], ax                      \n\
pop ds                               \n\
mov ds:[001c], si                    \n\
$(keyboard_09_eoi):                  \n\
mov al, 20                           \n\
out 20, al                           \n\
pop si                               \n\
pop ds                               \n\
pop bx                               \n\
pop ax                               \n\
iret                                 \n"

#define VBIOS_INT_SOFT_KEYBOARD_16 "\
cmp ah, 00                            \n\
jz $(keyboard_16_read)               \n\
cmp ah, 10                            \n\
jz $(keyboard_16_read)               \n\
cmp ah, 01                            \n\
jz $(keyboard_16_status)             \n\
cmp ah, 11                            \n\
jz $(keyboard_16_status)             \n\
cmp ah, 02                            \n\
jz $(keyboard_16_flags)              \n\
cmp ah, 05                            \n\
jz $(keyboard_16_store)              \n\
iret                                 \n\
$(keyboard_16_read):                 \n\
push bx                              \n\
push ds                              \n\
mov ax, 0040                         \n\
mov ds, ax                           \n\
$(keyboard_16_wait):                 \n\
mov bx, ds:[001a]                    \n\
cmp bx, ds:[001c]                    \n\
jnz $(keyboard_16_read_ready)        \n\
sti                                  \n\
hlt                                  \n\
cli                                  \n\
jmp near $(keyboard_16_wait)         \n\
$(keyboard_16_read_ready):           \n\
push ds                              \n\
xor ax, ax                           \n\
mov ds, ax                           \n\
mov ax, ds:[bx]                      \n\
pop ds                               \n\
add bx, 0002                         \n\
cmp bx, 043e                         \n\
jb $(keyboard_16_head_ready)         \n\
mov bx, 041e                         \n\
$(keyboard_16_head_ready):           \n\
mov ds:[001a], bx                    \n\
pop ds                               \n\
pop bx                               \n\
iret                                 \n\
$(keyboard_16_status):               \n\
push bx                              \n\
push ds                              \n\
mov ax, 0040                         \n\
mov ds, ax                           \n\
mov bx, ds:[001a]                    \n\
cmp bx, ds:[001c]                    \n\
jnz $(keyboard_16_status_ready)      \n\
mov bx, sp                           \n\
or word ss:[bx+08], 0040             \n\
jmp near $(keyboard_16_status_done)  \n\
$(keyboard_16_status_ready):         \n\
push ds                              \n\
xor ax, ax                           \n\
mov ds, ax                           \n\
mov ax, ds:[bx]                      \n\
pop ds                               \n\
mov bx, sp                           \n\
and word ss:[bx+08], ffbf            \n\
$(keyboard_16_status_done):          \n\
pop ds                               \n\
pop bx                               \n\
iret                                 \n\
$(keyboard_16_flags):                \n\
push bx                              \n\
push ds                              \n\
mov bx, 0040                         \n\
mov ds, bx                           \n\
mov al, ds:[0017]                    \n\
pop ds                               \n\
pop bx                               \n\
iret                                 \n\
$(keyboard_16_store):                \n\
push bx                              \n\
push ds                              \n\
push si                              \n\
mov ax, 0040                         \n\
mov ds, ax                           \n\
mov bx, ds:[001c]                    \n\
mov si, bx                           \n\
add si, 0002                         \n\
cmp si, 043e                         \n\
jb $(keyboard_16_store_tail_ready)   \n\
mov si, 041e                         \n\
$(keyboard_16_store_tail_ready):     \n\
cmp si, ds:[001a]                    \n\
jz $(keyboard_16_store_full)         \n\
mov ax, cx                           \n\
push ds                              \n\
push ax                              \n\
xor ax, ax                           \n\
mov ds, ax                           \n\
pop ax                               \n\
mov ds:[bx], ax                      \n\
pop ds                               \n\
mov ds:[001c], si                    \n\
xor al, al                           \n\
jmp near $(keyboard_16_store_done)   \n\
$(keyboard_16_store_full):           \n\
mov al, 01                           \n\
$(keyboard_16_store_done):           \n\
pop si                               \n\
pop ds                               \n\
pop bx                               \n\
iret                                 \n"


#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
