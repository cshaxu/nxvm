#ifndef CORE_MACHINE_GUEST_INPUT_INTERFACE_H
#define CORE_MACHINE_GUEST_INPUT_INTERFACE_H
#include "lib/types/types_interface.h"

#include "type.h"

typedef enum core_machine_guest_input_kind {
    CORE_MACHINE_GUEST_INPUT_KEY,
    CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE
} core_machine_guest_input_kind;

typedef struct core_machine_guest_input_event {
    core_machine_guest_input_kind kind;
    union {
        struct {
            lib_u16 scan_code;
            lib_u16 virtual_key;
            C_INT pressed;
        } key;
        struct {
            lib_i16 delta_x;
            lib_i16 delta_y;
            lib_u8 buttons;
        } relative_mouse;
    } data;
} core_machine_guest_input_event;

#endif
