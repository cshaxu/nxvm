/* Copyright 2012-2026 Neko. */
#ifndef X86_CT_VGA_INTERFACE_H
#define X86_CT_VGA_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-nxvm/chips/video/video_values_interface.h"

typedef struct x86_ct_vga x86_ct_vga;

typedef enum x86_ct_vga_variant {
    X86_CT_VGA_65535,
    X86_CT_VGA_65510
} x86_ct_vga_variant;

#define X86_CT_VGA_VRAM_BYTES (512u * 1024u)
#define X86_CT_VGA_EXTENSION_INDEX_PORT 0x03d6u
#define X86_CT_VGA_EXTENSION_DATA_PORT 0x03d7u
#define X86_CT_VGA_EXTENSION_REGISTER_COUNT 128u
#define X86_CT_VGA_EXTENSION_CHIP_VERSION 0x00u
#define X86_CT_VGA_65535_CHIP_VERSION 0xc1u
#define X86_CT_VGA_65510_CHIP_VERSION 0x90u

/* One execution owner. The component owns only controller-local register and
 * VRAM state; a board owns CPU port/memory route selection and presentation.
 * The 65510 projects standard VGA text and planar graphics onto a
 * monochrome flat panel. */
lib_status x86_ct_vga_create(x86_ct_vga_variant variant, x86_ct_vga **out_chip);
void x86_ct_vga_destroy(x86_ct_vga *chip);
void x86_ct_vga_reset(x86_ct_vga *chip);

/* The extension index has seven usable bits. XR00 is read-only and identifies
 * the selected variant. The 65510 models revision zero. */
lib_status x86_ct_vga_extension_read(x86_ct_vga *chip, lib_u16 port,
    lib_u8 *out_value);
lib_status x86_ct_vga_extension_write(x86_ct_vga *chip, lib_u16 port,
    lib_u8 value);

/* L2 standard-VGA compatibility surface.  CT_VGA's standard VGA identity is
 * directly sourced; the precise internal VGA implementation is a bounded
 * compatibility model.  The Board still owns guest port and aperture routes. */
lib_status x86_ct_vga_vga_read(x86_ct_vga *chip, lib_u16 port,
    lib_u8 *out_value);
lib_status x86_ct_vga_vga_write(x86_ct_vga *chip, lib_u16 port,
    lib_u8 value);

/* Offsets are within the controller's 512 KiB logical display memory. */
lib_status x86_ct_vga_vram_read(const x86_ct_vga *chip, lib_u32 offset,
    void *destination, lib_size bytes);
lib_status x86_ct_vga_vram_write(x86_ct_vga *chip, lib_u32 offset,
    const void *source, lib_size bytes);

/* VGA CPU aperture access, including 65510 legacy bank selection. Reads
 * load the four hardware latches unless observe_only is true. */
lib_status x86_ct_vga_memory_query(const x86_ct_vga *chip, lib_u32 physical,
    lib_size bytes, lib_bool write);
lib_status x86_ct_vga_memory_read(x86_ct_vga *chip, lib_u32 physical,
    void *destination, lib_size bytes, lib_bool observe_only);
lib_status x86_ct_vga_memory_write(x86_ct_vga *chip, lib_u32 physical,
    const void *source, lib_size bytes);
/* vsync_count is the board-supplied guest vertical-sync count. */
lib_status x86_ct_vga_capture_snapshot(x86_ct_vga *chip, lib_u64 vsync_count,
    x86_video_snapshot *out_snapshot);

#endif
