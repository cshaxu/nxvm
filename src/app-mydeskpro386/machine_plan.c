#include "core/machine/preparation_interface.h"
#include "app-mydeskpro386/machine_plan_interface.h"
#include "app-mydeskpro386/machine_construction_interface.h"
#include "app-mydeskpro386/machine_config.h"
#include "app-mydeskpro386/machine_observation_interface.h"

typedef struct mydeskpro386_machine_plan {
    vm_machine_construction construction;
    vm_profile_contract_values profile;
    struct {
        lib_u8 even[MYDESKPRO386_ROM_CHIP_BYTES];
        lib_u8 odd[MYDESKPRO386_ROM_CHIP_BYTES];
        lib_u8 video[MYDESKPRO386_VIDEO_ROM_BYTES];
        mydeskpro386_external_rom context;
        /* Borrowed only while Core's sole board attachment is alive. */
        mydeskpro386_d4_platform *board;
        core_machine_fdc_terminal_observation fdc_terminal;
        lib_bool fdc_terminal_valid;
    } firmware;
} mydeskpro386_machine_plan;

static lib_status vm_profile_machine_plan_model40(mydeskpro386_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    mydeskpro386_external_rom source;

    if (config->bios_count != 2u ||
        (config->memory_bytes != 0u && config->memory_bytes != 1024u * 1024u) ||
        vm_machine_floppy_select(config, VM_PROFILE_FLOPPY_525_1200K,
            (1u << VM_PROFILE_FLOPPY_525_1200K) | (1u << VM_PROFILE_FLOPPY_525_360K),
            &plan->construction.media_kind) != LIB_STATUS_OK ||
        mydeskpro386_values_create(&plan->profile) != LIB_STATUS_OK ||
        mydeskpro386_external_rom_create(assets->bios[0u].data, assets->bios[0u].bytes,
            assets->bios[1u].data, assets->bios[1u].bytes, assets->video.data,
            assets->video.bytes, &source) != LIB_STATUS_OK ||
        vm_machine_asset_copy(plan->firmware.even,
            MYDESKPRO386_ROM_CHIP_BYTES, assets->bios[0u]) != LIB_STATUS_OK ||
        vm_machine_asset_copy(plan->firmware.odd,
            MYDESKPRO386_ROM_CHIP_BYTES, assets->bios[1u]) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (assets->video.data != LIB_NULL && vm_machine_asset_copy(plan->firmware.video,
        source.video_byte_count, assets->video) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    plan->firmware.context = source;
    plan->firmware.context.even_bytes = plan->firmware.even;
    plan->firmware.context.odd_bytes = plan->firmware.odd;
    plan->firmware.context.video_bytes = assets->video.data == LIB_NULL ? LIB_NULL :
        plan->firmware.video;
    plan->construction.firmware_provider = mydeskpro386_firmware_provider();
    plan->construction.firmware_context = &plan->firmware.context;
    plan->construction.core_config = plan->profile.core.configuration;
    plan->construction.timing_rules = plan->profile.core.controller_timing_rules;
    if (mydeskpro386_topology_materialize(&plan->construction.topology) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->construction.floppy_kind = VM_PROFILE_FLOPPY_525_1200K;
    plan->construction.floppy_slot_count = 2u;
    plan->construction.hdc_present = LIB_TRUE;
    return LIB_STATUS_OK;
}

static void vm_profile_machine_plan_capture_fdc_terminal(void *context,
    const core_machine_fdc_terminal_observation *observation)
{
    mydeskpro386_machine_plan *plan = context;

    if (plan == LIB_NULL || observation == LIB_NULL) return;
    plan->firmware.fdc_terminal = *observation;
    plan->firmware.fdc_terminal_valid = LIB_TRUE;
}

static lib_status mydeskpro386_configure(void *context, core_machine_plan *core_plan)
{
    mydeskpro386_machine_plan *plan = context;

    return mydeskpro386_materialize_plan(core_plan,
        (core_machine_fdc_terminal_observation_provider) {
            vm_profile_machine_plan_capture_fdc_terminal, plan }, &plan->firmware.board);
}

static void mydeskpro386_notify(void *context, vm_machine_profile_event event)
{
    mydeskpro386_machine_plan *plan = context;

    if (event == VM_MACHINE_PROFILE_BOARD_DETACHED) plan->firmware.board = LIB_NULL;
    plan->firmware.fdc_terminal_valid = LIB_FALSE;
}

static void mydeskpro386_release(void *context)
{
    lib_release(context);
}

lib_status mydeskpro386_machine_plan_create(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction)
{
    mydeskpro386_machine_plan *plan;
    lib_status status = vm_machine_construction_begin(config, assets, out_construction);

    if (status != LIB_STATUS_OK) return status;
    plan = (mydeskpro386_machine_plan *)lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    plan->construction.profile = (vm_machine_profile_binding) {
        plan, mydeskpro386_configure, mydeskpro386_notify, mydeskpro386_release };
    plan->construction.fixed_geometry = LIB_TRUE;
    plan->construction.cylinders = 925u;
    plan->construction.heads = 5u;
    plan->construction.sectors = 17u;
    status = vm_profile_machine_plan_model40(plan, config, assets);
    return vm_machine_construction_finish(&plan->construction, config, assets, status, out_construction);
}

lib_status mydeskpro386_observe(
    const vm_machine_construction *construction, mydeskpro386_observation *out_observation)
{
    const mydeskpro386_machine_plan *model40;
    mydeskpro386_observation observation = {0};
    lib_status status;

    if (construction == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (construction->firmware_provider != mydeskpro386_firmware_provider())
        return LIB_STATUS_UNSUPPORTED;
    model40 = construction->profile.context;
    status = mydeskpro386_d4_platform_observe(model40->firmware.board, &observation.d4);
    if (status != LIB_STATUS_OK) return status;
    observation.fdc_terminal = model40->firmware.fdc_terminal;
    observation.fdc_terminal_valid = model40->firmware.fdc_terminal_valid;
    *out_observation = observation;
    return LIB_STATUS_OK;
}
