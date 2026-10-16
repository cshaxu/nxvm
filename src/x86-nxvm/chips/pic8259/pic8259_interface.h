/* Copyright 2012-2026 Neko. */
#ifndef X86_PIC8259_INTERFACE_H
#define X86_PIC8259_INTERFACE_H
#include "lib/types/types_interface.h"

typedef struct x86_pic x86_pic;
typedef struct x86_pic_request {
    lib_u8 line;
    lib_u8 vector;
    lib_bool cascade;
} x86_pic_request;

typedef struct x86_pic_register_state {
    lib_u8 irr;
    lib_u8 imr;
    lib_u8 isr;
} x86_pic_register_state;

/* Copied diagnostics, without changing OCW3, polling or acknowledgement. */
lib_status x86_pic_capture_registers(const x86_pic *pic,
    x86_pic_register_state *out_state);

/* One execution owner. Each instance models one controller, never a pair.
 * Board source aggregation, port decode and cascade wiring are external.
 * No operation calls or owns another chip. Stop callers before destruction. */
/* cascade_master is the construction-fixed SP/board role; it does not supply
 * a peer pointer or override the guest's ICW cascade programming. */
lib_status x86_pic_create(lib_bool cascade_master, x86_pic **out_pic);
void x86_pic_destroy(x86_pic *pic);
void x86_pic_reset(x86_pic *pic);
/* Selector 0 is command, 1 is data. Reads preserve *io_value when the chip
 * does not drive the bus. A poll read acknowledges; it is not an observation. */
void x86_pic_read_register(x86_pic *pic, lib_u8 selector, lib_u8 *io_value);
void x86_pic_write_register(x86_pic *pic, lib_u8 selector, lib_u8 value);
/* Resolved IRQ levels, newly asserted requests, and available slave INT lines.
 * The board supplies edge events once; level refresh may repeat. Cascade
 * inputs are accepted only at lines enabled by this controller's ICW3. */
void x86_pic_set_inputs(x86_pic *pic, lib_u8 levels, lib_u8 asserted_requests,
    lib_u8 cascade_lines);
/* The programmed CAS identity is available only in cascade mode. */
lib_bool x86_pic_cascade_address(const x86_pic *pic, lib_u8 *out_address);
/* Observation returns INVALID_STATE without changing output when no request
 * is eligible. Acknowledge selects and consumes one request; with no eligible
 * request it returns the existing spurious vector (zero before initialization).
 * Cascade routing and the slave's subsequent acknowledge belong to the board. */
lib_status x86_pic_select(const x86_pic *pic, x86_pic_request *out_request);
x86_pic_request x86_pic_acknowledge(x86_pic *pic);
/* Copied per-line durations in caller-defined guest units; no host time.
 * Reset preserves durations. No-event and bad arguments are distinct. */
void x86_pic_set_irq_timing(x86_pic *pic, const lib_u32 ticks[8]);
void x86_pic_advance(x86_pic *pic, lib_u64 elapsed_ticks);
lib_status x86_pic_ticks_until_event(const x86_pic *pic, lib_u64 *out_ticks);
#endif
