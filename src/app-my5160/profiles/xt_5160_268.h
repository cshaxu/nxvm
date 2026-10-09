#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#ifndef VM_PROFILE_XT_5160_268_H
#define VM_PROFILE_XT_5160_268_H

#include "core/board-base/profile_contract_interface.h"
#include "core/x86/firmware_interface.h"

#define VM_PROFILE_XT_5160_268_FDD_MEDIA_ID 1u
#define VM_PROFILE_XT_5160_268_HDD_MEDIA_ID 2u
#define VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES (64u * 1024u)
#define VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES (8u * 1024u)
#define VM_PROFILE_XT_5160_268_SYSTEM_ROM_PHYSICAL_START 0x000f0000u
#define VM_PROFILE_XT_5160_268_XEBEC_ROM_PHYSICAL_START 0x000c8000u

typedef struct vm_profile_xt_5160_268_external_rom {
    const lib_u8 *system_bytes;
    const lib_u8 *xebec_bytes;
    const lib_u8 *video_bytes;
    lib_size video_byte_count;
    lib_u8 xebec_present;
} vm_profile_xt_5160_268_external_rom;

/* This is a construction-only snapshot.  The copied Core topology records
 * only B2-selected shared board facts; later device batches own their own
 * source-qualified additions. */
typedef struct vm_profile_xt_5160_268_plan_snapshot {
    vm_profile_contract_values values;
    core_machine_plan_topology topology;
} vm_profile_xt_5160_268_plan_snapshot;

lib_status vm_profile_xt_5160_268_values_create(
    vm_profile_contract_values *out_values);
lib_status vm_profile_xt_5160_268_plan_create(
    vm_profile_xt_5160_268_plan_snapshot *out_profile,
    lib_u8 xebec_rom_present);
lib_status vm_profile_xt_5160_268_external_rom_create(
    const lib_u8 *system, lib_size system_bytes,
    const lib_u8 *xebec, lib_size xebec_bytes,
    const lib_u8 *video, lib_size video_bytes,
    vm_profile_xt_5160_268_external_rom *out_rom);
const core_machine_firmware_provider *vm_profile_xt_5160_268_firmware_provider(void);

#endif
