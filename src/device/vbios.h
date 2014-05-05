/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VBIOS_H
#define NXVM_VBIOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_BIOS "Unknown BIOS"

typedef struct {
    t_nubitcc postCount;         /* number of POST routines */
    t_strptr  postTable[0x100]; /* table of POST routine string pointers */
    t_strptr  intTable[0x100];  /* table of INT routine string pointers, null if not defined */
} t_bios;

extern t_bios vbios;

#define VBIOS_ADDR_START_SEG 0xf000
#define VBIOS_ADDR_START_OFF Zero16
#define VBIOS_ADDR_POST_SEG  VBIOS_ADDR_START_SEG
#define VBIOS_ADDR_POST_OFF  0xfff0

#define VBIOS_ADDR_ROM_INFO  0xe6f5 /* bios rom info offset */
#define VBIOS_ADDR_HDD_PARAM 0xe431 /* hard disk parameter table offset */

#define VBIOS_ADDR_HDD_PARAM_OFFSET   0x0104
#define VBIOS_ADDR_HDD_PARAM_SEGMENT  0x0106
#define VBIOS_ADDR_SERI_PORT_COM1     0x0400
#define VBIOS_ADDR_SERI_PORT_COM2     0x0402
#define VBIOS_ADDR_SERI_PORT_COM3     0x0404
#define VBIOS_ADDR_SERI_PORT_COM4     0x0406
#define VBIOS_ADDR_PARA_PORT_LPT1     0x0408
#define VBIOS_ADDR_PARA_PORT_LPT2     0x040a
#define VBIOS_ADDR_PARA_PORT_LPT3     0x040c
#define VBIOS_ADDR_PARA_PORT_LPT4     0x040e
#define VBIOS_ADDR_EQUIP_FLAG         0x0410
#define VBIOS_ADDR_MANUFACT_TEST      0x0412
#define VBIOS_ADDR_RAM_SIZE           0x0413
#define VBIOS_ADDR_ERROR_CODES        0x0415
#define VBIOS_ADDR_KEYB_FLAG0         0x0417
#define VBIOS_ADDR_KEYB_FLAG1         0x0418
#define VBIOS_ADDR_KEYB_NUMPAD        0x0419
#define VBIOS_ADDR_KEYB_BUF_HEAD      0x041a
#define VBIOS_ADDR_KEYB_BUF_TAIL      0x041c
/*#define VBIOS_ADDR_KEYB_BUFFER_START  0x041e
#define VBIOS_ADDR_KEYB_BUFFER_END    0x043d*/
#define VBIOS_ADDR_FDD_CALI_FLAG      0x043e
#define VBIOS_ADDR_FDD_MOTOR_FLAG     0x043f
#define VBIOS_ADDR_FDD_MOTOR_TIMEOUT  0x0440
#define VBIOS_ADDR_FDD_STATUS         0x0441
#define VBIOS_ADDR_FDC_STATUS0        0x0442
#define VBIOS_ADDR_FDC_STATUS1        0x0443
#define VBIOS_ADDR_FDC_STATUS2        0x0444
#define VBIOS_ADDR_FDC_CYLINDER       0x0445
#define VBIOS_ADDR_FDC_HEAD           0x0446
#define VBIOS_ADDR_FDC_SECTOR         0x0447
#define VBIOS_ADDR_FDC_BYTE_COUNT     0x0448
#define VBIOS_ADDR_VGA_VIDEO_MODE     0x0449
#define VBIOS_ADDR_VGA_COLUMN         0x044a
#define VBIOS_ADDR_VGA_PAGE_SIZE      0x044c
#define VBIOS_ADDR_VGA_PAGE_OFFSET    0x044e
#define VBIOS_ADDR_VGA_CURSOR_P0      0x0450
#define VBIOS_ADDR_VGA_CURSOR_P1      0x0452
#define VBIOS_ADDR_VGA_CURSOR_P2      0x0454
#define VBIOS_ADDR_VGA_CURSOR_P3      0x0456
#define VBIOS_ADDR_VGA_CURSOR_P4      0x0458
#define VBIOS_ADDR_VGA_CURSOR_P5      0x045a
#define VBIOS_ADDR_VGA_CURSOR_P6      0x045c
#define VBIOS_ADDR_VGA_CURSOR_P7      0x045e
#define VBIOS_ADDR_VGA_CURSOR_BOTTOM  0x0460
#define VBIOS_ADDR_VGA_CURSOR_TOP     0x0461
#define VBIOS_ADDR_VGA_ACT_PAGE_NUM   0x0462
#define VBIOS_ADDR_VGA_ACT_ADPT_PORT  0x0463
#define VBIOS_ADDR_VGA_MODE_REGISTER  0x0465
#define VBIOS_ADDR_VGA_COLOR_PALETTE  0x0466
#define VBIOS_ADDR_CPU_POINTER        0x0467
#define VBIOS_ADDR_RTC_DAILY_COUNTER  0x046c
#define VBIOS_ADDR_RTC_ROLLOVER       0x0470
#define VBIOS_ADDR_BIOS_BREAK_FLAG    0x0471 /* set by int 9 */
#define VBIOS_ADDR_SOFT_RESET_FLAG    0x0472 /* C-A-D or JUMP FFFF:0000 */
#define VBIOS_ADDR_HDD_LST_OP_STATUS  0x0474
#define VBIOS_ADDR_HDD_NUMBER         0x0475
#define VBIOS_ADDR_HDD_CONTROL        0x0476
#define VBIOS_ADDR_HDC_PORT_OFFSET    0x0477
#define VBIOS_ADDR_PARA_TIMEOUT_LPT1  0x0478
#define VBIOS_ADDR_PARA_TIMEOUT_LPT2  0x0479
#define VBIOS_ADDR_PARA_TIMEOUT_LPT3  0x047a
#define VBIOS_ADDR_PARA_TIMEOUT_LPT4  0x047b
#define VBIOS_ADDR_SERI_TIMEOUT_COM1  0x047c
#define VBIOS_ADDR_SERI_TIMEOUT_COM2  0x047d
#define VBIOS_ADDR_SERI_TIMEOUT_COM3  0x047e
#define VBIOS_ADDR_SERI_TIMEOUT_COM4  0x047f
#define VBIOS_ADDR_KEYB_BUFFER_START  0x0480 /* stores keyboard buffer start address */
#define VBIOS_ADDR_KEYB_BUFFER_END    0x0482 /* stores keyboard buffer end address */
#define VBIOS_ADDR_VGA_ROW_NUMBER     0x0484
#define VBIOS_ADDR_VGA_CHAR_HEIGHT    0x0485
#define VBIOS_ADDR_VGA_MODE_OPTIONS1  0x0487
#define VBIOS_ADDR_VGA_MODE_OPTIONS2  0x0488
#define VBIOS_ADDR_VGA_DISPLAY_DATA   0x0489
#define VBIOS_ADDR_VGA_DCC_INDEX      0x048a
#define VBIOS_ADDR_FDC_LAST_DATA_RATE 0x048b
#define VBIOS_ADDR_HDD_STATUS         0x048c
#define VBIOS_ADDR_HDD_ERROR          0x048d
#define VBIOS_ADDR_HDD_INT_FLAG       0x048e
#define VBIOS_ADDR_DRV_SAME_FLAG      0x048f
#define VBIOS_ADDR_DRV_MEDIA_STATE_D0 0x0490
#define VBIOS_ADDR_DRV_MEDIA_STATE_D1 0x0491
#define VBIOS_ADDR_DRV_MEDIA_STATE_D2 0x0492
#define VBIOS_ADDR_DRV_MEDIA_STATE_D3 0x0493
#define VBIOS_ADDR_DRV_CYLINDER_D0    0x0494
#define VBIOS_ADDR_DRV_CYLINDER_D1    0x0495
#define VBIOS_ADDR_KEYB_MODE_TYPE     0x0496
#define VBIOS_ADDR_KEYB_LED_FLAG      0x0497
#define VBIOS_ADDR_RTC_WAIT_POINTER   0x0498
#define VBIOS_ADDR_RTC_WAIT_TIMEOUT   0x049c
#define VBIOS_ADDR_RTC_WAIT_FLAG      0x04a0
#define VBIOS_ADDR_LANA_CHANNEL_FLAG  0x04a1
#define VBIOS_ADDR_LANA_STATUS_C0     0x04a2
#define VBIOS_ADDR_LANA_STATUS_C1     0x04a3
#define VBIOS_ADDR_LANA_HDD_INT_VEC   0x04a4
#define VBIOS_ADDR_VGA_VIDEO_TAB_PTR  0x04a8
#define VBIOS_ADDR_RESERVED_04AC      0x04ac
#define VBIOS_ADDR_KEYB_NMI_FLAG      0x04b4
#define VBIOS_ADDR_KEYB_BREAK_FLAG    0x04b5
#define VBIOS_ADDR_KEYB_P60_SG_BT_QUE 0x04b9
#define VBIOS_ADDR_KEYB_LAST_SCANCODE 0x04ba
#define VBIOS_ADDR_KEYB_NMI_BUF_HEAD  0x04bb
#define VBIOS_ADDR_KEYB_NMI_BUF_TAIL  0x04bc
#define VBIOS_ADDR_KEYB_NMI_SCCD_BUF  0x04bd
#define VBIOS_ADDR_RTC_DAY_COUNTER    0x04ce
#define VBIOS_ADDR_INTRAPP_COMM_AREA  0x04f0
#define VBIOS_ADDR_POST_WORK_AREA     0x0505

void vbiosAddPost(t_strptr stmt);
void vbiosAddInt(t_strptr stmt, t_nubit8 intid);

void vbiosRegister();

#define VBIOS_POST_BOOT "             \
$(label_post_boot_start):           \n\
mov bx, 0050                        \n\
mov ds, bx                          \n\
mov dl, ds:[0005] ; select boot disk\n\
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
