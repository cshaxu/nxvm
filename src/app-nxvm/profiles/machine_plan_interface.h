#include "lib/types/types_interface.h"
#ifndef VM_PROFILE_MACHINE_PLAN_INTERFACE_H
#define VM_PROFILE_MACHINE_PLAN_INTERFACE_H

#include "app-nxvm/profiles/selection_interface.h"

#include "app-nxvm/devices/firmware_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/profiles/device/floppy.h"

typedef struct vm_profile_machine_plan vm_profile_machine_plan;
typedef struct vm_profile_model40_external_rom vm_profile_model40_external_rom;

type_status vm_profile_machine_plan_create(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan);
type_status vm_profile_machine_plan_create_file_backed(const vm_machine_config *config,
    vm_profile_machine_plan **out_plan);
C_VOID vm_profile_machine_plan_destroy(vm_profile_machine_plan *plan);

const core_machine_config *vm_profile_machine_plan_core_config_get(
    const vm_profile_machine_plan *plan);
const core_machine_controller_timing_rules *
vm_profile_machine_plan_timing_rules_get(const vm_profile_machine_plan *plan);
const core_machine_plan_topology *vm_profile_machine_plan_topology_get(
    const vm_profile_machine_plan *plan);
const core_machine_firmware_provider *vm_profile_machine_plan_firmware_provider_get(
    const vm_profile_machine_plan *plan);
C_VOID *vm_profile_machine_plan_firmware_context_get(
    vm_profile_machine_plan *plan);
vm_profile_floppy_kind vm_profile_machine_plan_drive_floppy_get(
    const vm_profile_machine_plan *plan);
vm_profile_floppy_kind vm_profile_machine_plan_media_floppy_get(
    const vm_profile_machine_plan *plan);
type_bool vm_profile_machine_plan_hdc_present(const vm_profile_machine_plan *plan);
type_bool vm_profile_machine_plan_memory_reconfigurable(
    const vm_profile_machine_plan *plan);
lib_u8 vm_profile_machine_plan_floppy_slot_count(
    const vm_profile_machine_plan *plan);
const vm_profile_model40_external_rom *vm_profile_machine_plan_model40_rom_get(
    const vm_profile_machine_plan *plan);
type_bool vm_profile_machine_plan_is_model40(const vm_profile_machine_plan *plan);
type_bool vm_profile_machine_plan_external_firmware(const vm_profile_machine_plan *plan);
type_status vm_profile_machine_plan_copy_cmos_seed(const vm_profile_machine_plan *plan,
    lib_u8 *out_seed, type_bool *out_present);
type_status vm_profile_machine_plan_copy_text_glyphs(const vm_profile_machine_plan *plan,
    core_machine_vadp_text_glyph_config *out_glyphs);
type_status vm_profile_machine_plan_materialize(vm_profile_machine_plan *plan,
    core_machine_plan *core_plan,
    core_machine_fdc_terminal_observation_provider terminal_observation);
type_bool vm_profile_machine_plan_hdd_geometry_get(const vm_profile_machine_plan *plan,
    lib_u16 *out_cylinders, lib_u8 *out_heads,
    lib_u8 *out_sectors);

#endif
