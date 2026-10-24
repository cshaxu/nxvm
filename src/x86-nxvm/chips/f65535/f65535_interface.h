/* Copyright 2012-2026 Neko. */
#ifndef X86_F65535_INTERFACE_H
#define X86_F65535_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct x86_f65535 x86_f65535;

#define X86_F65535_VRAM_BYTES (512u * 1024u)
#define X86_F65535_EXTENSION_INDEX_PORT 0x03d6u
#define X86_F65535_EXTENSION_DATA_PORT 0x03d7u
#define X86_F65535_EXTENSION_REGISTER_COUNT 128u
#define X86_F65535_EXTENSION_CHIP_VERSION 0x00u
#define X86_F65535_PC110_CHIP_VERSION 0xc1u

/* One execution owner. The component owns only controller-local register and
 * VRAM state; a board owns CPU port/memory route selection and presentation.
 * Normal VGA/panel scanout remains a later receiver. */
lib_status x86_f65535_create(x86_f65535 **out_chip);
void x86_f65535_destroy(x86_f65535 *chip);
void x86_f65535_reset(x86_f65535 *chip);

/* The extension index has seven usable bits. XR00 is read-only and reports
 * the PC110-observed C1h revision; other extension values are retained only
 * as register state, not interpreted as panel or bus behavior. */
lib_status x86_f65535_extension_read(x86_f65535 *chip, lib_u16 port,
    lib_u8 *out_value);
lib_status x86_f65535_extension_write(x86_f65535 *chip, lib_u16 port,
    lib_u8 value);

/* Offsets are within the controller's 512 KiB logical display memory. */
lib_status x86_f65535_vram_read(const x86_f65535 *chip, lib_u32 offset,
    void *destination, lib_size bytes);
lib_status x86_f65535_vram_write(x86_f65535 *chip, lib_u32 offset,
    const void *source, lib_size bytes);

#endif
