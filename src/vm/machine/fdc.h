/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_FDC_H
#define VM_MACHINE_FDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/dma.h"
#include "core/machine/pic.h"
#include "vm/machine/fdd.h"

typedef struct t_pic t_pic;
typedef struct t_port t_port;

#define VM_MACHINE_DEVICE_FDC "Intel 8272A"

typedef struct {
    type_unsigned_8 dor; /* digital output register */
    type_unsigned_8 msr; /* main status register */
    type_unsigned_8 dr;  /* data register */
    type_unsigned_8 dir; /* digital input register */
    type_unsigned_8 ccr; /* configuration control register */

    type_unsigned_4 hut; /* head unload STD_TIME */
    type_unsigned_4 hlt; /* head load STD_TIME */
    type_unsigned_8 srt; /* step rate STD_TIME */
    type_bool flagNDMA; /* 0 = dma mode; 1 = non-dma mode */
    type_bool flagINTR; /* 0 = no intr; 1 = has intr */

    type_unsigned_8 rwCount; /* count of io port command/result rw times */
    type_unsigned_8 cmd[9];
    type_unsigned_8 ret[7];
    type_unsigned_8 st0, st1, st2, st3; /* state registers */
} t_fdc_data;

typedef struct {
    uint16_t dor_port;
    uint16_t status_port;
    uint16_t data_port;
    uint16_t direction_port;
    uint16_t control_port;
    uint8_t irq;
    uint8_t dma_channel;
} vm_machine_fdc_config;

typedef struct {
    t_fdd *fdd;
    core_machine_dma_request_binding dma_request;
    core_machine_pic_irq_source irq_source;
    t_port *port;
    vm_machine_fdc_config config;
} t_fdc_connect;

typedef struct {
    t_fdc_data data;
    t_fdc_connect connect;
} t_fdc;

/*
 * MSR: RQM | DIO | NDM | CB  | D3B | D2B | D1B | D0B
 * DIR: DC  | -   | -   | -   | -   | -   | -   | HD
 * DOR: ME3 | ME2 | ME1 | ME0 | -   | RST | DS1 | DS0
 * CCR: -   | -   | -   | -   | -   | -   | DRC | -
 * DR:  ?
 * ST0: ?
 * ST1: ?
 * ST2: ?
 * ST3: ?
 */

/* main status register bits */
#define VFDC_MSR_DB(id) (1 << (id)) /* fdd #id is in seek mode */
#define VFDC_MSR_CB  0x10 /* a read or write command is in process */
#define VFDC_MSR_NDM 0x20 /* non-dma mode in process */
#define VFDC_MSR_DIO 0x40 /* data read-by(1) or write-to(0) processor */
#define VFDC_MSR_RQM 0x80 /* request for master */
#define VFDC_MSR_ReadyRead    (VFDC_MSR_RQM | VFDC_MSR_DIO)
#define VFDC_MSR_ReadyWrite   (VFDC_MSR_RQM)
#define VFDC_MSR_ProcessRead  (VFDC_MSR_ReadyRead  | VFDC_MSR_CB)
#define VFDC_MSR_ProcessWrite (VFDC_MSR_ReadyWrite | VFDC_MSR_CB)

/* digital input register bits */
#define VFDC_DIR_HD 0x01 /* high density select */
#define VFDC_DIR_DC 0x80 /* diskette change */

/* digital output register bits */
#define VFDC_DOR_ME(id) (1 << ((id) + 4)) /* motor engine enable */
#define VFDC_DOR_DS   0x03 /* drive select */
#define VFDC_DOR_NRS  0x04 /* fdc enable(1) or hold(0) fdc at reset */
#define VFDC_DOR_ENRQ 0x08 /* dma and i/o interface enabled */

/* configuration control register bits */
#define VFDC_CCR_DRC 0x02 /* 0=500000 bps, 1=250000 bps */

/* status register 0 bits */
#define VFDC_ST0_DS       0x03 /* drive select */
#define VFDC_ST0_SEEK_END 0x20

/* status register 2 bits */
#define VFDC_ST2_SCAN_MATCH    0x04
#define VFDC_ST2_SCAN_MISMATCH 0x08

/* status register 3 bit */
#define VFDC_ST3_DS 0x03 /* drive select */

/* fdc command specify bytes */
#define VFDC_CMD_Specify1_HUT 0x0f /* head unload STD_TIME */
#define VFDC_CMD_Specify1_SRT 0xf0 /* step rate STD_TIME */
#define VFDC_CMD_Specify2_HLT 0xfe /* head load STD_TIME */
#define VFDC_CMD_Specify2_ND  0x01 /* non-dma */
#define VFDC_GetCMD_Specify1_HUT(cb) ((cb) & VFDC_CMD_Specify1_HUT)
#define VFDC_GetCMD_Specify1_SRT(cb) (((cb) & VFDC_CMD_Specify1_SRT) >> 4)
#define VFDC_GetCMD_Specify2_HLT(cb) (((cb) & VFDC_CMD_Specify2_HLT) >> 1)

/* fdc command sense-drive-status bytes */
#define VFDC_CMD_SenseDriveStatus1_HD 0x04 /* head select 1 or 0 */
#define VFDC_CMD_SenseDriveStatus1_US 0x03 /* us? */

/* fdc command seek bytes */
#define VFDC_CMD_Seek1_HD 0x04 /* head select 1 or 0 */
#define VFDC_CMD_Seek1_US 0x03 /* us? */

/* fdc command read-id bytes */
#define VFDC_CMD_ReadId0_MF 0x40 /* mf? */
#define VFDC_CMD_ReadId1_HD 0x04 /* head select 1 or 0 */
#define VFDC_CMD_ReadId1_US 0x03 /* us? */

/* fdc command format-track bytes */
#define VFDC_CMD_FormatTrack0_MF 0x40 /* mf? */
#define VFDC_CMD_FormatTrack1_HD 0x04 /* head select 1 or 0 */
#define VFDC_CMD_FormatTrack1_US 0x03 /* us? */

/* convert byte per sector from or to bps type  */
#define VFDC_GetBPS(cb) (0x0080 << (cb))  /* convert bps type to bps */
type_unsigned_8 VFDC_GetBPSC(type_unsigned_16 cb); /* convert bps to bps type */

/* #define VFDC_Get_A0(cbyte)  * Address Line 0 */
/* #define VFDC_Get_C(cbyte)   * Cylinder Number (0-79) */
/* #define VFDC_Get_D(cbyte)   * Data */
/* #define VFDC_Get_DB(cbyte)  * Data Bus (D7-D0) */
/* #define VFDC_Get_DTL(cbyte) * Data Length */
/* #define VFDC_Get_EOT(cbyte) * End Of Track */
/* #define VFDC_Get_GPL(cbyte) * Gap Length */
/* #define VFDC_Get_H(cbyte)   * Head Address (0-1) */
/* #define VFDC_Get_MFM(cbyte) * fm or mfm mode */
/* #define VFDC_Get_MT(cbyte)  * multi-track */
/* #define VFDC_Get_N(cbyte)   * number */
/* #define VFDC_Get_NCN(cbyte) * new cylinder number */
/* #define VFDC_Get_R(cbyte)   * record */
/* #define VFDC_Get_RW(cbyte)  * read/write signal */
/* #define VFDC_Get_SC(cbyte)  * sector */
/* #define VFDC_Get_SK(cbyte)  * skip */
/* #define VFDC_Get_STP(cbyte) * step */
/* #define VFDC_GetENRQ(cbyte) ((cbyte) & 0x08) * enable dma and intr */
/* #define VFDC_GetDS(cbyte)   ((cbyte) & 0x03) * drive select (ds0,ds1) */
/* #define VFDC_GetHUT(cbyte)  ((cbyte) & 0x0f) * head unload STD_TIME */
/* #define VFDC_GetSRT(cbyte)  ((cbyte) >> 4)   * step rate STD_TIME */
/* #define VFDC_GetHLT(cbyte)  ((cbyte) >> 1)   * head load STD_TIME */
/* #define VFDC_GetNDMA(cbyte) ((cbyte) & 0x01) * non-dma mode */
/* #define VFDC_GetHDS(cbyte)  (!!((cbyte) & 0x04)) * head select (0 or 1) */
/* #define VFDC_GetBPS(cbyte)  (0x0080 << (cbyte))  * bytes per sector */
/* sector size code */

C_VOID vm_machine_fdc_connect(t_fdc *fdc, t_fdd *fdd,
    const core_machine_dma_request_binding *dma_request, t_pic *pic_master,
    t_pic *pic_slave, t_port *port, const vm_machine_fdc_config *config);
const core_machine_dma_channel_provider *vm_machine_fdc_dma_provider(C_VOID);
C_VOID vm_machine_fdc_initialize(t_fdc *fdc);
C_VOID vm_machine_fdc_reset(t_fdc *fdc);
C_VOID vm_machine_fdc_refresh(t_fdc *fdc);
C_VOID vm_machine_fdc_finalize(t_fdc *fdc);
C_VOID vm_machine_fdc_print(const t_fdc *fdc);

#define VFDC_POST "           \
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

#define VFDC_INT_HARD_FDD_0E "  \
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
iret                     \n"

#define VFDC_INT_SOFT_FDD_40 "\
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
iret                        \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
