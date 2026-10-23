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
        LIB_STATUS_OK || value != 0x5au;

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
    x86_vl82c420_destroy(chip);
    return failed ? 1 : 0;
}
