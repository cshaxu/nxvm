#include "core/profiles/machine_plan_interface.h"

#include "lib/storage/file_interface.h"
#include "core/profiles/byob/blob.h"
#include "core/profiles/default_profile/external_pc_at_rom.h"
#include "core/profiles/default_profile/pc_at_profile_private.h"
#include "core/profiles/model40/composition_interface.h"
#include "core/profiles/model40/model40_private.h"
#include "core/profiles/xt/xt_5160_268.h"

#define VM_PROFILE_MACHINE_FDD_MEDIA_ID 1u
#define VM_PROFILE_MACHINE_HDD_MEDIA_ID 2u

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
    type_unsigned_8 floppy_slot_count;
    type_bool hdc_present;
    type_bool memory_reconfigurable;
    type_unsigned_8 cmos_seed[VM_MACHINE_CMOS_SEED_BYTES];
    type_bool cmos_seed_present;
    core_machine_vadp_text_glyph_config text_glyphs;
    const core_machine_firmware_provider *firmware_provider;
    C_VOID *firmware_context;
    union {
        vm_profile_default_pc_at_plan_snapshot pc_at;
        vm_profile_contract_values model40;
        vm_profile_xt_5160_268_plan_snapshot xt;
    } profile;
    union {
        struct {
            type_unsigned_8 image[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES];
            type_unsigned_8 video[VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES];
            STD_SIZE_T video_bytes;
            vm_profile_external_pc_at_rom_context context;
        } pc_at;
        struct {
            type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
            type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
            type_unsigned_8 video[VM_PROFILE_MODEL40_VIDEO_ROM_BYTES];
            vm_profile_model40_external_rom context;
        } model40;
        struct {
            type_unsigned_8 *system;
            type_unsigned_8 *xebec;
            type_unsigned_8 *video;
            vm_profile_xt_5160_268_external_rom context;
        } xt;
    } firmware;
};

static type_status vm_profile_machine_plan_copy(type_unsigned_8 *destination,
    STD_SIZE_T expected, vm_machine_asset_bytes source)
{
    if (destination == STD_NULL || source.data == STD_NULL || source.bytes != expected) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMCPY(destination, source.data, expected);
    return TYPE_STATUS_OK;
}

static type_status vm_profile_machine_plan_text_glyphs(vm_profile_machine_plan *plan,
    vm_machine_asset_bytes source)
{
    STD_SIZE_T character;

    if (plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (source.data == STD_NULL && source.bytes == 0u) return TYPE_STATUS_OK;
    if (source.data == STD_NULL || source.bytes != VM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    for (character = 0u; character < CORE_MACHINE_DISPLAY_TEXT_GLYPH_COUNT;
         ++character) {
        STD_MEMCPY(&plan->text_glyphs.bytes[character * CORE_MACHINE_DISPLAY_TEXT_GLYPH_ROWS],
            &source.data[character * 8u], 8u);
        STD_MEMCPY(&plan->text_glyphs.bytes[character * CORE_MACHINE_DISPLAY_TEXT_GLYPH_ROWS + 8u],
            &source.data[VM_MACHINE_TEXT_GLYPH_ROW_PLANE_BYTES + character * 8u], 8u);
    }
    plan->text_glyphs.present = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_machine_plan_cmos(vm_profile_machine_plan *plan,
    vm_machine_asset_bytes source)
{
    if (plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (source.data == STD_NULL && source.bytes == 0u) return TYPE_STATUS_OK;
    if (vm_profile_machine_plan_copy(plan->cmos_seed, VM_MACHINE_CMOS_SEED_BYTES,
            source) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_ARGUMENT;
    plan->cmos_seed_present = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_machine_plan_floppy(const vm_machine_config *config,
    vm_profile_floppy_kind default_kind, type_bool allow_360,
    vm_profile_floppy_kind *out_media)
{
    const vm_machine_floppy_format format = config == STD_NULL ?
        VM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT : config->floppy_format;

    if (out_media == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (format == VM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT) {
        *out_media = default_kind;
        return TYPE_STATUS_OK;
    }
    if ((default_kind == VM_PROFILE_FLOPPY_525_360K &&
            format == VM_MACHINE_FLOPPY_FORMAT_360K) ||
        (default_kind == VM_PROFILE_FLOPPY_525_1200K &&
            format == VM_MACHINE_FLOPPY_FORMAT_1200K)) {
        *out_media = default_kind;
        return TYPE_STATUS_OK;
    }
    if (allow_360 && format == VM_MACHINE_FLOPPY_FORMAT_360K) {
        *out_media = VM_PROFILE_FLOPPY_525_360K;
        return TYPE_STATUS_OK;
    }
    if (default_kind == VM_PROFILE_FLOPPY_35_1440K &&
        format == VM_MACHINE_FLOPPY_FORMAT_1440K) {
        *out_media = default_kind;
        return TYPE_STATUS_OK;
    }
    if (default_kind == VM_PROFILE_FLOPPY_35_1440K &&
        format == VM_MACHINE_FLOPPY_FORMAT_720K) {
        *out_media = VM_PROFILE_FLOPPY_35_720K;
        return TYPE_STATUS_OK;
    }
    if (default_kind == VM_PROFILE_FLOPPY_35_1440K &&
        format == VM_MACHINE_FLOPPY_FORMAT_1200K) {
        *out_media = VM_PROFILE_FLOPPY_525_1200K;
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_INVALID_ARGUMENT;
}

static type_status vm_profile_machine_plan_pc_at_rom(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    STD_SIZE_T index;

    if (plan == STD_NULL || config == STD_NULL || assets == STD_NULL ||
        config->bios_count == 0u || config->bios_count > 2u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (config->bios_count == 1u) {
        if (vm_profile_machine_plan_copy(plan->firmware.pc_at.image,
                VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES, assets->bios[0u]) != TYPE_STATUS_OK) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
    } else {
        if (assets->bios[0u].data == STD_NULL || assets->bios[1u].data == STD_NULL ||
            assets->bios[0u].bytes != VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES ||
            assets->bios[1u].bytes != VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        for (index = 0u; index < VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES; ++index) {
            plan->firmware.pc_at.image[index * 2u] = assets->bios[0u].data[index];
            plan->firmware.pc_at.image[index * 2u + 1u] = assets->bios[1u].data[index];
        }
    }
    if (assets->video.data != STD_NULL) {
        if (assets->video.bytes == 0u || assets->video.bytes >
            VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES) return TYPE_STATUS_INVALID_ARGUMENT;
        STD_MEMCPY(plan->firmware.pc_at.video, assets->video.data, assets->video.bytes);
        plan->firmware.pc_at.video_bytes = assets->video.bytes;
    } else if (assets->video.bytes != 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    plan->firmware.pc_at.context = (vm_profile_external_pc_at_rom_context) {
        plan->firmware.pc_at.image,
        plan->firmware.pc_at.video_bytes == 0u ? STD_NULL : plan->firmware.pc_at.video,
        plan->firmware.pc_at.video_bytes};
    plan->firmware_provider = vm_profile_external_pc_at_rom_provider();
    plan->firmware_context = &plan->firmware.pc_at.context;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_machine_plan_default(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    vm_profile_default_at_request request = {0};

    if (vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_35_1440K,
            TYPE_TRUE, &plan->media_floppy) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_ARGUMENT;
    if (config->cpu_profile != CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        config->fpu_profile != CORE_MACHINE_FPU_PROFILE_NONE) {
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
    if (vm_profile_default_at_plan_create(&request, &plan->profile.pc_at) != TYPE_STATUS_OK ||
        vm_profile_machine_plan_pc_at_rom(plan, config, assets) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    plan->core_config = plan->profile.pc_at.values.core.configuration;
    plan->timing_rules = plan->profile.pc_at.values.core.controller_timing_rules;
    plan->topology = plan->profile.pc_at.topology;
    plan->drive_floppy = plan->media_floppy;
    plan->floppy_slot_count = 1u;
    plan->hdc_present = plan->profile.pc_at.descriptor.hdc_present;
    plan->memory_reconfigurable = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_machine_plan_5170(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    if (vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_525_1200K,
            TYPE_TRUE, &plan->media_floppy) != TYPE_STATUS_OK ||
        vm_profile_ibm_5170_plan_create_memory(config->memory_bytes,
            &plan->profile.pc_at) != TYPE_STATUS_OK ||
        vm_profile_machine_plan_pc_at_rom(plan, config, assets) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    plan->core_config = plan->profile.pc_at.values.core.configuration;
    plan->timing_rules = plan->profile.pc_at.values.core.controller_timing_rules;
    plan->topology = plan->profile.pc_at.topology;
    plan->drive_floppy = VM_PROFILE_FLOPPY_525_1200K;
    plan->floppy_slot_count = 1u;
    plan->hdc_present = plan->profile.pc_at.descriptor.hdc_present;
    plan->memory_reconfigurable = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_machine_plan_xt(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    vm_profile_xt_5160_268_external_rom source;

    if (config->bios_count == 0u || config->bios_count > 2u ||
        config->cmos_seed != STD_NULL || config->memory_bytes != 0u || config->create_fdd ||
        config->create_hdd_cylinders != 0u ||
        config->cpu_profile != CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        config->fpu_profile != CORE_MACHINE_FPU_PROFILE_NONE ||
        vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_525_360K, TYPE_FALSE,
            &plan->media_floppy) != TYPE_STATUS_OK ||
        vm_profile_xt_5160_268_external_rom_create(assets->bios[0u].data,
            assets->bios[0u].bytes, config->bios_count == 2u ? assets->bios[1u].data : STD_NULL,
            config->bios_count == 2u ? assets->bios[1u].bytes : 0u, assets->video.data,
            assets->video.bytes, &source) != TYPE_STATUS_OK ||
        vm_profile_xt_5160_268_plan_create(&plan->profile.xt, source.xebec_present) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    plan->firmware.xt.system = (type_unsigned_8 *)STD_MALLOC(
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
    plan->firmware.xt.xebec = (type_unsigned_8 *)STD_MALLOC(
        VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES);
    plan->firmware.xt.video = source.video_bytes == STD_NULL ? STD_NULL :
        (type_unsigned_8 *)STD_MALLOC(source.video_byte_count);
    if (plan->firmware.xt.system == STD_NULL || plan->firmware.xt.xebec == STD_NULL ||
        (source.video_bytes != STD_NULL && plan->firmware.xt.video == STD_NULL)) {
        return TYPE_STATUS_NO_MEMORY;
    }
    STD_MEMCPY(plan->firmware.xt.system, source.system_bytes,
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
    if (source.xebec_present) STD_MEMCPY(plan->firmware.xt.xebec, source.xebec_bytes,
        VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES);
    if (source.video_bytes != STD_NULL) STD_MEMCPY(plan->firmware.xt.video,
        source.video_bytes, source.video_byte_count);
    plan->firmware.xt.context = source;
    plan->firmware.xt.context.system_bytes = plan->firmware.xt.system;
    plan->firmware.xt.context.xebec_bytes = source.xebec_present ? plan->firmware.xt.xebec : STD_NULL;
    plan->firmware.xt.context.video_bytes = source.video_bytes == STD_NULL ? STD_NULL :
        plan->firmware.xt.video;
    plan->firmware_provider = vm_profile_xt_5160_268_firmware_provider();
    plan->firmware_context = &plan->firmware.xt.context;
    plan->core_config = plan->profile.xt.values.core.configuration;
    plan->timing_rules = plan->profile.xt.values.core.controller_timing_rules;
    plan->topology = plan->profile.xt.topology;
    plan->drive_floppy = VM_PROFILE_FLOPPY_525_360K;
    plan->floppy_slot_count = 1u;
    plan->hdc_present = source.xebec_present;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_machine_plan_model40(vm_profile_machine_plan *plan,
    const vm_machine_config *config, const vm_machine_assets *assets)
{
    vm_profile_model40_external_rom source;

    if (config->bios_count != 2u ||
        (config->memory_bytes != 0u && config->memory_bytes != 1024u * 1024u) ||
        vm_profile_machine_plan_floppy(config, VM_PROFILE_FLOPPY_525_1200K, TYPE_TRUE,
            &plan->media_floppy) != TYPE_STATUS_OK ||
        vm_profile_model40_values_create(&plan->profile.model40) != TYPE_STATUS_OK ||
        vm_profile_model40_external_rom_create(assets->bios[0u].data, assets->bios[0u].bytes,
            assets->bios[1u].data, assets->bios[1u].bytes, assets->video.data,
            assets->video.bytes, &source) != TYPE_STATUS_OK ||
        vm_profile_machine_plan_copy(plan->firmware.model40.even,
            VM_PROFILE_MODEL40_ROM_CHIP_BYTES, assets->bios[0u]) != TYPE_STATUS_OK ||
        vm_profile_machine_plan_copy(plan->firmware.model40.odd,
            VM_PROFILE_MODEL40_ROM_CHIP_BYTES, assets->bios[1u]) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (assets->video.data != STD_NULL) STD_MEMCPY(plan->firmware.model40.video,
        assets->video.data, VM_PROFILE_MODEL40_VIDEO_ROM_BYTES);
    plan->firmware.model40.context = source;
    plan->firmware.model40.context.even_bytes = plan->firmware.model40.even;
    plan->firmware.model40.context.odd_bytes = plan->firmware.model40.odd;
    plan->firmware.model40.context.video_bytes = assets->video.data == STD_NULL ? STD_NULL :
        plan->firmware.model40.video;
    plan->firmware_provider = vm_profile_model40_firmware_provider();
    plan->firmware_context = &plan->firmware.model40.context;
    plan->core_config = plan->profile.model40.core.configuration;
    plan->timing_rules = plan->profile.model40.core.controller_timing_rules;
    if (vm_profile_model40_topology_materialize(&plan->topology) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    plan->drive_floppy = VM_PROFILE_FLOPPY_525_1200K;
    plan->floppy_slot_count = 2u;
    plan->hdc_present = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

type_status vm_profile_machine_plan_create(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan)
{
    vm_profile_machine_plan *plan;
    type_status status;

    if (out_plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_plan = STD_NULL;
    if (config == STD_NULL || assets == STD_NULL ||
        config->fixed_disk_image[1u] != STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    plan = (vm_profile_machine_plan *)STD_CALLOC(1u, sizeof(*plan));
    if (plan == STD_NULL) return TYPE_STATUS_NO_MEMORY;
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
    } else status = TYPE_STATUS_INVALID_ARGUMENT;
    if (status == TYPE_STATUS_OK) status = vm_profile_machine_plan_cmos(plan, assets->cmos_seed);
    if (status == TYPE_STATUS_OK) status = vm_profile_machine_plan_text_glyphs(plan, assets->font);
    if (status == TYPE_STATUS_OK && config->floppy_image[1u] != STD_NULL &&
        plan->floppy_slot_count < 2u) status = TYPE_STATUS_INVALID_ARGUMENT;
    if (status != TYPE_STATUS_OK) {
        vm_profile_machine_plan_destroy(plan);
        return status;
    }
    *out_plan = plan;
    return TYPE_STATUS_OK;
}

typedef struct vm_profile_machine_file_assets {
    type_unsigned_8 *bios[2];
    type_unsigned_8 *cmos_seed;
    type_unsigned_8 *video;
    type_unsigned_8 *font;
    vm_machine_assets view;
} vm_profile_machine_file_assets;

static C_VOID vm_profile_machine_file_assets_destroy(
    vm_profile_machine_file_assets *assets)
{
    if (assets == STD_NULL) return;
    STD_FREE(assets->bios[0u]);
    STD_FREE(assets->bios[1u]);
    STD_FREE(assets->cmos_seed);
    STD_FREE(assets->video);
    STD_FREE(assets->font);
    *assets = (vm_profile_machine_file_assets) {0};
}

static type_status vm_profile_machine_file_load(const C_CHAR *path,
    STD_SIZE_T bytes, vm_machine_asset_bytes *out_view, type_unsigned_8 **out_owned)
{
    type_unsigned_8 *owned;

    if (path == STD_NULL || out_view == STD_NULL || out_owned == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    owned = (type_unsigned_8 *)STD_MALLOC(bytes);
    if (owned == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (vm_profile_byob_blob_load(&(vm_profile_byob_blob) {path, STD_NULL, bytes},
            owned) != TYPE_STATUS_OK) {
        STD_FREE(owned);
        return TYPE_STATUS_FAULT;
    }
    *out_view = (vm_machine_asset_bytes) {owned, bytes};
    *out_owned = owned;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_machine_file_variable_load(const C_CHAR *path,
    STD_SIZE_T maximum, vm_machine_asset_bytes *out_view, type_unsigned_8 **out_owned)
{
    C_VOID *owned = STD_NULL;
    STD_SIZE_T bytes = 0u;

    if (path == STD_NULL || out_view == STD_NULL || out_owned == STD_NULL ||
        maximum == 0u || lib_storage_file_read_owned(path, maximum, &owned, &bytes) !=
            LIB_STATUS_OK || bytes == 0u) {
        STD_FREE(owned);
        return TYPE_STATUS_FAULT;
    }
    *out_view = (vm_machine_asset_bytes) {owned, bytes};
    *out_owned = owned;
    return TYPE_STATUS_OK;
}

type_status vm_profile_machine_plan_create_file_backed(const vm_machine_config *config,
    vm_profile_machine_plan **out_plan)
{
    vm_profile_machine_file_assets assets = {0};
    STD_SIZE_T bios_bytes;
    type_status status;

    if (out_plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_plan = STD_NULL;
    if (config == STD_NULL || config->bios_count == 0u ||
        config->bios_count > 2u || (config->profile_kind !=
            VM_MACHINE_PROFILE_IBM_5160_MODEL_268 && config->cmos_seed == STD_NULL)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (config->profile_kind == VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        if (config->bios_count != 2u) return TYPE_STATUS_INVALID_ARGUMENT;
        bios_bytes = VM_PROFILE_MODEL40_ROM_CHIP_BYTES;
    } else if (config->profile_kind == VM_MACHINE_PROFILE_IBM_5160_MODEL_268) {
        bios_bytes = VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES;
    } else {
        bios_bytes = config->bios_count == 1u ? VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES :
            VM_PROFILE_EXTERNAL_PC_AT_ROM_CHIP_BYTES;
    }
    status = vm_profile_machine_file_load(config->bios_path[0u], bios_bytes,
        &assets.view.bios[0u], &assets.bios[0u]);
    if (status == TYPE_STATUS_OK && config->bios_count == 2u) {
        const STD_SIZE_T secondary_bytes = config->profile_kind ==
            VM_MACHINE_PROFILE_IBM_5160_MODEL_268 ? VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES :
            bios_bytes;
        status = vm_profile_machine_file_load(config->bios_path[1u], secondary_bytes,
            &assets.view.bios[1u], &assets.bios[1u]);
    }
    if (status == TYPE_STATUS_OK && config->cmos_seed != STD_NULL) status =
        vm_profile_machine_file_load(config->cmos_seed, VM_MACHINE_CMOS_SEED_BYTES,
            &assets.view.cmos_seed, &assets.cmos_seed);
    if (status == TYPE_STATUS_OK && config->video_path != STD_NULL) status =
        config->profile_kind == VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40 ?
        vm_profile_machine_file_load(config->video_path, VM_PROFILE_MODEL40_VIDEO_ROM_BYTES,
            &assets.view.video, &assets.video) :
        vm_profile_machine_file_variable_load(config->video_path,
            VM_PROFILE_EXTERNAL_PC_AT_VIDEO_ROM_MAX_BYTES, &assets.view.video, &assets.video);
    if (status == TYPE_STATUS_OK && config->font_path != STD_NULL) status =
        vm_profile_machine_file_load(config->font_path, VM_MACHINE_TEXT_CHARACTER_GENERATOR_BYTES,
            &assets.view.font, &assets.font);
    if (status == TYPE_STATUS_OK) status = vm_profile_machine_plan_create(config,
        &assets.view, out_plan);
    vm_profile_machine_file_assets_destroy(&assets);
    return status;
}

C_VOID vm_profile_machine_plan_destroy(vm_profile_machine_plan *plan)
{
    if (plan == STD_NULL) return;
    if (plan->kind == VM_PROFILE_MACHINE_PLAN_XT) {
        STD_FREE(plan->firmware.xt.system);
        STD_FREE(plan->firmware.xt.xebec);
        STD_FREE(plan->firmware.xt.video);
    }
    STD_FREE(plan);
}

const core_machine_config *vm_profile_machine_plan_core_config_get(
    const vm_profile_machine_plan *plan) { return plan == STD_NULL ? STD_NULL : &plan->core_config; }
const core_machine_controller_timing_rules *vm_profile_machine_plan_timing_rules_get(
    const vm_profile_machine_plan *plan) { return plan == STD_NULL ? STD_NULL : &plan->timing_rules; }
const core_machine_plan_topology *vm_profile_machine_plan_topology_get(
    const vm_profile_machine_plan *plan) { return plan == STD_NULL ? STD_NULL : &plan->topology; }
const core_machine_firmware_provider *vm_profile_machine_plan_firmware_provider_get(
    const vm_profile_machine_plan *plan) { return plan == STD_NULL ? STD_NULL : plan->firmware_provider; }
C_VOID *vm_profile_machine_plan_firmware_context_get(vm_profile_machine_plan *plan)
{ return plan == STD_NULL ? STD_NULL : plan->firmware_context; }
vm_profile_floppy_kind vm_profile_machine_plan_drive_floppy_get(
    const vm_profile_machine_plan *plan) { return plan == STD_NULL ? VM_PROFILE_FLOPPY_35_1440K : plan->drive_floppy; }
vm_profile_floppy_kind vm_profile_machine_plan_media_floppy_get(
    const vm_profile_machine_plan *plan) { return plan == STD_NULL ? VM_PROFILE_FLOPPY_35_1440K : plan->media_floppy; }
type_bool vm_profile_machine_plan_hdc_present(const vm_profile_machine_plan *plan)
{ return plan != STD_NULL && plan->hdc_present; }
type_bool vm_profile_machine_plan_memory_reconfigurable(const vm_profile_machine_plan *plan)
{ return plan != STD_NULL && plan->memory_reconfigurable; }
type_unsigned_8 vm_profile_machine_plan_floppy_slot_count(const vm_profile_machine_plan *plan)
{ return plan == STD_NULL ? 0u : plan->floppy_slot_count; }
const vm_profile_default_pc_at_descriptor *
vm_profile_machine_plan_pc_at_descriptor_get(const vm_profile_machine_plan *plan)
{
    return plan == STD_NULL || (plan->kind != VM_PROFILE_MACHINE_PLAN_DEFAULT_PC_AT &&
        plan->kind != VM_PROFILE_MACHINE_PLAN_IBM_5170) ? STD_NULL :
        &plan->profile.pc_at.descriptor;
}
const vm_profile_model40_external_rom *vm_profile_machine_plan_model40_rom_get(
    const vm_profile_machine_plan *plan)
{
    return plan == STD_NULL || plan->kind != VM_PROFILE_MACHINE_PLAN_MODEL40 ? STD_NULL :
        &plan->firmware.model40.context;
}
type_bool vm_profile_machine_plan_is_model40(const vm_profile_machine_plan *plan)
{ return plan != STD_NULL && plan->kind == VM_PROFILE_MACHINE_PLAN_MODEL40; }
type_bool vm_profile_machine_plan_external_firmware(const vm_profile_machine_plan *plan)
{ return plan != STD_NULL && plan->firmware_provider != STD_NULL; }

type_status vm_profile_machine_plan_copy_cmos_seed(const vm_profile_machine_plan *plan,
    type_unsigned_8 *out_seed, type_bool *out_present)
{
    if (plan == STD_NULL || out_seed == STD_NULL || out_present == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_present = plan->cmos_seed_present;
    if (plan->cmos_seed_present) STD_MEMCPY(out_seed, plan->cmos_seed,
        VM_MACHINE_CMOS_SEED_BYTES);
    return TYPE_STATUS_OK;
}

type_status vm_profile_machine_plan_copy_text_glyphs(const vm_profile_machine_plan *plan,
    core_machine_vadp_text_glyph_config *out_glyphs)
{
    if (plan == STD_NULL || out_glyphs == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_glyphs = plan->text_glyphs;
    return TYPE_STATUS_OK;
}

static type_status vm_profile_machine_plan_materialize_pc_at(
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
        {0u, 0u, 0u, 0u}, 0u
    };
    core_machine_fdc_config fdc = {0};

    if (plan == STD_NULL || core_plan == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    profile = &plan->profile.pc_at.descriptor;
    if (!vm_profile_default_pc_at_descriptor_is_valid(profile)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
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
    if (dor_port == STD_NULL || status_port == STD_NULL || data_port == STD_NULL ||
        control_port == STD_NULL || route == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
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
    STD_MEMCPY(drives.cylinder_count, profile->fdc_cylinder_count,
        sizeof(drives.cylinder_count));
    drives.track_zero_active_low_mask = profile->fdc_track_zero_active_low_mask;
    fdc.diagnostic_port = profile->fdc_diagnostic_port;
    fdc.diagnostic_read_value = profile->fdc_diagnostic_read_value;
    if (core_machine_plan_configure_fdc(core_plan, &drives, &fdc) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!profile->hdc_present) return TYPE_STATUS_OK;
    return core_machine_plan_configure_hdc(core_plan, VM_PROFILE_MACHINE_HDD_MEDIA_ID,
        CORE_MACHINE_MEDIA_ID_INVALID, &profile->hdc);
}

type_status vm_profile_machine_plan_materialize(vm_profile_machine_plan *plan,
    core_machine_plan *core_plan,
    core_machine_fdc_terminal_observation_provider terminal_observation)
{
    if (plan == STD_NULL || core_plan == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (plan->kind == VM_PROFILE_MACHINE_PLAN_MODEL40) {
        return vm_profile_model40_materialize_plan(core_plan, terminal_observation);
    }
    if (plan->kind == VM_PROFILE_MACHINE_PLAN_XT) return TYPE_STATUS_OK;
    return vm_profile_machine_plan_materialize_pc_at(plan, core_plan);
}

type_bool vm_profile_machine_plan_hdd_geometry_get(const vm_profile_machine_plan *plan,
    type_unsigned_16 *out_cylinders, type_unsigned_8 *out_heads,
    type_unsigned_8 *out_sectors)
{
    if (plan == STD_NULL || plan->kind != VM_PROFILE_MACHINE_PLAN_MODEL40 ||
        out_cylinders == STD_NULL || out_heads == STD_NULL || out_sectors == STD_NULL) {
        return TYPE_FALSE;
    }
    *out_cylinders = 925u;
    *out_heads = 5u;
    *out_sectors = 17u;
    return TYPE_TRUE;
}
