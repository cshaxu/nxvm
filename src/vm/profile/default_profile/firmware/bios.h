/* Copyright 2012-2014 Neko. */

#ifndef VM_PROFILE_DEFAULT_BIOS_H
#define VM_PROFILE_DEFAULT_BIOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

typedef struct core_machine_media_registry core_machine_media_registry;
typedef struct core_machine_firmware_context core_machine_firmware_context;

#define VM_PROFILE_DEFAULT_DEVICE_BIOS "Unknown BIOS"

typedef struct vm_profile_default_bios_code {
    type_unsigned_8 *bytes;
    type_unsigned_16 length;
    type_unsigned_16 offset;
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
    type_unsigned_16 base_memory_kib;
} t_bios_data;

/* bios connections */
typedef struct t_bios {
    t_bios_data data;
    t_bios_connect connect;
    type_unsigned_8 reset_ivt[0x400];
    type_bool flagBoot;
    type_bool rom_materialized;
} t_bios;

#define VBIOS_ADDR_START_SEG 0xf000
#define VBIOS_ADDR_START_OFF TYPE_ZERO_16
#define VBIOS_ADDR_POST_SEG  VBIOS_ADDR_START_SEG
#define VBIOS_ADDR_POST_OFF  0xfff0

#define VBIOS_ADDR_ROM_INFO  0xe6f5 /* bios rom info offset */
#define VBIOS_ADDR_HDD_PARAM 0xe431 /* hard disk parameter table offset */
#define VBIOS_ADDR_KEYB_SCAN_ASCII_NORMAL 0xe000
#define VBIOS_ADDR_KEYB_SCAN_ASCII_SHIFT  0xe080
#define VBIOS_ADDR_FDC_SERVICE             0x0c00
#define VBIOS_ADDR_VIDEO_SERVICE           0x4000
#define VBIOS_ADDR_FDC_SERVICE_LIMIT       VBIOS_ADDR_VIDEO_SERVICE
#define VBIOS_ADDR_VIDEO_SERVICE_LIMIT     VBIOS_ADDR_KEYB_SCAN_ASCII_NORMAL

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
#define VBIOS_ADDR_FDD_SECTORS_PER_TRACK 0x04acu
#define VBIOS_ADDR_FDD_MAX_CYLINDER   0x04adu
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

C_VOID vm_profile_default_bios_add_post_code(t_bios *bios, type_unsigned_8 *bytes,
    type_unsigned_16 length);
C_VOID vm_profile_default_bios_add_interrupt_code(t_bios *bios, type_unsigned_8 *bytes,
    type_unsigned_16 length, type_unsigned_8 intid);
C_VOID vm_profile_default_bios_add_interrupt_code_at(t_bios *bios,
    type_unsigned_8 *bytes, type_unsigned_16 length, type_unsigned_16 offset,
    type_unsigned_8 intid);
C_VOID vm_profile_default_bios_set_boot_code(t_bios *bios, type_unsigned_8 *bytes,
    type_unsigned_16 length);
C_VOID vm_profile_default_bios_initialize(t_bios *bios);
C_INT vm_profile_default_bios_materialize(t_bios *bios,
    core_machine_firmware_context *firmware);
C_VOID vm_profile_default_bios_reset(t_bios *bios,
    core_machine_firmware_context *firmware,
    const core_machine_media_registry *media_registry,
    core_machine_media_id fdd_media_id,
    core_machine_media_id hdd_media_id);
C_INT vm_profile_default_bios_handle_int15_block_move(t_bios *bios,
    core_machine_firmware_context *firmware, type_unsigned_16 target_segment,
    type_unsigned_16 target_offset,
    const core_machine_firmware_interrupt_frame *input,
    core_machine_firmware_interrupt_result *output);
C_VOID vm_profile_default_bios_refresh(t_bios *bios);
C_VOID vm_profile_default_bios_finalize(t_bios *bios);
C_VOID vm_profile_default_bios_print(const t_bios *bios);
C_VOID vm_profile_default_bios_set_boot_hdd(t_bios *bios, C_INT enabled);
C_INT vm_profile_default_bios_get_boot_hdd(const t_bios *bios);
C_INT vm_profile_default_bios_take_boot_failure_report(
    core_machine_firmware_context *firmware);

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
mov dx, 03c2              \n\
mov al, 01                \n\
out dx, al                \n\
xor dx, dx                \n\
mov sp, fffe              \n\
 jmp 0000:7c00             \n"

/* Default-ROM INT 10h subset. It owns guest BDA/text memory and emits video
 * controller port transactions; VADP observes those real guest writes. */
#define VBIOS_INT_SOFT_VIDEO_10(CHUNK) \
    CHUNK("      cmp ah, 00                        \njnz $(label_int_10_cmp_02)        \njmp near $(label_int_10_set_mode) \n$(label_int_10_cmp_02):           \ncmp ah, 02                        \njnz $(label_int_10_cmp_05)        \njmp near $(label_int_10_cursor)   \n$(label_int_10_cmp_05):           \ncmp ah, 05                        \njnz $(label_int_10_cmp_06)        \njmp near $(label_int_10_page)     \n$(label_int_10_cmp_06):           \ncmp ah, 06                        \njnz $(label_int_10_cmp_08)        \njmp near $(label_int_10_clear)    \n$(label_int_10_cmp_08):           \ncmp ah, 08                        \njnz $(label_int_10_cmp_09)        \njmp near $(label_int_10_read_char) \n$(label_int_10_cmp_09):           \ncmp ah, 09                        \njnz $(label_int_10_cmp_0b)        \njmp near $(label_int_10_write_char) \n$(label_int_10_cmp_0b):           \ncmp ah, 0b                        \njnz $(label_int_10_cmp_0e)        \niret                              \n$(label_int_10_cmp_0e):           \ncmp ah, 0e                        \njnz $(label_int_10_cmp_0f)        \njmp near $(label_int_10_tty)      \n$(label_int_10_cmp_0f):           \ncmp ah, 0f                        \njnz $(label_int_10_ret)           \njmp near $(label_int_10_mode)     \n$(label_int_10_ret):              \niret                              \n$(label_int_10_set_mode):         \ncmp al, 06                        \njz $(label_int_10_set_cga_06_jump) \ncmp al, 0d                        \njz $(label_int_10_set_ega_0d_jump) \ncmp al, 0e                        \njz $(label_int_10_set_ega_0e_jump) \ncmp al, 10                        \njnz $(label_int_10_cmp_03)        \njmp near $(label_int_10_set_ega_10_jump) \n$(label_int_10_cmp_03):           \ncmp al, 03                        \njnz $(label_int_10_set_mode_ret)  \njmp near $(label_int_10_set_text_03) \n$(label_int_10_set_cga_06_jump):  \njmp near $(label_int_10_set_cga_06) \n$(label_int_10_set_ega_0d_jump):  \njmp near $(label_int_10_set_ega_0d) \n$(label_int_10_set_ega_0e_jump):  \njmp near $(label_int_10_set_ega_0e) \n$(label_int_10_set_ega_10_jump):  \njmp near $(label_int_10_set_ega_10) \n$(label_int_10_set_mode_ret):     \niret                              \n$(label_int_10_set_ega_0d):       \npush ax                           \npush bx                           \npush dx                           \npush ds                           \nmov bx, 0040                      \nmov ds, bx                        \nmov byte ds:[0049], 0d            \npop ds                            \nmov dx, 03c4                      \nmov al, 02                        \nout dx, al                        \ninc dx                            \nmov al, 0f                        \nout dx, al                        \nmov dx, 03ce                      \nmov al, 05                        \nout dx, al                        \ninc dx                            \nxor al, al                        \nout dx, al                        \ndec dx                            \nmov al, 06                        \nout dx, al                        \ninc dx                            \nmov al, 05                        \nout dx, al                        \n") \
    CHUNK("mov dx, 03d4                      \nmov al, 01                        \nout dx, al                        \ninc dx                            \nmov al, 27                        \nout dx, al                        \ndec dx                            \nmov al, 07                        \nout dx, al                        \ninc dx                            \nxor al, al                        \nout dx, al                        \ndec dx                            \nmov al, 12                        \nout dx, al                        \ninc dx                            \nmov ax, 00c7                      \nout dx, al                        \ndec dx                            \nmov al, 13                        \nout dx, al                        \ninc dx                            \nmov al, 14                        \nout dx, al                        \nmov dx, 03da                      \nin al, dx                         \nmov dx, 03c0                      \nmov al, 30                        \nout dx, al                        \nmov al, 01                        \nout dx, al                        \npop dx                            \n") \
    CHUNK("pop bx                            \npop ax                            \niret                              \n$(label_int_10_set_ega_0e):       \npush ax                           \npush bx                           \npush dx                           \npush ds                           \nmov bx, 0040                      \nmov ds, bx                        \nmov byte ds:[0049], 0e            \npop ds                            \nmov dx, 03c4                      \nmov al, 02                        \nout dx, al                        \ninc dx                            \nmov al, 0f                        \nout dx, al                        \nmov dx, 03ce                      \nmov al, 05                        \nout dx, al                        \ninc dx                            \nxor al, al                        \nout dx, al                        \ndec dx                            \nmov al, 06                        \nout dx, al                        \ninc dx                            \nmov al, 05                        \nout dx, al                        \n") \
    CHUNK("mov dx, 03d4                      \nmov al, 01                        \nout dx, al                        \ninc dx                            \nmov al, 4f                        \nout dx, al                        \ndec dx                            \nmov al, 07                        \nout dx, al                        \ninc dx                            \nxor al, al                        \nout dx, al                        \ndec dx                            \nmov al, 12                        \nout dx, al                        \ninc dx                            \nmov ax, 00c7                      \nout dx, al                        \ndec dx                            \nmov al, 13                        \nout dx, al                        \ninc dx                            \nmov al, 28                        \nout dx, al                        \nmov dx, 03da                      \nin al, dx                         \nmov dx, 03c0                      \nmov al, 30                        \nout dx, al                        \nmov al, 01                        \nout dx, al                        \npop dx                            \n") \
    CHUNK("pop bx                            \npop ax                            \niret                              \n$(label_int_10_set_ega_10):       \npush ax                           \npush bx                           \npush dx                           \npush ds                           \nmov bx, 0040                      \nmov ds, bx                        \nmov byte ds:[0049], 10            \npop ds                            \nmov dx, 03c4                      \nmov al, 02                        \nout dx, al                        \ninc dx                            \nmov al, 0f                        \nout dx, al                        \nmov dx, 03ce                      \nmov al, 05                        \nout dx, al                        \ninc dx                            \nxor al, al                        \nout dx, al                        \ndec dx                            \nmov al, 06                        \nout dx, al                        \ninc dx                            \nmov al, 05                        \nout dx, al                        \n") \
    CHUNK("mov dx, 03d4                      \nmov al, 01                        \nout dx, al                        \ninc dx                            \nmov al, 4f                        \nout dx, al                        \ndec dx                            \nmov al, 07                        \nout dx, al                        \ninc dx                            \nmov al, 02                        \nout dx, al                        \ndec dx                            \nmov al, 12                        \nout dx, al                        \ninc dx                            \nmov al, 5d                        \nout dx, al                        \ndec dx                            \nmov al, 0c                        \nout dx, al                        \ninc dx                            \nxor al, al                        \nout dx, al                        \ndec dx                            \nmov al, 0d                        \nout dx, al                        \ninc dx                            \nxor al, al                        \nout dx, al                        \ndec dx                            \nmov al, 13                        \nout dx, al                        \ninc dx                            \nmov al, 28                        \nout dx, al                        \nmov dx, 03da                      \nin al, dx                         \nmov dx, 03c0                      \nmov al, 30                        \nout dx, al                        \nmov al, 01                        \nout dx, al                        \npop dx                            \npop bx                            \npop ax                            \niret                              \n") \
    CHUNK("\n$(label_int_10_set_cga_06):       \npush ax                           \npush bx                           \npush dx                           \npush ds                           \nmov bx, 0040                      \nmov ds, bx                        \nmov byte ds:[0049], 06            \npop ds                            \nmov dx, 03d8                      \nmov al, 1a                        \nout dx, al                        \ninc dx                            \nmov al, 0f                        \nout dx, al                        \npop dx                            \npop bx                            \npop ax                            \niret                              \n$(label_int_10_set_text_03):      \npush ax                           \npush bx                           \npush dx                           \npush ds                           \nmov bx, 0040                      \nmov ds, bx                        \nmov byte ds:[0049], 03            \npop ds                            \nmov dx, 03c2                      \nmov al, 01                        \nout dx, al                        \nmov dx, 03d8                      \nmov al, 0d                        \nout dx, al                        \nmov dx, 03ce                      \nmov al, 06                        \nout dx, al                        \ninc dx                            \nmov al, 09                        \nout dx, al                        \nmov dx, 03d4                      \n") \
    CHUNK("mov al, 13                        \nout dx, al                        \ninc dx                            \nxor al, al                        \nout dx, al                        \npop dx                            \npop bx                            \npop ax                            \niret                              \n$(label_int_10_cursor):           \npush ax                           \npush bx                           \npush cx                           \npush dx                           \npush ds                           \nmov bx, 0040                      \nmov ds, bx                        \nmov ax, dx                         \nmov ds:[0050], ax                 \nmov al, dh                         \nxor ah, ah                         \nmov cl, 50                         \nmul cl                             \nxor dh, dh                         \nadd ax, dx                         \nmov cx, ax                         \nmov dx, 03d4                       \nmov al, 0e                         \nout dx, al                         \ninc dx                             \nmov al, ch                         \nout dx, al                         \ndec dx                             \nmov al, 0f                         \nout dx, al                         \ninc dx                             \nmov al, cl                         \nout dx, al                         \npop ds                             \npop dx                             \npop cx                             \npop bx                             \npop ax                             \niret                               \n$(label_int_10_page):             \npush ax                            \npush bx                            \npush cx                            \npush dx                            \npush ds                            \ncmp bh, 00                         \njnz $(label_int_10_page_done)      \nmov bx, 0040                       \nmov ds, bx                         \nmov byte ds:[0062], 00             \nmov word ds:[004e], 0000           \nmov dx, ds:[0050]                  \nmov al, dh                          \nxor ah, ah                          \nmov cl, 50                          \nmul cl                              \nxor dh, dh                          \nadd ax, dx                          \nmov cx, ax                          \nmov dx, 03d4                        \nmov al, 0e                          \nout dx, al                          \ninc dx                              \nmov al, ch                          \nout dx, al                          \ndec dx                              \nmov al, 0f                          \nout dx, al                          \ninc dx                              \nmov al, cl                          \nout dx, al                          \n$(label_int_10_page_done):         \npop ds                              \npop dx                              \npop cx                              \npop bx                              \npop ax                              \niret                                \n$(label_int_10_read_char):         \npush bx                             \npush cx                             \npush dx                             \npush si                             \npush di                             \npush ds                             \npush es                             \nmov si, bx                          \nmov bx, 0040                        \nmov ds, bx                          \n") \
    CHUNK("mov ax, si                          \ncmp ah, ds:[0062]                   \njnz $(label_int_10_read_char_done)  \nmov dx, ds:[0050]                   \nmov al, dh                           \nxor ah, ah                           \nmov cl, 50                           \nmul cl                               \nxor dh, dh                           \nadd ax, dx                           \nshl ax, 01                           \nadd ax, ds:[004e]                   \nmov di, ax                           \nmov ax, b800                         \nmov es, ax                           \nmov ax, es:[di]                      \n$(label_int_10_read_char_done):      \npop es                               \npop ds                               \npop di                               \npop si                               \npop dx                               \npop cx                               \npop bx                               \niret                                 \n$(label_int_10_write_char):         \npush ax                              \npush bx                              \npush cx                              \npush dx                              \npush si                              \npush di                              \npush bp                              \npush ds                              \npush es                              \nmov ah, bl                            \nmov bp, ax                            \nmov si, bx                            \nmov bx, 0040                          \nmov ds, bx                            \nmov ax, si                            \ncmp ah, ds:[0062]                     \njnz $(label_int_10_write_char_done)   \npush cx                               \nmov dx, ds:[0050]                     \nmov al, dh                             \nxor ah, ah                             \nmov cl, 50                             \nmul cl                                 \nxor dh, dh                             \nadd ax, dx                             \nshl ax, 01                             \nadd ax, ds:[004e]                     \nmov di, ax                             \npop cx                                \nmov ax, b800                           \nmov es, ax                             \nmov ax, bp                             \nrep:                                   \nstosw                                  \n$(label_int_10_write_char_done):      \npop es                                 \npop ds                                 \npop bp                                 \npop di                                 \npop si                                 \npop dx                                 \npop cx                                 \npop bx                                 \npop ax                                 \niret                                   \n$(label_int_10_clear):            \npush ax                           \npush bx                           \npush cx                           \npush dx                           \npush si                           \npush di                           \npush es                           \nor al, al                         \njnz $(label_int_10_clear_legacy)  \ncmp dh, ch                        \njb $(label_int_10_clear_done)     \ncmp dl, cl                        \njb $(label_int_10_clear_done)     \ncmp dh, 18                        \nja $(label_int_10_clear_done)     \ncmp dl, 4f                        \nja $(label_int_10_clear_done)     \n") \
    CHUNK("push dx                           \npush cx                           \nmov dl, cl                        \nmov al, ch                        \nxor ah, ah                        \nmov cl, 50                        \nmul cl                            \nxor dh, dh                        \nadd ax, dx                        \nshl ax, 01                        \nmov di, ax                        \npop cx                            \npop dx                            \nmov bl, dl                        \nsub bl, cl                        \ninc bl                            \nmov ah, bh                        \nxor al, al                        \nmov bh, dh                        \nsub bh, ch                        \ninc bh                            \nmov dx, b800                      \nmov es, dx                        \ncld                               \n$(label_int_10_clear_row):        \nxor ch, ch                        \nmov cl, bl                        \nrep:                              \nstosw                              \ndec bh                            \njz $(label_int_10_clear_done)    \nmov cx, 0050                      \nsub cl, bl                        \nshl cx, 01                        \nadd di, cx                        \njmp near $(label_int_10_clear_row) \n$(label_int_10_clear_legacy):     \nmov ax, b800                      \nmov es, ax                        \nxor di, di                        \nxor ax, ax                        \nmov cx, 07d0                      \ncld                               \nrep:                              \nstosw                              \n$(label_int_10_clear_done):       \npop es                            \npop di                            \npop si                            \npop dx                            \npop cx                            \npop bx                            \npop ax                            \niret                              \n$(label_int_10_tty):              \npush ax                           \npush bx                           \npush cx                           \npush dx                           \npush si                           \npush di                           \npush bp                           \npush ds                           \npush es                           \nmov si, ax                        \nmov bp, bx                        \nmov bx, 0040                      \nmov ds, bx                        \nmov dh, ds:[0051]                 \nmov dl, ds:[0050]                 \nmov ax, si                        \ncmp al, 0d                        \njnz $(label_int_10_tty_cmp_lf)    \nmov dl, 00                        \njmp near $(label_int_10_cursor_store) \n$(label_int_10_tty_cmp_lf):       \ncmp al, 0a                        \njnz $(label_int_10_tty_cmp_bs)    \ninc dh                            \njmp near $(label_int_10_tty_row)  \n$(label_int_10_tty_cmp_bs):       \ncmp al, 08                        \njnz $(label_int_10_tty_put)       \nor dl, dl                          \njnz $(label_int_10_tty_back_col)  \nor dh, dh                          \njz $(label_int_10_cursor_store)   \ndec dh                            \nmov dl, 4f                        \njmp near $(label_int_10_cursor_store) \n$(label_int_10_tty_back_col):     \ndec dl                            \njmp near $(label_int_10_cursor_store) \n$(label_int_10_tty_put):          \nmov al, dh                         \nxor ah, ah                         \n") \
    CHUNK("mov cl, 50                         \nmul cl                             \nmov cx, dx                         \nxor ch, ch                         \nadd ax, cx                         \nshl ax, 01                         \nmov di, ax                         \nmov ax, b800                       \nmov es, ax                         \nmov ax, si                         \nmov es:[di], al                    \nmov bx, bp                         \nmov es:[di+01], bl                 \ninc dl                             \ncmp dl, 50                         \njnb $(label_int_10_tty_wrap)       \njmp near $(label_int_10_cursor_store) \n$(label_int_10_tty_wrap):          \nmov dl, 00                         \ninc dh                             \n$(label_int_10_tty_row):           \ncmp dh, 19                         \njnb $(label_int_10_tty_scroll)     \njmp near $(label_int_10_cursor_store) \n$(label_int_10_tty_scroll):        \nmov bx, b800                       \nmov ds, bx                         \nmov es, bx                         \nmov si, 00a0                       \nxor di, di                         \nmov cx, 0780                       \nrep:                               \nmovsw                              \nmov ax, 0720                       \nmov cx, 0050                       \nrep:                               \nstosw                              \nmov bx, 0040                       \nmov ds, bx                         \nmov dh, 18                         \nmov dl, 00                         \n$(label_int_10_cursor_store):      \nmov ax, dx                          \nmov ds:[0050], ax                  \n$(label_int_10_cursor_crtc):       \nmov al, dh                          \nxor ah, ah                          \nmov cl, 50                          \nmul cl                              \nxor dh, dh                          \nadd ax, dx                          \nmov cx, ax                          \nmov dx, 03d4                        \nmov al, 0e                          \nout dx, al                          \ninc dx                              \nmov al, ch                          \nout dx, al                          \ndec dx                              \nmov al, 0f                          \nout dx, al                          \ninc dx                              \nmov al, cl                          \nout dx, al                          \npop es                              \npop ds                              \npop bp                              \npop di                              \npop si                              \npop dx                              \npop cx                              \npop bx                              \npop ax                              \niret                                \n$(label_int_10_mode):               \npush ds                             \npush bx                             \nmov bx, 0040                        \nmov ds, bx                          \nmov al, ds:[0049]                   \nmov ah, ds:[004a]                   \nmov bh, ds:[0062]                   \npop bx                              \npop ds                              \niret                                \n")


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

#define VBIOS_INT_SOFT_MISC_15_PREFIX "    \
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
mov ah, 86                         \n\
stc                               \n\
jmp near $(label_int_15_set_flag) \n\
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
mov ax, "

#define VBIOS_INT_SOFT_MISC_15_SUFFIX "\n\
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
