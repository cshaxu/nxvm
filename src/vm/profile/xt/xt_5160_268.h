#ifndef VM_PROFILE_XT_5160_268_H
#define VM_PROFILE_XT_5160_268_H

#include "vm/profile/profile_resolver_interface.h"
#include "core/machine/firmware_interface.h"

#define VM_PROFILE_XT_5160_268_FDD_MEDIA_ID 1u
#define VM_PROFILE_XT_5160_268_HDD_MEDIA_ID 2u
#define VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES (64u * 1024u)
#define VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES (8u * 1024u)
#define VM_PROFILE_XT_5160_268_SYSTEM_ROM_PHYSICAL_START 0x000f0000u
#define VM_PROFILE_XT_5160_268_XEBEC_ROM_PHYSICAL_START 0x000c8000u

typedef struct vm_profile_xt_5160_268_byob_manifest {
    const C_CHAR *system_path;
    const C_CHAR *system_sha256;
    const C_CHAR *xebec_path;
    const C_CHAR *xebec_sha256;
    const C_CHAR *provenance;
} vm_profile_xt_5160_268_byob_manifest;

typedef struct vm_profile_xt_5160_268_external_rom {
    const type_unsigned_8 *system_bytes;
    const type_unsigned_8 *xebec_bytes;
    const type_unsigned_8 *video_bytes;
    STD_SIZE_T video_byte_count;
    type_bool xebec_present;
} vm_profile_xt_5160_268_external_rom;

/* This is a construction-only snapshot.  The copied Core topology records
 * only B2-selected shared board facts; later device batches own their own
 * source-qualified additions. */
typedef struct vm_profile_xt_5160_268_resolved_profile {
    vm_resolved_profile resolved;
    core_machine_plan_topology topology;
} vm_profile_xt_5160_268_resolved_profile;

type_status vm_profile_xt_5160_268_declaration_create(
    vm_profile_resolver_declaration *out_declaration);
type_status vm_profile_xt_5160_268_resolve(
    vm_profile_xt_5160_268_resolved_profile *out_profile,
    type_bool xebec_rom_present);
C_INT vm_profile_xt_5160_268_byob_manifest_is_valid(
    const vm_profile_xt_5160_268_byob_manifest *manifest);
type_status vm_profile_xt_5160_268_byob_manifest_load(
    const vm_profile_xt_5160_268_byob_manifest *manifest,
    type_unsigned_8 *system_bytes, type_unsigned_8 *xebec_bytes,
    vm_profile_xt_5160_268_external_rom *out_rom);
type_status vm_profile_xt_5160_268_external_rom_create(
    const type_unsigned_8 *system, STD_SIZE_T system_bytes,
    const type_unsigned_8 *xebec, STD_SIZE_T xebec_bytes,
    const type_unsigned_8 *video, STD_SIZE_T video_bytes,
    vm_profile_xt_5160_268_external_rom *out_rom);
const core_machine_firmware_provider *vm_profile_xt_5160_268_firmware_provider(C_VOID);

#endif
