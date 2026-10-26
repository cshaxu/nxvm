#include "lib/types/test.h"
#include "x86-nxvm/chips/rb5c396/rb5c396_interface.h"

static lib_status select_register(x86_rb5c396 *chip, lib_u8 index)
{
    return x86_rb5c396_write(chip, X86_RB5C396_INDEX_PORT, index);
}

int main(void)
{
    x86_rb5c396 *chip = LIB_NULL;
    const x86_rb5c396_slot_inputs present = {
        .card_present = LIB_TRUE, .ready = LIB_TRUE};
    lib_u8 value = 0u;
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
    return failed ? 1 : 0;
}
