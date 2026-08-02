#include "core/machine/keyboard_state.h"

#include <stddef.h>

static void *coreMachineKeyboardContext;
static const core_machine_keyboard_provider *coreMachineKeyboardProvider;

void core_machine_keyboard_bind(void *context,
    const core_machine_keyboard_provider *provider)
{
    coreMachineKeyboardContext = context;
    coreMachineKeyboardProvider = provider;
}

int core_machine_keyboard_get_modifier(core_machine_keyboard_modifier modifier)
{
    if (coreMachineKeyboardProvider == NULL ||
        coreMachineKeyboardProvider->get_modifier == NULL) return 0;
    return coreMachineKeyboardProvider->get_modifier(coreMachineKeyboardContext,
        modifier);
}

void core_machine_keyboard_apply_host_state(uint32_t asynchronous_keys,
    uint32_t toggle_keys)
{
    if (coreMachineKeyboardProvider != NULL &&
        coreMachineKeyboardProvider->apply_host_state != NULL) {
        coreMachineKeyboardProvider->apply_host_state(coreMachineKeyboardContext,
            asynchronous_keys, toggle_keys);
    }
}

void core_machine_keyboard_receive_key_press(uint16_t code)
{
    if (coreMachineKeyboardProvider != NULL &&
        coreMachineKeyboardProvider->receive_key_press != NULL) {
        coreMachineKeyboardProvider->receive_key_press(coreMachineKeyboardContext,
            code);
    }
}
