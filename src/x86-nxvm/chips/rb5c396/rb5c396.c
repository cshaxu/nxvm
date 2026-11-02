/* Copyright 2012-2026 Neko. */
#include "x86-nxvm/chips/rb5c396/rb5c396.h"

static lib_bool x86_rb5c396_selected_slot(const x86_rb5c396 *chip,
    lib_u8 *out_slot, lib_u8 *out_register)
{
    if ((chip->index & 0x80u) != 0u) return LIB_FALSE;
    *out_slot = (chip->index >> 6u) & 1u;
    *out_register = chip->index & 0x3fu;
    return *out_register < X86_RB5C396_REGISTER_COUNT;
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
    if (chip->inputs[slot].card_present != inputs->card_present)
        chip->registers[slot][0x04u] |= 0x08u;
    chip->inputs[slot] = *inputs;
    return LIB_STATUS_OK;
}

lib_status x86_rb5c396_status_irq(const x86_rb5c396 *chip, lib_u8 slot,
    lib_u8 *out_irq)
{
    lib_u8 irq;
    lib_u8 pending;
    if (chip == LIB_NULL || out_irq == LIB_NULL || slot >= X86_RB5C396_SLOT_COUNT)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_irq = 0u;
    pending = chip->registers[slot][0x04u] & chip->registers[slot][0x05u] & 0x0fu;
    if ((chip->registers[slot][0x16u] & 0x10u) != 0u)
        pending &= (lib_u8)~0x08u;
    if ((chip->registers[slot][0x03u] & 0x10u) != 0u || pending == 0u)
        return LIB_STATUS_OK;
    irq = chip->registers[slot][0x05u] >> 4u;
    if (irq == 3u || irq == 4u || irq == 5u || irq == 7u || irq == 9u ||
        irq == 10u || irq == 11u || irq == 12u || irq == 14u || irq == 15u)
        *out_irq = irq;
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
    else if (selected == 0x1eu)
        *out_value = chip->registers[0u][selected];
    else if (selected == 0x04u) {
        *out_value = chip->registers[slot][selected];
        if ((chip->registers[0u][0x1eu] & 0x04u) == 0u)
            chip->registers[slot][selected] = 0u;
    } else
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
    if (selected == 0x1eu)
        chip->registers[0u][selected] = value;
    else if (selected == 0x04u) {
        if ((chip->registers[0u][0x1eu] & 0x04u) != 0u)
            chip->registers[slot][selected] &= (lib_u8)~value;
    } else if (selected == 0x16u) {
        if ((value & 0x20u) != 0u && (chip->registers[slot][0x05u] & 0x08u) != 0u)
            chip->registers[slot][0x04u] |= 0x08u;
        chip->registers[slot][selected] = value & (lib_u8)~0x20u;
    } else
        chip->registers[slot][selected] = value;
    return LIB_STATUS_OK;
}
