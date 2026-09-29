/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_KBC_H
#define CORE_MACHINE_KBC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"
#include "x86/devices/ps2mouse/ps2mouse_interface.h"
#include "x86/devices/keyboard/keyboard_interface.h"
#include "x86/devices/kbc8042/kbc8042_interface.h"

#include "app-nxvm/devices/pic_bus.h"
#include "app-nxvm/devices/port.h"

#define CORE_MACHINE_DEVICE_KBC "Intel 8042"

typedef struct core_machine_pic_bus core_machine_pic_bus;
typedef struct t_ram t_ram;

#define CORE_MACHINE_KBC_COMMAND_TRANSLATION 0x40u
#define CORE_MACHINE_KBC_COMMAND_IRQ12 0x02u
#define CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD 0x10u
#define CORE_MACHINE_KBC_COMMAND_DISABLE_AUX 0x20u
#define CORE_MACHINE_KBC_COMMAND_INHIBIT_OVERRIDE 0x08u

#define VKBC_STATUS_OBF 0x01 /* output buffer contains a byte */
#define VKBC_STATUS_IBF 0x02 /* synchronous command/data processing */
#define VKBC_STATUS_SYS 0x04 /* controller self test/system flag */
#define VKBC_STATUS_CD  0x08 /* last write selected the command port */
#define VKBC_STATUS_INHIBIT 0x10 /* keyboard inhibit switch is released */
#define VKBC_STATUS_AUX 0x20 /* current output byte has AUX origin */

typedef struct t_kbc_connect {
    x86_ps2_mouse *aux_device;
    x86_keyboard *keyboard;
    core_machine_pic_irq_source irq1_source;
    core_machine_pic_irq_source irq12_source;
    t_ram *memory;
    void (*request_reset)(void *context);
    void *reset_context;
} t_kbc_connect;

typedef struct t_kbc {
    x86_kbc8042 *chip;
    t_kbc_connect connect;
} t_kbc;

lib_status core_machine_kbc_initialize(t_kbc *controller, t_port *port);
void core_machine_kbc_bind_core_services(t_kbc *controller, core_machine_pic_bus *pic_master,
    core_machine_pic_bus *pic_slave, t_ram *memory,
    void (*request_reset)(void *context), void *reset_context,
    lib_u8 aux_present);
void core_machine_kbc_set_input_port(t_kbc *controller, lib_u8 value);
void core_machine_kbc_set_reset_output_port(t_kbc *controller,
    lib_u8 value);
void core_machine_kbc_set_test_inputs(t_kbc *controller, lib_u8 value);
void core_machine_kbc_reset(t_kbc *controller);
void core_machine_kbc_advance(t_kbc *controller, lib_u64 elapsed_ticks);
lib_status core_machine_kbc_ticks_until_event(const t_kbc *controller,
    lib_u64 *out_ticks);
void core_machine_kbc_set_typematic_timing(t_kbc *controller,
    lib_u32 initial_ticks, lib_u32 repeat_ticks);
void core_machine_kbc_set_command_response_timing(t_kbc *controller,
    lib_u32 response_ticks);
void core_machine_kbc_set_command_response_status_polls(t_kbc *controller,
    lib_u8 status_polls);
void core_machine_kbc_set_serial_delivery_timing(t_kbc *controller,
    lib_u32 delivery_ticks);
void core_machine_kbc_finalize(t_kbc *controller);
/* Submit a byte emitted by the attached physical keyboard.  This is the
 * production keyboard-to-controller boundary; it is not a guest-FIFO or test
 * injection path. */
lib_status core_machine_kbc_submit_native_byte(t_kbc *controller,
    lib_u8 native_byte);
lib_status core_machine_kbc_submit_native_bytes(t_kbc *controller,
    const lib_u8 *native_bytes, lib_size count);
lib_status core_machine_kbc_submit_aux_report(t_kbc *controller,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
