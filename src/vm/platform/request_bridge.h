#ifndef NXVM_PLATFORM_VM_REQUEST_BRIDGE_H
#define NXVM_PLATFORM_VM_REQUEST_BRIDGE_H

#include <stddef.h>
#include <stdint.h>

#include "core/machine/status.h"

#define NXVM_PLATFORM_VM_REQUEST_CAPACITY 32u

typedef enum nxvm_platform_vm_request_kind {
    NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE,
    NXVM_PLATFORM_VM_REQUEST_KEY_PRESS,
    NXVM_PLATFORM_VM_REQUEST_STOP,
    NXVM_PLATFORM_VM_REQUEST_DISPLAY_MODE
} nxvm_platform_vm_request_kind;

typedef struct nxvm_platform_vm_keyboard_state {
    uint32_t asynchronous_keys;
    uint32_t toggle_keys;
} nxvm_platform_vm_keyboard_state;

typedef struct nxvm_platform_vm_key_press {
    uint16_t scan_code;
    uint16_t virtual_key;
} nxvm_platform_vm_key_press;

typedef struct nxvm_platform_vm_request {
    nxvm_platform_vm_request_kind kind;
    union {
        nxvm_platform_vm_keyboard_state keyboard_state;
        nxvm_platform_vm_key_press key_press;
        int window_display;
    } data;
} nxvm_platform_vm_request;

typedef struct nxvm_platform_vm_request_bridge {
    nxvm_platform_vm_request entries[NXVM_PLATFORM_VM_REQUEST_CAPACITY];
    size_t head;
    size_t count;
} nxvm_platform_vm_request_bridge;

void nxvm_platform_vm_request_bridge_initialize(
    nxvm_platform_vm_request_bridge *bridge);
nxvm_core_status nxvm_platform_vm_request_bridge_enqueue(
    nxvm_platform_vm_request_bridge *bridge,
    const nxvm_platform_vm_request *request);
nxvm_core_status nxvm_platform_vm_request_bridge_dequeue(
    nxvm_platform_vm_request_bridge *bridge,
    nxvm_platform_vm_request *out_request);

#endif
