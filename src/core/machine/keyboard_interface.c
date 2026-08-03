#include "type.h"

#include "core/machine/keyboard_interface.h"


#include <stddef.h>

C_VOID core_machine_keyboard_provider_slot_initialize(
    core_machine_keyboard_provider_slot *slot)
{
    if (slot != STD_NULL) {
        slot->context = STD_NULL;
        slot->provider = STD_NULL;
        slot->frozen = 0;
    }
}

C_VOID core_machine_keyboard_provider_slot_bind(
    core_machine_keyboard_provider_slot *slot, C_VOID *context,
    const core_machine_keyboard_provider *provider)
{
    if (slot == STD_NULL || slot->frozen) return;
    slot->context = context;
    slot->provider = provider;
}

C_VOID core_machine_keyboard_provider_slot_freeze(
    core_machine_keyboard_provider_slot *slot)
{
    if (slot != STD_NULL) slot->frozen = 1;
}

C_VOID core_machine_keyboard_provider_slot_finalize(
    core_machine_keyboard_provider_slot *slot)
{
    core_machine_keyboard_provider_slot_initialize(slot);
}

C_INT core_machine_keyboard_get_modifier_from(
    const core_machine_keyboard_provider_slot *slot,
    core_machine_keyboard_modifier modifier)
{
    if (slot == STD_NULL || slot->provider == STD_NULL ||
        slot->provider->get_modifier == STD_NULL) return 0;
    return slot->provider->get_modifier(slot->context, modifier);
}

C_VOID core_machine_keyboard_apply_host_state_to(
    const core_machine_keyboard_provider_slot *slot,
    uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    if (slot != STD_NULL && slot->provider != STD_NULL &&
        slot->provider->apply_host_state != STD_NULL) {
        slot->provider->apply_host_state(slot->context, asynchronous_keys,
            toggle_keys);
    }
}

C_VOID core_machine_keyboard_receive_key_press_to(
    const core_machine_keyboard_provider_slot *slot, uint16_t code)
{
    if (slot != STD_NULL && slot->provider != STD_NULL &&
        slot->provider->receive_key_press != STD_NULL) {
        slot->provider->receive_key_press(slot->context, code);
    }
}
