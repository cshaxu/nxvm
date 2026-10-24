/* Copyright 2012-2026 Neko. */
#ifndef X86_NXVM_BOARD_KBC_INTERFACE_H
#define X86_NXVM_BOARD_KBC_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/machine/port_interface.h"
#include "x86-nxvm/chips/kbc8042/kbc8042_interface.h"

typedef struct t_kbc t_kbc;
typedef void (*x86_nxvm_board_kbc_irq_line)(void *context,
    lib_bool auxiliary, lib_bool asserted);
typedef x86_kbc8042_command_extension_result (*x86_nxvm_board_kbc_command_extension)(
    void *context, lib_u8 command, lib_u8 input_port, lib_u8 command_byte,
    lib_u8 *out_data);

/* Serialized PC/AT input owner. Construction publishes routes against one
 * stable opaque handle. Composition owns IRQ leases and supplies electrical
 * line/A20/reset sinks; their contexts outlive this adapter. Destroy only
 * after Core has stopped dispatching, including partial-construction teardown. */
lib_status x86_nxvm_board_kbc_create(x86_core_machine *machine, t_kbc **out_controller);
void x86_nxvm_board_kbc_destroy(t_kbc *controller);
void x86_nxvm_board_kbc_bind_core_services(t_kbc *controller,
    x86_nxvm_board_kbc_irq_line irq, void *irq_context,
    void (*set_a20)(void *context, lib_bool enabled), void *a20_context,
    void (*request_reset)(void *context), void *reset_context,
    lib_u8 aux_present);
/* One selected board may bind an otherwise-unhandled KBC command family
 * during construction.  The KBC retains port and output ownership. */
lib_status x86_nxvm_board_kbc_bind_command_extension(t_kbc *controller,
    x86_nxvm_board_kbc_command_extension extension, void *context);
void x86_nxvm_board_kbc_reset(t_kbc *controller);
void x86_nxvm_board_kbc_advance(t_kbc *controller, lib_u64 elapsed_ticks);
lib_status x86_nxvm_board_kbc_ticks_until_event(const t_kbc *controller,
    lib_u64 *out_ticks);
/* Copied keyboard protocol identity, used to encode the native input stream. */
lib_u8 x86_nxvm_board_kbc_get_native_scan_set(const t_kbc *controller);
void x86_nxvm_board_kbc_set_input_port(t_kbc *controller, lib_u8 value);
void x86_nxvm_board_kbc_set_reset_output_port(t_kbc *controller, lib_u8 value);
void x86_nxvm_board_kbc_set_reset_system_flag(t_kbc *controller,
    lib_bool enabled);
void x86_nxvm_board_kbc_set_typematic_timing(t_kbc *controller,
    lib_u32 initial_ticks, lib_u32 repeat_ticks);
void x86_nxvm_board_kbc_set_command_response_timing(t_kbc *controller,
    lib_u32 response_ticks);
void x86_nxvm_board_kbc_set_command_response_status_polls(t_kbc *controller,
    lib_u8 status_polls);
void x86_nxvm_board_kbc_set_serial_delivery_timing(t_kbc *controller,
    lib_u32 delivery_ticks);
void x86_nxvm_board_kbc_set_output_pulse_timing(t_kbc *controller,
    lib_u32 pulse_ticks);
lib_status x86_nxvm_board_kbc_submit_native_byte(t_kbc *controller, lib_u8 byte);
lib_status x86_nxvm_board_kbc_submit_native_bytes(t_kbc *controller,
    const lib_u8 *bytes, lib_size count);
lib_status x86_nxvm_board_kbc_submit_aux_report(t_kbc *controller,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons);

#endif
