/* Copyright 2012-2026 Neko. */
#ifndef X86_KBC8042_CONTROLLER_H
#define X86_KBC8042_CONTROLLER_H
#include "core/chips/kbc8042/kbc8042_interface.h"
#define KBC_FIFO_CAPACITY 64u
#define KBC_KEYBOARD_SERIAL_CAPACITY 64u
#define KBC_RESPONSE_CAPACITY 4u

#define KBC_COMMAND_TRANSLATION 0x40u
#define KBC_COMMAND_IRQ12 0x02u
#define KBC_COMMAND_DISABLE_KEYBOARD 0x10u
#define KBC_COMMAND_DISABLE_AUX 0x20u
#define KBC_COMMAND_INHIBIT_OVERRIDE 0x08u

#define KBC_STATUS_OBF 0x01 /* output buffer contains a byte */
#define KBC_STATUS_IBF 0x02 /* synchronous command/data processing */
#define KBC_STATUS_SYS 0x04 /* controller self test/system flag */
#define KBC_STATUS_CD  0x08 /* last write selected the command port */
#define KBC_STATUS_INHIBIT 0x10 /* keyboard inhibit switch is released */
#define KBC_STATUS_AUX 0x20 /* current output byte has AUX origin */


typedef enum kbc_pending_write {
    KBC_PENDING_NONE,
    KBC_PENDING_COMMAND_BYTE,
    KBC_PENDING_OUTPUT_PORT,
    KBC_PENDING_AUX_DEVICE,
    KBC_PENDING_AUX_DISCARD
} kbc_pending_write;

typedef struct kbc_data {
    lib_u8 command_byte;
    lib_u8 output_port;
    lib_u8 input_port;
    lib_u8 test_inputs;
    lib_u8 fifo[KBC_FIFO_CAPACITY];
    x86_kbc8042_origin fifo_origin[KBC_FIFO_CAPACITY];
    lib_u8 fifo_head;
    lib_u8 fifo_count;
    lib_u8 keyboard_serial[KBC_KEYBOARD_SERIAL_CAPACITY];
    lib_u8 keyboard_serial_head;
    lib_u8 keyboard_serial_count;
    kbc_pending_write pending_write;
    lib_bool input_buffer_full;
    lib_bool last_write_command;
    lib_bool irq1_asserted;
    lib_bool irq12_asserted;
    lib_bool aux_enabled;
    lib_bool set2_break_pending;
    lib_u8 set2_pause_bytes[8];
    lib_u8 set2_pause_count;
    lib_u8 delayed_response[KBC_RESPONSE_CAPACITY];
    x86_kbc8042_origin delayed_response_origin;
    lib_u8 delayed_response_count;
    lib_u8 delayed_response_index;
    lib_u8 response_status_polls_remaining;
    lib_u64 response_remaining_ticks;
    lib_u64 serial_delivery_remaining_ticks;
    lib_u32 command_response_ticks;
    lib_u32 serial_delivery_ticks;
    lib_u8 command_response_status_polls;
} kbc_data;


struct x86_kbc8042 {
    kbc_data data;
    x86_kbc8042_link link;
    lib_bool aux_present;
    lib_u8 reset_output_port;
};
#endif
