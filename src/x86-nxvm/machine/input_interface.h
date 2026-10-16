#ifndef X86_NXVM_MACHINE_INPUT_INTERFACE_H
#define X86_NXVM_MACHINE_INPUT_INTERFACE_H

#include "x86-nxvm/machine/construction_interface.h"

typedef enum x86_nxvm_machine_floppy_format {
    X86_NXVM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT,
    X86_NXVM_MACHINE_FLOPPY_FORMAT_360K,
    X86_NXVM_MACHINE_FLOPPY_FORMAT_720K,
    X86_NXVM_MACHINE_FLOPPY_FORMAT_1200K,
    X86_NXVM_MACHINE_FLOPPY_FORMAT_1440K
} x86_nxvm_machine_floppy_format;

#define X86_NXVM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES (2u * 1024u)
#define X86_NXVM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES \
    (4u * X86_NXVM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES)

/* Construction values and runtime media requests contain no machine identity.
 * The composition root supplies one constructor; byte views are borrowed only
 * during preparation and copied by the selected Profile before publication. */
typedef struct x86_nxvm_machine_config {
    lib_size memory_bytes;
    const char *floppy_image[X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT];
    const char *fixed_disk_image[X86_NXVM_MACHINE_FIXED_DISK_SLOT_COUNT];
    lib_storage_medium_mode floppy_mode[X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT];
    lib_storage_medium_mode fixed_disk_mode[X86_NXVM_MACHINE_FIXED_DISK_SLOT_COUNT];
    lib_size bios_count;
    x86_nxvm_machine_floppy_format floppy_format;
    lib_i32 create_fdd;
    lib_u16 create_hdd_cylinders;
    x86_core_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
} x86_nxvm_machine_config;

typedef struct x86_nxvm_machine_asset_bytes {
    const lib_u8 *data;
    lib_size bytes;
} x86_nxvm_machine_asset_bytes;

typedef struct x86_nxvm_machine_assets {
    x86_nxvm_machine_asset_bytes bios[2];
    x86_nxvm_machine_asset_bytes video;
    x86_nxvm_machine_asset_bytes cmos_seed;
    x86_nxvm_machine_asset_bytes font;
} x86_nxvm_machine_assets;

#endif
