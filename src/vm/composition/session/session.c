#include "type.h"

#include "vm/composition/session/session.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/display.h"
#include "vm/composition/session/media.h"
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

        if (vm_profile_default_keyboard_map_host_key(
                request->data.key_event.scan_code,
                request->data.key_event.virtual_key,
                request->data.key_event.pressed, &sequence) ==
            TYPE_STATUS_OK) {
            (C_VOID)core_machine_keyboard_submit_scan_codes(session->core_machine,
                sequence.bytes, sequence.count);
        }
    } else if (request->kind == VM_PLATFORM_REQUEST_MOUSE_EVENT) {
        vm_profile_default_mouse_report report;

        if (vm_profile_default_mouse_map_host_relative(
                request->data.mouse_event.delta_x,
                request->data.mouse_event.delta_y,
                request->data.mouse_event.buttons, &report) == TYPE_STATUS_OK) {
            (C_VOID)core_machine_mouse_submit_relative(session->core_machine,
                report.delta_x, report.delta_y, report.buttons);
        }
    }
}

static uint16_t vm_session_read_u16(const C_VOID *source)
{
    uint16_t value;

    STD_MEMCPY(&value, source, sizeof(value));
    return value;
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

C_INT vm_session_insert_fdd(vm_session *session, const C_CHAR *path)
{
    if (session == STD_NULL || !vm_session_copy_path(session->fdd_image_path,
            sizeof(session->fdd_image_path), path) ||
        vm_machine_fdd_insert_for(&session->fdd, path) != 0) return -1;
    session->retained_config.fdd_image = session->fdd_image_path;
    vm_session_apply_boot_preference(session);
    return 0;
}

C_INT vm_session_insert_hdd(vm_session *session, const C_CHAR *path)
{
    if (session == STD_NULL || !vm_session_copy_path(session->hdd_image_path,
            sizeof(session->hdd_image_path), path) ||
        vm_machine_hdd_insert(&session->hdd, path) != 0) return -1;
    session->retained_config.hdd_image = session->hdd_image_path;
    vm_session_apply_boot_preference(session);
    return 0;
}

C_VOID vm_session_apply_boot_preference(vm_session *session)
{
    C_INT boot_hdd;

    if (session == STD_NULL) return;
    boot_hdd = session->boot_preference == VM_SESSION_BOOT_PREFERENCE_HDD ||
        (session->boot_preference == VM_SESSION_BOOT_PREFERENCE_AUTO &&
            !session->fdd.connect.flagDiskExist && session->hdd.connect.flagDiskExist);
    vm_profile_default_bios_set_boot_hdd(&session->default_bios, boot_hdd);
}

C_VOID vm_session_set_boot_hdd(vm_session *session, C_INT enabled)
{
    if (session == STD_NULL) return;
    session->boot_preference = enabled ? VM_SESSION_BOOT_PREFERENCE_HDD :
        VM_SESSION_BOOT_PREFERENCE_FDD;
    session->retained_config.boot_hdd = enabled != 0;
    vm_session_apply_boot_preference(session);
}



C_VOID vm_session_storage_initialize(vm_session *machine)
{
    core_machine_profile_binding profile_binding;

    if (machine == STD_NULL || machine->core_machine != STD_NULL) return;
    if (machine->profile == STD_NULL) {
        machine->profile = vm_profile_default_pc_at_descriptor_get();
    }
    if (machine->profile == STD_NULL) return;
    {
        if (core_machine_create(&machine->core_machine_config,
                &machine->core_machine) != TYPE_STATUS_OK) {
            core_machine_destroy(machine->core_machine);
            machine->core_machine = STD_NULL;
            return;
        }
    }
    if (core_machine_profile_binding_initialize(machine->core_machine,
            &profile_binding) != TYPE_STATUS_OK) {
        core_machine_destroy(machine->core_machine);
        machine->core_machine = STD_NULL;
        return;
    }
    if (core_machine_profile_binding_configure_text_raster(&profile_binding,
            &machine->profile->cga_text_timing) != TYPE_STATUS_OK) {
        core_machine_destroy(machine->core_machine);
        machine->core_machine = STD_NULL;
        return;
    }
    if (core_machine_profile_binding_configure_ega_sequencer(&profile_binding,
            &machine->profile->ega_sequencer) != TYPE_STATUS_OK) {
        core_machine_destroy(machine->core_machine);
        machine->core_machine = STD_NULL;
        return;
    }
    if (core_machine_profile_binding_configure_ega_controllers(&profile_binding,
            &machine->profile->ega_controllers) != TYPE_STATUS_OK) {
        core_machine_destroy(machine->core_machine);
        machine->core_machine = STD_NULL;
        return;
    }
    vm_profile_default_context_initialize(&machine->default_profile_context,
        &machine->default_bios, profile_binding,
        STD_NULL, VM_SESSION_MEDIA_HDD_ID);
    core_machine_media_registry_initialize(&machine->media_registry);
    machine->default_profile_context.media_registry = &machine->media_registry;
    core_machine_display_provider_slot_initialize(&machine->display_provider);
    machine->default_profile_context.display_provider = &machine->display_provider;
    core_platform_presentation_mailbox_initialize(&machine->presentation_mailbox);
    core_product_debug_context_initialize(&machine->debugger_context);
    machine->display_generation = 0u;
}

C_VOID vm_session_storage_finalize(vm_session *machine)
{
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    core_platform_presentation_mailbox_finalize(&machine->presentation_mailbox);
    core_machine_media_registry_finalize(&machine->media_registry);
    core_machine_display_provider_slot_finalize(&machine->display_provider);
    core_machine_destroy(machine->core_machine);
    machine->core_machine = STD_NULL;
}

C_INT vm_session_create(const vm_session_config *config, vm_session **out_session)
{
    vm_session *session;

    if (out_session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    session->profile = vm_profile_default_pc_at_descriptor_get();
    if (session->profile == STD_NULL) {
        STD_FREE(session);
        return TYPE_STATUS_FAULT;
    }
    if (config != STD_NULL) {
        session->retained_config = *config;
        session->core_machine_config.memory_bytes = config->memory_bytes;
        session->core_machine_config.cpu_profile = config->cpu_profile;
        session->core_machine_config.fpu_profile = config->fpu_profile;
        session->core_machine_config.ticks_per_instruction =
            session->profile->ticks_per_instruction;
        session->core_machine_config.instruction_timing =
            session->profile->instruction_timing;
        session->core_machine_config.clock_plan = session->profile->clock_plan;
        session->core_machine_config.kbc_typematic_initial_ticks =
            session->profile->kbc_typematic_initial_ticks;
        session->core_machine_config.kbc_typematic_repeat_ticks =
            session->profile->kbc_typematic_repeat_ticks;
        session->core_machine_config.kbc_command_response_ticks =
            session->profile->kbc_command_response_ticks;
    } else {
        session->core_machine_config.memory_bytes = session->profile->default_memory_bytes;
        session->core_machine_config.cpu_profile = session->profile->cpu_profile;
        session->core_machine_config.fpu_profile = session->profile->fpu_profile;
        session->core_machine_config.ticks_per_instruction =
            session->profile->ticks_per_instruction;
        session->core_machine_config.instruction_timing =
            session->profile->instruction_timing;
        session->core_machine_config.clock_plan = session->profile->clock_plan;
        session->core_machine_config.kbc_typematic_initial_ticks =
            session->profile->kbc_typematic_initial_ticks;
        session->core_machine_config.kbc_typematic_repeat_ticks =
            session->profile->kbc_typematic_repeat_ticks;
        session->core_machine_config.kbc_command_response_ticks =
            session->profile->kbc_command_response_ticks;
    }
    vm_session_initialize(session);
    if (session->core_machine == STD_NULL) {
        STD_FREE(session);
        return TYPE_STATUS_FAULT;
    }
    if (config != STD_NULL &&
        ((config->fdd_image != STD_NULL && vm_session_insert_fdd(session,
            config->fdd_image)) ||
         (config->hdd_image != STD_NULL && vm_session_insert_hdd(session,
            config->hdd_image)))) {
        vm_session_destroy(session);
        return TYPE_STATUS_FAULT;
    }
    if (config != STD_NULL && config->create_fdd) vm_machine_fdd_create_for(&session->fdd);
    if (config != STD_NULL && config->create_hdd_cylinders != 0u) {
        vm_machine_hdd_create(&session->hdd, config->create_hdd_cylinders);
    }
    if (config != STD_NULL && config->boot_hdd) {
        session->boot_preference = VM_SESSION_BOOT_PREFERENCE_HDD;
    }
    vm_session_apply_boot_preference(session);
    vm_session_control_reset(&session->control);
    *out_session = session;
    return TYPE_STATUS_OK;
}

type_status vm_session_reconfigure_memory(vm_session *session,
    STD_SIZE_T memory_bytes)
{
    if (session == STD_NULL ||
        vm_session_control_is_running(&session->control) ||
        vm_platform_run_handle_is_active(&session->platform_run_handle)) {
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
    STD_FREE(session);
}

C_INT vm_session_get_reset_vector(const vm_session *session,
    vm_session_reset_vector *out_vector)
{
    core_machine_observation observation;

    if (session == STD_NULL || session->core_machine == STD_NULL ||
        out_vector == STD_NULL) return TYPE_STATUS_INVALID_STATE;
    if (core_machine_capture_observation(session->core_machine, &observation) !=
        TYPE_STATUS_OK) return TYPE_STATUS_INVALID_STATE;
    out_vector->cs = observation.cpu.cs;
    out_vector->ip = (uint16_t)observation.cpu.eip;
    return TYPE_STATUS_OK;
}
