/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_BIOS_H
#define VM_PROFILE_DEFAULT_BIOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

typedef struct t_ram t_ram;
typedef struct core_machine_media_registry core_machine_media_registry;

#define VM_PROFILE_DEFAULT_DEVICE_BIOS "Unknown BIOS"

typedef struct vm_profile_default_bios_code {
    uint8_t *bytes;
    uint16_t length;
} vm_profile_default_bios_code;

typedef struct {
    type_native_unsigned postCount; /* number of POST routines */
    vm_profile_default_bios_code postTable[0x100];
    vm_profile_default_bios_code intTable[0x100];
    vm_profile_default_bios_code bootCode;
} t_bios_connect;

typedef struct {
    type_unsigned_16 buildCS;
    type_unsigned_16 buildIP;
} t_bios_data;

/* bios connections */
typedef struct t_bios {
    t_bios_data data;
    t_bios_connect connect;
    type_bool flagBoot;
} t_bios;

#define VBIOS_ADDR_START_SEG 0xf000
#define VBIOS_ADDR_START_OFF TYPE_ZERO_16
#define VBIOS_ADDR_POST_SEG  VBIOS_ADDR_START_SEG
#define VBIOS_ADDR_POST_OFF  0xfff0

#define VBIOS_ADDR_ROM_INFO  0xe6f5 /* bios rom info offset */
#define VBIOS_ADDR_HDD_PARAM 0xe431 /* hard disk parameter table offset */
#define VBIOS_ADDR_KEYB_SCAN_ASCII_NORMAL 0xe000
#define VBIOS_ADDR_KEYB_SCAN_ASCII_SHIFT  0xe080

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
#define VBIOS_ADDR_BIOS_BREAK_FLAG    0x0471 /* set by C_INT 9 */
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

/* Default-profile POST report protocol stored in the BDA work area. The ROM
 * reports a terminal boot condition; the VM session owns the stop action. */
#define VBIOS_POST_REPORT_NONE                      0x00u
#define VBIOS_POST_REPORT_BOOT_FAILURE_ACKNOWLEDGED 0x01u

C_VOID vm_profile_default_bios_add_post_code(t_bios *bios, uint8_t *bytes,
    uint16_t length);
C_VOID vm_profile_default_bios_add_interrupt_code(t_bios *bios, uint8_t *bytes,
    uint16_t length, uint8_t intid);
C_VOID vm_profile_default_bios_set_boot_code(t_bios *bios, uint8_t *bytes,
    uint16_t length);
C_VOID vm_profile_default_bios_initialize(t_bios *bios);
C_VOID vm_profile_default_bios_reset(t_bios *bios, t_ram *ram,
    const core_machine_media_registry *media_registry,
    core_machine_media_id hdd_media_id);
C_VOID vm_profile_default_bios_refresh(t_bios *bios);
C_VOID vm_profile_default_bios_finalize(t_bios *bios);
C_VOID vm_profile_default_bios_print(const t_bios *bios);
C_VOID vm_profile_default_bios_set_boot_hdd(t_bios *bios, C_INT enabled);
C_INT vm_profile_default_bios_get_boot_hdd(const t_bios *bios);
C_INT vm_profile_default_bios_take_boot_failure_report(t_ram *ram);

#define VBIOS_POST_BOOT "             \
$(label_post_boot_start):           \n\
mov bx, 0040                        \n\
mov ds, bx                          \n\
mov dl, ds:[0072] ; select boot disk\n\
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
 mov bx, 0040                      \n\
 mov ds, bx                        \n\
 mov byte ds:[0505], 01            \n\
 jmp near $(label_post_boot_start) \n\
\
$(label_post_boot_succ):  \n\
; start operating system  \n\
xor bx, bx                \n\
mov cx, 0001              \n\
xor dx, dx                \n\
mov sp, fffe              \n\
 jmp 0000:7c00             \n"

/* Default-ROM INT 10h subset. It owns guest BDA/text memory and emits video
 * controller port transactions; VADP observes those real guest writes. */
#define VBIOS_INT_SOFT_VIDEO_10 "      \
cmp ah, 00                        \n\
jnz $(label_int_10_cmp_02)        \n\
jmp near $(label_int_10_set_mode) \n\
$(label_int_10_cmp_02):           \n\
cmp ah, 02                        \n\
jnz $(label_int_10_cmp_06)        \n\
jmp near $(label_int_10_cursor)   \n\
$(label_int_10_cmp_06):           \n\
cmp ah, 06                        \n\
jnz $(label_int_10_cmp_0b)        \n\
jmp near $(label_int_10_clear)    \n\
$(label_int_10_cmp_0b):           \n\
cmp ah, 0b                        \n\
jnz $(label_int_10_cmp_0e)        \n\
iret                              \n\
$(label_int_10_cmp_0e):           \n\
cmp ah, 0e                        \n\
jnz $(label_int_10_cmp_0f)        \n\
jmp near $(label_int_10_tty)      \n\
$(label_int_10_cmp_0f):           \n\
cmp ah, 0f                        \n\
jnz $(label_int_10_ret)           \n\
jmp near $(label_int_10_mode)     \n\
$(label_int_10_ret):              \n\
iret                              \n\
\
$(label_int_10_set_mode):         \n\
cmp al, 06                        \n\
jz $(label_int_10_set_cga_06_jump) \n\
cmp al, 0d                        \n\
jz $(label_int_10_set_ega_0d_jump) \n\
cmp al, 10                        \n\
jz $(label_int_10_set_ega_10_jump) \n\
cmp al, 03                        \n\
jnz $(label_int_10_set_mode_ret)  \n\
jmp near $(label_int_10_set_text_03) \n\
$(label_int_10_set_cga_06_jump):  \n\
jmp near $(label_int_10_set_cga_06) \n\
$(label_int_10_set_ega_0d_jump):  \n\
jmp near $(label_int_10_set_ega_0d) \n\
$(label_int_10_set_ega_10_jump):  \n\
jmp near $(label_int_10_set_ega_10) \n\
$(label_int_10_set_mode_ret):     \n\
iret                              \n\
$(label_int_10_set_ega_0d):       \n\
push ax                           \n\
push bx                           \n\
push dx                           \n\
push ds                           \n\
mov bx, 0040                      \n\
mov ds, bx                        \n\
mov byte ds:[0049], 0d            \n\
pop ds                            \n\
mov dx, 03c4                      \n\
mov al, 02                        \n\
out dx, al                        \n\
inc dx                            \n\
mov al, 0f                        \n\
out dx, al                        \n\
mov dx, 03ce                      \n\
mov al, 05                        \n\
out dx, al                        \n\
inc dx                            \n\
xor al, al                        \n\
out dx, al                        \n\
dec dx                            \n\
mov al, 06                        \n\
out dx, al                        \n\
inc dx                            \n\
mov al, 05                        \n\
out dx, al                        \n\
mov dx, 03d4                      \n\
mov al, 13                        \n\
out dx, al                        \n\
inc dx                            \n\
xor al, al                        \n\
out dx, al                        \n\
mov dx, 03da                      \n\
in al, dx                         \n\
mov dx, 03c0                      \n\
mov al, 30                        \n\
out dx, al                        \n\
mov al, 01                        \n\
out dx, al                        \n\
pop dx                            \n\
pop bx                            \n\
pop ax                            \n\
iret                              \n\
$(label_int_10_set_ega_10):       \n\
push ax                           \n\
push bx                           \n\
push dx                           \n\
push ds                           \n\
mov bx, 0040                      \n\
mov ds, bx                        \n\
mov byte ds:[0049], 10            \n\
pop ds                            \n\
mov dx, 03c4                      \n\
mov al, 02                        \n\
out dx, al                        \n\
inc dx                            \n\
mov al, 0f                        \n\
out dx, al                        \n\
mov dx, 03ce                      \n\
mov al, 05                        \n\
out dx, al                        \n\
inc dx                            \n\
xor al, al                        \n\
out dx, al                        \n\
dec dx                            \n\
mov al, 06                        \n\
out dx, al                        \n\
inc dx                            \n\
mov al, 05                        \n\
out dx, al                        \n\
mov dx, 03d4                      \n\
mov al, 0c                        \n\
out dx, al                        \n\
inc dx                            \n\
xor al, al                        \n\
out dx, al                        \n\
dec dx                            \n\
mov al, 0d                        \n\
out dx, al                        \n\
inc dx                            \n\
xor al, al                        \n\
out dx, al                        \n\
dec dx                            \n\
mov al, 13                        \n\
out dx, al                        \n\
inc dx                            \n\
mov al, 28                        \n\
out dx, al                        \n\
mov dx, 03da                      \n\
in al, dx                         \n\
mov dx, 03c0                      \n\
mov al, 30                        \n\
out dx, al                        \n\
mov al, 01                        \n\
out dx, al                        \n\
pop dx                            \n\
pop bx                            \n\
pop ax                            \n\
iret                              \n\
$(label_int_10_set_cga_06):       \n\
push ax                           \n\
push bx                           \n\
push dx                           \n\
push ds                           \n\
mov bx, 0040                      \n\
mov ds, bx                        \n\
mov byte ds:[0049], 06            \n\
pop ds                            \n\
mov dx, 03d8                      \n\
mov al, 1a                        \n\
out dx, al                        \n\
inc dx                            \n\
mov al, 0f                        \n\
out dx, al                        \n\
pop dx                            \n\
pop bx                            \n\
pop ax                            \n\
iret                              \n\
$(label_int_10_set_text_03):      \n\
push ax                           \n\
push bx                           \n\
push dx                           \n\
push ds                           \n\
mov bx, 0040                      \n\
mov ds, bx                        \n\
mov byte ds:[0049], 03            \n\
pop ds                            \n\
mov dx, 03d8                      \n\
mov al, 05                        \n\
out dx, al                        \n\
mov dx, 03ce                      \n\
mov al, 06                        \n\
out dx, al                        \n\
inc dx                            \n\
mov al, 09                        \n\
out dx, al                        \n\
mov dx, 03d4                      \n\
mov al, 13                        \n\
out dx, al                        \n\
inc dx                            \n\
xor al, al                        \n\
out dx, al                        \n\
pop dx                            \n\
pop bx                            \n\
pop ax                            \n\
iret                              \n\
\
$(label_int_10_cursor):           \n\
push ax                           \n\
push bx                           \n\
push cx                           \n\
push dx                           \n\
push ds                           \n\
mov bx, 0040                      \n\
mov ds, bx                        \n\
mov ax, dx                         \n\
mov ds:[0050], ax                 \n\
mov al, dh                         \n\
xor ah, ah                         \n\
mov cl, 50                         \n\
mul cl                             \n\
xor dh, dh                         \n\
add ax, dx                         \n\
mov cx, ax                         \n\
mov dx, 03d4                       \n\
mov al, 0e                         \n\
out dx, al                         \n\
inc dx                             \n\
mov al, ch                         \n\
out dx, al                         \n\
dec dx                             \n\
mov al, 0f                         \n\
out dx, al                         \n\
inc dx                             \n\
mov al, cl                         \n\
out dx, al                         \n\
pop ds                             \n\
pop dx                             \n\
pop cx                             \n\
pop bx                             \n\
pop ax                             \n\
iret                               \n\
\
$(label_int_10_clear):            \n\
push ax                           \n\
push cx                           \n\
push di                           \n\
push es                           \n\
mov ax, b800                      \n\
mov es, ax                        \n\
xor di, di                        \n\
xor ax, ax                        \n\
mov cx, 07d0                      \n\
rep:                              \n\
stosw                              \n\
pop es                            \n\
pop di                            \n\
pop cx                            \n\
pop ax                            \n\
iret                              \n\
\
$(label_int_10_tty):              \n\
push ax                           \n\
push bx                           \n\
push cx                           \n\
push dx                           \n\
push si                           \n\
push di                           \n\
push bp                           \n\
push ds                           \n\
push es                           \n\
mov si, ax                        \n\
mov bp, bx                        \n\
mov bx, 0040                      \n\
mov ds, bx                        \n\
mov dh, ds:[0051]                 \n\
mov dl, ds:[0050]                 \n\
mov ax, si                        \n\
cmp al, 0d                        \n\
jnz $(label_int_10_tty_cmp_lf)    \n\
mov dl, 00                        \n\
jmp near $(label_int_10_cursor_store) \n\
$(label_int_10_tty_cmp_lf):       \n\
cmp al, 0a                        \n\
jnz $(label_int_10_tty_cmp_bs)    \n\
inc dh                            \n\
jmp near $(label_int_10_tty_row)  \n\
$(label_int_10_tty_cmp_bs):       \n\
cmp al, 08                        \n\
jnz $(label_int_10_tty_put)       \n\
or dl, dl                          \n\
jnz $(label_int_10_tty_back_col)  \n\
or dh, dh                          \n\
jz $(label_int_10_cursor_store)   \n\
dec dh                            \n\
mov dl, 4f                        \n\
jmp near $(label_int_10_cursor_store) \n\
$(label_int_10_tty_back_col):     \n\
dec dl                            \n\
jmp near $(label_int_10_cursor_store) \n\
$(label_int_10_tty_put):          \n\
mov al, dh                         \n\
xor ah, ah                         \n\
mov cl, 50                         \n\
mul cl                             \n\
mov cx, dx                         \n\
xor ch, ch                         \n\
add ax, cx                         \n\
shl ax, 01                         \n\
mov di, ax                         \n\
mov ax, b800                       \n\
mov es, ax                         \n\
mov ax, si                         \n\
mov es:[di], al                    \n\
mov bx, bp                         \n\
mov es:[di+01], bl                 \n\
inc dl                             \n\
cmp dl, 50                         \n\
jnb $(label_int_10_tty_wrap)       \n\
jmp near $(label_int_10_cursor_store) \n\
$(label_int_10_tty_wrap):          \n\
mov dl, 00                         \n\
inc dh                             \n\
$(label_int_10_tty_row):           \n\
cmp dh, 19                         \n\
jnb $(label_int_10_tty_scroll)     \n\
jmp near $(label_int_10_cursor_store) \n\
$(label_int_10_tty_scroll):        \n\
mov bx, b800                       \n\
mov ds, bx                         \n\
mov es, bx                         \n\
mov si, 00a0                       \n\
xor di, di                         \n\
mov cx, 0780                       \n\
rep:                               \n\
movsw                              \n\
mov ax, 0720                       \n\
mov cx, 0050                       \n\
rep:                               \n\
stosw                              \n\
mov bx, 0040                       \n\
mov ds, bx                         \n\
mov dh, 18                         \n\
mov dl, 00                         \n\
$(label_int_10_cursor_store):      \n\
mov ax, dx                          \n\
mov ds:[0050], ax                  \n\
$(label_int_10_cursor_crtc):       \n\
mov al, dh                          \n\
xor ah, ah                          \n\
mov cl, 50                          \n\
mul cl                              \n\
xor dh, dh                          \n\
add ax, dx                          \n\
mov cx, ax                          \n\
mov dx, 03d4                        \n\
mov al, 0e                          \n\
out dx, al                          \n\
inc dx                              \n\
mov al, ch                          \n\
out dx, al                          \n\
dec dx                              \n\
mov al, 0f                          \n\
out dx, al                          \n\
inc dx                              \n\
mov al, cl                          \n\
out dx, al                          \n\
pop es                              \n\
pop ds                              \n\
pop bp                              \n\
pop di                              \n\
pop si                              \n\
pop dx                              \n\
pop cx                              \n\
pop bx                              \n\
pop ax                              \n\
iret                                \n\
\
$(label_int_10_mode):               \n\
push ds                             \n\
push bx                             \n\
mov bx, 0040                        \n\
mov ds, bx                          \n\
mov al, ds:[0049]                   \n\
mov ah, ds:[004a]                   \n\
mov bh, ds:[0062]                   \n\
pop bx                              \n\
pop ds                              \n\
iret                                \n"

#define VBIOS_INT_SOFT_MISC_11 "\
; device test            \n\
push ds                  \n\
push bx                  \n\
mov bx, 0040             \n\
mov ds, bx               \n\
pop bx                   \n\
mov ax, ds:[0010]        \n\
pop ds                   \n\
iret                     \n"

#define VBIOS_INT_SOFT_MISC_12 "\
; memory test           \n\
push ds                 \n\
push bx                 \n\
mov bx, 0040            \n\
mov ds, bx              \n\
pop bx                  \n\
mov ax, ds:[0013]       \n\
pop ds                  \n\
iret                    \n"

#define VBIOS_INT_SOFT_MISC_15 "    \
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
iret                 \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
