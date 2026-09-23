/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_KBC_H
#define CORE_MACHINE_KBC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "type.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/port.h"

#define CORE_MACHINE_DEVICE_KBC "Intel 8042"

typedef struct t_pic t_pic;
typedef struct t_ram t_ram;
typedef struct core_machine_cpu_execution_context
    core_machine_cpu_execution_context;

#define CORE_MACHINE_KBC_FIFO_CAPACITY 64u
#define CORE_MACHINE_KBC_KEYBOARD_SERIAL_CAPACITY 64u
#define CORE_MACHINE_KBC_RESPONSE_CAPACITY 4u

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

typedef enum core_machine_kbc_output_origin {
    CORE_MACHINE_KBC_OUTPUT_CONTROLLER,
    CORE_MACHINE_KBC_OUTPUT_KEYBOARD,
    CORE_MACHINE_KBC_OUTPUT_AUX
} core_machine_kbc_output_origin;

typedef enum core_machine_kbc_pending_write {
    CORE_MACHINE_KBC_PENDING_NONE,
    CORE_MACHINE_KBC_PENDING_COMMAND_BYTE,
    CORE_MACHINE_KBC_PENDING_OUTPUT_PORT,
    CORE_MACHINE_KBC_PENDING_KEYBOARD_LEDS,
    CORE_MACHINE_KBC_PENDING_KEYBOARD_TYPEMATIC,
    CORE_MACHINE_KBC_PENDING_KEYBOARD_SCAN_SET,
    CORE_MACHINE_KBC_PENDING_AUX_DEVICE,
    CORE_MACHINE_KBC_PENDING_AUX_DISCARD
} core_machine_kbc_pending_write;

typedef enum core_machine_kbc_aux_pending_parameter {
    CORE_MACHINE_KBC_AUX_PENDING_NONE,
    CORE_MACHINE_KBC_AUX_PENDING_SAMPLE_RATE,
    CORE_MACHINE_KBC_AUX_PENDING_RESOLUTION
} core_machine_kbc_aux_pending_parameter;

typedef struct t_kbc_data {
    lib_u8 command_byte;
    lib_u8 output_port;
    lib_u8 input_port;
    lib_u8 test_inputs;
    lib_u8 fifo[CORE_MACHINE_KBC_FIFO_CAPACITY];
    core_machine_kbc_output_origin fifo_origin[CORE_MACHINE_KBC_FIFO_CAPACITY];
    lib_u8 fifo_head;
    lib_u8 fifo_count;
    lib_u8 keyboard_serial[CORE_MACHINE_KBC_KEYBOARD_SERIAL_CAPACITY];
    lib_u8 keyboard_serial_head;
    lib_u8 keyboard_serial_count;
    core_machine_kbc_pending_write pending_write;
    core_machine_kbc_aux_pending_parameter aux_pending_parameter;
    type_bool keyboard_enabled;
    type_bool scanning_enabled;
    type_bool input_buffer_full;
    type_bool last_write_command;
    type_bool irq1_asserted;
    type_bool irq12_asserted;
    type_bool aux_enabled;
    type_bool aux_reporting_enabled;
    type_bool aux_scaling_2_to_1;
    lib_u8 aux_button_state;
    lib_u8 aux_resolution;
    lib_u8 aux_sample_rate;
    lib_u8 scan_set;
    lib_u8 led_state;
    lib_u8 typematic;
    type_bool set2_break_pending;
    type_bool set2_typematic_break_pending;
    type_bool set2_extended_pending;
    lib_u8 set2_pause_bytes[8];
    lib_u8 set2_pause_count;
    lib_u8 last_keyboard_output_byte;
    lib_u8 previous_keyboard_output_byte;
    type_bool keyboard_has_output;
    type_bool keyboard_bat_pending;
    /* The first clock/data release after controller reset starts the attached
     * keyboard.  ADh/AEh subsequently gate the interface; they are not
     * additional keyboard power cycles. */
    type_bool keyboard_startup_released;
    lib_u8 typematic_scan_code;
    lib_u8 delayed_response[CORE_MACHINE_KBC_RESPONSE_CAPACITY];
    core_machine_kbc_output_origin delayed_response_origin;
    lib_u8 delayed_response_count;
    lib_u8 delayed_response_index;
    lib_u8 response_status_polls_remaining;
    lib_u64 typematic_remaining_ticks;
    lib_u64 response_remaining_ticks;
    lib_u64 serial_delivery_remaining_ticks;
    lib_u32 typematic_nominal_initial_ticks;
    lib_u32 typematic_nominal_repeat_ticks;
    lib_u32 typematic_initial_ticks;
    lib_u32 typematic_repeat_ticks;
    lib_u32 command_response_ticks;
    lib_u32 serial_delivery_ticks;
    lib_u8 command_response_status_polls;
    type_bool typematic_active;
} t_kbc_data;

typedef struct t_kbc_connect {
    core_machine_pic_irq_source irq1_source;
    core_machine_pic_irq_source irq12_source;
    type_bool aux_present;
    t_ram *memory;
    core_machine_cpu_execution_context *execution;
    lib_u8 reset_output_port;
} t_kbc_connect;

typedef struct t_kbc {
    t_kbc_data data;
    t_kbc_connect connect;
} t_kbc;

C_VOID core_machine_kbc_initialize(t_kbc *controller, t_port *port);
C_VOID core_machine_kbc_bind_core_services(t_kbc *controller, t_pic *pic_master,
    t_pic *pic_slave, t_ram *memory,
    core_machine_cpu_execution_context *execution, type_bool aux_present);
C_VOID core_machine_kbc_set_input_port(t_kbc *controller, lib_u8 value);
C_VOID core_machine_kbc_set_reset_output_port(t_kbc *controller,
    lib_u8 value);
C_VOID core_machine_kbc_set_test_inputs(t_kbc *controller, lib_u8 value);
C_VOID core_machine_kbc_reset(t_kbc *controller);
C_VOID core_machine_kbc_advance(t_kbc *controller, lib_u64 elapsed_ticks);
type_status core_machine_kbc_ticks_until_event(const t_kbc *controller,
    lib_u64 *out_ticks);
C_VOID core_machine_kbc_set_typematic_timing(t_kbc *controller,
    lib_u32 initial_ticks, lib_u32 repeat_ticks);
C_VOID core_machine_kbc_set_command_response_timing(t_kbc *controller,
    lib_u32 response_ticks);
C_VOID core_machine_kbc_set_command_response_status_polls(t_kbc *controller,
    lib_u8 status_polls);
C_VOID core_machine_kbc_set_serial_delivery_timing(t_kbc *controller,
    lib_u32 delivery_ticks);
C_VOID core_machine_kbc_finalize(t_kbc *controller);
/* Submit a byte emitted by the attached physical keyboard.  This is the
 * production keyboard-to-controller boundary; it is not a guest-FIFO or test
 * injection path. */
type_status core_machine_kbc_submit_native_byte(t_kbc *controller,
    lib_u8 native_byte);
type_status core_machine_kbc_submit_native_bytes(t_kbc *controller,
    const lib_u8 *native_bytes, lib_size count);
type_status core_machine_kbc_submit_aux_report(t_kbc *controller,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
