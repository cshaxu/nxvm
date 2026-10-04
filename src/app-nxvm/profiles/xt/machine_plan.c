#include "app-nxvm/profiles/machine_plan.h"
#include "app-nxvm/profiles/xt/xt_5160_268.h"

typedef struct vm_profile_xt_machine_plan {
    vm_profile_machine_plan common;
    vm_profile_xt_5160_268_plan_snapshot profile;
    struct {
        lib_u8 *system;
        lib_u8 *xebec;
        lib_u8 *video;
        vm_profile_xt_5160_268_external_rom context;
    } firmware;
} vm_profile_xt_machine_plan;

static lib_status vm_profile_machine_plan_xt(vm_profile_xt_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    vm_profile_xt_5160_268_external_rom source;

    if (config->bios_count == 0u || config->bios_count > 2u ||
        (assets->cmos_seed.data != LIB_NULL || assets->cmos_seed.bytes != 0u) || config->memory_bytes != 0u || config->create_fdd ||
        config->create_hdd_cylinders != 0u ||
        config->cpu_profile != CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        config->fpu_profile != X86_FPU_PROFILE_NONE ||
        vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_525_360K, LIB_FALSE,
            &plan->common.construction.media_kind) != LIB_STATUS_OK ||
        vm_profile_xt_5160_268_external_rom_create(assets->bios[0u].data,
            assets->bios[0u].bytes, config->bios_count == 2u ? assets->bios[1u].data : LIB_NULL,
            config->bios_count == 2u ? assets->bios[1u].bytes : 0u, assets->video.data,
            assets->video.bytes, &source) != LIB_STATUS_OK ||
        vm_profile_xt_5160_268_plan_create(&plan->profile, source.xebec_present) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->firmware.system = (lib_u8 *)lib_allocate(
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
    plan->firmware.xebec = (lib_u8 *)lib_allocate(
        VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES);
    plan->firmware.video = source.video_bytes == LIB_NULL ? LIB_NULL :
        (lib_u8 *)lib_allocate(source.video_byte_count);
    if (plan->firmware.system == LIB_NULL || plan->firmware.xebec == LIB_NULL ||
        (source.video_bytes != LIB_NULL && plan->firmware.video == LIB_NULL)) {
        return LIB_STATUS_NO_MEMORY;
    }
    lib_memory_copy(plan->firmware.system, source.system_bytes,
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
    if (source.xebec_present) lib_memory_copy(plan->firmware.xebec, source.xebec_bytes,
        VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES);
    if (source.video_bytes != LIB_NULL) lib_memory_copy(plan->firmware.video,
        source.video_bytes, source.video_byte_count);
    plan->firmware.context = source;
    plan->firmware.context.system_bytes = plan->firmware.system;
    plan->firmware.context.xebec_bytes = source.xebec_present ? plan->firmware.xebec : LIB_NULL;
    plan->firmware.context.video_bytes = source.video_bytes == LIB_NULL ? LIB_NULL :
        plan->firmware.video;
    plan->common.construction.firmware_provider = vm_profile_xt_5160_268_firmware_provider();
    plan->common.construction.firmware_context = &plan->firmware.context;
    plan->common.construction.core_config = plan->profile.values.core.configuration;
    plan->common.construction.timing_rules = plan->profile.values.core.controller_timing_rules;
    plan->common.construction.topology = plan->profile.topology;
    plan->common.construction.floppy_kind = VM_PROFILE_FLOPPY_525_360K;
    plan->common.construction.floppy_slot_count = 1u;
    plan->common.construction.hdc_present = source.xebec_present;
    return LIB_STATUS_OK;
}

static void vm_profile_xt_release(void *context)
{
    vm_profile_xt_machine_plan *plan = context;

    lib_release(plan->firmware.system);
    lib_release(plan->firmware.xebec);
    lib_release(plan->firmware.video);
    lib_release(plan);
}

lib_status vm_profile_machine_plan_create_xt(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan)
{
    vm_profile_xt_machine_plan *plan;
    lib_status status = vm_profile_machine_plan_validate(config, assets, out_plan);

    if (status != LIB_STATUS_OK) return status;
    plan = (vm_profile_xt_machine_plan *)lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    plan->common.construction.profile = (vm_machine_profile_binding) {
        plan, LIB_NULL, LIB_NULL, vm_profile_xt_release };
    status = vm_profile_machine_plan_xt(plan, config, assets);
    return vm_profile_machine_plan_publish(&plan->common, config, assets, status, out_plan);
}
