#include "lib/types/types_interface.h"
#include "x86/ibmpc-common/machine_board_interface.h"
#include "app-nxvm/profiles/machine_plan_interface.h"

#include "x86/ibmpc-common/rom_validation_interface.h"
#include "app-nxvm/profiles/default_profile/external_pc_at_rom.h"
#include "app-nxvm/profiles/default_profile/pc_at_profile_private.h"
#include "app-nxvm/profiles/model40/composition_interface.h"
#include "app-nxvm/profiles/model40/model40_private.h"
#include "app-nxvm/profiles/xt/xt_5160_268.h"
#include "x86/ibmpc-common/floppy_interface.h"

#define VM_PROFILE_MACHINE_FDD_MEDIA_ID 1u
#define VM_PROFILE_MACHINE_HDD_MEDIA_ID 2u

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

typedef enum vm_profile_machine_plan_kind {
    VM_PROFILE_MACHINE_PLAN_DEFAULT_PC_AT,
    VM_PROFILE_MACHINE_PLAN_IBM_5170,
    VM_PROFILE_MACHINE_PLAN_XT,
    VM_PROFILE_MACHINE_PLAN_MODEL40
} vm_profile_machine_plan_kind;

struct vm_profile_machine_plan {
    vm_profile_machine_plan_kind kind;
    core_machine_config core_config;
    core_machine_controller_timing_rules timing_rules;
    core_machine_plan_topology topology;
    vm_profile_floppy_kind drive_floppy;
    vm_profile_floppy_kind media_floppy;
    lib_u8 floppy_slot_count;
    lib_u8 hdc_present;
    lib_u8 memory_reconfigurable;
    lib_u8 cmos_seed[VM_MACHINE_CMOS_SEED_BYTES];
    lib_u8 cmos_seed_present;
    x86_video_text_glyph_config text_glyphs;
    const core_machine_firmware_provider *firmware_provider;
    void *firmware_context;
    union {
        vm_profile_default_pc_at_plan_snapshot pc_at;
        vm_profile_contract_values model40;
        vm_profile_xt_5160_268_plan_snapshot xt;
    } profile;
    union {
        struct {
            lib_u8 image[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES];
            lib_u8 video[VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES];
            lib_size video_bytes;
            vm_profile_external_pc_at_rom_context context;
        } pc_at;
        struct {
            lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
            lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
            lib_u8 video[VM_PROFILE_MODEL40_VIDEO_ROM_BYTES];
            vm_profile_model40_external_rom context;
            /* Borrowed only while the sole Core attachment is alive. */
            core_machine_d4_platform *board;
            core_machine_fdc_terminal_observation fdc_terminal;
            lib_bool fdc_terminal_valid;
        } model40;
        struct {
            lib_u8 *system;
            lib_u8 *xebec;
            lib_u8 *video;
            vm_profile_xt_5160_268_external_rom context;
        } xt;
    } firmware;
};

static lib_status vm_profile_machine_plan_copy(lib_u8 *destination,
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
        lib_memory_copy(&plan->text_glyphs.bytes[character * X86_VIDEO_TEXT_GLYPH_ROWS],
            &source.data[character * 8u], 8u);
        lib_memory_copy(&plan->text_glyphs.bytes[character * X86_VIDEO_TEXT_GLYPH_ROWS + 8u],
            &source.data[VM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES + character * 8u], 8u);
    }
    plan->text_glyphs.present = LIB_TRUE;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_machine_plan_cmos(vm_profile_machine_plan *plan,
    vm_machine_asset_bytes source)
{
    if (plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (source.data == LIB_NULL && source.bytes == 0u) return LIB_STATUS_OK;
    if (vm_profile_machine_plan_copy(plan->cmos_seed, VM_MACHINE_CMOS_SEED_BYTES,
            source) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    plan->cmos_seed_present = LIB_TRUE;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_machine_plan_floppy(const vm_machine_config *config,
    vm_profile_floppy_kind default_kind, lib_u8 allow_360,
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

static lib_status vm_profile_machine_plan_pc_at_rom(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    lib_size index;

    if (plan == LIB_NULL || config == LIB_NULL || assets == LIB_NULL ||
        config->bios_count == 0u || config->bios_count > 2u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (config->bios_count == 1u) {
        if (vm_profile_machine_plan_copy(plan->firmware.pc_at.image,
                VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES, assets->bios[0u]) != LIB_STATUS_OK) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    } else {
        if (assets->bios[0u].data == LIB_NULL || assets->bios[1u].data == LIB_NULL ||
            assets->bios[0u].bytes != VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES ||
            assets->bios[1u].bytes != VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES) {
            return LIB_STATUS_INVALID_ARGUMENT;
        }
        for (index = 0u; index < VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES; ++index) {
            plan->firmware.pc_at.image[index * 2u] = assets->bios[0u].data[index];
            plan->firmware.pc_at.image[index * 2u + 1u] = assets->bios[1u].data[index];
        }
    }
    if (assets->video.data != LIB_NULL) {
        if (assets->video.bytes == 0u || assets->video.bytes >
            VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES) return LIB_STATUS_INVALID_ARGUMENT;
        lib_memory_copy(plan->firmware.pc_at.video, assets->video.data, assets->video.bytes);
        plan->firmware.pc_at.video_bytes = assets->video.bytes;
    } else if (assets->video.bytes != 0u) return LIB_STATUS_INVALID_ARGUMENT;
    plan->firmware.pc_at.context = (vm_profile_external_pc_at_rom_context) {
        plan->firmware.pc_at.image,
        plan->firmware.pc_at.video_bytes == 0u ? LIB_NULL : plan->firmware.pc_at.video,
        plan->firmware.pc_at.video_bytes};
    plan->firmware_provider = vm_profile_external_pc_at_rom_provider();
    plan->firmware_context = &plan->firmware.pc_at.context;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_machine_plan_default(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    vm_profile_default_at_request request = {0};

    if (vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_35_1440K,
            LIB_TRUE, &plan->media_floppy) != LIB_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
    if (config->cpu_profile != CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        config->fpu_profile != X86_FPU_PROFILE_NONE) {
        request.requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU;
        request.cpu_profile = config->cpu_profile;
        request.fpu_profile = config->fpu_profile;
    }
    if (config->memory_bytes != 0u) {
        request.requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY;
        request.memory_bytes = config->memory_bytes;
    }
    if (plan->media_floppy != VM_PROFILE_FLOPPY_35_1440K) {
        request.requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY;
        request.floppy_cmos_type = vm_profile_floppy_cmos_type_get(plan->media_floppy);
    }
    if (vm_profile_default_at_plan_create(&request, &plan->profile.pc_at) != LIB_STATUS_OK ||
        vm_profile_machine_plan_pc_at_rom(plan, config, assets) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->core_config = plan->profile.pc_at.values.core.configuration;
    plan->timing_rules = plan->profile.pc_at.values.core.controller_timing_rules;
    plan->topology = plan->profile.pc_at.topology;
    plan->drive_floppy = plan->media_floppy;
    plan->floppy_slot_count = 1u;
    plan->hdc_present = plan->profile.pc_at.descriptor.hdc_present;
    plan->memory_reconfigurable = LIB_TRUE;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_machine_plan_5170(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    if (vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_525_1200K,
            LIB_TRUE, &plan->media_floppy) != LIB_STATUS_OK ||
        vm_profile_ibm_5170_plan_create_memory(config->memory_bytes,
            &plan->profile.pc_at) != LIB_STATUS_OK ||
        vm_profile_machine_plan_pc_at_rom(plan, config, assets) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->core_config = plan->profile.pc_at.values.core.configuration;
    plan->timing_rules = plan->profile.pc_at.values.core.controller_timing_rules;
    plan->topology = plan->profile.pc_at.topology;
    plan->drive_floppy = VM_PROFILE_FLOPPY_525_1200K;
    plan->floppy_slot_count = 1u;
    plan->hdc_present = plan->profile.pc_at.descriptor.hdc_present;
    plan->memory_reconfigurable = LIB_TRUE;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_machine_plan_xt(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    vm_profile_xt_5160_268_external_rom source;

    if (config->bios_count == 0u || config->bios_count > 2u ||
        (assets->cmos_seed.data != LIB_NULL || assets->cmos_seed.bytes != 0u) || config->memory_bytes != 0u || config->create_fdd ||
        config->create_hdd_cylinders != 0u ||
        config->cpu_profile != CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        config->fpu_profile != X86_FPU_PROFILE_NONE ||
        vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_525_360K, LIB_FALSE,
            &plan->media_floppy) != LIB_STATUS_OK ||
        vm_profile_xt_5160_268_external_rom_create(assets->bios[0u].data,
            assets->bios[0u].bytes, config->bios_count == 2u ? assets->bios[1u].data : LIB_NULL,
            config->bios_count == 2u ? assets->bios[1u].bytes : 0u, assets->video.data,
            assets->video.bytes, &source) != LIB_STATUS_OK ||
        vm_profile_xt_5160_268_plan_create(&plan->profile.xt, source.xebec_present) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->firmware.xt.system = (lib_u8 *)lib_allocate(
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
    plan->firmware.xt.xebec = (lib_u8 *)lib_allocate(
        VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES);
    plan->firmware.xt.video = source.video_bytes == LIB_NULL ? LIB_NULL :
        (lib_u8 *)lib_allocate(source.video_byte_count);
    if (plan->firmware.xt.system == LIB_NULL || plan->firmware.xt.xebec == LIB_NULL ||
        (source.video_bytes != LIB_NULL && plan->firmware.xt.video == LIB_NULL)) {
        return LIB_STATUS_NO_MEMORY;
    }
    lib_memory_copy(plan->firmware.xt.system, source.system_bytes,
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
    if (source.xebec_present) lib_memory_copy(plan->firmware.xt.xebec, source.xebec_bytes,
        VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES);
    if (source.video_bytes != LIB_NULL) lib_memory_copy(plan->firmware.xt.video,
        source.video_bytes, source.video_byte_count);
    plan->firmware.xt.context = source;
    plan->firmware.xt.context.system_bytes = plan->firmware.xt.system;
    plan->firmware.xt.context.xebec_bytes = source.xebec_present ? plan->firmware.xt.xebec : LIB_NULL;
    plan->firmware.xt.context.video_bytes = source.video_bytes == LIB_NULL ? LIB_NULL :
        plan->firmware.xt.video;
    plan->firmware_provider = vm_profile_xt_5160_268_firmware_provider();
    plan->firmware_context = &plan->firmware.xt.context;
    plan->core_config = plan->profile.xt.values.core.configuration;
    plan->timing_rules = plan->profile.xt.values.core.controller_timing_rules;
    plan->topology = plan->profile.xt.topology;
    plan->drive_floppy = VM_PROFILE_FLOPPY_525_360K;
    plan->floppy_slot_count = 1u;
    plan->hdc_present = source.xebec_present;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_machine_plan_model40(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    vm_profile_model40_external_rom source;

    if (config->bios_count != 2u ||
        (config->memory_bytes != 0u && config->memory_bytes != 1024u * 1024u) ||
        vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_525_1200K, LIB_TRUE,
            &plan->media_floppy) != LIB_STATUS_OK ||
        vm_profile_model40_values_create(&plan->profile.model40) != LIB_STATUS_OK ||
        vm_profile_model40_external_rom_create(assets->bios[0u].data, assets->bios[0u].bytes,
            assets->bios[1u].data, assets->bios[1u].bytes, assets->video.data,
            assets->video.bytes, &source) != LIB_STATUS_OK ||
        vm_profile_machine_plan_copy(plan->firmware.model40.even,
            VM_PROFILE_MODEL40_ROM_CHIP_BYTES, assets->bios[0u]) != LIB_STATUS_OK ||
        vm_profile_machine_plan_copy(plan->firmware.model40.odd,
            VM_PROFILE_MODEL40_ROM_CHIP_BYTES, assets->bios[1u]) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (assets->video.data != LIB_NULL) lib_memory_copy(plan->firmware.model40.video,
        assets->video.data, VM_PROFILE_MODEL40_VIDEO_ROM_BYTES);
    plan->firmware.model40.context = source;
    plan->firmware.model40.context.even_bytes = plan->firmware.model40.even;
    plan->firmware.model40.context.odd_bytes = plan->firmware.model40.odd;
    plan->firmware.model40.context.video_bytes = assets->video.data == LIB_NULL ? LIB_NULL :
        plan->firmware.model40.video;
    plan->firmware_provider = vm_profile_model40_firmware_provider();
    plan->firmware_context = &plan->firmware.model40.context;
    plan->core_config = plan->profile.model40.core.configuration;
    plan->timing_rules = plan->profile.model40.core.controller_timing_rules;
    if (vm_profile_model40_topology_materialize(&plan->topology) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    plan->drive_floppy = VM_PROFILE_FLOPPY_525_1200K;
    plan->floppy_slot_count = 2u;
    plan->hdc_present = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status vm_profile_machine_plan_create(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan)
{
    vm_profile_machine_plan *plan;
    lib_status status;

    if (out_plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_plan = LIB_NULL;
    if (config == LIB_NULL || assets == LIB_NULL ||
        config->fixed_disk_image[1u] != LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    plan = (vm_profile_machine_plan *)lib_allocate_zero(1u, sizeof(*plan));
    if (plan == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (config->profile_kind == VM_MACHINE_PROFILE_DEFAULT_PC_AT) {
        plan->kind = VM_PROFILE_MACHINE_PLAN_DEFAULT_PC_AT;
        status = vm_profile_machine_plan_default(plan, config, assets);
    } else if (config->profile_kind == VM_MACHINE_PROFILE_IBM_5170_MODEL_339) {
        plan->kind = VM_PROFILE_MACHINE_PLAN_IBM_5170;
        status = vm_profile_machine_plan_5170(plan, config, assets);
    } else if (config->profile_kind == VM_MACHINE_PROFILE_IBM_5160_MODEL_268) {
        plan->kind = VM_PROFILE_MACHINE_PLAN_XT;
        status = vm_profile_machine_plan_xt(plan, config, assets);
    } else if (config->profile_kind == VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        plan->kind = VM_PROFILE_MACHINE_PLAN_MODEL40;
        status = vm_profile_machine_plan_model40(plan, config, assets);
    } else status = LIB_STATUS_INVALID_ARGUMENT;
    if (status == LIB_STATUS_OK) status = vm_profile_machine_plan_cmos(plan, assets->cmos_seed);
    if (status == LIB_STATUS_OK) status = vm_profile_machine_plan_text_glyphs(plan, assets->font);
    if (status == LIB_STATUS_OK && config->floppy_image[1u] != LIB_NULL &&
        plan->floppy_slot_count < 2u) status = LIB_STATUS_INVALID_ARGUMENT;
    if (status != LIB_STATUS_OK) {
        vm_profile_machine_plan_destroy(plan);
        return status;
    }
    *out_plan = plan;
    return LIB_STATUS_OK;
}

void vm_profile_machine_plan_destroy(vm_profile_machine_plan *plan)
{
    if (plan == LIB_NULL) return;
    if (plan->kind == VM_PROFILE_MACHINE_PLAN_XT) {
        lib_release(plan->firmware.xt.system);
        lib_release(plan->firmware.xt.xebec);
        lib_release(plan->firmware.xt.video);
    }
    lib_release(plan);
}

const core_machine_config *vm_profile_machine_plan_core_config_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? LIB_NULL : &plan->core_config; }
const core_machine_controller_timing_rules *vm_profile_machine_plan_timing_rules_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? LIB_NULL : &plan->timing_rules; }
const core_machine_plan_topology *vm_profile_machine_plan_topology_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? LIB_NULL : &plan->topology; }
const core_machine_firmware_provider *vm_profile_machine_plan_firmware_provider_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? LIB_NULL : plan->firmware_provider; }
void *vm_profile_machine_plan_firmware_context_get(vm_profile_machine_plan *plan)
{ return plan == LIB_NULL ? LIB_NULL : plan->firmware_context; }
vm_profile_floppy_kind vm_profile_machine_plan_drive_floppy_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? VM_PROFILE_FLOPPY_35_1440K : plan->drive_floppy; }
vm_profile_floppy_kind vm_profile_machine_plan_media_floppy_get(
    const vm_profile_machine_plan *plan) { return plan == LIB_NULL ? VM_PROFILE_FLOPPY_35_1440K : plan->media_floppy; }
lib_u8 vm_profile_machine_plan_hdc_present(const vm_profile_machine_plan *plan)
{ return plan != LIB_NULL && plan->hdc_present; }
lib_u8 vm_profile_machine_plan_memory_reconfigurable(const vm_profile_machine_plan *plan)
{ return plan != LIB_NULL && plan->memory_reconfigurable; }
lib_u8 vm_profile_machine_plan_floppy_slot_count(const vm_profile_machine_plan *plan)
{ return plan == LIB_NULL ? 0u : plan->floppy_slot_count; }
const vm_profile_model40_external_rom *vm_profile_machine_plan_model40_rom_get(
    const vm_profile_machine_plan *plan)
{
    return plan == LIB_NULL || plan->kind != VM_PROFILE_MACHINE_PLAN_MODEL40 ? LIB_NULL :
        &plan->firmware.model40.context;
}
lib_u8 vm_profile_machine_plan_is_model40(const vm_profile_machine_plan *plan)
{ return plan != LIB_NULL && plan->kind == VM_PROFILE_MACHINE_PLAN_MODEL40; }
lib_u8 vm_profile_machine_plan_external_firmware(const vm_profile_machine_plan *plan)
{ return plan != LIB_NULL && plan->firmware_provider != LIB_NULL; }

lib_status vm_profile_machine_plan_copy_cmos_seed(const vm_profile_machine_plan *plan,
    lib_u8 *out_seed, lib_u8 *out_present)
{
    if (plan == LIB_NULL || out_seed == LIB_NULL || out_present == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_present = plan->cmos_seed_present;
    if (plan->cmos_seed_present) lib_memory_copy(out_seed, plan->cmos_seed,
        VM_MACHINE_CMOS_SEED_BYTES);
    return LIB_STATUS_OK;
}

lib_status vm_profile_machine_plan_copy_text_glyphs(const vm_profile_machine_plan *plan,
    x86_video_text_glyph_config *out_glyphs)
{
    if (plan == LIB_NULL || out_glyphs == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_glyphs = plan->text_glyphs;
    return LIB_STATUS_OK;
}

static lib_status vm_profile_machine_plan_materialize_pc_at(
    const vm_profile_machine_plan *plan, core_machine_plan *core_plan)
{
    const vm_profile_default_pc_at_descriptor *profile;
    const vm_profile_default_pc_at_port_leaf *dor_port;
    const vm_profile_default_pc_at_port_leaf *status_port;
    const vm_profile_default_pc_at_port_leaf *data_port;
    const vm_profile_default_pc_at_port_leaf *control_port;
    const vm_profile_default_pc_at_route *route;
    core_machine_fdc_drive_bindings drives = {
        {VM_PROFILE_MACHINE_FDD_MEDIA_ID, CORE_MACHINE_MEDIA_ID_INVALID,
            CORE_MACHINE_MEDIA_ID_INVALID, CORE_MACHINE_MEDIA_ID_INVALID}, 0x01u, 0x01u,
        {0u, 0u, 0u, 0u}, 0u, {0}
    };
    core_machine_fdc_config fdc = {0};

    if (plan == LIB_NULL || core_plan == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    profile = &plan->profile.pc_at.descriptor;
    if (!vm_profile_default_pc_at_descriptor_is_valid(profile)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    dor_port = vm_profile_default_pc_at_port_leaf_at(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0u);
    status_port = vm_profile_default_pc_at_port_leaf_at(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 1u);
    data_port = vm_profile_default_pc_at_port_leaf_at(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 2u);
    control_port = vm_profile_default_pc_at_port_leaf_at(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 3u);
    route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2);
    if (dor_port == LIB_NULL || status_port == LIB_NULL || data_port == LIB_NULL ||
        control_port == LIB_NULL || route == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    fdc.dor_port = dor_port->port;
    fdc.status_port = status_port->port;
    fdc.data_port = data_port->port;
    fdc.direction_port = control_port->port;
    fdc.control_port = control_port->port;
    fdc.irq = route->irq;
    fdc.dma_channel = route->dma_channel;
    fdc.ready_mask = profile->fdc_ready_mask;
    fdc.clock_ticks_per_second = plan->core_config.time_axis.ticks_per_second;
    drives.installed_mask = profile->fdc_installed_mask;
    drives.double_sided_mask = profile->fdc_double_sided_mask;
    lib_memory_copy(drives.cylinder_count, profile->fdc_cylinder_count,
        sizeof(drives.cylinder_count));
    drives.track_zero_active_low_mask = profile->fdc_track_zero_active_low_mask;
    for (vm_profile_floppy_kind kind = VM_PROFILE_FLOPPY_35_1440K;
            kind <= VM_PROFILE_FLOPPY_35_720K; ++kind) {
        if (vm_profile_floppy_cmos_type_get(kind) == profile->cmos.floppy_type) {
            drives.channel = vm_profile_floppy_channel_get(kind);
            break;
        }
    }
    if (drives.channel.sample == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    fdc.diagnostic_port = profile->fdc_diagnostic_port;
    fdc.diagnostic_read_value = profile->fdc_diagnostic_read_value;
    if (core_machine_plan_configure_fdc(core_plan, &drives, &fdc) != LIB_STATUS_OK) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!profile->hdc_present) return LIB_STATUS_OK;
    return core_machine_plan_configure_hdc(core_plan, VM_PROFILE_MACHINE_HDD_MEDIA_ID,
        CORE_MACHINE_MEDIA_ID_INVALID, &profile->hdc);
}

static void vm_profile_machine_plan_capture_fdc_terminal(void *context,
    const core_machine_fdc_terminal_observation *observation)
{
    vm_profile_machine_plan *plan = context;

    if (plan == LIB_NULL || observation == LIB_NULL) return;
    plan->firmware.model40.fdc_terminal = *observation;
    plan->firmware.model40.fdc_terminal_valid = LIB_TRUE;
}

lib_status vm_profile_machine_plan_materialize(vm_profile_machine_plan *plan,
    core_machine_plan *core_plan)
{
    if (plan == LIB_NULL || core_plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (plan->kind == VM_PROFILE_MACHINE_PLAN_MODEL40) {
        return vm_profile_model40_materialize_plan(core_plan,
            (core_machine_fdc_terminal_observation_provider) {
                vm_profile_machine_plan_capture_fdc_terminal, plan },
            &plan->firmware.model40.board);
    }
    if (plan->kind == VM_PROFILE_MACHINE_PLAN_XT) return LIB_STATUS_OK;
    return vm_profile_machine_plan_materialize_pc_at(plan, core_plan);
}

lib_status vm_profile_machine_plan_observe_model40(
    const vm_profile_machine_plan *plan, vm_profile_model40_observation *out_observation)
{
    vm_profile_model40_observation observation = {0};
    lib_status status;

    if (plan == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (plan->kind != VM_PROFILE_MACHINE_PLAN_MODEL40) return LIB_STATUS_UNSUPPORTED;
    status = core_machine_d4_platform_observe(plan->firmware.model40.board,
        &observation.d4);
    if (status != LIB_STATUS_OK) return status;
    observation.fdc_terminal = plan->firmware.model40.fdc_terminal;
    observation.fdc_terminal_valid = plan->firmware.model40.fdc_terminal_valid;
    *out_observation = observation;
    return LIB_STATUS_OK;
}

void vm_profile_machine_plan_reset_observation(vm_profile_machine_plan *plan)
{
    if (plan != LIB_NULL && plan->kind == VM_PROFILE_MACHINE_PLAN_MODEL40)
        plan->firmware.model40.fdc_terminal_valid = LIB_FALSE;
}

void vm_profile_machine_plan_detach_board(vm_profile_machine_plan *plan)
{
    if (plan == LIB_NULL || plan->kind != VM_PROFILE_MACHINE_PLAN_MODEL40) return;
    plan->firmware.model40.board = LIB_NULL;
    plan->firmware.model40.fdc_terminal_valid = LIB_FALSE;
}

lib_u8 vm_profile_machine_plan_hdd_geometry_get(const vm_profile_machine_plan *plan,
    lib_u16 *out_cylinders, lib_u8 *out_heads,
    lib_u8 *out_sectors)
{
    if (plan == LIB_NULL || plan->kind != VM_PROFILE_MACHINE_PLAN_MODEL40 ||
        out_cylinders == LIB_NULL || out_heads == LIB_NULL || out_sectors == LIB_NULL) {
        return LIB_FALSE;
    }
    *out_cylinders = 925u;
    *out_heads = 5u;
    *out_sectors = 17u;
    return LIB_TRUE;
}

static lib_status vm_profile_machine_plan_configure(void *context,
    core_machine_plan *plan)
{
    return vm_profile_machine_plan_materialize(context, plan);
}

static void vm_profile_machine_plan_notify(void *context,
    vm_machine_profile_event event)
{
    if (event == VM_MACHINE_PROFILE_RESET_COMPLETED)
        vm_profile_machine_plan_reset_observation(context);
    else if (event == VM_MACHINE_PROFILE_BOARD_DETACHED)
        vm_profile_machine_plan_detach_board(context);
}

static void vm_profile_machine_plan_release(void *context)
{
    vm_profile_machine_plan_destroy(context);
}

lib_status vm_profile_machine_plan_describe(vm_profile_machine_plan *plan,
    vm_machine_construction *out_construction)
{
    vm_machine_construction construction;

    if (plan == LIB_NULL || out_construction == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    construction = (vm_machine_construction) {
        .core_config = plan->core_config,
        .timing_rules = plan->timing_rules,
        .topology = plan->topology,
        .floppy_kind = plan->drive_floppy,
        .media_kind = plan->media_floppy,
        .floppy_slot_count = plan->floppy_slot_count,
        .hdc_present = plan->hdc_present,
        .memory_reconfigurable = plan->memory_reconfigurable,
        .cmos_seed_present = plan->cmos_seed_present,
        .text_glyphs = plan->text_glyphs,
        .firmware_provider = plan->firmware_provider,
        .firmware_context = plan->firmware_context,
        .profile = { plan, vm_profile_machine_plan_configure,
            vm_profile_machine_plan_notify, vm_profile_machine_plan_release }
    };
    lib_memory_copy(construction.cmos_seed, plan->cmos_seed,
        sizeof(construction.cmos_seed));
    construction.fixed_geometry = vm_profile_machine_plan_hdd_geometry_get(plan,
        &construction.cylinders, &construction.heads, &construction.sectors);
    *out_construction = construction;
    return LIB_STATUS_OK;
}
