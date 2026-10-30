#include "x86-core/chips/vl82c420/vl82c420_interface.h"

static lib_bool unlock_block2(x86_vl82c420 *chip)
{
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_FC23);
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_F023);
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_C023);
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_0023);
    return x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xb8u) ==
        LIB_STATUS_OK;
}

static lib_bool asmi_contract(x86_vl82c420 *chip)
{
    const lib_u8 configuration[][2u] = {
        {0xfau, 1u}, {0x84u, 0xeeu}, {0x85u, 0x15u}, {0xb5u, 4u}, {0xb6u, 0x80u}};
    lib_u8 value = 0u;

    if (!unlock_block2(chip)) return LIB_FALSE;
    for (lib_size index = 0u; index < sizeof(configuration) / sizeof(configuration[0]); ++index) {
        x86_vl82c420_write_asmi(chip, 0x15eeu);
        if (x86_vl82c420_smi_asserted(chip) ||
            x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2,
                configuration[index][0]) != LIB_STATUS_OK ||
            x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2,
                configuration[index][1]) != LIB_STATUS_OK) return LIB_FALSE;
    }
    x86_vl82c420_write_asmi(chip, 0x15ecu);
    if (x86_vl82c420_smi_asserted(chip)) return LIB_FALSE;
    x86_vl82c420_relock(chip);
    x86_vl82c420_write_asmi(chip, 0x15eeu);
    if (!x86_vl82c420_smi_asserted(chip) || !unlock_block2(chip) ||
        x86_vl82c420_read_register(chip, X86_VL82C420_BANK_BLOCK2, 0xb7u, &value) !=
            LIB_STATUS_OK || value != 0x20u ||
        x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xb7u) != LIB_STATUS_OK ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 0xdfu) != LIB_STATUS_OK ||
        x86_vl82c420_smi_asserted(chip)) return LIB_FALSE;
    /* A new source between acknowledge and EOSMI remains pending. */
    x86_vl82c420_write_asmi(chip, 0x15eeu);
    if (x86_vl82c420_smi_asserted(chip) ||
        x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xfau) != LIB_STATUS_OK ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 1u) != LIB_STATUS_OK ||
        !x86_vl82c420_smi_asserted(chip) ||
        x86_vl82c420_read_selected(chip, X86_VL82C420_BANK_BLOCK2, &value) != LIB_STATUS_OK ||
        value != 0xffu) return LIB_FALSE;
    x86_vl82c420_reset_protocol(chip);
    return !x86_vl82c420_smi_asserted(chip);
}

static lib_bool resume_button_contract(x86_vl82c420 *chip)
{
    lib_u8 value = 0u;

    if (!unlock_block2(chip) ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 0xa0u) != LIB_STATUS_OK ||
        x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xfbu) != LIB_STATUS_OK ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 0x48u) != LIB_STATUS_OK ||
        x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xfau) != LIB_STATUS_OK ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 1u) != LIB_STATUS_OK ||
        x86_vl82c420_smi_asserted(chip) ||
        x86_vl82c420_set_resume_button(chip, LIB_TRUE) ||
        x86_vl82c420_suspend_refresh_enabled(chip) ||
        x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xffu) != LIB_STATUS_OK ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 0x82u) != LIB_STATUS_OK ||
        !x86_vl82c420_suspend_refresh_enabled(chip) ||
        x86_vl82c420_set_resume_button(chip, LIB_TRUE) ||
        x86_vl82c420_set_resume_button(chip, LIB_FALSE) ||
        !x86_vl82c420_suspend_refresh_enabled(chip)) return LIB_FALSE;
    x86_vl82c420_relock(chip);
    if (!x86_vl82c420_set_resume_button(chip, LIB_TRUE) ||
        x86_vl82c420_suspend_refresh_enabled(chip) ||
        x86_vl82c420_set_resume_button(chip, LIB_TRUE)) return LIB_FALSE;
    if (!unlock_block2(chip) ||
        x86_vl82c420_read_register(chip, X86_VL82C420_BANK_BLOCK2, 0xb8u, &value) !=
            LIB_STATUS_OK || value != 0xa8u ||
        x86_vl82c420_read_register(chip, X86_VL82C420_BANK_BLOCK2, 0xfbu, &value) !=
            LIB_STATUS_OK || value != 0x48u ||
        x86_vl82c420_read_register(chip, X86_VL82C420_BANK_BLOCK2, 0xffu, &value) !=
            LIB_STATUS_OK || value != 2u || x86_vl82c420_smi_asserted(chip) ||
        x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xfau) != LIB_STATUS_OK ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 1u) != LIB_STATUS_OK ||
        !x86_vl82c420_smi_asserted(chip)) return LIB_FALSE;
    const lib_u8 registers[] = {0xb8u, 0xfbu};
    for (lib_size index = 0u; index < sizeof(registers); ++index) {
        if (x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, registers[index]) !=
                LIB_STATUS_OK ||
            x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 0u) != LIB_STATUS_OK ||
            x86_vl82c420_read_selected(chip, X86_VL82C420_BANK_BLOCK2, &value) !=
                LIB_STATUS_OK || value != 0u) return LIB_FALSE;
    }
    return !x86_vl82c420_smi_asserted(chip) &&
        !x86_vl82c420_set_resume_button(chip, LIB_TRUE) &&
        !x86_vl82c420_set_resume_button(chip, LIB_FALSE);
}

lib_i32 main(void)
{
    x86_vl82c420 *chip = LIB_NULL;
    lib_u8 value = 0u;
    lib_bool failed = x86_vl82c420_create(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;

    failed |= x86_vl82c420_create(&chip) != LIB_STATUS_OK || chip == LIB_NULL;
    failed |= x86_vl82c420_select(chip, X86_VL82C420_BANK_SCAMP, 0x80u) !=
        LIB_STATUS_OK ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_SCAMP, 0x5au) !=
        LIB_STATUS_OK ||
        x86_vl82c420_select(chip, X86_VL82C420_BANK_SCAMP, 0u) != LIB_STATUS_OK ||
        x86_vl82c420_read_selected(chip, X86_VL82C420_BANK_SCAMP, &value) !=
        LIB_STATUS_OK || value != 0x5au ||
        x86_vl82c420_selected_index(chip, X86_VL82C420_BANK_SCAMP, &value) !=
        LIB_STATUS_OK || value != 0u ||
        x86_vl82c420_selected_index(chip, X86_VL82C420_BANK_SCAMP, LIB_NULL) !=
        LIB_STATUS_INVALID_ARGUMENT ||
        x86_vl82c420_selected_index(chip, X86_VL82C420_BANK_BLOCK2, &value) !=
        LIB_STATUS_INVALID_STATE;

    failed |= x86_vl82c420_read_register(chip, X86_VL82C420_BANK_SCAMP,
            0u, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_vl82c420_select(chip, X86_VL82C420_BANK_SCAMP, 2u) != LIB_STATUS_OK ||
        x86_vl82c420_read_register(chip, X86_VL82C420_BANK_SCAMP, 0x80u,
            &value) != LIB_STATUS_OK || value != 0x5au ||
        x86_vl82c420_read_selected(chip, X86_VL82C420_BANK_SCAMP, &value) !=
            LIB_STATUS_OK || value != 0u ||
        x86_vl82c420_read_register(chip, X86_VL82C420_BANK_BLOCK2, 0xb8u,
            &value) != LIB_STATUS_INVALID_STATE;

    failed |= x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xb8u) !=
        LIB_STATUS_INVALID_STATE;
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_FC23);
    x86_vl82c420_observe_unlock_read(chip, X86_VL82C420_UNLOCK_READ_C023);
    failed |= x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xb8u) !=
        LIB_STATUS_INVALID_STATE;
    failed |= !unlock_block2(chip) ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_BLOCK2, 0x3cu) !=
        LIB_STATUS_OK;
    x86_vl82c420_relock(chip);
    failed |= x86_vl82c420_read_selected(chip, X86_VL82C420_BANK_BLOCK2, &value) !=
        LIB_STATUS_INVALID_STATE;

    failed |= !unlock_block2(chip) ||
        x86_vl82c420_read_selected(chip, X86_VL82C420_BANK_BLOCK2, &value) !=
        LIB_STATUS_OK || value != 0x3cu;
    failed |= x86_vl82c420_select(chip, X86_VL82C420_BANK_ECED, 0x02u) !=
        LIB_STATUS_OK ||
        x86_vl82c420_write_selected(chip, X86_VL82C420_BANK_ECED, 0x0bu) !=
        LIB_STATUS_OK;
    x86_vl82c420_reset_protocol(chip);
    failed |= x86_vl82c420_select(chip, X86_VL82C420_BANK_BLOCK2, 0xb8u) !=
        LIB_STATUS_INVALID_STATE ||
        x86_vl82c420_select(chip, X86_VL82C420_BANK_ECED, 0x02u) != LIB_STATUS_OK ||
        x86_vl82c420_read_selected(chip, X86_VL82C420_BANK_ECED, &value) !=
        LIB_STATUS_OK || value != 0x0bu;
    failed |= !asmi_contract(chip);
    failed |= !resume_button_contract(chip);
    x86_vl82c420_destroy(chip);
    return failed ? 1 : 0;
}
