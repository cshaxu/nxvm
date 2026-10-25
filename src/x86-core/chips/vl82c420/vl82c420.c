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
    chip->smi_armed = LIB_FALSE;
    chip->smi_asserted = LIB_FALSE;
    chip->block2[0xb7u] = 0u;
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
    lib_status status;

    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_vl82c420_access_status(chip, bank);
    if (status != LIB_STATUS_OK) return status;
    return x86_vl82c420_read_register(chip, bank, chip->selected[bank], out_value);
}

lib_status x86_vl82c420_read_register(const x86_vl82c420 *chip,
    x86_vl82c420_bank bank, lib_u8 index, lib_u8 *out_value)
{
    const lib_u8 *data;
    lib_status status;

    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_vl82c420_access_status(chip, bank);
    if (status != LIB_STATUS_OK) return status;
    data = x86_vl82c420_bank_data_const(chip, bank);
    *out_value = data[bank == X86_VL82C420_BANK_SCAMP ? index & 0x7fu : index];
    if (bank == X86_VL82C420_BANK_BLOCK2 && index == 0xfau) *out_value = 0xffu;
    return LIB_STATUS_OK;
}

static void x86_vl82c420_update_smi(x86_vl82c420 *chip)
{
    if ((chip->block2[0xb5u] & 4u) == 0u ||
        (chip->block2[0xb6u] & 0x80u) == 0u || chip->block2[0xb7u] == 0u)
        chip->smi_asserted = LIB_FALSE;
    else if (chip->smi_armed) {
        chip->smi_armed = LIB_FALSE;
        chip->smi_asserted = LIB_TRUE;
    }
}

lib_status x86_vl82c420_write_selected(x86_vl82c420 *chip,
    x86_vl82c420_bank bank, lib_u8 value)
{
    lib_u8 *data;
    lib_status status = x86_vl82c420_access_status(chip, bank);

    if (status != LIB_STATUS_OK) return status;
    data = x86_vl82c420_bank_data(chip, bank);
    if (bank == X86_VL82C420_BANK_BLOCK2 && chip->selected[bank] == 0xb7u) {
        data[0xb7u] &= value;
    } else if (bank == X86_VL82C420_BANK_BLOCK2 && chip->selected[bank] == 0xfau) {
        if (value == 1u) {
            chip->smi_armed = LIB_TRUE;
            chip->smi_asserted = LIB_FALSE;
        }
    } else {
        data[chip->selected[bank]] = value;
    }
    if (bank == X86_VL82C420_BANK_BLOCK2) x86_vl82c420_update_smi(chip);
    return LIB_STATUS_OK;
}

void x86_vl82c420_write_asmi(x86_vl82c420 *chip, lib_u16 port)
{
    if (chip == LIB_NULL || (chip->block2[0xb5u] & 4u) == 0u ||
        (chip->block2[0xb6u] & 0x80u) == 0u ||
        (port & 0xfffeu) != (((lib_u32)chip->block2[0x84u] |
            ((lib_u32)chip->block2[0x85u] << 8u)) & 0xfffeu)) return;
    chip->block2[0xb7u] |= 0x20u;
    x86_vl82c420_update_smi(chip);
}

lib_bool x86_vl82c420_smi_asserted(const x86_vl82c420 *chip)
{
    return chip != LIB_NULL && chip->smi_asserted;
}
