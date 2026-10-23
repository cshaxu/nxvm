#include "x86-core/chips/vl82c420/vl82c420.h"

static lib_bool x86_vl82c420_bank_valid(x86_vl82c420_bank bank)
{
    return bank >= X86_VL82C420_BANK_SCAMP &&
        bank <= X86_VL82C420_BANK_ECED;
}

static lib_u8 *x86_vl82c420_bank_data(x86_vl82c420 *chip,
    x86_vl82c420_bank bank)
{
    switch (bank) {
    case X86_VL82C420_BANK_SCAMP: return chip->scamp;
    case X86_VL82C420_BANK_BLOCK2: return chip->block2;
    case X86_VL82C420_BANK_ECED: return chip->eced;
    default: return LIB_NULL;
    }
}

static const lib_u8 *x86_vl82c420_bank_data_const(const x86_vl82c420 *chip,
    x86_vl82c420_bank bank)
{
    switch (bank) {
    case X86_VL82C420_BANK_SCAMP: return chip->scamp;
    case X86_VL82C420_BANK_BLOCK2: return chip->block2;
    case X86_VL82C420_BANK_ECED: return chip->eced;
    default: return LIB_NULL;
    }
}

static lib_status x86_vl82c420_access_status(const x86_vl82c420 *chip,
    x86_vl82c420_bank bank)
{
    if (chip == LIB_NULL || !x86_vl82c420_bank_valid(bank)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (bank == X86_VL82C420_BANK_BLOCK2 && !chip->block2_unlocked) {
        return LIB_STATUS_INVALID_STATE;
    }
    return LIB_STATUS_OK;
}

lib_status x86_vl82c420_create(x86_vl82c420 **out_chip)
{
    x86_vl82c420 *chip;

    if (out_chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_chip = LIB_NULL;
    chip = lib_allocate_zero(1u, sizeof(*chip));
    if (chip == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *out_chip = chip;
    return LIB_STATUS_OK;
}

void x86_vl82c420_destroy(x86_vl82c420 *chip)
{
    lib_release(chip);
}

void x86_vl82c420_reset_protocol(x86_vl82c420 *chip)
{
    lib_size index;

    if (chip == LIB_NULL) return;
    chip->unlock_progress = 0u;
    chip->block2_unlocked = LIB_FALSE;
    for (index = 0u; index < sizeof(chip->selected); ++index) {
        chip->selected[index] = 0u;
    }
}

void x86_vl82c420_observe_unlock_read(x86_vl82c420 *chip,
    x86_vl82c420_unlock_read read)
{
    if (chip == LIB_NULL) return;
    if ((lib_u8)read == chip->unlock_progress) {
        ++chip->unlock_progress;
        if (chip->unlock_progress == 4u) {
            chip->unlock_progress = 0u;
            chip->block2_unlocked = LIB_TRUE;
        }
    } else {
        chip->unlock_progress = read == X86_VL82C420_UNLOCK_READ_FC23 ? 1u : 0u;
    }
}

void x86_vl82c420_relock(x86_vl82c420 *chip)
{
    if (chip == LIB_NULL) return;
    chip->unlock_progress = 0u;
    chip->block2_unlocked = LIB_FALSE;
}

lib_status x86_vl82c420_select(x86_vl82c420 *chip,
    x86_vl82c420_bank bank, lib_u8 index)
{
    lib_status status = x86_vl82c420_access_status(chip, bank);

    if (status != LIB_STATUS_OK) return status;
    chip->selected[bank] = bank == X86_VL82C420_BANK_SCAMP ?
        (lib_u8)(index & 0x7fu) : index;
    return LIB_STATUS_OK;
}

lib_status x86_vl82c420_read_selected(const x86_vl82c420 *chip,
    x86_vl82c420_bank bank, lib_u8 *out_value)
{
    const lib_u8 *data;
    lib_status status;

    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_vl82c420_access_status(chip, bank);
    if (status != LIB_STATUS_OK) return status;
    data = x86_vl82c420_bank_data_const(chip, bank);
    *out_value = data[chip->selected[bank]];
    return LIB_STATUS_OK;
}

lib_status x86_vl82c420_write_selected(x86_vl82c420 *chip,
    x86_vl82c420_bank bank, lib_u8 value)
{
    lib_u8 *data;
    lib_status status = x86_vl82c420_access_status(chip, bank);

    if (status != LIB_STATUS_OK) return status;
    data = x86_vl82c420_bank_data(chip, bank);
    data[chip->selected[bank]] = value;
    return LIB_STATUS_OK;
}
