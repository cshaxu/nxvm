/* Copyright 2012-2026 Neko. */
#ifndef CORE_MACHINE_KBC_INTERFACE_H
#define CORE_MACHINE_KBC_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86/core/port_interface.h"

typedef struct t_kbc t_kbc;
typedef void (*core_machine_kbc_irq_line)(void *context,
    lib_bool auxiliary, lib_bool asserted);

/* Serialized PC/AT input owner. Construction publishes routes against one
 * stable opaque handle. Composition owns IRQ leases and supplies electrical
 * line/A20/reset sinks; their contexts outlive this adapter. Destroy only
 * after Core has stopped dispatching, including partial-construction teardown. */
lib_status core_machine_kbc_create(core_machine *machine, t_kbc **out_controller);
void core_machine_kbc_destroy(t_kbc *controller);
void core_machine_kbc_bind_core_services(t_kbc *controller,
    core_machine_kbc_irq_line irq, void *irq_context,
    void (*set_a20)(void *context, lib_bool enabled), void *a20_context,
    void (*request_reset)(void *context), void *reset_context,
    lib_u8 aux_present);
void core_machine_kbc_reset(t_kbc *controller);
void core_machine_kbc_advance(t_kbc *controller, lib_u64 elapsed_ticks);
lib_status core_machine_kbc_ticks_until_event(const t_kbc *controller,
    lib_u64 *out_ticks);
/* Copied keyboard protocol identity, used to encode the native input stream. */
lib_u8 core_machine_kbc_get_native_scan_set(const t_kbc *controller);
void core_machine_kbc_set_input_port(t_kbc *controller, lib_u8 value);
void core_machine_kbc_set_reset_output_port(t_kbc *controller, lib_u8 value);
void core_machine_kbc_set_typematic_timing(t_kbc *controller,
    lib_u32 initial_ticks, lib_u32 repeat_ticks);
void core_machine_kbc_set_command_response_timing(t_kbc *controller,
    lib_u32 response_ticks);
void core_machine_kbc_set_command_response_status_polls(t_kbc *controller,
    lib_u8 status_polls);
void core_machine_kbc_set_serial_delivery_timing(t_kbc *controller,
    lib_u32 delivery_ticks);
lib_status core_machine_kbc_submit_native_byte(t_kbc *controller, lib_u8 byte);
lib_status core_machine_kbc_submit_native_bytes(t_kbc *controller,
    const lib_u8 *bytes, lib_size count);
lib_status core_machine_kbc_submit_aux_report(t_kbc *controller,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons);

#endif
