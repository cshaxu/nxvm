#include "app-nxvm/profiles/machine_plan.h"
#include "app-nxvm/profiles/model40/composition_interface.h"
#include "app-nxvm/profiles/model40/model40_private.h"

typedef struct vm_profile_model40_machine_plan {
    vm_profile_machine_plan common;
    vm_profile_contract_values profile;
    struct {
        lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
        lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
        lib_u8 video[VM_PROFILE_MODEL40_VIDEO_ROM_BYTES];
        vm_profile_model40_external_rom context;
        /* Borrowed only while Core's sole board attachment is alive. */
        core_machine_d4_platform *board;
        core_machine_fdc_terminal_observation fdc_terminal;
        lib_bool fdc_terminal_valid;
    } firmware;
} vm_profile_model40_machine_plan;

static lib_status vm_profile_machine_plan_model40(vm_profile_model40_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    vm_profile_model40_external_rom source;

    if (config->bios_count != 2u ||
        (config->memory_bytes != 0u && config->memory_bytes != 1024u * 1024u) ||
        vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_525_1200K, LIB_TRUE,
            &plan->common.construction.media_kind) != LIB_STATUS_OK ||
        vm_profile_model40_values_create(&plan->profile) != LIB_STATUS_OK ||
        vm_profile_model40_external_rom_create(assets->bios[0u].data, assets->bios[0u].bytes,
            assets->bios[1u].data, assets->bios[1u].bytes, assets->video.data,
            assets->video.bytes, &source) != LIB_STATUS_OK ||
        vm_profile_machine_plan_copy(plan->firmware.even,
            VM_PROFILE_MODEL40_ROM_CHIP_BYTES, assets->bios[0u]) != LIB_STATUS_OK ||
        vm_profile_machine_plan_copy(plan->firmware.odd,
            VM_PROFILE_MODEL40_ROM_CHIP_BYTES, assets->bios[1u]) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (assets->video.data != LIB_NULL) lib_memory_copy(plan->firmware.video,
        assets->video.data, VM_PROFILE_MODEL40_VIDEO_ROM_BYTES);
    plan->firmware.context = source;
    plan->firmware.context.even_bytes = plan->firmware.even;
    plan->firmware.context.odd_bytes = plan->firmware.odd;
    plan->firmware.context.video_bytes = assets->video.data == LIB_NULL ? LIB_NULL :
        plan->firmware.video;
    plan->common.construction.firmware_provider = vm_profile_model40_firmware_provider();
    plan->common.construction.firmware_context = &plan->firmware.context;
    plan->common.construction.core_config = plan->profile.core.configuration;
    plan->common.construction.timing_rules = plan->profile.core.controller_timing_rules;
    if (vm_profile_model40_topology_materialize(&plan->common.construction.topology) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->common.construction.floppy_kind = VM_PROFILE_FLOPPY_525_1200K;
    plan->common.construction.floppy_slot_count = 2u;
    plan->common.construction.hdc_present = LIB_TRUE;
    return LIB_STATUS_OK;
}

static void vm_profile_machine_plan_capture_fdc_terminal(void *context,
    const core_machine_fdc_terminal_observation *observation)
{
    vm_profile_model40_machine_plan *plan = context;

    if (plan == LIB_NULL || observation == LIB_NULL) return;
    plan->firmware.fdc_terminal = *observation;
    plan->firmware.fdc_terminal_valid = LIB_TRUE;
}

static lib_status vm_profile_model40_configure(void *context, core_machine_plan *core_plan)
{
    vm_profile_model40_machine_plan *plan = context;

    return vm_profile_model40_materialize_plan(core_plan,
        (core_machine_fdc_terminal_observation_provider) {
            vm_profile_machine_plan_capture_fdc_terminal, plan }, &plan->firmware.board);
}

static void vm_profile_model40_notify(void *context, vm_machine_profile_event event)
{
    vm_profile_model40_machine_plan *plan = context;

    if (event == VM_MACHINE_PROFILE_BOARD_DETACHED) plan->firmware.board = LIB_NULL;
    plan->firmware.fdc_terminal_valid = LIB_FALSE;
}

static void vm_profile_model40_release(void *context)
{
    lib_release(context);
}

lib_status vm_profile_machine_plan_create_model40(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan)
{
    vm_profile_model40_machine_plan *plan;
    lib_status status = vm_profile_machine_plan_validate(config, assets, out_plan);

    if (status != LIB_STATUS_OK) return status;
    plan = (vm_profile_model40_machine_plan *)lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    plan->common.construction.profile = (vm_machine_profile_binding) {
        plan, vm_profile_model40_configure, vm_profile_model40_notify, vm_profile_model40_release };
    plan->common.construction.fixed_geometry = LIB_TRUE;
    plan->common.construction.cylinders = 925u;
    plan->common.construction.heads = 5u;
    plan->common.construction.sectors = 17u;
    status = vm_profile_machine_plan_model40(plan, config, assets);
    return vm_profile_machine_plan_publish(&plan->common, config, assets, status, out_plan);
}

lib_u8 vm_profile_machine_plan_is_model40(const vm_profile_machine_plan *plan)
{
    return plan != LIB_NULL &&
        plan->construction.firmware_provider == vm_profile_model40_firmware_provider();
}

const vm_profile_model40_external_rom *vm_profile_machine_plan_model40_rom_get(
    const vm_profile_machine_plan *plan)
{
    const vm_profile_model40_machine_plan *model40 = (const vm_profile_model40_machine_plan *)plan;

    return !vm_profile_machine_plan_is_model40(plan) ? LIB_NULL : &model40->firmware.context;
}

lib_status vm_profile_machine_plan_observe_model40(
    const vm_profile_machine_plan *plan, vm_profile_model40_observation *out_observation)
{
    const vm_profile_model40_machine_plan *model40 = (const vm_profile_model40_machine_plan *)plan;
    vm_profile_model40_observation observation = {0};
    lib_status status;

    if (plan == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!vm_profile_machine_plan_is_model40(plan)) return LIB_STATUS_UNSUPPORTED;
    status = core_machine_d4_platform_observe(model40->firmware.board, &observation.d4);
    if (status != LIB_STATUS_OK) return status;
    observation.fdc_terminal = model40->firmware.fdc_terminal;
    observation.fdc_terminal_valid = model40->firmware.fdc_terminal_valid;
    *out_observation = observation;
    return LIB_STATUS_OK;
}
