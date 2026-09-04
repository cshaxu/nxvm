#include "type.h"

#include "vm/composition/session/session_private.h"

#include "core/platform/file.h"
#include "vm/profile/byob/blob.h"

static type_status vm_session_asset_copy(type_unsigned_8 *destination,
    STD_SIZE_T expected_bytes, vm_session_asset_bytes source)
{
    if (destination == STD_NULL || source.data == STD_NULL ||
        source.bytes != expected_bytes) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_MEMCPY(destination, source.data, expected_bytes);
    return TYPE_STATUS_OK;
}

static type_status vm_session_cmos_seed_copy(vm_session *session,
    vm_session_asset_bytes source)
{
    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (source.data == STD_NULL && source.bytes == 0u) return TYPE_STATUS_OK;
    if (vm_session_asset_copy(session->cmos_seed, VM_SESSION_CMOS_SEED_BYTES,
            source) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_ARGUMENT;
    session->cmos_seed_present = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

static type_status vm_session_pc_at_rom_copy(vm_session *session,
    const vm_session_config *config, const vm_session_assets *assets)
{
    STD_SIZE_T index;

    if (session == STD_NULL || config == STD_NULL || assets == STD_NULL ||
        config->bios_count == 0u || config->bios_count > 2u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (config->bios_count == 1u) {
        if (vm_session_asset_copy(session->pc_at_rom, VM_SESSION_PC_AT_ROM_BYTES,
                assets->bios[0u]) != TYPE_STATUS_OK) return TYPE_STATUS_INVALID_ARGUMENT;
    } else {
        if (assets->bios[0u].data == STD_NULL || assets->bios[1u].data == STD_NULL ||
            assets->bios[0u].bytes != VM_SESSION_PC_AT_ROM_CHIP_BYTES ||
            assets->bios[1u].bytes != VM_SESSION_PC_AT_ROM_CHIP_BYTES) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        for (index = 0u; index < VM_SESSION_PC_AT_ROM_CHIP_BYTES; ++index) {
            session->pc_at_rom[index * 2u] = assets->bios[0u].data[index];
            session->pc_at_rom[index * 2u + 1u] = assets->bios[1u].data[index];
        }
    }
    if (assets->video.data != STD_NULL) {
        if (assets->video.bytes == 0u ||
            assets->video.bytes > VM_SESSION_PC_AT_VIDEO_ROM_MAX_BYTES) {
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        STD_MEMCPY(session->pc_at_video_rom, assets->video.data,
            assets->video.bytes);
        session->pc_at_video_rom_bytes = assets->video.bytes;
    } else if (assets->video.bytes != 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    session->pc_at_rom_external = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/display.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/profile/default_profile/keyboard_mapper.h"
#include "vm/profile/default_profile/mouse_mapper.h"

C_VOID vm_session_consume_request(
    C_VOID *opaque, const vm_platform_request *request)
{
    vm_session *session = (vm_session *)opaque;

    if (session == STD_NULL || !session->active || request == STD_NULL) return;
    if (request->kind == VM_PLATFORM_REQUEST_KEY_EVENT) {
        vm_profile_default_keyboard_sequence sequence;
        type_unsigned_8 native_scan_set;

        if (core_machine_keyboard_get_native_scan_set(session->core_machine,
                &native_scan_set) == TYPE_STATUS_OK &&
            vm_profile_default_keyboard_map_host_key_for_scan_set(
                request->data.key_event.scan_code,
                request->data.key_event.virtual_key, request->data.key_event.pressed,
                native_scan_set, &sequence) ==
            TYPE_STATUS_OK) {
            (C_VOID)core_machine_keyboard_receive_native_bytes(session->core_machine,
                sequence.bytes, sequence.count);
        }
    } else if (request->kind == VM_PLATFORM_REQUEST_MOUSE_EVENT) {
        vm_profile_default_mouse_report report;

        if (vm_profile_default_mouse_map_host_relative(
                request->data.mouse_event.delta_x,
                request->data.mouse_event.delta_y,
                request->data.mouse_event.buttons, &report) == TYPE_STATUS_OK) {
            (C_VOID)core_machine_mouse_receive_relative(session->core_machine,
                report.delta_x, report.delta_y, report.buttons);
        }
    }
}

const C_CHAR *vm_session_profile_name(vm_session_profile_kind kind)
{
    if (kind == VM_SESSION_PROFILE_DEFAULT_PC_AT) return "default-pc-at";
    if (kind == VM_SESSION_PROFILE_IBM_5170_MODEL_339) {
        return "ibm-5170-model-339";
    }
    if (kind == VM_SESSION_PROFILE_IBM_5160_MODEL_268) {
        return "ibm-5160-model-268";
    }
    if (kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        return "compaq-deskpro-386-model-40";
    }
    return "unknown";
}

static C_INT vm_session_copy_path(C_CHAR *destination, STD_SIZE_T capacity,
    const C_CHAR *source)
{
    STD_SIZE_T length;

    if (destination == STD_NULL || capacity == 0u) return 0;
    destination[0] = '\0';
    if (source == STD_NULL) return 1;
    length = STD_STRLEN(source);
    if (length >= capacity) return 0;
    STD_MEMCPY(destination, source, length + 1u);
    return 1;
}

static type_status vm_session_retain_font_path(vm_session *session,
    const C_CHAR *path)
{
    if (session == STD_NULL || !vm_session_copy_path(session->font_path,
            sizeof(session->font_path), path)) return TYPE_STATUS_INVALID_ARGUMENT;
    session->retained_config.font_path = path == STD_NULL ? STD_NULL :
        session->font_path;
    return TYPE_STATUS_OK;
}

type_status vm_session_submit_host_input(vm_session *session,
    const core_platform_input_event *event)
{
    if (session == STD_NULL || !session->active) return TYPE_STATUS_INVALID_STATE;
    if (event == STD_NULL || (event->kind != CORE_PLATFORM_INPUT_KEY &&
        event->kind != CORE_PLATFORM_INPUT_RELATIVE_MOUSE)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return core_platform_input_source_submit(session->input_source, event);
}

static const C_CHAR *vm_session_config_floppy(const vm_session_config *config,
    STD_SIZE_T slot)
{
    if (config == STD_NULL || slot >= VM_SESSION_FLOPPY_SLOT_COUNT) return STD_NULL;
    return config->floppy_image[slot];
}

static const C_CHAR *vm_session_config_fixed_disk(const vm_session_config *config,
    STD_SIZE_T slot)
{
    if (config == STD_NULL || slot >= VM_SESSION_FIXED_DISK_SLOT_COUNT) return STD_NULL;
    return config->fixed_disk_image[slot];
}

type_status vm_session_apply_cmos_seed(const vm_session *session,
    core_machine_plan_topology *topology)
{
    STD_SIZE_T index;

    if (session == STD_NULL || topology == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (!session->cmos_seed_present) return TYPE_STATUS_OK;
    if (!topology->rtc_cmos_present) return TYPE_STATUS_INVALID_STATE;
    /* A seed is a board configuration image, not a replacement RTC state:
     * copy only the manual-defined CMOS NVRAM 0Eh--3Fh.  It is the sole
     * owner of board configuration, including the firmware checksum. */
    for (index = 0u; index < CORE_MACHINE_RTC_DEFAULT_CAPACITY; ++index) {
        topology->rtc_cmos.defaults[index] = (core_machine_rtc_default_byte) {
            (type_unsigned_8)(0x0eu + index), session->cmos_seed[0x0eu + index] };
    }
    topology->rtc_cmos.default_count = CORE_MACHINE_RTC_DEFAULT_CAPACITY;
    topology->rtc_cmos.derive_configuration_checksum = TYPE_FALSE;
    return TYPE_STATUS_OK;
}

type_status vm_session_get_speed(const vm_session *session,
    vm_session_speed *out_speed)
{
    if (session == STD_NULL || !session->active || out_speed == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_speed = session->speed;
    return TYPE_STATUS_OK;
}

type_status vm_session_set_speed(vm_session *session, vm_session_speed speed)
{
    if (session == STD_NULL || !session->active ||
        (speed != VM_SESSION_SPEED_STANDARD && speed != VM_SESSION_SPEED_TURBO)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (vm_session_control_is_running(&session->control)) return TYPE_STATUS_INVALID_STATE;
    session->speed = speed;
    return TYPE_STATUS_OK;
}

static type_status vm_session_default_at_floppy_select(const vm_session_config *config,
    vm_profile_floppy_kind *out_kind)
{
    vm_session_floppy_format format;

    if (out_kind == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    format = config == STD_NULL ? VM_SESSION_FLOPPY_FORMAT_PROFILE_DEFAULT :
        config->floppy_format;
    switch (format) {
    case VM_SESSION_FLOPPY_FORMAT_PROFILE_DEFAULT:
    case VM_SESSION_FLOPPY_FORMAT_1440K:
        *out_kind = VM_PROFILE_FLOPPY_35_1440K;
        return TYPE_STATUS_OK;
    case VM_SESSION_FLOPPY_FORMAT_1200K:
        *out_kind = VM_PROFILE_FLOPPY_525_1200K;
        return TYPE_STATUS_OK;
    case VM_SESSION_FLOPPY_FORMAT_720K:
        *out_kind = VM_PROFILE_FLOPPY_35_720K;
        return TYPE_STATUS_OK;
    case VM_SESSION_FLOPPY_FORMAT_360K:
        *out_kind = VM_PROFILE_FLOPPY_525_360K;
        return TYPE_STATUS_OK;
    default:
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
}

static C_VOID vm_session_default_at_request_create(const vm_session_config *config,
    vm_profile_floppy_kind floppy_kind, vm_profile_default_at_request *out_request)
{
    if (out_request == STD_NULL) return;
    *out_request = (vm_profile_default_at_request) {0};
    if (config == STD_NULL) return;
    if (config->cpu_profile != CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        config->fpu_profile != CORE_MACHINE_FPU_PROFILE_NONE) {
        out_request->requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU;
        out_request->cpu_profile = config->cpu_profile;
        out_request->fpu_profile = config->fpu_profile;
    }
    if (config->memory_bytes != 0u) {
        out_request->requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY;
        out_request->memory_bytes = config->memory_bytes;
    }
    if (floppy_kind != VM_PROFILE_FLOPPY_35_1440K) {
        out_request->requested_options |= VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY;
        out_request->floppy_cmos_type = vm_profile_floppy_cmos_type_get(floppy_kind);
    }
}

static type_status vm_session_default_at_resolve(vm_session *session,
    const vm_session_config *config)
{
    vm_profile_default_at_request request;

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_session_default_at_floppy_select(config, &session->floppy_kind) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    session->fdd_media_kind = session->floppy_kind;
    vm_session_default_at_request_create(config, session->floppy_kind, &request);
    if (vm_profile_default_at_child_resolve(&request, &session->default_at_resolved) !=
        TYPE_STATUS_OK) return TYPE_STATUS_INVALID_ARGUMENT;
    session->profile = &session->default_at_resolved.descriptor;
    session->profile_topology = &session->default_at_resolved.topology;
    session->core_machine_config =
        session->default_at_resolved.resolved.values.core.configuration;
    session->controller_timing_rules =
        session->default_at_resolved.resolved.values.core.controller_timing_rules;
    return TYPE_STATUS_OK;
}

static type_status vm_session_ibm_5170_floppy_select(const vm_session_config *config,
    vm_profile_floppy_kind *out_kind)
{
    const vm_session_floppy_format format = config == STD_NULL ?
        VM_SESSION_FLOPPY_FORMAT_PROFILE_DEFAULT : config->floppy_format;

    if (out_kind == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (format == VM_SESSION_FLOPPY_FORMAT_PROFILE_DEFAULT ||
        format == VM_SESSION_FLOPPY_FORMAT_1200K) {
        *out_kind = VM_PROFILE_FLOPPY_525_1200K;
        return TYPE_STATUS_OK;
    }
    if (format == VM_SESSION_FLOPPY_FORMAT_360K) {
        *out_kind = VM_PROFILE_FLOPPY_525_360K;
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_INVALID_ARGUMENT;
}

static C_VOID vm_session_storage_rollback(vm_session *machine)
{
    if (machine == STD_NULL) return;
    core_platform_presentation_mailbox_destroy(machine->presentation_mailbox);
    machine->presentation_mailbox = STD_NULL;
    core_product_debugger_destroy(machine->debugger);
    machine->debugger = STD_NULL;
    core_machine_destroy(machine->core_machine);
    machine->core_machine = STD_NULL;
    core_machine_display_provider_slot_destroy(machine->display_provider);
    machine->display_provider = STD_NULL;
    core_machine_media_registry_destroy(machine->media_registry);
    machine->media_registry = STD_NULL;
    core_machine_plan_destroy(machine->core_machine_plan);
    machine->core_machine_plan = STD_NULL;
}

static C_INT vm_session_insert_floppy_at(vm_session *session, STD_SIZE_T slot,
    const C_CHAR *path)
{
    C_CHAR candidate[sizeof(session->floppy_image_path[slot])];

    if (session == STD_NULL || slot >= VM_SESSION_FLOPPY_SLOT_COUNT ||
        (slot != 0u && !session->model40_private) ||
        vm_session_control_is_running(&session->control) ||
        !vm_session_copy_path(candidate, sizeof(candidate), path) ||
        vm_machine_fdd_insert_for(&session->floppy[slot], candidate) != 0 ||
        !vm_session_copy_path(session->floppy_image_path[slot],
            sizeof(session->floppy_image_path[slot]),
            candidate)) return -1;
    session->retained_config.floppy_image[slot] = session->floppy_image_path[slot];
    return 0;
}

C_INT vm_session_insert_fdd(vm_session *session, const C_CHAR *path)
{ return vm_session_insert_floppy_at(session, 0u, path); }

C_INT vm_session_remove_fdd(vm_session *session, const C_CHAR *path)
{
    if (session == STD_NULL || vm_session_control_is_running(&session->control) ||
        vm_machine_fdd_remove_for(&session->fdd, path) != 0) return -1;
    session->fdd_image_path[0] = '\0';
    session->retained_config.floppy_image[0u] = STD_NULL;
    return 0;
}
static C_INT vm_session_insert_hdd_at_startup(vm_session *session,
    const C_CHAR *path)
{
    C_CHAR candidate[sizeof(session->hdd_image_path)];

    if (session == STD_NULL || session->model40_private ||
        (!session->xt_private && (session->profile == STD_NULL ||
            !session->profile->hdc_present)) || !vm_session_copy_path(candidate,
            sizeof(candidate), path) || vm_machine_hdd_insert(&session->hdd, candidate) != 0 ||
        !vm_session_copy_path(session->hdd_image_path, sizeof(session->hdd_image_path),
            candidate)) return -1;
    session->retained_config.fixed_disk_image[0u] = session->hdd_image_path;
    return 0;
}

C_INT vm_session_insert_hdd(vm_session *session, const C_CHAR *path)
{
    (C_VOID)session;
    (C_VOID)path;
    return -1;
}
type_status vm_session_storage_initialize(vm_session *machine)
{
    core_machine_plan_topology topology = {0};
    type_status status;

    if (machine == STD_NULL || machine->core_machine != STD_NULL) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (machine->model40_private) {
        return vm_session_model40_storage_initialize(machine);
    }
    if (!machine->xt_private && machine->profile == STD_NULL &&
        vm_session_default_at_resolve(machine, STD_NULL) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (!machine->xt_private && !vm_profile_default_pc_at_descriptor_is_valid(machine->profile)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_plan_create(&machine->core_machine_config,
        &machine->core_machine_plan);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_plan_set_controller_timing_rules(
        machine->core_machine_plan, &machine->controller_timing_rules);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_media_registry_create(&machine->media_registry);
    if (status != TYPE_STATUS_OK) {
        vm_session_storage_rollback(machine);
        return status;
    }
    status = core_machine_display_provider_slot_create(&machine->display_provider);
    if (status != TYPE_STATUS_OK) {
        vm_session_storage_rollback(machine);
        return status;
    }
    vm_session_bind_display(machine);
    if (machine->profile_topology != STD_NULL) {
        topology = *machine->profile_topology;
    } else {
        /* Direct white-box fixtures retain their descriptor failure coverage
         * through the same profile owner; product sessions always copy first. */
        status = vm_profile_default_pc_at_topology_materialize(machine->profile,
            &machine->controller_timing_rules, &topology);
        if (status != TYPE_STATUS_OK) {
            vm_session_storage_rollback(machine);
            return status;
        }
    }
    status = vm_session_apply_cmos_seed(machine, &topology);
    if (status != TYPE_STATUS_OK) { vm_session_storage_rollback(machine); return status; }
    status = core_machine_plan_set_topology(machine->core_machine_plan, &topology);
    if (status == TYPE_STATUS_OK) {
        status = core_machine_plan_bind_media_registry(machine->core_machine_plan,
            machine->media_registry);
    }
    if (status == TYPE_STATUS_OK) {
        status = core_machine_plan_bind_display_provider(machine->core_machine_plan,
            machine->display_provider);
    }
    if (status != TYPE_STATUS_OK) {
        vm_session_storage_rollback(machine);
        return status;
    }
    status = vm_session_machine_devices_materialize_plan(machine,
        machine->core_machine_plan);
    if (status != TYPE_STATUS_OK) {
        vm_session_storage_rollback(machine);
        return status;
    }
    status = core_machine_create_from_plan(machine->core_machine_plan,
        &machine->core_machine);
    if (status == TYPE_STATUS_OK) {
        status = core_machine_get_fdc_dma_request_binding(machine->core_machine,
            &machine->fdc_dma_request);
    }
    if (status != TYPE_STATUS_OK) {
        vm_session_storage_rollback(machine);
        return status;
    }
    if (machine->firmware_kind == VM_SESSION_FIRMWARE_EXTERNAL_PC_AT_ROM)
        machine->pc_at_rom_context.image = machine->pc_at_rom;
    if (machine->firmware_kind == VM_SESSION_FIRMWARE_EXTERNAL_PC_AT_ROM) {
        machine->pc_at_rom_context.video = machine->pc_at_video_rom_bytes == 0u ?
            STD_NULL : machine->pc_at_video_rom;
        machine->pc_at_rom_context.video_bytes = machine->pc_at_video_rom_bytes;
    }
    if (core_platform_presentation_mailbox_create(&machine->presentation_mailbox) !=
        TYPE_STATUS_OK) {
        vm_session_storage_rollback(machine);
        return TYPE_STATUS_NO_MEMORY;
    }
    status = core_product_debugger_create(&machine->debugger);
    if (status != TYPE_STATUS_OK) {
        vm_session_storage_rollback(machine);
        return status;
    }
    machine->display_generation = 0u;
    return TYPE_STATUS_OK;
}

C_VOID vm_session_storage_finalize(vm_session *machine)
{
    if (machine == STD_NULL) return;
    core_product_debugger_destroy(machine->debugger);
    machine->debugger = STD_NULL;
    core_platform_presentation_mailbox_destroy(machine->presentation_mailbox);
    machine->presentation_mailbox = STD_NULL;
    core_machine_destroy(machine->core_machine);
    machine->core_machine = STD_NULL;
    core_machine_display_provider_slot_destroy(machine->display_provider);
    machine->display_provider = STD_NULL;
    core_machine_media_registry_destroy(machine->media_registry);
    machine->media_registry = STD_NULL;
    core_machine_plan_destroy(machine->core_machine_plan);
    machine->core_machine_plan = STD_NULL;
}

static type_status vm_session_initialize_model40_configuration(vm_session *session,
    const vm_session_config *config)
{
    const vm_session_floppy_format format = config == STD_NULL ?
        VM_SESSION_FLOPPY_FORMAT_PROFILE_DEFAULT : config->floppy_format;

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    session->model40_private = 1;
    session->firmware_kind = VM_SESSION_FIRMWARE_MODEL40_BYOB;
    session->floppy_kind = VM_PROFILE_FLOPPY_525_1200K;
    if (format == VM_SESSION_FLOPPY_FORMAT_PROFILE_DEFAULT ||
        format == VM_SESSION_FLOPPY_FORMAT_1200K) {
        session->fdd_media_kind = VM_PROFILE_FLOPPY_525_1200K;
        return TYPE_STATUS_OK;
    }
    if (format == VM_SESSION_FLOPPY_FORMAT_360K) {
        /* A 1.2MB drive remains the sole physical drive; this selects only
           compatible 48-TPI media for its one removable-media provider. */
        session->fdd_media_kind = VM_PROFILE_FLOPPY_525_360K;
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_INVALID_ARGUMENT;
}

static type_status vm_session_create_xt_from_assets(const vm_session_config *config,
    const vm_session_assets *assets, vm_session **out_session)
{
    vm_session *session;
    vm_profile_xt_5160_268_external_rom source_rom;
    type_status status;

    if (config == STD_NULL || assets == STD_NULL || out_session == STD_NULL ||
        config->bios_count == 0u || config->bios_count > 2u ||
        config->cmos_seed != STD_NULL || config->memory_bytes != 0u ||
        (config->floppy_format != VM_SESSION_FLOPPY_FORMAT_PROFILE_DEFAULT &&
         config->floppy_format != VM_SESSION_FLOPPY_FORMAT_360K) || config->create_fdd ||
        config->create_hdd_cylinders != 0u ||
        config->cpu_profile != CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        config->fpu_profile != CORE_MACHINE_FPU_PROFILE_NONE ||
        vm_session_config_fixed_disk(config, 1u) != STD_NULL ||
        vm_session_config_floppy(config, 1u) != STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = vm_profile_xt_5160_268_external_rom_create(assets->bios[0u].data,
        assets->bios[0u].bytes, config->bios_count == 2u ? assets->bios[1u].data :
        STD_NULL, config->bios_count == 2u ? assets->bios[1u].bytes : 0u,
        assets->video.data, assets->video.bytes, &source_rom);
    if (status != TYPE_STATUS_OK) return status;
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    session->xt_private = 1;
    session->firmware_kind = VM_SESSION_FIRMWARE_XT_BYOB;
    session->floppy_kind = VM_PROFILE_FLOPPY_525_360K;
    session->fdd_media_kind = VM_PROFILE_FLOPPY_525_360K;
    if (vm_profile_xt_5160_268_resolve(&session->xt_resolved,
            source_rom.xebec_present) != TYPE_STATUS_OK) {
        STD_FREE(session);
        return TYPE_STATUS_FAULT;
    }
    session->profile_topology = &session->xt_resolved.topology;
    session->core_machine_config = session->xt_resolved.resolved.values.core.configuration;
    session->controller_timing_rules =
        session->xt_resolved.resolved.values.core.controller_timing_rules;
    session->xt_system_rom = (type_unsigned_8 *)STD_MALLOC(
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
    session->xt_xebec_rom = (type_unsigned_8 *)STD_MALLOC(
        VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES);
    if (source_rom.video_bytes != STD_NULL) {
        session->xt_video_rom = (type_unsigned_8 *)STD_MALLOC(source_rom.video_byte_count);
    }
    if (session->xt_system_rom == STD_NULL || session->xt_xebec_rom == STD_NULL ||
        (source_rom.video_bytes != STD_NULL && session->xt_video_rom == STD_NULL)) {
        vm_session_destroy(session);
        return TYPE_STATUS_NO_MEMORY;
    }
    STD_MEMCPY(session->xt_system_rom, source_rom.system_bytes,
        VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES);
    if (source_rom.xebec_present) STD_MEMCPY(session->xt_xebec_rom,
        source_rom.xebec_bytes, VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES);
    if (source_rom.video_bytes != STD_NULL) STD_MEMCPY(session->xt_video_rom,
        source_rom.video_bytes, source_rom.video_byte_count);
    session->xt_rom = source_rom;
    session->xt_rom.system_bytes = session->xt_system_rom;
    session->xt_rom.xebec_bytes = source_rom.xebec_present ? session->xt_xebec_rom : STD_NULL;
    session->xt_rom.video_bytes = source_rom.video_bytes == STD_NULL ? STD_NULL :
        session->xt_video_rom;
    session->retained_config = *config;
    session->retained_config.bios_path[0u] = STD_NULL;
    session->retained_config.bios_path[1u] = STD_NULL;
    if (vm_session_retain_font_path(session, config->font_path) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = vm_session_initialize(session);
    if (status != TYPE_STATUS_OK) { vm_session_destroy(session); return status; }
    if ((vm_session_config_floppy(config, 0u) != STD_NULL &&
            vm_session_insert_fdd(session, vm_session_config_floppy(config, 0u))) ||
        (vm_session_config_fixed_disk(config, 0u) != STD_NULL &&
            vm_session_insert_hdd_at_startup(session,
                vm_session_config_fixed_disk(config, 0u)))) {
        vm_session_destroy(session);
        return TYPE_STATUS_FAULT;
    }
    status = vm_session_reset(session);
    if (status != TYPE_STATUS_OK) { vm_session_destroy(session); return status; }
    *out_session = session;
    return TYPE_STATUS_OK;
}

static type_status vm_session_create_model40_from_assets(
    const vm_session_config *config, const vm_session_assets *assets,
    vm_session **out_session)
{
    vm_session *session;
    vm_profile_model40_external_rom source_rom;
    type_status status;

    if (config == STD_NULL || assets == STD_NULL || out_session == STD_NULL ||
        config->bios_count != 2u ||
        (config->memory_bytes != 0u && config->memory_bytes != 1024u * 1024u) ||
        vm_session_config_fixed_disk(config, 1u) != STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    status = vm_session_initialize_model40_configuration(session, config);
    if (status == TYPE_STATUS_OK) status = vm_profile_model40_child_resolve(
        &session->model40_resolved);
    if (status == TYPE_STATUS_OK) {
        session->core_machine_config =
            session->model40_resolved.values.core.configuration;
        session->controller_timing_rules =
            session->model40_resolved.values.core.controller_timing_rules;
        status = vm_profile_model40_external_rom_create(assets->bios[0u].data,
            assets->bios[0u].bytes, assets->bios[1u].data, assets->bios[1u].bytes,
            assets->video.data, assets->video.bytes, &source_rom);
    }
    if (status == TYPE_STATUS_OK) {
        STD_MEMCPY(session->model40_even_rom, assets->bios[0u].data,
            VM_PROFILE_MODEL40_ROM_CHIP_BYTES);
        STD_MEMCPY(session->model40_odd_rom, assets->bios[1u].data,
            VM_PROFILE_MODEL40_ROM_CHIP_BYTES);
        if (assets->video.data != STD_NULL) {
            STD_MEMCPY(session->model40_video_rom, assets->video.data,
                VM_PROFILE_MODEL40_VIDEO_ROM_BYTES);
        }
        session->model40_rom = source_rom;
        session->model40_rom.even_bytes = session->model40_even_rom;
        session->model40_rom.odd_bytes = session->model40_odd_rom;
        session->model40_rom.video_bytes = assets->video.data == STD_NULL ? STD_NULL :
            session->model40_video_rom;
    }
    if (status != TYPE_STATUS_OK || vm_session_cmos_seed_copy(session,
            assets->cmos_seed) != TYPE_STATUS_OK) {
        STD_FREE(session);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    session->retained_config = *config;
    session->retained_config.cmos_seed = STD_NULL;
    session->retained_config.bios_path[0u] = STD_NULL;
    session->retained_config.bios_path[1u] = STD_NULL;
    session->retained_config.video_path = STD_NULL;
    if (vm_session_retain_font_path(session, config->font_path) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = vm_session_initialize(session);
    if (status != TYPE_STATUS_OK) { vm_session_destroy(session); return status; }
    if ((vm_session_config_floppy(config, 0u) != STD_NULL &&
            vm_session_insert_fdd(session, vm_session_config_floppy(config, 0u))) ||
        (vm_session_config_floppy(config, 1u) != STD_NULL &&
            vm_session_insert_floppy_at(session, 1u, vm_session_config_floppy(config, 1u))) ||
        (vm_session_config_fixed_disk(config, 0u) != STD_NULL &&
            vm_session_model40_insert_hdd_at_startup(session,
                vm_session_config_fixed_disk(config, 0u)))) {
        vm_session_destroy(session);
        return TYPE_STATUS_FAULT;
    }
    status = vm_session_reset(session);
    if (status != TYPE_STATUS_OK) { vm_session_destroy(session); return status; }
    *out_session = session;
    return TYPE_STATUS_OK;
}

type_status vm_session_create_from_assets(const vm_session_config *config,
    const vm_session_assets *assets, vm_session **out_session)
{
    vm_session *session;
    type_status status;

    if (config != STD_NULL && config->profile_kind ==
        VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        return vm_session_create_model40_from_assets(config, assets, out_session);
    }
    if (config != STD_NULL && config->profile_kind == VM_SESSION_PROFILE_IBM_5160_MODEL_268) {
        return vm_session_create_xt_from_assets(config, assets, out_session);
    }
    if (config == STD_NULL || assets == STD_NULL || out_session == STD_NULL ||
        (config->profile_kind != VM_SESSION_PROFILE_DEFAULT_PC_AT &&
         config->profile_kind != VM_SESSION_PROFILE_IBM_5170_MODEL_339) ||
        config->fixed_disk_image[1u] != STD_NULL || config->floppy_image[1u] != STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (config->profile_kind == VM_SESSION_PROFILE_DEFAULT_PC_AT) {
        status = vm_session_default_at_resolve(session, config);
    } else {
        status = vm_session_ibm_5170_floppy_select(config, &session->fdd_media_kind);
        /* The 5170's 1.2 MB drive is an immutable board fact. The YAML
         * format selects the removable medium only (including 360 KB). */
        session->floppy_kind = VM_PROFILE_FLOPPY_525_1200K;
        if (status == TYPE_STATUS_OK) status = vm_profile_ibm_5170_root_resolve_memory(
            config->memory_bytes, &session->ibm_5170_root);
        if (status == TYPE_STATUS_OK) {
            session->profile = &session->ibm_5170_root.descriptor;
            session->profile_topology = &session->ibm_5170_root.topology;
            session->core_machine_config =
                session->ibm_5170_root.resolved.values.core.configuration;
            session->controller_timing_rules =
                session->ibm_5170_root.resolved.values.core.controller_timing_rules;
        }
    }
    if (status != TYPE_STATUS_OK || vm_session_cmos_seed_copy(session,
            assets->cmos_seed) != TYPE_STATUS_OK || vm_session_pc_at_rom_copy(session,
            config, assets) != TYPE_STATUS_OK || (!session->profile->hdc_present &&
            (vm_session_config_fixed_disk(config, 0u) != STD_NULL ||
             config->create_hdd_cylinders != 0u))) {
        STD_FREE(session);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    session->firmware_kind = VM_SESSION_FIRMWARE_EXTERNAL_PC_AT_ROM;
    session->retained_config = *config;
    session->retained_config.cmos_seed = STD_NULL;
    session->retained_config.bios_path[0u] = STD_NULL;
    session->retained_config.bios_path[1u] = STD_NULL;
    if (vm_session_retain_font_path(session, config->font_path) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = vm_session_initialize(session);
    if (status != TYPE_STATUS_OK) { vm_session_destroy(session); return status; }
    if ((vm_session_config_floppy(config, 0u) != STD_NULL &&
            vm_session_insert_fdd(session, vm_session_config_floppy(config, 0u))) ||
        (vm_session_config_fixed_disk(config, 0u) != STD_NULL &&
            vm_session_insert_hdd_at_startup(session,
                vm_session_config_fixed_disk(config, 0u)))) {
        vm_session_destroy(session);
        return TYPE_STATUS_FAULT;
    }
    if (config->create_fdd) vm_machine_fdd_create_for(&session->fdd);
    if (session->profile->hdc_present && config->create_hdd_cylinders != 0u) {
        vm_machine_hdd_create(&session->hdd, config->create_hdd_cylinders);
    }
    status = vm_session_reset(session);
    if (status != TYPE_STATUS_OK) { vm_session_destroy(session); return status; }
    *out_session = session;
    return TYPE_STATUS_OK;
}

typedef struct vm_session_file_assets {
    type_unsigned_8 *bios[2];
    type_unsigned_8 *cmos_seed;
    type_unsigned_8 *video;
    vm_session_assets view;
} vm_session_file_assets;

static C_VOID vm_session_file_assets_destroy(vm_session_file_assets *assets)
{
    if (assets == STD_NULL) return;
    STD_FREE(assets->bios[0u]);
    STD_FREE(assets->bios[1u]);
    STD_FREE(assets->cmos_seed);
    STD_FREE(assets->video);
    *assets = (vm_session_file_assets) {0};
}

static type_status vm_session_file_asset_load(const C_CHAR *path,
    STD_SIZE_T bytes, vm_session_asset_bytes *out_view,
    type_unsigned_8 **out_owned)
{
    type_unsigned_8 *owned;

    if (path == STD_NULL || out_view == STD_NULL || out_owned == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    owned = (type_unsigned_8 *)STD_MALLOC(bytes);
    if (owned == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (vm_profile_byob_blob_load(&(vm_profile_byob_blob) { path, STD_NULL, bytes },
            owned) != TYPE_STATUS_OK) {
        STD_FREE(owned);
        return TYPE_STATUS_FAULT;
    }
    *out_view = (vm_session_asset_bytes) { owned, bytes };
    *out_owned = owned;
    return TYPE_STATUS_OK;
}

static type_status vm_session_file_variable_asset_load(const C_CHAR *path,
    STD_SIZE_T maximum, vm_session_asset_bytes *out_view,
    type_unsigned_8 **out_owned)
{
    C_VOID *owned = STD_NULL;
    STD_SIZE_T bytes = 0u;

    if (path == STD_NULL || out_view == STD_NULL || out_owned == STD_NULL ||
        maximum == 0u || core_platform_file_read_all(path, maximum, &owned, &bytes) ||
        bytes == 0u) {
        STD_FREE(owned);
        return TYPE_STATUS_FAULT;
    }
    *out_view = (vm_session_asset_bytes) { owned, bytes };
    *out_owned = owned;
    return TYPE_STATUS_OK;
}

static type_status vm_session_file_assets_load(const vm_session_config *config,
    vm_session_file_assets *assets)
{
    STD_SIZE_T bios_bytes;
    type_status status;

    if (config == STD_NULL || assets == STD_NULL || config->bios_count == 0u ||
        config->bios_count > 2u || (config->profile_kind !=
            VM_SESSION_PROFILE_IBM_5160_MODEL_268 && config->cmos_seed == STD_NULL)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *assets = (vm_session_file_assets) {0};
    if (config->profile_kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        if (config->bios_count != 2u) return TYPE_STATUS_INVALID_ARGUMENT;
        bios_bytes = VM_PROFILE_MODEL40_ROM_CHIP_BYTES;
    } else if (config->profile_kind == VM_SESSION_PROFILE_IBM_5160_MODEL_268) {
        bios_bytes = VM_PROFILE_XT_5160_268_SYSTEM_ROM_BYTES;
    } else {
        bios_bytes = config->bios_count == 1u ? VM_SESSION_PC_AT_ROM_BYTES :
            VM_SESSION_PC_AT_ROM_CHIP_BYTES;
    }
    status = vm_session_file_asset_load(config->bios_path[0u], bios_bytes,
        &assets->view.bios[0u], &assets->bios[0u]);
    if (status == TYPE_STATUS_OK && config->bios_count == 2u) {
        const STD_SIZE_T secondary_bytes = config->profile_kind ==
            VM_SESSION_PROFILE_IBM_5160_MODEL_268 ?
            VM_PROFILE_XT_5160_268_XEBEC_ROM_BYTES : bios_bytes;
        status = vm_session_file_asset_load(config->bios_path[1u], secondary_bytes,
            &assets->view.bios[1u], &assets->bios[1u]);
    }
    if (status == TYPE_STATUS_OK && config->cmos_seed != STD_NULL) {
        status = vm_session_file_asset_load(config->cmos_seed,
            VM_SESSION_CMOS_SEED_BYTES, &assets->view.cmos_seed,
            &assets->cmos_seed);
    }
    if (status == TYPE_STATUS_OK && config->video_path != STD_NULL) {
        status = config->profile_kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40 ?
            vm_session_file_asset_load(config->video_path,
                VM_PROFILE_MODEL40_VIDEO_ROM_BYTES, &assets->view.video,
                &assets->video) : vm_session_file_variable_asset_load(config->video_path,
                VM_SESSION_PC_AT_VIDEO_ROM_MAX_BYTES, &assets->view.video,
                &assets->video);
    }
    if (status != TYPE_STATUS_OK) vm_session_file_assets_destroy(assets);
    return status;
}

static type_status vm_session_create_file_backed(const vm_session_config *config,
    vm_session **out_session)
{
    vm_session_file_assets assets;
    type_status status = vm_session_file_assets_load(config, &assets);

    if (status != TYPE_STATUS_OK) return status;
    status = vm_session_create_from_assets(config, &assets.view, out_session);
    vm_session_file_assets_destroy(&assets);
    return status;
}

C_INT vm_session_create(const vm_session_config *config, vm_session **out_session)
{
    if (config == STD_NULL || out_session == STD_NULL ||
        vm_session_config_fixed_disk(config, 1u) != STD_NULL ||
        (config->profile_kind != VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40 &&
         vm_session_config_floppy(config, 1u) != STD_NULL)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return vm_session_create_file_backed(config, out_session);
}

type_status vm_session_reconfigure_memory(vm_session *session,
    STD_SIZE_T memory_bytes)
{
    if (session == STD_NULL ||
        session->model40_private ||
        vm_session_control_is_running(&session->control) ||
        vm_platform_run_handle_is_active(session->platform_run_handle)) {
        return TYPE_STATUS_INVALID_STATE;
    }
    if (core_machine_reconfigure_memory(session->core_machine, memory_bytes) !=
        TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    session->retained_config.memory_bytes = memory_bytes;
    session->core_machine_config.memory_bytes = memory_bytes;
    vm_machine_debug_reset(&session->debug);
    vm_session_publish_display(session, 1);
    return TYPE_STATUS_OK;
}

C_VOID vm_session_destroy(vm_session *session)
{
    if (session == STD_NULL) return;
    vm_session_finalize(session);
    STD_FREE(session->xt_system_rom);
    STD_FREE(session->xt_xebec_rom);
    STD_FREE(session->xt_video_rom);
    STD_FREE(session);
}

type_status vm_session_get_reset_vector(const vm_session *session,
    vm_session_reset_vector *out_vector)
{
    core_machine_observation observation;

    if (session == STD_NULL || session->core_machine == STD_NULL ||
        out_vector == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    if (core_machine_capture_observation(session->core_machine, &observation) !=
        TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    out_vector->cs = observation.cpu.cs;
    out_vector->ip = (type_unsigned_16)observation.cpu.eip;
    return TYPE_STATUS_OK;
}
