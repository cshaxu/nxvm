/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_FDC_H
#define CORE_MACHINE_FDC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "app-nxvm/devices/controller_interface.h"
#include "app-nxvm/devices/dma.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/fdc_observation_interface.h"

typedef struct t_pic t_pic;
typedef struct t_port t_port;

#define CORE_MACHINE_DEVICE_FDC "Intel 8272A"

typedef enum core_machine_fdc_phase {
    core_machine_fdc_PHASE_COMMAND = 0,
    core_machine_fdc_PHASE_PENDING_COMMAND,
    core_machine_fdc_PHASE_EXECUTION_READ,
    core_machine_fdc_PHASE_EXECUTION_WRITE,
    core_machine_fdc_PHASE_EXECUTION_SCAN,
    core_machine_fdc_PHASE_EXECUTION_FORMAT,
    core_machine_fdc_PHASE_PENDING_COMPLETE,
    core_machine_fdc_PHASE_RESULT
} core_machine_fdc_phase;

typedef struct {
    lib_u8 dor; /* digital output register */
    lib_u8 msr; /* main status register */
    lib_u8 dr;  /* data register */
    lib_u8 dir; /* digital input register */
    lib_u8 ccr; /* configuration control register */

    lib_u8 hut; /* head unload duration */
    lib_u8 hlt; /* head load duration */
    lib_u8 srt; /* step rate duration */
    lib_u8 flagNDMA; /* 0 = dma mode; 1 = non-dma mode */
    lib_u8 flagINTR; /* 0 = no intr; 1 = has intr */

    core_machine_fdc_phase phase;
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
    lib_u16 drive_cylinder[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_u16 seek_target[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_u64 seek_due_tick[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_u8 seek_pending[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_u8 seek_result_st0[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_u8 seek_result_cylinder[CORE_MACHINE_FDC_DRIVE_COUNT];
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
    lib_u64 observed_media_generation[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_u8 media_changed[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_u8 observed_ready[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_u8 initial_media_baseline_pending;
    lib_u8 ready_poll_enabled;
    lib_u8 dma_byte_gate_pending;
    lib_u8 ndma_byte_gate_pending;
    lib_u64 elapsed_ticks;
    lib_u64 next_dma_byte_tick;
    lib_u64 next_ndma_byte_tick;
} core_machine_fdc_data;

typedef void (*core_machine_fdc_dma_request_operation)(void *owner,
    const core_machine_dma_request_binding *binding);

typedef struct {
    const core_machine_media_registry *media_registry;
    core_machine_fdc_drive_bindings drives;
    core_machine_dma_request_binding dma_request;
    core_machine_fdc_dma_request_operation dma_request_assert;
    core_machine_fdc_dma_request_operation dma_request_deassert;
    void *dma_request_owner;
    core_machine_pic_irq_source irq_source;
    t_port *port;
    core_machine_fdc_config config;
    core_machine_fdc_terminal_observation_provider observation_provider;
    lib_u64 observation_sequence;
} core_machine_fdc_connection;

typedef struct {
    core_machine_fdc_data data;
    core_machine_fdc_connection connect;
} core_machine_fdc;

/*
 * MSR: RQM | DIO | NDM | CB  | D3B | D2B | D1B | D0B
 * DIR: DC  | -   | -   | -   | -   | -   | -   | HD
 * DOR: ME3 | ME2 | ME1 | ME0 | -   | RST | DS1 | DS0
 * CCR: -   | -   | -   | -   | -   | -   | DRS1| DRS0
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

/* Configuration control register data-rate select.  The 8272A-compatible
 * PC/AT route uses 500, 300 and 250 kbps; 1 Mbps is not admitted here. */
#define VFDC_CCR_RATE_500 0x00u
#define VFDC_CCR_RATE_300 0x01u
#define VFDC_CCR_RATE_250 0x02u
#define VFDC_CCR_RATE_MASK 0x03u

/* status register 0 bits */
#define VFDC_ST0_DS       0x03 /* drive select */
#define VFDC_ST0_SEEK_END 0x20
#define VFDC_ST0_EQUIPMENT_CHECK 0x10
#define core_machine_fdc_ST0_NORMAL 0x20
#define core_machine_fdc_ST0_ABNORMAL 0x40
#define core_machine_fdc_ST0_READY_CHANGE 0xc0
#define core_machine_fdc_ST0_NOT_READY 0x08

/* status register 2 bits */
#define VFDC_ST2_SCAN_MATCH    0x04
#define VFDC_ST2_SCAN_MISMATCH 0x08
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

/* convert byte per sector from or to bps type  */
#define VFDC_GetBPS(cb) (0x0080 << (cb))  /* convert bps type to bps */
lib_u8 VFDC_GetBPSC(lib_u16 cb); /* convert bps to bps type */

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
/* #define VFDC_GetHUT(cbyte)  ((cbyte) & 0x0f) * head unload duration */
/* #define VFDC_GetSRT(cbyte)  ((cbyte) >> 4)   * step rate duration */
/* #define VFDC_GetHLT(cbyte)  ((cbyte) >> 1)   * head load duration */
/* #define VFDC_GetNDMA(cbyte) ((cbyte) & 0x01) * non-dma mode */
/* #define VFDC_GetHDS(cbyte)  (!!((cbyte) & 0x04)) * head select (0 or 1) */
/* #define VFDC_GetBPS(cbyte)  (0x0080 << (cbyte))  * bytes per sector */
/* sector size code */

void core_machine_fdc_connect(core_machine_fdc *fdc,
    const core_machine_media_registry *media_registry,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_dma_request_binding *dma_request,
    core_machine_fdc_dma_request_operation dma_request_assert,
    core_machine_fdc_dma_request_operation dma_request_deassert,
    void *dma_request_owner, t_pic *pic_master, t_pic *pic_slave,
    t_port *port, const core_machine_fdc_config *config,
    const core_machine_fdc_terminal_observation_provider *observation_provider);
const core_machine_dma_channel_provider *core_machine_fdc_dma_provider(void);
void core_machine_fdc_initialize(core_machine_fdc *fdc);
void core_machine_fdc_reset(core_machine_fdc *fdc);
void core_machine_fdc_advance(core_machine_fdc *fdc);
void core_machine_fdc_advance_at(core_machine_fdc *fdc,
    lib_u64 elapsed_ticks);
lib_status core_machine_fdc_next_due_tick(const core_machine_fdc *fdc,
    lib_u64 *out_due_tick);
void core_machine_fdc_refresh(core_machine_fdc *fdc);
void core_machine_fdc_finalize(core_machine_fdc *fdc);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
