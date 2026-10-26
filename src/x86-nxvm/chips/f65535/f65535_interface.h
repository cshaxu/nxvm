/* Copyright 2012-2026 Neko. */
#ifndef X86_F65535_INTERFACE_H
#define X86_F65535_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-nxvm/chips/video/video_values_interface.h"

typedef struct x86_f65535 x86_f65535;

#define X86_F65535_VRAM_BYTES (512u * 1024u)
#define X86_F65535_EXTENSION_INDEX_PORT 0x03d6u
#define X86_F65535_EXTENSION_DATA_PORT 0x03d7u
#define X86_F65535_EXTENSION_REGISTER_COUNT 128u
#define X86_F65535_EXTENSION_CHIP_VERSION 0x00u
#define X86_F65535_PC110_CHIP_VERSION 0xc1u

/* One execution owner. The component owns only controller-local register and
 * VRAM state; a board owns CPU port/memory route selection and presentation.
 * Standard VGA text and the bounded mode-12 copied scanout are supported;
 * panel scanout remains a later receiver. */
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

/* L2 standard-VGA compatibility surface.  F65535's standard VGA identity is
 * directly sourced; the precise internal VGA implementation is a bounded
 * compatibility model.  The Board still owns guest port and aperture routes. */
lib_status x86_f65535_vga_read(x86_f65535 *chip, lib_u16 port,
    lib_u8 *out_value);
lib_status x86_f65535_vga_write(x86_f65535 *chip, lib_u16 port,
    lib_u8 value);

/* Offsets are within the controller's 512 KiB logical display memory. */
lib_status x86_f65535_vram_read(const x86_f65535 *chip, lib_u32 offset,
    void *destination, lib_size bytes);
lib_status x86_f65535_vram_write(x86_f65535 *chip, lib_u32 offset,
    const void *source, lib_size bytes);

/* Standard-VGA CPU aperture access and copied text/mode-12 capture. Reads
 * load the four hardware latches unless observe_only is true; inspection
 * returns the same bytes without changing controller state. Panel scanout
 * remains outside this compatibility receiver. */
lib_status x86_f65535_memory_query(const x86_f65535 *chip, lib_u32 physical,
    lib_size bytes, lib_bool write);
lib_status x86_f65535_memory_read(x86_f65535 *chip, lib_u32 physical,
    void *destination, lib_size bytes, lib_bool observe_only);
lib_status x86_f65535_memory_write(x86_f65535 *chip, lib_u32 physical,
    const void *source, lib_size bytes);
lib_status x86_f65535_capture_snapshot(x86_f65535 *chip,
    x86_video_snapshot *out_snapshot);

#endif
