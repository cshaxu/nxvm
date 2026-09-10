#ifndef VM_SESSION_REQUEST_BRIDGE_H
#define VM_SESSION_REQUEST_BRIDGE_H


#include "type.h"

#define VM_SESSION_REQUEST_CAPACITY 32u

typedef enum vm_session_request_kind {
    VM_SESSION_REQUEST_KEY_EVENT,
    VM_SESSION_REQUEST_MOUSE_EVENT
} vm_session_request_kind;

typedef struct vm_session_key_event {
    type_unsigned_16 scan_code;
    type_unsigned_16 virtual_key;
    C_INT pressed;
} vm_session_key_event;

typedef struct vm_session_mouse_event {
    type_signed_16 delta_x;
    type_signed_16 delta_y;
    type_unsigned_8 buttons;
} vm_session_mouse_event;

typedef struct vm_session_request {
    vm_session_request_kind kind;
    union {
        vm_session_key_event key_event;
        vm_session_mouse_event mouse_event;
    } data;
} vm_session_request;

typedef struct vm_session_request_bridge {
    vm_session_request entries[VM_SESSION_REQUEST_CAPACITY];
    STD_SIZE_T head;
    STD_SIZE_T count;
} vm_session_request_bridge;

C_VOID vm_session_request_bridge_initialize(
    vm_session_request_bridge *bridge);
type_status vm_session_request_bridge_enqueue(
    vm_session_request_bridge *bridge,
    const vm_session_request *request);
type_status vm_session_request_bridge_dequeue(
    vm_session_request_bridge *bridge,
    vm_session_request *out_request);

#endif
