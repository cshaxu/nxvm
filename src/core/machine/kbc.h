/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_KBC_H
#define CORE_MACHINE_KBC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"

#define CORE_MACHINE_DEVICE_KBC "Intel 8042"

typedef struct t_pic t_pic;
typedef struct t_ram t_ram;
typedef struct core_machine_cpu_execution_context
    core_machine_cpu_execution_context;

#define CORE_MACHINE_KBC_FIFO_CAPACITY 64u
#define CORE_MACHINE_KBC_RESPONSE_CAPACITY 3u
#define CORE_MACHINE_KBC_SCAN_SET_1 1u

#define CORE_MACHINE_KBC_COMMAND_TRANSLATION 0x40u
#define CORE_MACHINE_KBC_COMMAND_IRQ12 0x02u
#define CORE_MACHINE_KBC_COMMAND_DISABLE_AUX 0x20u

#define VKBC_STATUS_OBF 0x01 /* output buffer contains a byte */
#define VKBC_STATUS_IBF 0x02 /* synchronous command/data processing */
#define VKBC_STATUS_SYS 0x04 /* controller self test/system flag */
#define VKBC_STATUS_CD  0x08 /* last write selected the command port */
#define VKBC_STATUS_KE  0x10 /* keyboard interface enabled */
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
    CORE_MACHINE_KBC_PENDING_AUX_DEVICE
} core_machine_kbc_pending_write;

typedef struct t_kbc_data {
    type_unsigned_8 command_byte;
    type_unsigned_8 output_port;
    type_unsigned_8 fifo[CORE_MACHINE_KBC_FIFO_CAPACITY];
    core_machine_kbc_output_origin fifo_origin[CORE_MACHINE_KBC_FIFO_CAPACITY];
    type_unsigned_8 fifo_head;
    type_unsigned_8 fifo_count;
    core_machine_kbc_pending_write pending_write;
    type_bool keyboard_enabled;
    type_bool scanning_enabled;
    type_bool system_flag;
    type_bool input_buffer_full;
    type_bool last_write_command;
    type_bool irq1_asserted;
    type_bool irq12_asserted;
    type_bool aux_enabled;
    type_bool aux_reporting_enabled;
    type_unsigned_8 aux_button_state;
    type_unsigned_8 scan_set;
    type_unsigned_8 led_state;
    type_unsigned_8 typematic;
    type_unsigned_8 last_output_byte;
    type_unsigned_8 typematic_scan_code;
    type_unsigned_8 delayed_response[CORE_MACHINE_KBC_RESPONSE_CAPACITY];
    core_machine_kbc_output_origin delayed_response_origin;
    type_unsigned_8 delayed_response_count;
    type_unsigned_8 delayed_response_index;
    uint64_t typematic_remaining_ticks;
    uint64_t response_remaining_ticks;
    uint32_t typematic_initial_ticks;
    uint32_t typematic_repeat_ticks;
    uint32_t command_response_ticks;
    type_bool typematic_active;
} t_kbc_data;

typedef struct t_kbc_connect {
    core_machine_pic_irq_source irq1_source;
    core_machine_pic_irq_source irq12_source;
    t_ram *memory;
    core_machine_cpu_execution_context *execution;
} t_kbc_connect;

typedef struct t_kbc {
    t_kbc_data data;
    t_kbc_connect connect;
} t_kbc;

C_VOID core_machine_kbc_register_ports(t_kbc *controller, t_port *port);
C_VOID core_machine_kbc_initialize(t_kbc *controller, t_port *port);
C_VOID core_machine_kbc_bind_core_services(t_kbc *controller, t_pic *pic_master,
    t_pic *pic_slave, t_ram *memory,
    core_machine_cpu_execution_context *execution);
C_VOID core_machine_kbc_reset(t_kbc *controller);
C_VOID core_machine_kbc_refresh(t_kbc *controller);
C_VOID core_machine_kbc_advance(t_kbc *controller, uint64_t elapsed_ticks);
C_VOID core_machine_kbc_set_typematic_timing(t_kbc *controller,
    uint32_t initial_ticks, uint32_t repeat_ticks);
C_VOID core_machine_kbc_set_command_response_timing(t_kbc *controller,
    uint32_t response_ticks);
C_VOID core_machine_kbc_finalize(t_kbc *controller);
type_status core_machine_kbc_submit_scan_code(t_kbc *controller, uint8_t scan_code);
type_status core_machine_kbc_submit_scan_codes(t_kbc *controller,
    const uint8_t *scan_codes, STD_SIZE_T count);
type_status core_machine_kbc_submit_aux_report(t_kbc *controller,
    int16_t delta_x, int16_t delta_y, uint8_t buttons);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
