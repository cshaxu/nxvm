#include "lib/types/test.h"
#include "x86-nxvm/chips/rb5c396/rb5c396_interface.h"

static lib_status select_register(x86_rb5c396 *chip, lib_u8 index)
{
    return x86_rb5c396_write(chip, X86_RB5C396_INDEX_PORT, index);
}

static lib_bool write_register(x86_rb5c396 *chip, lib_u8 index, lib_u8 value)
{
    return select_register(chip, index) == LIB_STATUS_OK &&
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, value) == LIB_STATUS_OK;
}

static lib_bool memory_windows(void)
{
    x86_rb5c396 *chip = LIB_NULL;
    x86_rb5c396_memory_mapping mapping;
    lib_bool failed = x86_rb5c396_create(&chip) != LIB_STATUS_OK;
    if (failed) return LIB_FALSE;
    for (lib_u8 slot = 0u; slot < X86_RB5C396_SLOT_COUNT; ++slot) {
        for (lib_u8 window = 0u; window < X86_RB5C396_MEMORY_WINDOW_COUNT; ++window) {
            const lib_u8 bank = (lib_u8)(slot << 6u);
            const lib_u8 base = (lib_u8)(bank + 0x10u + window * 8u);
            failed |= !write_register(chip, (lib_u8)(bank + 0x06u), 0u) ||
                x86_rb5c396_decode_memory(chip, slot, window, 0xd0000u, &mapping) !=
                    LIB_STATUS_UNSUPPORTED ||
                !write_register(chip, base, 0xd0u) ||
                !write_register(chip, (lib_u8)(base + 1u), 0xc0u) ||
                !write_register(chip, (lib_u8)(base + 2u), 0xdfu) ||
                !write_register(chip, (lib_u8)(base + 3u), 0xc0u) ||
                !write_register(chip, (lib_u8)(base + 4u), 0x30u) ||
                !write_register(chip, (lib_u8)(base + 5u), 0xffu) ||
                !write_register(chip, (lib_u8)(bank + 0x06u), (lib_u8)(1u << window));
            failed |= x86_rb5c396_decode_memory(chip, slot, window, 0xd0000u, &mapping) !=
                    LIB_STATUS_OK || mapping.card_address != 0u ||
                mapping.contiguous_bytes != 0x10000u || !mapping.attribute ||
                !mapping.write_protected || !mapping.data_16bit || !mapping.zero_wait ||
                mapping.wait_states != 3u ||
                x86_rb5c396_decode_memory(chip, slot, window, 0xdffffu, &mapping) !=
                    LIB_STATUS_OK || mapping.card_address != 0xffffu ||
                mapping.contiguous_bytes != 1u ||
                x86_rb5c396_decode_memory(chip, slot, window, 0xe0000u, &mapping) !=
                    LIB_STATUS_UNSUPPORTED ||
                !write_register(chip, (lib_u8)(base + 4u), 0x2fu) ||
                x86_rb5c396_decode_memory(chip, slot, window, 0xd0000u, &mapping) !=
                    LIB_STATUS_OK || mapping.card_address != 0x03fff000u ||
                mapping.contiguous_bytes != 0x1000u;
        }
    }
    failed |= !write_register(chip, 0x06u, 1u) ||
        !write_register(chip, 0x10u, 0xd0u) || !write_register(chip, 0x12u, 0xcfu) ||
        x86_rb5c396_decode_memory(chip, 0u, 0u, 0xd0000u, &mapping) !=
            LIB_STATUS_UNSUPPORTED ||
        !write_register(chip, 0x10u, 0u) || !write_register(chip, 0x12u, 0x0fu) ||
        x86_rb5c396_decode_memory(chip, 0u, 0u, 0u, &mapping) !=
            LIB_STATUS_UNSUPPORTED;
    x86_rb5c396_reset(chip);
    failed |= x86_rb5c396_decode_memory(chip, 0u, 0u, 0xd0000u, &mapping) !=
            LIB_STATUS_UNSUPPORTED ||
        x86_rb5c396_decode_memory(chip, 2u, 0u, 0xd0000u, &mapping) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        x86_rb5c396_decode_memory(chip, 0u, 5u, 0xd0000u, &mapping) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        x86_rb5c396_decode_memory(chip, 0u, 0u, 0xd0000u, LIB_NULL) !=
            LIB_STATUS_INVALID_ARGUMENT;
    x86_rb5c396_destroy(chip);
    return !failed;
}

int main(void)
{
    x86_rb5c396 *chip = LIB_NULL;
    const x86_rb5c396_slot_inputs present = {
        .card_present = LIB_TRUE, .ready = LIB_TRUE};
    const x86_rb5c396_slot_inputs absent = {0};
    lib_u8 value = 0u;
    lib_u8 irq = 0u;
    lib_bool failed = LIB_FALSE;

    failed |= x86_rb5c396_create(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_rb5c396_create(&chip) != LIB_STATUS_OK || chip == LIB_NULL ||
        x86_rb5c396_read(chip, X86_RB5C396_INDEX_PORT, &value) != LIB_STATUS_OK ||
        value != 0u || select_register(chip, 0u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_RB5C396_IDENTIFICATION_VALUE ||
        select_register(chip, 0x3au) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_RB5C396_CHIP_IDENTIFICATION_VALUE ||
        select_register(chip, 0x40u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_RB5C396_IDENTIFICATION_VALUE ||
        select_register(chip, 0x7au) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_RB5C396_CHIP_IDENTIFICATION_VALUE ||
        select_register(chip, 0x80u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0xffu;
    failed |= select_register(chip, 0x3fu) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0xffu || x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x55u) !=
            LIB_STATUS_OK || select_register(chip, 0x7cu) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0xffu || x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x55u) !=
            LIB_STATUS_OK;
    failed |= select_register(chip, X86_RB5C396_INTERFACE_STATUS_REGISTER) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0u || x86_rb5c396_set_slot_inputs(chip, 0u, &present) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0x2cu || x86_rb5c396_set_slot_inputs(chip, 2u, &present) !=
            LIB_STATUS_INVALID_ARGUMENT;
    failed |= select_register(chip, 2u) != LIB_STATUS_OK ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x10u) != LIB_STATUS_OK ||
        select_register(chip, X86_RB5C396_INTERFACE_STATUS_REGISTER) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0x6cu;
    /* Insertion latches independently per slot; the default read acknowledges. */
    failed |= select_register(chip, 0x04u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0x08u || x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) !=
            LIB_STATUS_OK || value != 0u ||
        select_register(chip, 0x44u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0u;
    /* A pending detect change is delivered only through an enabled valid IRQ. */
    failed |= select_register(chip, 0x05u) != LIB_STATUS_OK ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x58u) != LIB_STATUS_OK ||
        x86_rb5c396_set_slot_inputs(chip, 0u, &absent) != LIB_STATUS_OK ||
        x86_rb5c396_status_irq(chip, 0u, &irq) != LIB_STATUS_OK || irq != 5u ||
        x86_rb5c396_status_irq(chip, 1u, &irq) != LIB_STATUS_OK || irq != 0u ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x68u) != LIB_STATUS_OK ||
        x86_rb5c396_status_irq(chip, 0u, &irq) != LIB_STATUS_OK || irq != 0u ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0xa8u) != LIB_STATUS_OK ||
        x86_rb5c396_status_irq(chip, 0u, &irq) != LIB_STATUS_OK || irq != 10u ||
        select_register(chip, 0x16u) != LIB_STATUS_OK ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x10u) != LIB_STATUS_OK ||
        x86_rb5c396_status_irq(chip, 0u, &irq) != LIB_STATUS_OK || irq != 0u ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0u) != LIB_STATUS_OK ||
        x86_rb5c396_status_irq(chip, 0u, &irq) != LIB_STATUS_OK || irq != 10u ||
        select_register(chip, 0x03u) != LIB_STATUS_OK ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x10u) != LIB_STATUS_OK ||
        x86_rb5c396_status_irq(chip, 0u, &irq) != LIB_STATUS_OK || irq != 0u ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0u) != LIB_STATUS_OK ||
        select_register(chip, 0x04u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0x08u || x86_rb5c396_status_irq(chip, 0u, &irq) != LIB_STATUS_OK ||
        irq != 0u || select_register(chip, 0x16u) != LIB_STATUS_OK ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x20u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0u || x86_rb5c396_status_irq(chip, 0u, &irq) != LIB_STATUS_OK ||
        irq != 10u || select_register(chip, 0x04u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0x08u || x86_rb5c396_set_slot_inputs(chip, 0u, &present) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK;
    /* Global control aliases across sockets; explicit acknowledge writes one. */
    failed |= select_register(chip, 0x5eu) != LIB_STATUS_OK ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x04u) != LIB_STATUS_OK ||
        select_register(chip, 0x1eu) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0x04u || x86_rb5c396_set_slot_inputs(chip, 0u, &absent) !=
            LIB_STATUS_OK || select_register(chip, 0x04u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0x08u || x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) !=
            LIB_STATUS_OK || value != 0x08u ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x08u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0u || x86_rb5c396_set_slot_inputs(chip, 0u, &present) != LIB_STATUS_OK;
    failed |= select_register(chip, 0x18u) != LIB_STATUS_OK ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0x5au) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0x5au || select_register(chip, 0x58u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0u || x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0xa5u) !=
            LIB_STATUS_OK || x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) !=
            LIB_STATUS_OK || value != 0xa5u;
    failed |= select_register(chip, X86_RB5C396_IDENTIFICATION_REGISTER) != LIB_STATUS_OK ||
        x86_rb5c396_write(chip, X86_RB5C396_DATA_PORT, 0u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_RB5C396_IDENTIFICATION_VALUE ||
        x86_rb5c396_read(chip, 0x03e2u, &value) != LIB_STATUS_UNSUPPORTED ||
        x86_rb5c396_write(chip, 0x03e2u, 0u) != LIB_STATUS_UNSUPPORTED;
    x86_rb5c396_reset(chip);
    failed |= x86_rb5c396_read(chip, X86_RB5C396_INDEX_PORT, &value) != LIB_STATUS_OK ||
        value != 0u || select_register(chip, 0x18u) != LIB_STATUS_OK ||
        x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != 0u || select_register(chip, X86_RB5C396_INTERFACE_STATUS_REGISTER) !=
            LIB_STATUS_OK || x86_rb5c396_read(chip, X86_RB5C396_DATA_PORT, &value) !=
            LIB_STATUS_OK || value != 0x2cu;
    x86_rb5c396_destroy(chip);
    return failed || !memory_windows() ? 1 : 0;
}
