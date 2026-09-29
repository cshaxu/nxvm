/* Copyright 2012-2026 Neko. */
#ifndef X86_HDC_INTERFACE_H
#define X86_HDC_INTERFACE_H
#include "lib/types/types_interface.h"

typedef struct x86_hdc x86_hdc;
#define X86_HDC_STATUS_ERR 0x01u
#define X86_HDC_STATUS_DRQ 0x08u
#define X86_HDC_STATUS_DSC 0x10u
#define X86_HDC_STATUS_DRDY 0x40u
#define X86_HDC_STATUS_BSY 0x80u

#define X86_HDC_ERROR_ABORT 0x04u
#define X86_HDC_ERROR_ID_NOT_FOUND 0x10u
#define X86_HDC_ERROR_DIAGNOSTIC_OK 0x01u

#define X86_HDC_DEVICE_CONTROL_NIEN 0x02u
#define X86_HDC_DEVICE_CONTROL_SRST 0x04u

typedef enum x86_hdc_phase {
    X86_HDC_PHASE_IDLE,
    X86_HDC_PHASE_PENDING_COMMAND,
    X86_HDC_PHASE_DATA_READ,
    X86_HDC_PHASE_DATA_WRITE,
    X86_HDC_PHASE_PENDING_READ_SECTOR,
    X86_HDC_PHASE_PENDING_WRITE_SECTOR
} x86_hdc_phase;

typedef enum x86_xebec_phase {
    X86_XEBEC_PHASE_IDLE,
    X86_XEBEC_PHASE_DCB,
    X86_XEBEC_PHASE_INITIALIZE,
    X86_XEBEC_PHASE_PENDING_COMMAND,
    X86_XEBEC_PHASE_RESPONSE,
    X86_XEBEC_PHASE_DMA_READ,
    X86_XEBEC_PHASE_DMA_WRITE
} x86_xebec_phase;
typedef enum x86_hdc_protocol {
    X86_HDC_PROTOCOL_ATA_PIO,
    X86_HDC_PROTOCOL_COMPAQ_WD_40MB,
    X86_HDC_PROTOCOL_IBM_WD1003_ST506,
    X86_HDC_PROTOCOL_XEBEC_XT
} x86_hdc_protocol;

typedef enum x86_hdc_register {
    X86_HDC_REGISTER_DATA,
    X86_HDC_REGISTER_ERROR_FEATURES,
    X86_HDC_REGISTER_SECTOR_COUNT,
    X86_HDC_REGISTER_SECTOR_NUMBER,
    X86_HDC_REGISTER_CYLINDER_LOW,
    X86_HDC_REGISTER_CYLINDER_HIGH,
    X86_HDC_REGISTER_DRIVE_HEAD,
    X86_HDC_REGISTER_STATUS_COMMAND,
    X86_HDC_REGISTER_CONTROL,
    X86_HDC_REGISTER_DRIVE_ADDRESS,
    X86_HDC_REGISTER_XEBEC_RESET,
    X86_HDC_REGISTER_XEBEC_SELECT,
    X86_HDC_REGISTER_XEBEC_MASK
} x86_hdc_register;

typedef struct x86_hdc_geometry {
    lib_u64 logical_sector_count;
    lib_u32 bytes_per_sector;
    lib_u32 cylinders, heads, sectors_per_track;
} x86_hdc_geometry;

typedef struct x86_hdc_medium {
    x86_hdc_geometry geometry;
    lib_bool present, read_only, geometry_known;
} x86_hdc_medium;

typedef enum x86_hdc_record_result {
    X86_HDC_RECORD_OK,
    X86_HDC_RECORD_ABSENT,
    X86_HDC_RECORD_RANGE,
    X86_HDC_RECORD_PROTECTED,
    X86_HDC_RECORD_FAILURE
} x86_hdc_record_result;

typedef struct x86_hdc_config {
    x86_hdc_protocol protocol;
    lib_bool lba28_supported;
    lib_u32 command_ticks, next_sector_ticks;
    lib_u32 step_ticks[16];
    x86_hdc_geometry xebec_geometry;
} x86_hdc_config;

/* Unit is the controller's drive selection, not a registry or file identity.
 * Read/write transfer exactly one 512-byte sector. Failed queries leave no
 * usable geometry. All callbacks are synchronous, non-reentrant and borrowed
 * until destroy completes; one execution owner calls this component.
 * IRQ/DRQ are level outputs, including withdrawal on reset/destruction.
 * Missing query means unavailable media; missing read/write fails that
 * transfer. A missing signal callback leaves that output unconnected. */
typedef struct x86_hdc_connection {
    x86_hdc_record_result (*query)(void *context, lib_u8 unit, x86_hdc_medium *out_medium);
    x86_hdc_record_result (*read)(void *context, lib_u8 unit, lib_u64 sector, lib_u8 *data);
    x86_hdc_record_result (*write)(void *context, lib_u8 unit, lib_u64 sector, const lib_u8 *data);
    void (*irq)(void *context, lib_bool asserted);
    void (*drq)(void *context, lib_bool asserted);
    void *context;
} x86_hdc_connection;

/* Copied diagnostics, not a mutable command or scheduling interface. */
typedef struct x86_hdc_observation {
    lib_u64 elapsed_ticks, next_service_tick;
    lib_u32 command_count, step_rate_ticks;
    lib_u16 data_index, sectors_remaining, step_pulse_limit;
    lib_u8 phase, status, error, last_command;
    lib_u8 sector_count, sector_number, cylinder_low, cylinder_high, drive_head;
    lib_u8 fixed_disk_register, step_rate_selector;
    lib_u8 xebec_phase, xebec_dcb_count, xebec_mask, xebec_initialize[8];
    lib_bool irq_pending;
} x86_hdc_observation;

lib_status x86_hdc_create(const x86_hdc_config *config,
    const x86_hdc_connection *connection, x86_hdc **out_hdc);
void x86_hdc_destroy(x86_hdc *hdc);
/* Cold reset starts a new time epoch at zero; guest reset-register writes
 * reset controller state without rewinding the caller's current epoch. */
void x86_hdc_reset(x86_hdc *hdc);
lib_status x86_hdc_read(x86_hdc *hdc, x86_hdc_register reg, lib_u32 *out_value);
lib_status x86_hdc_write(x86_hdc *hdc, x86_hdc_register reg, lib_u32 value);
/* An unavailable DMA read preserves the caller's byte. */
void x86_hdc_dma_read(x86_hdc *hdc, lib_u8 *io_byte);
void x86_hdc_dma_write(x86_hdc *hdc, lib_u8 byte);
void x86_hdc_terminal_count(x86_hdc *hdc);
/* Caller-owned monotonic guest time within the current cold-reset epoch.
 * Equal timestamps may service due-now work, never replay a completed step. */
void x86_hdc_advance_at(x86_hdc *hdc, lib_u64 now);
/* OK includes tick zero; INVALID_STATE means there is no pending event. */
lib_status x86_hdc_next_due_tick(const x86_hdc *hdc, lib_u64 *out_tick);
lib_bool x86_hdc_irq_pending(const x86_hdc *hdc);
lib_status x86_hdc_capture(const x86_hdc *hdc, x86_hdc_observation *out_observation);
#endif
