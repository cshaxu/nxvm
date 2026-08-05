#ifndef VM_PLATFORM_REQUEST_BRIDGE_H
#define VM_PLATFORM_REQUEST_BRIDGE_H


#include "type.h"

#define VM_PLATFORM_REQUEST_CAPACITY 32u

typedef enum vm_platform_request_kind {
    VM_PLATFORM_REQUEST_KEY_EVENT,
    VM_PLATFORM_REQUEST_MOUSE_EVENT,
    VM_PLATFORM_REQUEST_STOP,
    VM_PLATFORM_REQUEST_DISPLAY_MODE
} vm_platform_request_kind;

typedef struct vm_platform_key_event {
    uint16_t scan_code;
    uint16_t virtual_key;
    C_INT pressed;
} vm_platform_key_event;

typedef struct vm_platform_mouse_event {
    int16_t delta_x;
    int16_t delta_y;
    uint8_t buttons;
} vm_platform_mouse_event;

typedef struct vm_platform_request {
    vm_platform_request_kind kind;
    union {
        vm_platform_key_event key_event;
        vm_platform_mouse_event mouse_event;
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
