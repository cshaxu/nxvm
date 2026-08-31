#ifndef VM_PROFILE_MODEL40_PRIVATE_H
#define VM_PROFILE_MODEL40_PRIVATE_H

#include "type.h"
#include "core/machine/firmware_interface.h"
#include "core/machine/machine_interface.h"
#include "vm/profile/model40/model40.h"
#include "vm/profile/profile_resolver_interface.h"

#define VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES (2u * VM_PROFILE_MODEL40_ROM_CHIP_BYTES)
#define VM_PROFILE_MODEL40_ROM_WINDOW_BYTES VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES
#define VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START 0x000f8000u
#define VM_PROFILE_MODEL40_ROM_COMPATIBILITY_ALIAS_START 0x000f0000u
#define VM_PROFILE_MODEL40_ROM_HIGH_ALIAS_START 0xffff0000u
#define VM_PROFILE_MODEL40_ROM_HIGH_RESET_ALIAS_START 0xffff8000u
#define VM_PROFILE_MODEL40_VIDEO_ROM_PHYSICAL_START 0x000c0000u

/* Test-visible board addresses; their runtime decoder is Core-owned. */
#define VM_PROFILE_MODEL40_D4_COMPATIBILITY_START 0x00fe0000u
#define VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL 0x80c00000u

typedef struct vm_profile_model40_external_rom {
    const type_unsigned_8 *even_bytes;
    const type_unsigned_8 *odd_bytes;
    STD_SIZE_T chip_byte_count;
    const type_unsigned_8 *video_bytes;
    STD_SIZE_T video_byte_count;
} vm_profile_model40_external_rom;

C_INT vm_profile_model40_external_rom_is_valid(
    const vm_profile_model40_external_rom *rom);
C_VOID vm_profile_model40_core_config_initialize(core_machine_config *out_config);
C_VOID vm_profile_model40_cmos_seed_materialize(core_machine_rtc_cmos_config *out_cmos);
type_status vm_profile_model40_child_declaration_create(
    const vm_profile_resolver_declaration *parent,
    vm_profile_resolver_declaration *out_declaration);
type_status vm_profile_model40_child_resolve(vm_resolved_profile *out_profile);
type_status vm_profile_model40_byob_manifest_load(
    const vm_profile_model40_byob_manifest *manifest,
    type_unsigned_8 *even_bytes, type_unsigned_8 *odd_bytes,
    type_unsigned_8 *video_bytes,
    vm_profile_model40_external_rom *out_rom);
const core_machine_firmware_provider *vm_profile_model40_firmware_provider(C_VOID);
#endif
