#ifndef VM_PLATFORM_INPUT_H
#define VM_PLATFORM_INPUT_H

#include "type.h"


typedef struct vm_platform_keyboard_sink {
    C_VOID (*receive_key_event)(C_VOID *context, uint16_t scan_code,
        uint16_t virtual_key, C_INT pressed);
} vm_platform_keyboard_sink;

typedef struct vm_platform_keyboard_transport {
    const vm_platform_keyboard_sink *sink;
    C_VOID *context;
} vm_platform_keyboard_transport;

typedef struct vm_platform_mouse_sink {
    C_VOID (*receive_relative_event)(C_VOID *context, int16_t delta_x,
        int16_t delta_y, uint8_t buttons);
} vm_platform_mouse_sink;

typedef struct vm_platform_mouse_transport {
    const vm_platform_mouse_sink *sink;
    C_VOID *context;
} vm_platform_mouse_transport;

C_VOID vm_platform_keyboard_transport_initialize(
    vm_platform_keyboard_transport *transport,
    const vm_platform_keyboard_sink *sink, C_VOID *context);
C_VOID vm_platform_keyboard_receive_key_event_for(
    const vm_platform_keyboard_transport *transport, uint16_t scan_code,
    uint16_t virtual_key, C_INT pressed);
C_VOID vm_platform_mouse_transport_initialize(
    vm_platform_mouse_transport *transport, const vm_platform_mouse_sink *sink,
    C_VOID *context);
C_VOID vm_platform_mouse_receive_relative_event_for(
    const vm_platform_mouse_transport *transport, int16_t delta_x,
    int16_t delta_y, uint8_t buttons);
C_VOID vm_platform_input_flush_console_input(C_VOID);

#endif
