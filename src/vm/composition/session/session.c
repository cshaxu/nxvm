#include "type.h"

#include "vm/composition/session/session_private.h"

#include "vm/profile/byob/blob.h"

static type_status vm_session_cmos_seed_load(vm_session *session,
    const C_CHAR *path)
{
    const vm_profile_byob_blob seed = { path, STD_NULL, VM_SESSION_CMOS_SEED_BYTES };

    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (path == STD_NULL) return TYPE_STATUS_OK;
    if (vm_profile_byob_blob_load(&seed, session->cmos_seed) != TYPE_STATUS_OK) {
        return TYPE_STATUS_FAULT;
    }
    session->cmos_seed_present = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

static type_status vm_session_pc_at_rom_load(vm_session *session,
    const vm_session_config *config)
{
    type_status status;
    STD_SIZE_T index;

    if (session == STD_NULL || config == STD_NULL || config->bios_count == 0u) {
        return TYPE_STATUS_OK;
    }
    if (config->bios_count == 1u) {
        status = vm_profile_byob_blob_load(&(vm_profile_byob_blob) {
            config->bios_path[0u], STD_NULL, VM_SESSION_PC_AT_ROM_BYTES },
            session->pc_at_rom);
    } else if (config->bios_count == 2u) {
        type_unsigned_8 even[VM_SESSION_PC_AT_ROM_CHIP_BYTES];
        type_unsigned_8 odd[VM_SESSION_PC_AT_ROM_CHIP_BYTES];

        status = vm_profile_byob_blob_load(&(vm_profile_byob_blob) {
            config->bios_path[0u], STD_NULL, VM_SESSION_PC_AT_ROM_CHIP_BYTES }, even);
        if (status == TYPE_STATUS_OK) status = vm_profile_byob_blob_load(
            &(vm_profile_byob_blob) { config->bios_path[1u], STD_NULL,
                VM_SESSION_PC_AT_ROM_CHIP_BYTES }, odd);
        if (status == TYPE_STATUS_OK) {
            for (index = 0u; index < VM_SESSION_PC_AT_ROM_CHIP_BYTES; ++index) {
                session->pc_at_rom[index * 2u] = even[index];
                session->pc_at_rom[index * 2u + 1u] = odd[index];
            }
        }
    } else {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (status != TYPE_STATUS_OK) return status;
    session->pc_at_rom_external = TYPE_TRUE;
    return TYPE_STATUS_OK;
}

static const vm_profile_model40_byob_manifest *
vm_session_model40_firmware(const vm_session_config *config,
    vm_profile_model40_byob_manifest *out_manifest)
{
    if (config == STD_NULL || out_manifest == STD_NULL) return STD_NULL;
    if (config->bios_count != 2u) return STD_NULL;
    *out_manifest = (vm_profile_model40_byob_manifest) {
        config->bios_path[0u], STD_NULL, config->bios_path[1u], STD_NULL,
        config->video_path, STD_NULL, STD_NULL };
    return out_manifest;
}

static const vm_profile_xt_5160_268_byob_manifest *
vm_session_xt_firmware(const vm_session_config *config,
    vm_profile_xt_5160_268_byob_manifest *out_manifest)
{
    if (config == STD_NULL || out_manifest == STD_NULL) return STD_NULL;
    if (config->bios_count == 0u || config->bios_count > 2u) return STD_NULL;
    *out_manifest = (vm_profile_xt_5160_268_byob_manifest) {
        config->bios_path[0u], STD_NULL,
        config->bios_count == 2u ? config->bios_path[1u] : STD_NULL,
        STD_NULL, STD_NULL };
    return out_manifest;
}

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/display.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/machine_devices.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/profile/default_profile/firmware/bios.h"
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
    for (index = 0u; index < CORE_MACHINE_RTC_DEFAULT_CAPACITY; ++index) {
        topology->rtc_cmos.defaults[index] = (core_machine_rtc_default_byte) {
            (type_unsigned_8)(0x10u + index), session->cmos_seed[0x10u + index] };
    }
    topology->rtc_cmos.default_count = CORE_MACHINE_RTC_DEFAULT_CAPACITY;
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
    vm_session_apply_boot_preference(session);
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
    vm_session_apply_boot_preference(session);
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
    vm_session_apply_boot_preference(session);
    return 0;
}

C_INT vm_session_insert_hdd(vm_session *session, const C_CHAR *path)
{
    (C_VOID)session;
    (C_VOID)path;
    return -1;
}
C_VOID vm_session_apply_boot_preference(vm_session *session)
{
    C_INT boot_hdd;

    if (session == STD_NULL || session->firmware_kind !=
        VM_SESSION_FIRMWARE_DEFAULT_PC_AT || session->profile == STD_NULL) return;
    if (!session->profile->hdc_present) {
        vm_profile_default_bios_set_boot_hdd(&session->default_bios, 0);
        return;
    }
    boot_hdd = session->boot_preference == VM_SESSION_BOOT_PREFERENCE_HDD ||
        (session->boot_preference == VM_SESSION_BOOT_PREFERENCE_AUTO &&
            !vm_machine_fdd_has_media(&session->fdd) &&
            vm_machine_hdd_has_media(&session->hdd));
    vm_profile_default_bios_set_boot_hdd(&session->default_bios, boot_hdd);
}

C_VOID vm_session_set_boot_hdd(vm_session *session, C_INT enabled)
{
    if (session == STD_NULL || session->profile == STD_NULL ||
        !session->profile->hdc_present) return;
    session->boot_preference = enabled ? VM_SESSION_BOOT_PREFERENCE_HDD :
        VM_SESSION_BOOT_PREFERENCE_FDD;
    session->retained_config.boot_hdd = enabled != 0;
    vm_session_apply_boot_preference(session);
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
    if (machine->firmware_kind == VM_SESSION_FIRMWARE_DEFAULT_PC_AT) {
        vm_profile_default_context_initialize(&machine->default_profile_context,
            &machine->default_bios, machine->media_registry, VM_SESSION_MEDIA_FDD_ID,
            VM_SESSION_MEDIA_HDD_ID,
            machine->profile->firmware_slot, machine->pc_at_rom_external ?
            machine->pc_at_rom : STD_NULL);
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

static type_status vm_session_create_xt_byob(const vm_session_config *config,
    vm_session **out_session)
{
    vm_session *session;
    vm_profile_xt_5160_268_byob_manifest firmware;
    const vm_profile_xt_5160_268_byob_manifest *selected_firmware;
    type_status status;

    selected_firmware = vm_session_xt_firmware(config, &firmware);
    if (config == STD_NULL || out_session == STD_NULL || config->cmos_seed != STD_NULL ||
        config->memory_bytes != 0u ||
        (config->floppy_format != VM_SESSION_FLOPPY_FORMAT_PROFILE_DEFAULT &&
         config->floppy_format != VM_SESSION_FLOPPY_FORMAT_360K) ||
        config->create_fdd ||
        config->create_hdd_cylinders != 0u || config->boot_hdd ||
        config->cpu_profile != CORE_MACHINE_CPU_PROFILE_DEFAULT ||
        config->fpu_profile != CORE_MACHINE_FPU_PROFILE_NONE ||
        !vm_profile_xt_5160_268_byob_manifest_is_valid(selected_firmware)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    session->xt_private = 1;
    session->firmware_kind = VM_SESSION_FIRMWARE_XT_BYOB;
    session->floppy_kind = VM_PROFILE_FLOPPY_525_360K;
    if (vm_profile_xt_5160_268_resolve(&session->xt_resolved,
            selected_firmware->xebec_path != STD_NULL) != TYPE_STATUS_OK) {
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
    if (session->xt_system_rom == STD_NULL || session->xt_xebec_rom == STD_NULL) {
        STD_FREE(session->xt_system_rom);
        STD_FREE(session->xt_xebec_rom);
        STD_FREE(session);
        return TYPE_STATUS_NO_MEMORY;
    }
    status = vm_profile_xt_5160_268_byob_manifest_load(selected_firmware,
        session->xt_system_rom, session->xt_xebec_rom, &session->xt_rom);
    if (status != TYPE_STATUS_OK) {
        STD_FREE(session->xt_system_rom);
        STD_FREE(session->xt_xebec_rom);
        STD_FREE(session);
        return status;
    }
    session->retained_config = *config;
    if (vm_session_retain_font_path(session, config->font_path) != TYPE_STATUS_OK) {
        STD_FREE(session->xt_system_rom);
        STD_FREE(session->xt_xebec_rom);
        STD_FREE(session);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = vm_session_initialize(session);
    if (status != TYPE_STATUS_OK) { vm_session_destroy(session); return status; }
    if ((vm_session_config_floppy(config, 0u) != STD_NULL && vm_session_insert_fdd(session,
            vm_session_config_floppy(config, 0u))) ||
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

static type_status vm_session_create_model40_byob(const vm_session_config *config,
    vm_session **out_session)
{
    vm_session *session;
    vm_profile_model40_byob_manifest firmware;
    const vm_profile_model40_byob_manifest *selected_firmware;
    type_status status;

    selected_firmware = vm_session_model40_firmware(config, &firmware);
    if (config == STD_NULL || out_session == STD_NULL ||
        (config->memory_bytes != 0u && config->memory_bytes != 1024u * 1024u) ||
        !vm_profile_model40_byob_manifest_is_valid(selected_firmware)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    status = vm_session_initialize_model40_configuration(session, config);
    if (status != TYPE_STATUS_OK) {
        STD_FREE(session);
        return status;
    }
    if (vm_profile_model40_child_resolve(&session->model40_resolved) !=
        TYPE_STATUS_OK) {
        STD_FREE(session);
        return TYPE_STATUS_FAULT;
    }
    session->core_machine_config =
        session->model40_resolved.values.core.configuration;
    session->controller_timing_rules =
        session->model40_resolved.values.core.controller_timing_rules;
    status = vm_profile_model40_byob_manifest_load(selected_firmware,
        session->model40_even_rom, session->model40_odd_rom,
        session->model40_video_rom, &session->model40_rom);
    if (status != TYPE_STATUS_OK) { STD_FREE(session); return status; }
    session->retained_config = *config;
    if (vm_session_retain_font_path(session, config->font_path) != TYPE_STATUS_OK) {
        STD_FREE(session);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    status = vm_session_cmos_seed_load(session, config->cmos_seed);
    if (status != TYPE_STATUS_OK) { STD_FREE(session); return status; }
    session->retained_config.cmos_seed = STD_NULL;
    status = vm_session_initialize(session);
    if (status != TYPE_STATUS_OK) { STD_FREE(session); return status; }
    if ((vm_session_config_floppy(config, 0u) != STD_NULL && vm_session_insert_fdd(session,
            vm_session_config_floppy(config, 0u))) ||
        (vm_session_config_floppy(config, 1u) != STD_NULL &&
            vm_session_insert_floppy_at(session, 1u, vm_session_config_floppy(config, 1u))) ||
        (vm_session_config_fixed_disk(config, 0u) != STD_NULL &&
            vm_session_model40_insert_hdd_at_startup(session,
                vm_session_config_fixed_disk(config, 0u)))) {
        vm_session_destroy(session);
        return TYPE_STATUS_FAULT;
    }
    if (config->boot_hdd) session->boot_preference = VM_SESSION_BOOT_PREFERENCE_HDD;
    vm_session_apply_boot_preference(session);
    status = vm_session_reset(session);
    if (status != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return status;
    }
    *out_session = session;
    return TYPE_STATUS_OK;
}
C_INT vm_session_create(const vm_session_config *config, vm_session **out_session)
{
    vm_session *session;
    const vm_session_profile_kind profile_kind = config == STD_NULL ?
        VM_SESSION_PROFILE_DEFAULT_PC_AT : config->profile_kind;

    if (config != STD_NULL &&
        (vm_session_config_fixed_disk(config, 1u) != STD_NULL ||
         (config->profile_kind != VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40 &&
          vm_session_config_floppy(config, 1u) != STD_NULL))) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (config != STD_NULL && config->profile_kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        return vm_session_create_model40_byob(config, out_session);
    }
    if (config != STD_NULL && config->profile_kind == VM_SESSION_PROFILE_IBM_5160_MODEL_268) {
        return vm_session_create_xt_byob(config, out_session);
    }
    if (out_session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    if (profile_kind == VM_SESSION_PROFILE_DEFAULT_PC_AT) {
        if (vm_session_default_at_resolve(session, config) != TYPE_STATUS_OK) {
            STD_FREE(session);
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
    } else if (profile_kind == VM_SESSION_PROFILE_IBM_5170_MODEL_339) {
        if (vm_session_ibm_5170_floppy_select(config, &session->floppy_kind) !=
            TYPE_STATUS_OK || vm_profile_ibm_5170_root_resolve_memory(
                config == STD_NULL ? 0u : config->memory_bytes, &session->ibm_5170_root) !=
            TYPE_STATUS_OK) {
            STD_FREE(session);
            return TYPE_STATUS_FAULT;
        }
        session->profile = &session->ibm_5170_root.descriptor;
        session->profile_topology = &session->ibm_5170_root.topology;
        session->core_machine_config =
            session->ibm_5170_root.resolved.values.core.configuration;
        session->controller_timing_rules =
            session->ibm_5170_root.resolved.values.core.controller_timing_rules;
    } else {
        STD_FREE(session);
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (config != STD_NULL) {
        session->retained_config = *config;
        if (vm_session_retain_font_path(session, config->font_path) != TYPE_STATUS_OK) {
            STD_FREE(session);
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
        if (vm_session_cmos_seed_load(session, config->cmos_seed) != TYPE_STATUS_OK) {
            STD_FREE(session);
            return TYPE_STATUS_FAULT;
        }
        session->retained_config.cmos_seed = STD_NULL;
        if (vm_session_pc_at_rom_load(session, config) != TYPE_STATUS_OK) {
            STD_FREE(session);
            return TYPE_STATUS_FAULT;
        }
        if (!session->profile->hdc_present &&
            (vm_session_config_fixed_disk(config, 0u) != STD_NULL ||
            config->create_hdd_cylinders != 0u || config->boot_hdd)) {
            STD_FREE(session);
            return TYPE_STATUS_INVALID_ARGUMENT;
        }
    }
    {
        type_status status = vm_session_initialize(session);

        if (status != TYPE_STATUS_OK) {
            vm_session_destroy(session);
            return status;
        }
    }
    if (session->core_machine == STD_NULL) {
        vm_session_destroy(session);
        return TYPE_STATUS_FAULT;
    }
    if (config != STD_NULL &&
        ((vm_session_config_floppy(config, 0u) != STD_NULL && vm_session_insert_fdd(session,
            vm_session_config_floppy(config, 0u))) ||
         (vm_session_config_fixed_disk(config, 0u) != STD_NULL &&
            vm_session_insert_hdd_at_startup(session,
                vm_session_config_fixed_disk(config, 0u))))) {
        vm_session_destroy(session);
        return TYPE_STATUS_FAULT;
    }
    if (config != STD_NULL && config->create_fdd) vm_machine_fdd_create_for(&session->fdd);
    if (config != STD_NULL && session->profile->hdc_present &&
        config->create_hdd_cylinders != 0u) {
        vm_machine_hdd_create(&session->hdd, config->create_hdd_cylinders);
    }
    if (config != STD_NULL && session->profile->hdc_present && config->boot_hdd) {
        session->boot_preference = VM_SESSION_BOOT_PREFERENCE_HDD;
    }
    vm_session_apply_boot_preference(session);
    {
        type_status status = vm_session_reset(session);

        if (status != TYPE_STATUS_OK) {
            vm_session_destroy(session);
            return status;
        }
    }
    *out_session = session;
    return TYPE_STATUS_OK;
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
