/* Copyright 2012-2026 Neko. */
#include "x86-nxvm/chips/f65535/f65535.h"

static lib_bool x86_f65535_vram_contains(lib_u32 offset, lib_size bytes)
{
    return offset < X86_F65535_VRAM_BYTES &&
        bytes <= X86_F65535_VRAM_BYTES - offset;
}

lib_status x86_f65535_create(x86_f65535 **out_chip)
{
    x86_f65535 *chip;

    if (out_chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_chip = LIB_NULL;
    chip = lib_allocate_zero(1u, sizeof(*chip));
    if (chip == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    x86_f65535_reset(chip);
    *out_chip = chip;
    return LIB_STATUS_OK;
}

void x86_f65535_destroy(x86_f65535 *chip)
{
    lib_release(chip);
}

void x86_f65535_reset(x86_f65535 *chip)
{
    if (chip == LIB_NULL) return;
    chip->extension_index = 0u;
    lib_memory_set(chip->extensions, 0, sizeof(chip->extensions));
    lib_memory_set(chip->vram, 0, sizeof(chip->vram));
}

lib_status x86_f65535_extension_read(x86_f65535 *chip, lib_u16 port,
    lib_u8 *out_value)
{
    if (chip == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_F65535_EXTENSION_INDEX_PORT) {
        *out_value = chip->extension_index;
        return LIB_STATUS_OK;
    }
    if (port != X86_F65535_EXTENSION_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    *out_value = chip->extension_index == X86_F65535_EXTENSION_CHIP_VERSION ?
        X86_F65535_PC110_CHIP_VERSION : chip->extensions[chip->extension_index];
    return LIB_STATUS_OK;
}

lib_status x86_f65535_extension_write(x86_f65535 *chip, lib_u16 port,
    lib_u8 value)
{
    if (chip == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == X86_F65535_EXTENSION_INDEX_PORT) {
        chip->extension_index = value & 0x7fu;
        return LIB_STATUS_OK;
    }
    if (port != X86_F65535_EXTENSION_DATA_PORT) return LIB_STATUS_UNSUPPORTED;
    if (chip->extension_index != X86_F65535_EXTENSION_CHIP_VERSION)
        chip->extensions[chip->extension_index] = value;
    return LIB_STATUS_OK;
}

lib_status x86_f65535_vram_read(const x86_f65535 *chip, lib_u32 offset,
    void *destination, lib_size bytes)
{
    if (chip == LIB_NULL || destination == LIB_NULL ||
        !x86_f65535_vram_contains(offset, bytes)) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(destination, chip->vram + offset, bytes);
    return LIB_STATUS_OK;
}

lib_status x86_f65535_vram_write(x86_f65535 *chip, lib_u32 offset,
    const void *source, lib_size bytes)
{
    if (chip == LIB_NULL || source == LIB_NULL ||
        !x86_f65535_vram_contains(offset, bytes)) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(chip->vram + offset, source, bytes);
    return LIB_STATUS_OK;
}
