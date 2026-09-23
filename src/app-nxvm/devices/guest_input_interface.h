#ifndef CORE_MACHINE_GUEST_INPUT_INTERFACE_H
#define CORE_MACHINE_GUEST_INPUT_INTERFACE_H

#include "type.h"

typedef enum core_machine_guest_input_kind {
    CORE_MACHINE_GUEST_INPUT_KEY,
    CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE
} core_machine_guest_input_kind;

typedef struct core_machine_guest_input_event {
    core_machine_guest_input_kind kind;
    union {
        struct {
            type_unsigned_16 scan_code;
            type_unsigned_16 virtual_key;
            C_INT pressed;
        } key;
        struct {
            type_signed_16 delta_x;
            type_signed_16 delta_y;
            type_unsigned_8 buttons;
        } relative_mouse;
    } data;
} core_machine_guest_input_event;

#endif
