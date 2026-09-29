/* Copyright 2012-2026 Neko. */
#ifndef X86_FDC8272_INTERFACE_H
#define X86_FDC8272_INTERFACE_H
#include "lib/types/types_interface.h"

#define X86_FDC_DRIVE_COUNT 4u
typedef struct x86_fdc x86_fdc;

typedef struct x86_fdc_pins {
    lib_bool ready;
    lib_bool track_zero;
    lib_bool write_protected;
    lib_bool two_sided;
    lib_bool fault;
    /* The connected recording channel can supply data; distinct from READY. */
    lib_bool data_available;
} x86_fdc_pins;

typedef struct x86_fdc_track {
    lib_u16 cylinder;
    lib_u16 cylinders;
    lib_u16 heads;
    lib_u16 sectors;
    lib_u16 bytes_per_sector;
    lib_bool id_readable;
} x86_fdc_track;

typedef struct x86_fdc_record {
    lib_u16 cylinder;
    lib_u16 head;
    lib_u16 sector;
    lib_u16 offset;
} x86_fdc_record;

typedef enum x86_fdc_record_result {
    X86_FDC_RECORD_OK,
    X86_FDC_RECORD_ABSENT,
    X86_FDC_RECORD_PROTECTED,
    X86_FDC_RECORD_FAILURE
} x86_fdc_record_result;

/* All values use the caller's monotonic guest axis. Step duration is
 * ceil((16-SRT)*step_numerator/step_denominator). Zero byte duration retains
 * the qualified logical next-progression DMA gate, not a physical claim. */
typedef struct x86_fdc_timing {
    lib_u64 reset_ticks;
    lib_u64 step_numerator;
    lib_u64 step_denominator;
    lib_u64 fm_byte_ticks;
    lib_u64 mfm_byte_ticks;
} x86_fdc_timing;

typedef struct x86_fdc_terminal {
    lib_u8 command;
    lib_u8 drive;
    lib_u8 result[7];
    lib_bool successful;
} x86_fdc_terminal;

/* Copied diagnostics for observers, never an execution or mutation handle. */
typedef struct x86_fdc_observation {
    lib_u64 elapsed_ticks;
    lib_u64 reset_due_tick;
    lib_u64 next_dma_byte_tick;
    lib_u32 transfer_remaining;
    lib_u16 cylinder, head, sector, eot;
    lib_u8 phase, msr;
    lib_u8 command[9], result[7];
    lib_u8 pcn[X86_FDC_DRIVE_COUNT];
    lib_u8 command_index, result_length, result_index;
    lib_u8 st0, st1, st2, st3;
    lib_u8 reset_sense_mask, seek_result_count;
    lib_bool reset_pending, dma_byte_gate_pending, interrupt_pending;
    lib_bool irq, drq;
} x86_fdc_observation;

/* Unit is the chip's US output. The board resolves it against its select
 * wiring. Track succeeds when record bounds are available; id_readable says
 * whether the selected channel can read an ID at command entry. A failed
 * query leaves zero bounds and may retain the physical cylinder for status.
 * No media handle or image offset crosses this contract.
 * Providers execute synchronously on the sole owner; no reentrant chip calls.
 * Every callback/context remains valid until after destroy. */
typedef struct x86_fdc_connection {
    x86_fdc_pins (*sample)(void *context, lib_u8 unit);
    void (*step)(void *context, lib_u8 unit, lib_bool outward);
    lib_bool (*track)(void *context, lib_u8 unit, lib_bool mfm,
        x86_fdc_track *out_track);
    x86_fdc_record_result (*read)(void *context, lib_u8 unit,
        const x86_fdc_record *record, lib_u8 *out_byte);
    x86_fdc_record_result (*write)(void *context, lib_u8 unit,
        const x86_fdc_record *record, lib_u8 byte);
    x86_fdc_record_result (*read_mark)(void *context, lib_u8 unit,
        const x86_fdc_record *record, lib_bool *out_deleted);
    x86_fdc_record_result (*write_mark)(void *context, lib_u8 unit,
        const x86_fdc_record *record, lib_bool deleted);
    x86_fdc_record_result (*format)(void *context, lib_u8 unit,
        const x86_fdc_record *record, lib_u8 fill);
    void (*irq)(void *context, lib_bool asserted);
    void (*drq)(void *context, lib_bool asserted);
    void (*terminal)(void *context, const x86_fdc_terminal *result);
    void *context;
} x86_fdc_connection;

lib_status x86_fdc_create(const x86_fdc_connection *connection,
    const x86_fdc_timing *timing, x86_fdc **out_fdc);
void x86_fdc_destroy(x86_fdc *fdc);
void x86_fdc_reset(x86_fdc *fdc);
void x86_fdc_set_reset(x86_fdc *fdc, lib_bool released);
void x86_fdc_set_service_enabled(x86_fdc *fdc, lib_bool enabled);
lib_status x86_fdc_set_timing(x86_fdc *fdc, const x86_fdc_timing *timing);
lib_u8 x86_fdc_read_status(const x86_fdc *fdc);
/* Unavailable reads leave *io_byte unchanged. */
void x86_fdc_read_data(x86_fdc *fdc, lib_u8 *io_byte);
void x86_fdc_write_data(x86_fdc *fdc, lib_u8 byte);
void x86_fdc_dma_read(x86_fdc *fdc, lib_u8 *io_byte);
void x86_fdc_dma_write(x86_fdc *fdc, lib_u8 byte);
void x86_fdc_terminal_count(x86_fdc *fdc);
void x86_fdc_refresh(x86_fdc *fdc);
/* Sample READY transitions into SIS causes; distinct from transfer-input
 * refresh, which aborts unavailable records without manufacturing a poll. */
void x86_fdc_poll_ready(x86_fdc *fdc);
void x86_fdc_advance_at(x86_fdc *fdc, lib_u64 now);
/* OK includes due-now; INVALID_STATE means no pending event. */
lib_status x86_fdc_next_due_tick(const x86_fdc *fdc, lib_u64 *out_tick);
lib_status x86_fdc_capture(const x86_fdc *fdc, x86_fdc_observation *out_observation);
#endif
