/* Copyright 2012-2014 Neko. */

#ifndef X86_NXVM_BOARD_KBC_H
#define X86_NXVM_BOARD_KBC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "x86-nxvm/board-at/kbc_interface.h"
#include "x86-nxvm/chips/ps2mouse/ps2mouse_interface.h"
#include "x86-nxvm/chips/keyboard/keyboard_interface.h"
#include "x86-nxvm/chips/kbc8042/kbc8042_interface.h"

#include "x86-core/machine/port_interface.h"

#define X86_NXVM_BOARD_DEVICE_KBC "Intel 8042"

#define X86_NXVM_BOARD_KBC_COMMAND_TRANSLATION 0x40u
#define X86_NXVM_BOARD_KBC_COMMAND_IRQ12 0x02u
#define X86_NXVM_BOARD_KBC_COMMAND_DISABLE_KEYBOARD 0x10u
#define X86_NXVM_BOARD_KBC_COMMAND_DISABLE_AUX 0x20u
#define X86_NXVM_BOARD_KBC_COMMAND_INHIBIT_OVERRIDE 0x08u

#define VKBC_STATUS_OBF 0x01 /* output buffer contains a byte */
#define VKBC_STATUS_IBF 0x02 /* synchronous command/data processing */
#define VKBC_STATUS_SYS 0x04 /* controller self test/system flag */
#define VKBC_STATUS_CD  0x08 /* last write selected the command port */
#define VKBC_STATUS_INHIBIT 0x10 /* keyboard inhibit switch is released */
#define VKBC_STATUS_AUX 0x20 /* current output byte has AUX origin */

typedef struct t_kbc_connect {
    x86_ps2_mouse *aux_device;
    x86_keyboard *keyboard;
    x86_nxvm_board_kbc_irq_line irq;
    void *irq_context;
    void (*set_a20)(void *context, lib_bool enabled);
    void *a20_context;
    void (*request_reset)(void *context);
    void *reset_context;
    x86_nxvm_board_kbc_command_extension command_extension;
    void *command_extension_context;
} t_kbc_connect;

struct t_kbc {
    x86_kbc8042 *chip;
    t_kbc_connect connect;
};

lib_status x86_nxvm_board_kbc_initialize(t_kbc *controller, x86_core_machine *machine);
void x86_nxvm_board_kbc_set_input_port(t_kbc *controller, lib_u8 value);
void x86_nxvm_board_kbc_set_reset_output_port(t_kbc *controller,
    lib_u8 value);
void x86_nxvm_board_kbc_set_reset_system_flag(t_kbc *controller,
    lib_bool enabled);
void x86_nxvm_board_kbc_set_test_inputs(t_kbc *controller, lib_u8 value);
void x86_nxvm_board_kbc_reset(t_kbc *controller);
void x86_nxvm_board_kbc_advance(t_kbc *controller, lib_u64 elapsed_ticks);
lib_status x86_nxvm_board_kbc_ticks_until_event(const t_kbc *controller,
    lib_u64 *out_ticks);
void x86_nxvm_board_kbc_set_typematic_timing(t_kbc *controller,
    lib_u32 initial_ticks, lib_u32 repeat_ticks);
void x86_nxvm_board_kbc_set_command_response_timing(t_kbc *controller,
    lib_u32 response_ticks);
void x86_nxvm_board_kbc_set_command_response_status_polls(t_kbc *controller,
    lib_u8 status_polls);
void x86_nxvm_board_kbc_set_serial_delivery_timing(t_kbc *controller,
    lib_u32 delivery_ticks);
void x86_nxvm_board_kbc_finalize(t_kbc *controller);
/* Submit a byte emitted by the attached physical keyboard.  This is the
 * production keyboard-to-controller boundary; it is not a guest-FIFO or test
 * injection path. */
lib_status x86_nxvm_board_kbc_submit_native_byte(t_kbc *controller,
    lib_u8 native_byte);
lib_status x86_nxvm_board_kbc_submit_native_bytes(t_kbc *controller,
    const lib_u8 *native_bytes, lib_size count);
lib_status x86_nxvm_board_kbc_submit_aux_report(t_kbc *controller,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
