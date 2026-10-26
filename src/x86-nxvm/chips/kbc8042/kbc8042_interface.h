/* Copyright 2012-2026 Neko. */
#ifndef X86_KBC8042_INTERFACE_H
#define X86_KBC8042_INTERFACE_H
#include "lib/types/types_interface.h"

typedef struct x86_kbc8042 x86_kbc8042;
typedef enum x86_kbc8042_origin {
    X86_KBC8042_OUTPUT_CONTROLLER,
    X86_KBC8042_OUTPUT_KEYBOARD,
    X86_KBC8042_OUTPUT_AUX
} x86_kbc8042_origin;

/* A selected board may own a documented command outside the portable 8042
 * command set.  The controller still owns port-64 parsing and every read of
 * port 60.  SPECIAL_READ is the one documented form that supplies a byte
 * without raising OBF or an IRQ. */
typedef enum x86_kbc8042_command_extension_result {
    X86_KBC8042_COMMAND_EXTENSION_UNHANDLED,
    X86_KBC8042_COMMAND_EXTENSION_HANDLED,
    X86_KBC8042_COMMAND_EXTENSION_SPECIAL_READ
} x86_kbc8042_command_extension_result;

typedef struct x86_kbc8042_keyboard_inputs {
    lib_u8 scan_set;
    lib_bool scanning;
    lib_bool bat_ready;
    lib_bool repeat_pending;
    lib_u64 repeat_ticks;
    lib_u8 repeat_bytes;
} x86_kbc8042_keyboard_inputs;

/* Qualified controller/transport model, not an 8042 MCU. One execution owner.
 * Connections are copied; their context is borrowed through destruction.
 * Endpoint state is sampled, never retained. No endpoint object is owned here.
 * Writes may synchronously deliver receive_reply/reset_keyboard_stream; an
 * advance may call its supplied sequence sink. Other recursive controller calls,
 * rebinding or destruction from a callback are forbidden. */
typedef struct x86_kbc8042_link {
    x86_kbc8042_keyboard_inputs (*keyboard_inputs)(void *context);
    lib_bool (*keyboard_start)(void *context);
    lib_bool (*keyboard_take_bat)(void *context);
    void (*keyboard_clear_bat)(void *context);
    void (*keyboard_cancel_parameter)(void *context);
    void (*keyboard_write)(void *context, lib_u8 byte);
    void (*aux_write)(void *context, lib_u8 byte);
    void (*keyboard_accepted)(void *context, lib_u8 byte);
    lib_status (*keyboard_admit)(void *context, const lib_u8 *bytes,
        lib_size count, lib_u8 *output, lib_size capacity, lib_size *output_count);
    lib_bool (*keyboard_advance)(void *context, lib_u64 ticks,
        lib_status (*emit)(void *context, const lib_u8 *bytes, lib_size count),
        void *emit_context);
    void (*irq)(void *context, lib_bool auxiliary, lib_bool asserted);
    void (*output_port)(void *context, lib_u8 value);
    x86_kbc8042_command_extension_result (*command_extension)(void *context,
        lib_u8 command, lib_u8 input_port, lib_u8 command_byte, lib_u8 *out_data);
    void *context;
} x86_kbc8042_link;

lib_status x86_kbc8042_create(const x86_kbc8042_link *link, x86_kbc8042 **out_controller);
void x86_kbc8042_destroy(x86_kbc8042 *controller);
/* Reset preserves connections and configured delays, releases IRQ outputs,
 * and reapplies the reset output port. The composer resets endpoints first. */
void x86_kbc8042_reset(x86_kbc8042 *controller);
lib_u8 x86_kbc8042_read_data(x86_kbc8042 *controller);
lib_u8 x86_kbc8042_read_status(x86_kbc8042 *controller);
void x86_kbc8042_write_data(x86_kbc8042 *controller, lib_u8 value);
void x86_kbc8042_write_command(x86_kbc8042 *controller, lib_u8 value);
void x86_kbc8042_set_aux_present(x86_kbc8042 *controller, lib_bool present);
lib_bool x86_kbc8042_aux_enabled(const x86_kbc8042 *controller);
void x86_kbc8042_set_input_port(x86_kbc8042 *controller, lib_u8 value);
void x86_kbc8042_set_test_inputs(x86_kbc8042 *controller, lib_u8 value);
void x86_kbc8042_set_reset_output_port(x86_kbc8042 *controller, lib_u8 value);
/* Some controller implementations complete their own power-on test before
 * host reset.  This frozen electrical reset state controls only status bit 2;
 * it does not synthesize a guest command reply or alter other command bits. */
void x86_kbc8042_set_reset_system_flag(x86_kbc8042 *controller,
    lib_bool enabled);
void x86_kbc8042_set_reset_command_byte(x86_kbc8042 *controller, lib_u8 value);
void x86_kbc8042_set_command_response_timing(x86_kbc8042 *controller, lib_u32 ticks);
void x86_kbc8042_set_command_response_status_polls(x86_kbc8042 *controller, lib_u8 polls);
void x86_kbc8042_set_serial_delivery_timing(x86_kbc8042 *controller, lib_u32 ticks);
/* Output pulse duration in caller service units; zero selects synchronous
 * low/restore ordering only. Changes configure subsequent pulses. */
void x86_kbc8042_set_output_pulse_timing(x86_kbc8042 *controller, lib_u32 ticks);
/* Caller-configured service units, not host time. Deadline query is read-only;
 * INVALID_STATE means no pending timed work. All delivery uses one output path. */
void x86_kbc8042_advance(x86_kbc8042 *controller, lib_u64 ticks);
lib_status x86_kbc8042_ticks_until_event(const x86_kbc8042 *controller, lib_u64 *out_ticks);
/* The endpoint prepares a whole submission within available backlog capacity
 * before any output is queued. The byte entry uses this same admission path. */
lib_status x86_kbc8042_receive_keyboard_byte(x86_kbc8042 *controller, lib_u8 byte);
lib_status x86_kbc8042_receive_keyboard_bytes(x86_kbc8042 *controller,
    const lib_u8 *bytes, lib_size count);
/* Replies are copied immediately; delivery retains the configured response
 * delay and existing single pending-reply behavior. AUX packets are atomic. */
void x86_kbc8042_receive_reply(x86_kbc8042 *controller,
    x86_kbc8042_origin origin, const lib_u8 *bytes, lib_u8 count);
lib_status x86_kbc8042_receive_aux_packet(x86_kbc8042 *controller, const lib_u8 packet[3]);
/* Discard only unsent native bytes and partial translation. Controller output,
 * pending command replies, IRQ ownership and BAT are not flushed. */
void x86_kbc8042_reset_keyboard_stream(x86_kbc8042 *controller);
#endif
