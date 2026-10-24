#include "x86-nxvm/chips/f65535/f65535_interface.h"

int main(void)
{
    x86_f65535 *chip = LIB_NULL;
    lib_u8 value = 0u;
    lib_u8 write = 0xa5u;
    lib_i32 failed = 0;

    failed |= x86_f65535_create(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_f65535_create(&chip) != LIB_STATUS_OK || chip == LIB_NULL ||
        x86_f65535_extension_read(chip, X86_F65535_EXTENSION_INDEX_PORT,
            &value) != LIB_STATUS_OK || value != 0u ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_INDEX_PORT,
            0x80u) != LIB_STATUS_OK || x86_f65535_extension_read(chip,
            X86_F65535_EXTENSION_INDEX_PORT, &value) != LIB_STATUS_OK ||
        value != 0u || x86_f65535_extension_read(chip,
            X86_F65535_EXTENSION_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_F65535_PC110_CHIP_VERSION ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_DATA_PORT,
            0x12u) != LIB_STATUS_OK || x86_f65535_extension_read(chip,
            X86_F65535_EXTENSION_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_F65535_PC110_CHIP_VERSION ||
        x86_f65535_extension_write(chip, X86_F65535_EXTENSION_INDEX_PORT,
            0x82u) != LIB_STATUS_OK || x86_f65535_extension_write(chip,
            X86_F65535_EXTENSION_DATA_PORT, write) != LIB_STATUS_OK ||
        x86_f65535_extension_read(chip, X86_F65535_EXTENSION_DATA_PORT,
            &value) != LIB_STATUS_OK || value != write ||
        x86_f65535_vram_write(chip, X86_F65535_VRAM_BYTES - 1u, &write,
            1u) != LIB_STATUS_OK || x86_f65535_vram_read(chip,
            X86_F65535_VRAM_BYTES - 1u, &value, 1u) != LIB_STATUS_OK ||
        value != write || x86_f65535_vram_read(chip, X86_F65535_VRAM_BYTES,
            &value, 1u) != LIB_STATUS_INVALID_ARGUMENT;
    x86_f65535_reset(chip);
    failed |= x86_f65535_extension_read(chip, X86_F65535_EXTENSION_INDEX_PORT,
        &value) != LIB_STATUS_OK || value != 0u ||
        x86_f65535_vram_read(chip, X86_F65535_VRAM_BYTES - 1u, &value,
            1u) != LIB_STATUS_OK || value != 0u;
    x86_f65535_destroy(chip);
    return failed;
}
