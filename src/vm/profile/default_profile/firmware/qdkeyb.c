/* Copyright 2012-2014 Neko. */

#include "type.h"

#include "core/machine/memory.h"

#include "vm/profile/default_profile/firmware/context.h"
#include "vm/profile/default_profile/firmware/qdkeyb.h"

static type_unsigned_8 keyboard_read_flag0(
    vm_profile_default_context *profile)
{
    type_unsigned_8 value = 0u;

    (C_VOID)core_machine_memory_read_real_from(
        vm_profile_default_context_memory(profile), TYPE_ZERO_16,
        QDKEYB_VBIOS_ADDR_KEYB_FLAG0, &value, sizeof(value));
    return value;
}

static C_VOID keyboard_set_bda_flag(vm_profile_default_context *profile,
    type_unsigned_16 offset, type_unsigned_8 mask, C_INT enabled)
{
    type_unsigned_8 value = 0u;
    t_ram *memory = vm_profile_default_context_memory(profile);

    if (memory == STD_NULL) return;
    (C_VOID)core_machine_memory_read_real_from(memory, TYPE_ZERO_16, offset,
        &value, sizeof(value));
    if (enabled) TYPE_SET_BIT(value, mask); else TYPE_CLEAR_BIT(value, mask);
    (C_VOID)core_machine_memory_write_real_to(memory, TYPE_ZERO_16, offset,
        &value, sizeof(value));
}

static C_INT keyboard_get_modifier(C_VOID *context,
    core_machine_keyboard_modifier modifier)
{
    type_unsigned_8 flags = keyboard_read_flag0(context);

    switch (modifier) {
    case CORE_MACHINE_KEYBOARD_MODIFIER_ALT:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_D_ALT);
    case CORE_MACHINE_KEYBOARD_MODIFIER_CONTROL:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_D_CTRL);
    case CORE_MACHINE_KEYBOARD_MODIFIER_SHIFT:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_D_LSHIFT) ||
            TYPE_GET_BIT(flags, QDKEYB_FLAG0_D_RSHIFT);
    case CORE_MACHINE_KEYBOARD_MODIFIER_CAPS_LOCK:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_A_CAPLCK);
    case CORE_MACHINE_KEYBOARD_MODIFIER_NUM_LOCK:
        return TYPE_GET_BIT(flags, QDKEYB_FLAG0_A_NUMLCK);
    }
    return TYPE_FALSE;
}

static C_VOID keyboard_apply_host_state(C_VOID *context,
    uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    vm_profile_default_context *profile = context;

    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0,
        QDKEYB_FLAG0_D_RSHIFT,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_RIGHT_SHIFT) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0,
        QDKEYB_FLAG0_D_LSHIFT,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_LEFT_SHIFT) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0,
        QDKEYB_FLAG0_D_CTRL,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_CONTROL) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0,
        QDKEYB_FLAG0_D_ALT,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_ALT) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG1,
        QDKEYB_FLAG1_D_SCRLCK,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_SCROLL_LOCK) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG1,
        QDKEYB_FLAG1_D_NUMLCK,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_NUM_LOCK) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG1,
        QDKEYB_FLAG1_D_CAPLCK,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_CAPS_LOCK) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG1,
        QDKEYB_FLAG1_D_INSERT,
        (asynchronous_keys & CORE_MACHINE_KEYBOARD_ASYNC_INSERT) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0,
        QDKEYB_FLAG0_A_SCRLCK,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_SCROLL_LOCK) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0,
        QDKEYB_FLAG0_A_NUMLCK,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_NUM_LOCK) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0,
        QDKEYB_FLAG0_A_CAPLCK,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_CAPS_LOCK) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG0,
        QDKEYB_FLAG0_A_INSERT,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_INSERT) != 0u);
    keyboard_set_bda_flag(profile, QDKEYB_VBIOS_ADDR_KEYB_FLAG1,
        QDKEYB_FLAG1_D_PAUSE,
        (toggle_keys & CORE_MACHINE_KEYBOARD_TOGGLE_PAUSE) != 0u);
}

static const core_machine_keyboard_provider keyboard_provider = {
    keyboard_get_modifier,
    keyboard_apply_host_state
};

C_VOID vm_profile_default_keyboard_reset(vm_profile_default_context *profile)
{
    (C_VOID)profile;
}

const core_machine_keyboard_provider *vm_profile_default_keyboard_provider(C_VOID)
{
    return &keyboard_provider;
}
