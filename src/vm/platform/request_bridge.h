#ifndef VM_PLATFORM_REQUEST_BRIDGE_H
#define VM_PLATFORM_REQUEST_BRIDGE_H


#include "core/machine/keyboard_interface.h"
#include "type.h"

#define VM_PLATFORM_REQUEST_CAPACITY 32u

typedef enum vm_platform_request_kind {
    VM_PLATFORM_REQUEST_KEYBOARD_STATE,
    VM_PLATFORM_REQUEST_KEY_PRESS,
    VM_PLATFORM_REQUEST_STOP,
    VM_PLATFORM_REQUEST_DISPLAY_MODE
} vm_platform_request_kind;

typedef struct vm_platform_keyboard_state {
    uint32_t asynchronous_keys;
    uint32_t toggle_keys;
} vm_platform_keyboard_state;

typedef struct vm_platform_key_press {
    uint16_t scan_code;
    uint16_t virtual_key;
} vm_platform_key_press;

typedef struct vm_platform_request {
    vm_platform_request_kind kind;
    union {
        vm_platform_keyboard_state keyboard_state;
        vm_platform_key_press key_press;
        C_INT window_display;
    } data;
} vm_platform_request;

typedef struct vm_platform_request_bridge {
    vm_platform_request entries[VM_PLATFORM_REQUEST_CAPACITY];
    STD_SIZE_T head;
    STD_SIZE_T count;
} vm_platform_request_bridge;

C_VOID vm_platform_request_bridge_initialize(
    vm_platform_request_bridge *bridge);
type_status vm_platform_request_bridge_enqueue(
    vm_platform_request_bridge *bridge,
    const vm_platform_request *request);
type_status vm_platform_request_bridge_dequeue(
    vm_platform_request_bridge *bridge,
    vm_platform_request *out_request);

#endif
