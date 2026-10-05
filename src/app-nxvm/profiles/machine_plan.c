#include "app-nxvm/profiles/machine_plan.h"
#include "app-nxvm/profiles/selection_interface.h"

const char *vm_profile_name(vm_machine_profile_kind kind)
{
    if (kind == VM_MACHINE_PROFILE_DEFAULT_PC_AT) return "default-pc-at";
    if (kind == VM_MACHINE_PROFILE_IBM_5170_MODEL_339) {
        return "ibm-5170-model-339";
    }
    if (kind == VM_MACHINE_PROFILE_IBM_5160_MODEL_268) {
        return "ibm-5160-model-268";
    }
    if (kind == VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        return "compaq-deskpro-386-model-40";
    }
    return "unknown";
}

lib_status vm_profile_machine_plan_copy(lib_u8 *destination,
    lib_size expected, vm_machine_asset_bytes source)
{
    if (destination == LIB_NULL || source.data == LIB_NULL || source.bytes != expected) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_copy(destination, source.data, expected);
    return LIB_STATUS_OK;
}

static lib_status vm_profile_machine_plan_text_glyphs(vm_profile_machine_plan *plan,
    vm_machine_asset_bytes source)
{
    lib_size character;

    if (plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (source.data == LIB_NULL && source.bytes == 0u) return LIB_STATUS_OK;
    if (source.data == LIB_NULL || source.bytes != VM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    for (character = 0u; character < X86_VIDEO_TEXT_GLYPH_COUNT;
         ++character) {
        lib_memory_copy(&plan->construction.text_glyphs.bytes[character * X86_VIDEO_TEXT_GLYPH_ROWS],
            &source.data[character * 8u], 8u);
        lib_memory_copy(&plan->construction.text_glyphs.bytes[character * X86_VIDEO_TEXT_GLYPH_ROWS + 8u],
            &source.data[VM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES + character * 8u], 8u);
    }
    plan->construction.text_glyphs.present = LIB_TRUE;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_machine_plan_cmos(vm_profile_machine_plan *plan,
    vm_machine_asset_bytes source)
{
    if (plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (source.data == LIB_NULL && source.bytes == 0u) return LIB_STATUS_OK;
    if (vm_profile_machine_plan_copy(plan->construction.cmos_seed, VM_MACHINE_CMOS_SEED_BYTES,
            source) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    plan->construction.cmos_seed_present = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status vm_profile_machine_plan_floppy(const vm_machine_config *config,
    vm_profile_floppy_kind default_kind, lib_bool allow_360,
    vm_profile_floppy_kind *out_media)
{
    const vm_machine_floppy_format format = config == LIB_NULL ?
        VM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT : config->floppy_format;

    if (out_media == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (format == VM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT) {
        *out_media = default_kind;
        return LIB_STATUS_OK;
    }
    if ((default_kind == VM_PROFILE_FLOPPY_525_360K &&
            format == VM_MACHINE_FLOPPY_FORMAT_360K) ||
        (default_kind == VM_PROFILE_FLOPPY_525_1200K &&
            format == VM_MACHINE_FLOPPY_FORMAT_1200K)) {
        *out_media = default_kind;
        return LIB_STATUS_OK;
    }
    if (allow_360 && format == VM_MACHINE_FLOPPY_FORMAT_360K) {
        *out_media = VM_PROFILE_FLOPPY_525_360K;
        return LIB_STATUS_OK;
    }
    if (default_kind == VM_PROFILE_FLOPPY_35_1440K &&
        format == VM_MACHINE_FLOPPY_FORMAT_1440K) {
        *out_media = default_kind;
        return LIB_STATUS_OK;
    }
    if (default_kind == VM_PROFILE_FLOPPY_35_1440K &&
        format == VM_MACHINE_FLOPPY_FORMAT_720K) {
        *out_media = VM_PROFILE_FLOPPY_35_720K;
        return LIB_STATUS_OK;
    }
    if (default_kind == VM_PROFILE_FLOPPY_35_1440K &&
        format == VM_MACHINE_FLOPPY_FORMAT_1200K) {
        *out_media = VM_PROFILE_FLOPPY_525_1200K;
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_INVALID_ARGUMENT;
}

lib_status vm_profile_machine_plan_validate(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan)
{
    if (out_plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_plan = LIB_NULL;
    if (config == LIB_NULL || assets == LIB_NULL ||
        config->fixed_disk_image[1u] != LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_OK;
}

lib_status vm_profile_machine_plan_publish(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets,
    lib_status status, vm_profile_machine_plan **out_plan)
{
    if (status == LIB_STATUS_OK)
        status = vm_profile_machine_plan_cmos(plan, assets->cmos_seed);
    if (status == LIB_STATUS_OK)
        status = vm_profile_machine_plan_text_glyphs(plan, assets->font);
    if (status == LIB_STATUS_OK && config->floppy_image[1u] != LIB_NULL &&
        plan->construction.floppy_slot_count < 2u) status = LIB_STATUS_INVALID_ARGUMENT;
    if (status != LIB_STATUS_OK) {
        vm_profile_machine_plan_destroy(plan);
        return status;
    }
    *out_plan = plan;
    return LIB_STATUS_OK;
}

void vm_profile_machine_plan_destroy(vm_profile_machine_plan *plan)
{
    if (plan != LIB_NULL)
        plan->construction.profile.release(plan->construction.profile.context);
}

lib_status vm_profile_machine_plan_describe(vm_profile_machine_plan *plan,
    vm_machine_construction *out_construction)
{
    if (plan == LIB_NULL || out_construction == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_construction = plan->construction;
    return LIB_STATUS_OK;
}

const core_machine_config *vm_profile_machine_plan_core_config_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? LIB_NULL : &plan->construction.core_config; }

const core_machine_controller_timing_rules *vm_profile_machine_plan_timing_rules_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? LIB_NULL : &plan->construction.timing_rules; }

const core_machine_plan_topology *vm_profile_machine_plan_topology_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? LIB_NULL : &plan->construction.topology; }

const core_machine_firmware_provider *vm_profile_machine_plan_firmware_provider_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? LIB_NULL : plan->construction.firmware_provider; }

void *vm_profile_machine_plan_firmware_context_get(vm_profile_machine_plan *plan)
{ return plan == LIB_NULL ? LIB_NULL : plan->construction.firmware_context; }

lib_u8 vm_profile_machine_plan_hdc_present(const vm_profile_machine_plan *plan)
{ return plan != LIB_NULL && plan->construction.hdc_present; }

lib_u8 vm_profile_machine_plan_external_firmware(const vm_profile_machine_plan *plan)
{ return plan != LIB_NULL && plan->construction.firmware_provider != LIB_NULL; }

lib_status vm_profile_machine_plan_materialize(vm_profile_machine_plan *plan,
    core_machine_plan *core_plan)
{
    if (plan == LIB_NULL || core_plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return plan->construction.profile.configure == LIB_NULL ? LIB_STATUS_OK :
        plan->construction.profile.configure(plan->construction.profile.context, core_plan);
}
