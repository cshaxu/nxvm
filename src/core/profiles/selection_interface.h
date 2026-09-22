#ifndef VM_PROFILE_SELECTION_INTERFACE_H
#define VM_PROFILE_SELECTION_INTERFACE_H

#include "type.h"

#include "core/devices/cpu_interface.h"
#include "core/devices/fpu_interface.h"
#include "lib/storage/medium_interface.h"

typedef enum vm_machine_profile_kind {
    VM_MACHINE_PROFILE_DEFAULT_PC_AT,
    VM_MACHINE_PROFILE_IBM_5170_MODEL_339,
    VM_MACHINE_PROFILE_IBM_5160_MODEL_268,
    VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
} vm_machine_profile_kind;

const C_CHAR *vm_profile_name(vm_machine_profile_kind kind);

typedef enum vm_machine_floppy_format {
    VM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT,
    VM_MACHINE_FLOPPY_FORMAT_360K,
    VM_MACHINE_FLOPPY_FORMAT_720K,
    VM_MACHINE_FLOPPY_FORMAT_1200K,
    VM_MACHINE_FLOPPY_FORMAT_1440K
} vm_machine_floppy_format;

#define VM_MACHINE_FLOPPY_SLOT_COUNT 2u
#define VM_MACHINE_FIXED_DISK_SLOT_COUNT 2u
#define VM_MACHINE_CMOS_SEED_BYTES 64u
#define VM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES (2u * 1024u)
#define VM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES \
    (4u * VM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES)

typedef struct vm_machine_config {
    vm_machine_profile_kind profile_kind;
    STD_SIZE_T memory_bytes;
    const C_CHAR *floppy_image[VM_MACHINE_FLOPPY_SLOT_COUNT];
    const C_CHAR *fixed_disk_image[VM_MACHINE_FIXED_DISK_SLOT_COUNT];
    lib_storage_medium_mode floppy_mode[VM_MACHINE_FLOPPY_SLOT_COUNT];
    lib_storage_medium_mode fixed_disk_mode[VM_MACHINE_FIXED_DISK_SLOT_COUNT];
    const C_CHAR *cmos_seed;
    const C_CHAR *font_path;
    const C_CHAR *bios_path[2];
    STD_SIZE_T bios_count;
    const C_CHAR *video_path;
    vm_machine_floppy_format floppy_format;
    C_INT create_fdd;
    type_unsigned_16 create_hdd_cylinders;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
} vm_machine_config;

typedef struct vm_machine_asset_bytes {
    const type_unsigned_8 *data;
    STD_SIZE_T bytes;
} vm_machine_asset_bytes;

typedef struct vm_machine_assets {
    vm_machine_asset_bytes bios[2];
    vm_machine_asset_bytes video;
    vm_machine_asset_bytes cmos_seed;
    vm_machine_asset_bytes font;
} vm_machine_assets;

#endif
