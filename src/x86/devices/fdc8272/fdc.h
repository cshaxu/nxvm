/* Copyright 2012-2026 Neko. */
#ifndef X86_FDC_H
#define X86_FDC_H
#include "x86/devices/fdc8272/fdc8272_interface.h"

typedef enum x86_fdc_phase {
    x86_fdc_PHASE_COMMAND = 0,
    x86_fdc_PHASE_PENDING_COMMAND,
    x86_fdc_PHASE_EXECUTION_READ,
    x86_fdc_PHASE_EXECUTION_WRITE,
    x86_fdc_PHASE_EXECUTION_WRITE_TAIL,
    x86_fdc_PHASE_EXECUTION_SCAN,
    x86_fdc_PHASE_EXECUTION_FORMAT,
    x86_fdc_PHASE_PENDING_COMPLETE,
    x86_fdc_PHASE_RESULT
} x86_fdc_phase;

typedef struct {

    lib_u8 hut; /* head unload duration */
    lib_u8 hlt; /* head load duration */
    lib_u8 srt; /* step rate duration */
    lib_u8 flagNDMA; /* 0 = dma mode; 1 = non-dma mode */
    lib_u8 flagINTR; /* 0 = no intr; 1 = has intr */

    x86_fdc_phase phase;
    lib_u8 command_length;
    lib_u8 command_index;
    lib_u8 result_length;
    lib_u8 result_index;
    lib_u8 cmd[9];
    lib_u8 ret[7];
    lib_u8 st0, st1, st2, st3; /* state registers */
    lib_u8 pending_st0;
    lib_u8 pending_st1;
    lib_u8 pending_st2;
    lib_u8 transfer_expect_deleted;
    lib_u8 transfer_write_deleted;
    lib_u8 scan_mode;
    lib_u8 scan_sector_satisfies;
    lib_u16 cylinder;
    lib_u8 pcn[X86_FDC_DRIVE_COUNT];
    lib_u16 seek_target[X86_FDC_DRIVE_COUNT];
    lib_u64 seek_due_tick[X86_FDC_DRIVE_COUNT];
    lib_u8 seek_pending[X86_FDC_DRIVE_COUNT];
    lib_bool seek_recalibrate[X86_FDC_DRIVE_COUNT];
    lib_u8 seek_head[X86_FDC_DRIVE_COUNT];
    lib_u8 seek_steps[X86_FDC_DRIVE_COUNT];
    lib_u8 seek_result_st0[X86_FDC_DRIVE_COUNT];
    lib_u8 seek_result_cylinder[X86_FDC_DRIVE_COUNT];
    lib_u8 seek_result_count;
    lib_u16 head;
    lib_u16 sector;
    lib_u16 eot;
    lib_u16 byte_offset;
    lib_u32 transfer_remaining;
    lib_u16 format_headers_remaining;
    lib_u8 format_id[4];
    lib_u8 format_id_index;
    lib_u8 selected_drive;
    /* Reset queues the controller's pending Sense-Interrupt drive reports. */
    lib_u8 reset_sense_mask;
    lib_u64 reset_due_tick;
    lib_u8 reset_pending;
    lib_u8 observed_ready[X86_FDC_DRIVE_COUNT];
    lib_u8 ready_sense_mask;
    lib_u8 ready_poll_enabled;
    lib_u8 dma_byte_gate_pending;
    lib_u8 ndma_byte_gate_pending;
    lib_u64 elapsed_ticks;
    lib_u64 next_dma_byte_tick;
    lib_u64 next_ndma_byte_tick;
} x86_fdc_data;


struct x86_fdc {
    x86_fdc_data data;
    x86_fdc_connection connect;
    x86_fdc_timing timing;
    lib_bool reset_released;
    lib_bool service_enabled;
    lib_bool irq;
    lib_bool drq;
};

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

/* status register 0 bits */
#define VFDC_ST0_DS       0x03 /* drive select */
#define VFDC_ST0_SEEK_END 0x20
#define VFDC_ST0_EQUIPMENT_CHECK 0x10
#define x86_fdc_ST0_NORMAL 0x00
#define x86_fdc_ST0_ABNORMAL 0x40
#define x86_fdc_ST0_READY_CHANGE 0xc0
#define x86_fdc_ST0_NOT_READY 0x08

/* status register 2 bits */
#define VFDC_ST2_SCAN_MATCH    0x08
#define VFDC_ST2_SCAN_MISMATCH 0x04
#define VFDC_ST2_CONTROL_MARK  0x40

/* status register 3 bit */
#define VFDC_ST3_DS 0x03 /* drive select */

/* fdc command specify bytes */
#define VFDC_CMD_Specify1_HUT 0x0f /* head unload duration */
#define VFDC_CMD_Specify1_SRT 0xf0 /* step rate duration */
#define VFDC_CMD_Specify2_HLT 0xfe /* head load duration */
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


#endif
