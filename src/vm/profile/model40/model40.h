#ifndef VM_PROFILE_MODEL40_H
#define VM_PROFILE_MODEL40_H

#include "type.h"
#include "core/machine/firmware_interface.h"
#include "core/machine/memory_interface.h"

#define VM_PROFILE_MODEL40_ROM_CHIP_BYTES (16u * 1024u)
#define VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES (2u * VM_PROFILE_MODEL40_ROM_CHIP_BYTES)
#define VM_PROFILE_MODEL40_ROM_WINDOW_BYTES (2u * VM_PROFILE_MODEL40_ROM_LOGICAL_BYTES)
#define VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START 0x000f0000u
#define VM_PROFILE_MODEL40_ROM_HIGH_PHYSICAL_START 0xffef0000u

#define VM_PROFILE_MODEL40_D4_COMPATIBILITY_START 0x00fe0000u
#define VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES (128u * 1024u)
#define VM_PROFILE_MODEL40_D4_REPLACEMENT_START 0x000e0000u
#define VM_PROFILE_MODEL40_D4_REPLACEMENT_BYTES VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES
#define VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL 0x80c00000u

typedef struct vm_profile_model40_d4_memory {
    type_unsigned_8 compatibility[VM_PROFILE_MODEL40_D4_COMPATIBILITY_BYTES];
    type_unsigned_8 control;
} vm_profile_model40_d4_memory;
typedef struct vm_profile_model40_external_rom {
    const type_unsigned_8 *even_bytes;
    const type_unsigned_8 *odd_bytes;
    STD_SIZE_T chip_byte_count;
} vm_profile_model40_external_rom;

/* A local owner-supplied manifest is transient composition input only. */
typedef struct vm_profile_model40_byob_manifest {
    const C_CHAR *even_path;
    const C_CHAR *even_sha256;
    const C_CHAR *odd_path;
    const C_CHAR *odd_sha256;
    const C_CHAR *provenance;
} vm_profile_model40_byob_manifest;

C_INT vm_profile_model40_external_rom_is_valid(
    const vm_profile_model40_external_rom *rom);
C_INT vm_profile_model40_byob_manifest_is_valid(
    const vm_profile_model40_byob_manifest *manifest);
type_status vm_profile_model40_byob_manifest_load(
    const vm_profile_model40_byob_manifest *manifest,
    type_unsigned_8 *even_bytes, type_unsigned_8 *odd_bytes,
    vm_profile_model40_external_rom *out_rom);
const core_machine_firmware_provider *vm_profile_model40_firmware_provider(C_VOID);
C_VOID vm_profile_model40_d4_memory_initialize(vm_profile_model40_d4_memory *memory);
C_VOID vm_profile_model40_d4_memory_reset(vm_profile_model40_d4_memory *memory);
type_status vm_profile_model40_d4_memory_register(core_machine *machine,
    vm_profile_model40_d4_memory *memory);

#endif
