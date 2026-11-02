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

lib_status x86_rb5c396_get_slot_inputs(const x86_rb5c396 *chip, lib_u8 slot,
    x86_rb5c396_slot_inputs *out_inputs)
{
    if (chip == LIB_NULL || out_inputs == LIB_NULL || slot >= X86_RB5C396_SLOT_COUNT)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_inputs = chip->inputs[slot];
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

lib_status x86_rb5c396_memory_window_range(const x86_rb5c396 *chip, lib_u8 slot,
    lib_u8 window, lib_u32 *out_first, lib_u32 *out_bytes)
{
    const lib_u8 *registers;
    lib_u8 base;
    lib_u32 first, last;

    if (chip == LIB_NULL || out_first == LIB_NULL || out_bytes == LIB_NULL ||
        slot >= X86_RB5C396_SLOT_COUNT ||
        window >= X86_RB5C396_MEMORY_WINDOW_COUNT) return LIB_STATUS_INVALID_ARGUMENT;
    registers = chip->registers[slot];
    if ((registers[0x06u] & (1u << window)) == 0u) return LIB_STATUS_UNSUPPORTED;
    base = (lib_u8)(0x10u + window * 8u);
    first = ((lib_u32)(registers[base + 1u] & 0x0fu) << 20u) |
        ((lib_u32)registers[base] << 12u);
    last = ((lib_u32)(registers[base + 3u] & 0x0fu) << 20u) |
        ((lib_u32)registers[base + 2u] << 12u) | 0x0fffu;
    if (first < 0x00010000u || first > last) return LIB_STATUS_UNSUPPORTED;
    *out_first = first;
    *out_bytes = last - first + 1u;
    return LIB_STATUS_OK;
}

lib_status x86_rb5c396_decode_memory(const x86_rb5c396 *chip, lib_u8 slot,
    lib_u8 window, lib_u32 physical, x86_rb5c396_memory_mapping *out_mapping)
{
    const lib_u8 *registers;
    lib_u8 base;
    lib_u32 first, size, offset, address, span;
    lib_status status;

    if (out_mapping == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_rb5c396_memory_window_range(chip, slot, window, &first, &size);
    if (status != LIB_STATUS_OK) return status;
    if (physical < first || physical - first >= size) return LIB_STATUS_UNSUPPORTED;
    registers = chip->registers[slot];
    base = (lib_u8)(0x10u + window * 8u);
    offset = ((lib_u32)(registers[base + 5u] & 0x3fu) << 20u) |
        ((lib_u32)registers[base + 4u] << 12u);
    address = (physical + offset) & 0x03ffffffu;
    span = size - (physical - first);
    if (span > 0x04000000u - address) span = 0x04000000u - address;
    *out_mapping = (x86_rb5c396_memory_mapping){
        .card_address = address,
        .contiguous_bytes = span,
        .attribute = (registers[base + 5u] & 0x40u) != 0u,
        .write_protected = (registers[base + 5u] & 0x80u) != 0u,
        .data_16bit = (registers[base + 1u] & 0x80u) != 0u,
        .zero_wait = (registers[base + 1u] & 0x40u) != 0u,
        .wait_states = registers[base + 3u] >> 6u};
    return LIB_STATUS_OK;
}

lib_status x86_rb5c396_decode_io(const x86_rb5c396 *chip, lib_u8 slot,
    lib_u8 window, lib_u16 port, x86_rb5c396_io_mapping *out_mapping)
{
    const lib_u8 *registers;
    lib_u8 base, offset_base, control;
    lib_u32 first, last, card, span;
    if (chip == LIB_NULL || out_mapping == LIB_NULL || slot >= X86_RB5C396_SLOT_COUNT ||
        window >= X86_RB5C396_IO_WINDOW_COUNT) return LIB_STATUS_INVALID_ARGUMENT;
    registers = chip->registers[slot];
    if ((registers[0x06u] & (0x40u << window)) == 0u) return LIB_STATUS_UNSUPPORTED;
    base = (lib_u8)(0x08u + window * 4u);
    offset_base = (lib_u8)(0x36u + window * 2u);
    first = (lib_u32)registers[base] | ((lib_u32)registers[base + 1u] << 8u);
    last = (lib_u32)registers[base + 2u] | ((lib_u32)registers[base + 3u] << 8u);
    if (first > last || port < first || port > last) return LIB_STATUS_UNSUPPORTED;
    card = (port + ((lib_u32)registers[offset_base] & 0xfeu) +
        ((lib_u32)registers[offset_base + 1u] << 8u)) & 0xffffu;
    span = last - port + 1u;
    if (span > 0x10000u - card) span = 0x10000u - card;
    control = (lib_u8)(registers[0x07u] >> (window * 4u));
    *out_mapping = (x86_rb5c396_io_mapping){
        .card_port = (lib_u16)card, .contiguous_bytes = span,
        .data_16bit = (control & 1u) != 0u, .card_io16 = (control & 2u) != 0u,
        .zero_wait = (control & 4u) != 0u, .wait_state = (control & 8u) != 0u};
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
    } else if (selected == 0x36u || selected == 0x38u)
        chip->registers[slot][selected] = value & 0xfeu;
    else
        chip->registers[slot][selected] = value;
    return LIB_STATUS_OK;
}
