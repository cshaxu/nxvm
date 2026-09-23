#ifndef CORE_MACHINE_XT_KEYBOARD_H
#define CORE_MACHINE_XT_KEYBOARD_H
#include "lib/types/types_interface.h"

#include "type.h"

#include "app-nxvm/devices/xt_ppi_keyboard.h"

#define CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY 16u

typedef struct core_machine_xt_keyboard {
    core_machine_xt_ppi_keyboard *ppi;
    lib_u64 ticks_per_second;
    lib_u64 reset_low_ticks;
    lib_u64 serial_remaining_ticks;
    lib_u64 bat_remaining_ticks;
    lib_u64 clock_low_ticks;
    lib_u8 fifo[CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY];
    lib_u8 fifo_head;
    lib_u8 fifo_count;
    lib_u8 serial_byte;
    lib_u8 serial_bits_remaining;
    type_bool clock_held;
    type_bool clear_asserted;
    type_bool serial_active;
    type_bool serial_response;
    type_bool bat_active;
    type_bool bat_result_pending;
} core_machine_xt_keyboard;

type_status core_machine_xt_keyboard_initialize(core_machine_xt_keyboard *keyboard,
    core_machine_xt_ppi_keyboard *ppi, lib_u64 ticks_per_second);
C_VOID core_machine_xt_keyboard_reset(core_machine_xt_keyboard *keyboard);
C_VOID core_machine_xt_keyboard_finalize(core_machine_xt_keyboard *keyboard);
C_VOID core_machine_xt_keyboard_observe_ppi_lines(C_VOID *owner,
    type_bool clock_held, type_bool clear_asserted);
C_VOID core_machine_xt_keyboard_notify_ppi_byte_released(C_VOID *owner);
type_status core_machine_xt_keyboard_receive_native_bytes(core_machine_xt_keyboard *keyboard,
    const lib_u8 *bytes, lib_size count);
C_VOID core_machine_xt_keyboard_advance(core_machine_xt_keyboard *keyboard,
    lib_u64 ticks);
type_status core_machine_xt_keyboard_ticks_until_event(const core_machine_xt_keyboard *keyboard,
    lib_u64 *out_ticks);

#endif
