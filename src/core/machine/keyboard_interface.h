#ifndef CORE_MACHINE_KEYBOARD_INTERFACE_H
#define CORE_MACHINE_KEYBOARD_INTERFACE_H

#include <stdint.h>

typedef enum core_machine_keyboard_modifier {
    CORE_MACHINE_KEYBOARD_MODIFIER_ALT,
    CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL,
    CORE_MACHINE_KEYBOARD_MODIFIER_SHIFT,
    CORE_MACHINE_KEYBOARD_MODIFIER_CAPS_LOCK,
    CORE_MACHINE_KEYBOARD_MODIFIER_NUM_LOCK
} core_machine_keyboard_modifier;

typedef struct core_machine_keyboard_provider {
    int (*get_modifier)(void *context, core_machine_keyboard_modifier modifier);
    void (*apply_host_state)(void *context, uint32_t asynchronous_keys,
        uint32_t toggle_keys);
    void (*receive_key_press)(void *context, uint16_t code);
} core_machine_keyboard_provider;

typedef struct core_machine_keyboard_provider_slot {
    void *context;
    const core_machine_keyboard_provider *provider;
    int frozen;
} core_machine_keyboard_provider_slot;

#define NXVM_KEYBOARD_ASYNC_RIGHT_SHIFT (UINT32_C(1) << 0)
#define NXVM_KEYBOARD_ASYNC_LEFT_SHIFT  (UINT32_C(1) << 1)
#define NXVM_KEYBOARD_ASYNC_CONTROL     (UINT32_C(1) << 2)
#define NXVM_KEYBOARD_ASYNC_ALT         (UINT32_C(1) << 3)
#define NXVM_KEYBOARD_ASYNC_SCROLL_LOCK (UINT32_C(1) << 4)
#define NXVM_KEYBOARD_ASYNC_NUM_LOCK    (UINT32_C(1) << 5)
#define NXVM_KEYBOARD_ASYNC_CAPS_LOCK   (UINT32_C(1) << 6)
#define NXVM_KEYBOARD_ASYNC_INSERT      (UINT32_C(1) << 7)

#define NXVM_KEYBOARD_TOGGLE_SCROLL_LOCK (UINT32_C(1) << 0)
#define NXVM_KEYBOARD_TOGGLE_NUM_LOCK    (UINT32_C(1) << 1)
#define NXVM_KEYBOARD_TOGGLE_CAPS_LOCK   (UINT32_C(1) << 2)
#define NXVM_KEYBOARD_TOGGLE_INSERT      (UINT32_C(1) << 3)
#define NXVM_KEYBOARD_TOGGLE_PAUSE       (UINT32_C(1) << 4)

void core_machine_keyboard_provider_slot_initialize(
    core_machine_keyboard_provider_slot *slot);
void core_machine_keyboard_provider_slot_bind(
    core_machine_keyboard_provider_slot *slot, void *context,
    const core_machine_keyboard_provider *provider);
void core_machine_keyboard_provider_slot_freeze(
    core_machine_keyboard_provider_slot *slot);
void core_machine_keyboard_provider_slot_finalize(
    core_machine_keyboard_provider_slot *slot);
int core_machine_keyboard_get_modifier_from(
    const core_machine_keyboard_provider_slot *slot,
    core_machine_keyboard_modifier modifier);
void core_machine_keyboard_apply_host_state_to(
    const core_machine_keyboard_provider_slot *slot,
    uint32_t asynchronous_keys, uint32_t toggle_keys);
void core_machine_keyboard_receive_key_press_to(
    const core_machine_keyboard_provider_slot *slot, uint16_t code);

#endif
