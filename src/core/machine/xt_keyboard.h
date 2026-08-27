#ifndef CORE_MACHINE_XT_KEYBOARD_H
#define CORE_MACHINE_XT_KEYBOARD_H

#include "type.h"

#include "core/machine/xt_ppi_keyboard.h"

#define CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY 16u

typedef struct core_machine_xt_keyboard {
    core_machine_xt_ppi_keyboard *ppi;
    type_unsigned_64 ticks_per_second;
    type_unsigned_64 reset_low_ticks;
    type_unsigned_64 serial_remaining_ticks;
    type_unsigned_64 bat_remaining_ticks;
    type_unsigned_64 clock_low_ticks;
    type_unsigned_8 fifo[CORE_MACHINE_XT_KEYBOARD_FIFO_CAPACITY];
    type_unsigned_8 fifo_head;
    type_unsigned_8 fifo_count;
    type_unsigned_8 serial_byte;
    type_unsigned_8 serial_bits_remaining;
    type_bool clock_held;
    type_bool clear_asserted;
    type_bool serial_active;
    type_bool serial_response;
    type_bool bat_active;
    type_bool bat_result_pending;
} core_machine_xt_keyboard;

type_status core_machine_xt_keyboard_initialize(core_machine_xt_keyboard *keyboard,
    core_machine_xt_ppi_keyboard *ppi, type_unsigned_64 ticks_per_second);
C_VOID core_machine_xt_keyboard_reset(core_machine_xt_keyboard *keyboard);
C_VOID core_machine_xt_keyboard_finalize(core_machine_xt_keyboard *keyboard);
C_VOID core_machine_xt_keyboard_observe_ppi_lines(C_VOID *owner,
    type_bool clock_held, type_bool clear_asserted);
C_VOID core_machine_xt_keyboard_notify_ppi_byte_released(C_VOID *owner);
type_status core_machine_xt_keyboard_receive_native_bytes(core_machine_xt_keyboard *keyboard,
    const type_unsigned_8 *bytes, STD_SIZE_T count);
C_VOID core_machine_xt_keyboard_advance(core_machine_xt_keyboard *keyboard,
    type_unsigned_64 ticks);
type_status core_machine_xt_keyboard_ticks_until_event(const core_machine_xt_keyboard *keyboard,
    type_unsigned_64 *out_ticks);

#endif
