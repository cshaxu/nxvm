#ifndef VM_PROFILE_SELECTION_INTERFACE_H
#define VM_PROFILE_SELECTION_INTERFACE_H
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/cpu_interface.h"
#include "app-nxvm/devices/fpu_interface.h"
#include "lib/storage/medium_interface.h"

typedef enum vm_machine_profile_kind {
    VM_MACHINE_PROFILE_DEFAULT_PC_AT,
    VM_MACHINE_PROFILE_IBM_5170_MODEL_339,
    VM_MACHINE_PROFILE_IBM_5160_MODEL_268,
    VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
} vm_machine_profile_kind;

const char *vm_profile_name(vm_machine_profile_kind kind);

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
    lib_size memory_bytes;
    const char *floppy_image[VM_MACHINE_FLOPPY_SLOT_COUNT];
    const char *fixed_disk_image[VM_MACHINE_FIXED_DISK_SLOT_COUNT];
    lib_storage_medium_mode floppy_mode[VM_MACHINE_FLOPPY_SLOT_COUNT];
    lib_storage_medium_mode fixed_disk_mode[VM_MACHINE_FIXED_DISK_SLOT_COUNT];
    const char *cmos_seed;
    const char *font_path;
    const char *bios_path[2];
    lib_size bios_count;
    const char *video_path;
    vm_machine_floppy_format floppy_format;
    lib_i32 create_fdd;
    lib_u16 create_hdd_cylinders;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
} vm_machine_config;

typedef struct vm_machine_asset_bytes {
    const lib_u8 *data;
    lib_size bytes;
} vm_machine_asset_bytes;

typedef struct vm_machine_assets {
    vm_machine_asset_bytes bios[2];
    vm_machine_asset_bytes video;
    vm_machine_asset_bytes cmos_seed;
    vm_machine_asset_bytes font;
} vm_machine_assets;

#endif
