/* Copyright 2012-2026 Neko. */
#include "x86-nxvm/chips/rb5c396/rb5c396.h"

static lib_bool x86_rb5c396_selected_slot(const x86_rb5c396 *chip,
    lib_u8 *out_slot, lib_u8 *out_register)
{
    if ((chip->index & 0x80u) != 0u) return LIB_FALSE;
    *out_slot = (chip->index >> 6u) & 1u;
    *out_register = chip->index & 0x3fu;
    return LIB_TRUE;
}

static lib_u8 x86_rb5c396_interface_status(const x86_rb5c396 *chip,
    lib_u8 slot)
{
    const x86_rb5c396_slot_inputs *inputs = &chip->inputs[slot];
    lib_u8 value = 0u;

    if ((chip->registers[slot][2u] & 0x10u) != 0u &&
        ((chip->registers[slot][2u] & 0x20u) == 0u || inputs->card_present)) {
        value |= 0x40u;
    }
    if (inputs->ready) value |= 0x20u;
    if (inputs->write_protected) value |= 0x10u;
    if (inputs->card_present) value |= 0x0cu;
    if (inputs->battery_detect_2) value |= 0x02u;
    if (inputs->battery_detect_1) value |= 0x01u;
    return value;
}

lib_status x86_rb5c396_create(x86_rb5c396 **out_chip)
{
    x86_rb5c396 *chip;

    if (out_chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_chip = LIB_NULL;
    chip = lib_allocate_zero(1u, sizeof(*chip));
    if (chip == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    x86_rb5c396_reset(chip);
    *out_chip = chip;
    return LIB_STATUS_OK;
}

void x86_rb5c396_destroy(x86_rb5c396 *chip)
{
    lib_release(chip);
}

void x86_rb5c396_reset(x86_rb5c396 *chip)
{
    x86_rb5c396_slot_inputs first;
    x86_rb5c396_slot_inputs second;

    if (chip == LIB_NULL) return;
    first = chip->inputs[0u];
    second = chip->inputs[1u];
    *chip = (x86_rb5c396){0};
    chip->inputs[0u] = first;
    chip->inputs[1u] = second;
}

lib_status x86_rb5c396_set_slot_inputs(x86_rb5c396 *chip, lib_u8 slot,
    const x86_rb5c396_slot_inputs *inputs)
{
    if (chip == LIB_NULL || inputs == LIB_NULL || slot >= X86_RB5C396_SLOT_COUNT)
        return LIB_STATUS_INVALID_ARGUMENT;
    chip->inputs[slot] = *inputs;
    return LIB_STATUS_OK;
}

lib_status x86_rb5c396_read(x86_rb5c396 *chip, lib_u16 port,
    lib_u8 *out_value)
{
    lib_u8 slot;
    lib_u8 selected;

    if (chip == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_RB5C396_INDEX_PORT) {
        *out_value = chip->index;
        return LIB_STATUS_OK;
    }
    if (port != X86_RB5C396_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    if (!x86_rb5c396_selected_slot(chip, &slot, &selected)) {
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    }
    if (selected == X86_RB5C396_IDENTIFICATION_REGISTER)
        *out_value = X86_RB5C396_IDENTIFICATION_VALUE;
    else if (selected == X86_RB5C396_INTERFACE_STATUS_REGISTER)
        *out_value = x86_rb5c396_interface_status(chip, slot);
    else if (selected == X86_RB5C396_CHIP_IDENTIFICATION_REGISTER)
        *out_value = X86_RB5C396_CHIP_IDENTIFICATION_VALUE;
    else
        *out_value = chip->registers[slot][selected];
    return LIB_STATUS_OK;
}

lib_status x86_rb5c396_write(x86_rb5c396 *chip, lib_u16 port,
    lib_u8 value)
{
    lib_u8 slot;
    lib_u8 selected;

    if (chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_RB5C396_INDEX_PORT) {
        chip->index = value;
        return LIB_STATUS_OK;
    }
    if (port != X86_RB5C396_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    if (!x86_rb5c396_selected_slot(chip, &slot, &selected) ||
        selected == X86_RB5C396_IDENTIFICATION_REGISTER ||
        selected == X86_RB5C396_INTERFACE_STATUS_REGISTER ||
        selected == X86_RB5C396_CHIP_IDENTIFICATION_REGISTER)
        return LIB_STATUS_OK;
    chip->registers[slot][selected] = value;
    return LIB_STATUS_OK;
}
