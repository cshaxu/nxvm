#ifndef VM_PLATFORM_INPUT_H
#define VM_PLATFORM_INPUT_H

#include "type.h"


typedef struct vm_platform_keyboard_sink {
    C_VOID (*receive_key_press)(C_VOID *context, uint16_t scan_code,
        uint16_t virtual_key);
} vm_platform_keyboard_sink;

typedef C_INT (*vm_platform_keyboard_state_sink)(
    C_VOID *context, uint32_t asynchronous_keys, uint32_t toggle_keys);

typedef struct vm_platform_keyboard_transport {
    const vm_platform_keyboard_sink *sink;
    C_VOID *context;
} vm_platform_keyboard_transport;

C_VOID vm_platform_keyboard_transport_initialize(
    vm_platform_keyboard_transport *transport,
    const vm_platform_keyboard_sink *sink, C_VOID *context);
C_VOID vm_platform_keyboard_receive_key_press_for(
    const vm_platform_keyboard_transport *transport, uint16_t scan_code,
    uint16_t virtual_key);
C_VOID vm_platform_input_flush_console_input(C_VOID);

#endif
