/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_FDC_H
#define CORE_MACHINE_FDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/controller_interface.h"
#include "core/machine/dma.h"
#include "core/machine/media_interface.h"
#include "core/machine/pic.h"
#include "core/machine/fdc_observation_interface.h"

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

    core_machine_fdc_phase phase;
    type_unsigned_8 command_length;
    type_unsigned_8 command_index;
    type_unsigned_8 result_length;
    type_unsigned_8 result_index;
    type_unsigned_8 cmd[9];
    type_unsigned_8 ret[7];
    type_unsigned_8 st0, st1, st2, st3; /* state registers */
    type_unsigned_8 pending_st0;
    type_unsigned_8 pending_st1;
    type_unsigned_8 pending_st2;
    type_bool transfer_expect_deleted;
    type_bool transfer_write_deleted;
    type_unsigned_8 scan_mode;
    type_bool scan_sector_satisfies;
    type_unsigned_16 cylinder;
    type_unsigned_16 drive_cylinder[CORE_MACHINE_FDC_DRIVE_COUNT];
    type_unsigned_16 seek_target[CORE_MACHINE_FDC_DRIVE_COUNT];
    type_unsigned_64 seek_due_tick[CORE_MACHINE_FDC_DRIVE_COUNT];
    type_bool seek_pending[CORE_MACHINE_FDC_DRIVE_COUNT];
    type_unsigned_8 seek_result_st0[CORE_MACHINE_FDC_DRIVE_COUNT];
    type_unsigned_8 seek_result_cylinder[CORE_MACHINE_FDC_DRIVE_COUNT];
    type_unsigned_8 seek_result_count;
    type_unsigned_16 head;
    type_unsigned_16 sector;
    type_unsigned_16 eot;
    type_unsigned_16 byte_offset;
    type_unsigned_32 transfer_remaining;
    type_unsigned_16 format_headers_remaining;
    type_unsigned_8 format_id[4];
    type_unsigned_8 format_id_index;
    type_unsigned_8 selected_drive;
    /* A reset release reports only the ready inputs sampled at that edge. */
    type_unsigned_8 reset_sense_mask;
    type_unsigned_64 reset_due_tick;
    type_bool reset_pending;
    type_unsigned_64 observed_media_generation[CORE_MACHINE_FDC_DRIVE_COUNT];
    type_bool media_changed[CORE_MACHINE_FDC_DRIVE_COUNT];
    type_bool observed_ready[CORE_MACHINE_FDC_DRIVE_COUNT];
    type_bool initial_media_baseline_pending;
    type_bool ready_poll_enabled;
    type_bool dma_byte_gate_pending;
    type_bool ndma_byte_gate_pending;
    type_unsigned_64 elapsed_ticks;
    type_unsigned_64 next_dma_byte_tick;
    type_unsigned_64 next_ndma_byte_tick;
} core_machine_fdc_data;

typedef C_VOID (*core_machine_fdc_dma_request_operation)(C_VOID *owner,
    const core_machine_dma_request_binding *binding);

typedef struct {
    const core_machine_media_registry *media_registry;
    core_machine_fdc_drive_bindings drives;
    core_machine_dma_request_binding dma_request;
    core_machine_fdc_dma_request_operation dma_request_assert;
    core_machine_fdc_dma_request_operation dma_request_deassert;
    C_VOID *dma_request_owner;
    core_machine_pic_irq_source irq_source;
    t_port *port;
    core_machine_fdc_config config;
    core_machine_fdc_terminal_observation_provider observation_provider;
    type_unsigned_64 observation_sequence;
} core_machine_fdc_connection;

typedef struct {
    core_machine_fdc_data data;
    core_machine_fdc_connection connect;
} core_machine_fdc;

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

C_VOID core_machine_fdc_connect(core_machine_fdc *fdc,
    const core_machine_media_registry *media_registry,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_dma_request_binding *dma_request,
    core_machine_fdc_dma_request_operation dma_request_assert,
    core_machine_fdc_dma_request_operation dma_request_deassert,
    C_VOID *dma_request_owner, t_pic *pic_master, t_pic *pic_slave,
    t_port *port, const core_machine_fdc_config *config,
    const core_machine_fdc_terminal_observation_provider *observation_provider);
const core_machine_dma_channel_provider *core_machine_fdc_dma_provider(C_VOID);
C_VOID core_machine_fdc_initialize(core_machine_fdc *fdc);
C_VOID core_machine_fdc_reset(core_machine_fdc *fdc);
C_VOID core_machine_fdc_advance(core_machine_fdc *fdc);
C_VOID core_machine_fdc_advance_at(core_machine_fdc *fdc,
    type_unsigned_64 elapsed_ticks);
type_status core_machine_fdc_next_due_tick(const core_machine_fdc *fdc,
    type_unsigned_64 *out_due_tick);
C_VOID core_machine_fdc_refresh(core_machine_fdc *fdc);
C_VOID core_machine_fdc_finalize(core_machine_fdc *fdc);
C_VOID core_machine_fdc_print(const core_machine_fdc *fdc);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
