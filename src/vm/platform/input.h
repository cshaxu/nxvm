#ifndef NTVDM64_VM_PLATFORM_INPUT_H
#define NTVDM64_VM_PLATFORM_INPUT_H

#include <stdint.h>

typedef enum vm_platform_keyboard_modifier {
    VM_PLATFORM_KEYBOARD_MODIFIER_ALT,
    VM_PLATFORM_KEYBOARD_MODIFIER_CONTROL,
    VM_PLATFORM_KEYBOARD_MODIFIER_SHIFT,
    VM_PLATFORM_KEYBOARD_MODIFIER_CAPS_LOCK,
    VM_PLATFORM_KEYBOARD_MODIFIER_NUM_LOCK
} vm_platform_keyboard_modifier;

typedef struct vm_platform_keyboard_sink {
    int (*get_modifier)(void *context, vm_platform_keyboard_modifier modifier);
    void (*apply_host_state)(void *context, uint32_t asynchronous_keys,
                             uint32_t toggle_keys);
    void (*receive_key_press)(void *context, uint16_t code);
    void (*request_stop)(void *context);
} vm_platform_keyboard_sink;

typedef int (*vm_platform_keyboard_state_sink)(
    void *context, uint32_t asynchronous_keys, uint32_t toggle_keys);

typedef struct vm_platform_keyboard_transport {
    const vm_platform_keyboard_sink *sink;
    void *context;
} vm_platform_keyboard_transport;

void vm_platform_keyboard_transport_initialize(
    vm_platform_keyboard_transport *transport,
    const vm_platform_keyboard_sink *sink, void *context);
int vm_platform_keyboard_get_modifier_for(
    const vm_platform_keyboard_transport *transport,
    vm_platform_keyboard_modifier modifier);
void vm_platform_keyboard_apply_host_state_for(
    const vm_platform_keyboard_transport *transport,
    uint32_t asynchronous_keys, uint32_t toggle_keys);
void vm_platform_keyboard_receive_key_press_for(
    const vm_platform_keyboard_transport *transport, uint16_t code);
void vm_platform_keyboard_request_stop_for(
    const vm_platform_keyboard_transport *transport);

void vm_platform_keyboard_bind(const vm_platform_keyboard_sink *sink,
                               void *context);
int vm_platform_keyboard_get_modifier(vm_platform_keyboard_modifier modifier);
void vm_platform_keyboard_apply_host_state(uint32_t asynchronous_keys,
                                           uint32_t toggle_keys);
void vm_platform_keyboard_receive_key_press(uint16_t code);
void vm_platform_keyboard_request_stop(void);

#endif
