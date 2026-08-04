#include "type.h"

#include "vm/composition/session/session.h"

#include "core/machine/machine_interface.h"
#include "core/machine/keyboard_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/display.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/profile/default_profile/firmware/bios.h"

C_INT vm_session_enqueue_keyboard_state(
    C_VOID *opaque, uint32_t asynchronous_keys, uint32_t toggle_keys)
{
    nxvm_platform_vm_request request;

    request.kind = NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE;
    request.data.keyboard_state.asynchronous_keys = asynchronous_keys;
    request.data.keyboard_state.toggle_keys = toggle_keys;
    return vm_platform_request_transport_enqueue_ingress(
        (vm_platform_request_transport *)opaque, &request);
}

C_VOID vm_session_consume_request(
    C_VOID *opaque, const nxvm_platform_vm_request *request)
{
    vm_session *session = (vm_session *)opaque;

    if (session != STD_NULL && session->active && request != STD_NULL &&
        request->kind == NXVM_PLATFORM_VM_REQUEST_KEYBOARD_STATE) {
        core_machine_keyboard_apply_host_state_to(&session->keyboard_provider,
            request->data.keyboard_state.asynchronous_keys,
            request->data.keyboard_state.toggle_keys);
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
    return 0;
}

C_INT vm_session_insert_hdd(vm_session *session, const C_CHAR *path)
{
    if (session == STD_NULL || !vm_session_copy_path(session->hdd_image_path,
            sizeof(session->hdd_image_path), path) ||
        vm_machine_hdd_insert(&session->hdd, path) != 0) return -1;
    session->retained_config.hdd_image = session->hdd_image_path;
    return 0;
}



C_VOID vm_session_storage_initialize(vm_session *machine)
{
    core_machine_cpu_execution_context *execution;
    t_ram *memory;
    t_pic *pic_master;
    t_pic *pic_slave;
    t_vadp *vadp;

    if (machine == STD_NULL || machine->core_machine != STD_NULL) return;
    {
        if (core_machine_create(&machine->core_machine_config,
                &machine->core_machine) != NTVDM64_STATUS_OK) {
            core_machine_destroy(machine->core_machine);
            machine->core_machine = STD_NULL;
            return;
        }
    }
    memory = core_machine_configuration_memory_borrow(machine->core_machine);
    execution = core_machine_configuration_cpu_execution_borrow(machine->core_machine);
    pic_master = core_machine_configuration_shared_pic_master_borrow(machine->core_machine);
    pic_slave = core_machine_configuration_shared_pic_slave_borrow(machine->core_machine);
    core_machine_cpu_execution_context_bind_pic(execution,
        pic_master, pic_slave);
    vadp = core_machine_configuration_shared_vadp_borrow(machine->core_machine);
    vm_profile_default_context_initialize(&machine->default_profile_context,
        &machine->default_bios, &machine->default_qdx, memory,
        vadp, STD_NULL, STD_NULL);
    core_machine_cpu_execution_context_bind_extension(execution,
        &machine->default_profile_context);
    machine->default_profile_context.execution = execution;
    core_machine_block_provider_slot_initialize(&machine->block_provider);
    machine->default_profile_context.block_provider = &machine->block_provider;
    core_machine_keyboard_provider_slot_initialize(&machine->keyboard_provider);
    machine->default_profile_context.keyboard_provider = &machine->keyboard_provider;
    core_machine_display_provider_slot_initialize(&machine->display_provider);
    machine->default_profile_context.display_provider = &machine->display_provider;
    vm_platform_presentation_mailbox_initialize(&machine->presentation_mailbox);
    machine->default_profile_context.wait_scope = &machine->wait_scope;
    core_product_debug_context_initialize(&machine->debugger_context);
    machine->display_generation = 0u;
    machine->control = (vm_session_control_state *)STD_CALLOC(1u,
        sizeof(*machine->control));
}

C_VOID vm_session_storage_finalize(vm_session *machine)
{
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    core_machine_cpu_execution_context_bind_extension(
        core_machine_configuration_cpu_execution_borrow(machine->core_machine), STD_NULL);
    core_machine_block_provider_slot_finalize(&machine->block_provider);
    core_machine_keyboard_provider_slot_finalize(&machine->keyboard_provider);
    core_machine_display_provider_slot_finalize(&machine->display_provider);
    STD_FREE(machine->control);
    machine->control = STD_NULL;
    core_machine_destroy(machine->core_machine);
    machine->core_machine = STD_NULL;
}

C_INT vm_session_create(const vm_session_config *config, vm_session **out_session)
{
    vm_session *session;

    if (out_session == STD_NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    *out_session = STD_NULL;
    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return NTVDM64_STATUS_NO_MEMORY;
    if (config != STD_NULL) {
        session->retained_config = *config;
        session->core_machine_config.memory_bytes = config->memory_bytes;
        session->core_machine_config.cpu_profile = config->cpu_profile;
        session->core_machine_config.fpu_profile = config->fpu_profile;
    }
    vm_session_initialize(session);
    if (session->core_machine == STD_NULL) {
        STD_FREE(session);
        return NTVDM64_STATUS_FAULT;
    }
    if (config != STD_NULL &&
        ((config->fdd_image != STD_NULL && vm_session_insert_fdd(session,
            config->fdd_image)) ||
         (config->hdd_image != STD_NULL && vm_session_insert_hdd(session,
            config->hdd_image)))) {
        vm_session_destroy(session);
        return NTVDM64_STATUS_FAULT;
    }
    if (config != STD_NULL && config->create_fdd) vm_machine_fdd_create_for(&session->fdd);
    if (config != STD_NULL && config->create_hdd_cylinders != 0u) {
        vm_machine_hdd_create(&session->hdd, config->create_hdd_cylinders);
    }
    if (config != STD_NULL) {
        vm_profile_default_bios_set_boot_hdd(&session->default_bios,
            config->boot_hdd != 0);
    }
    vm_session_control_reset(session->control);
    *out_session = session;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_session_reconfigure_memory(vm_session *session,
    STD_SIZE_T memory_bytes)
{
    if (session == STD_NULL || session->control == STD_NULL ||
        vm_session_control_is_running(session->control) ||
        vm_platform_run_handle_is_active(&session->platform_run_handle)) {
        return NTVDM64_STATUS_INVALID_STATE;
    }
    if (core_machine_reconfigure_memory(session->core_machine, memory_bytes) !=
        NTVDM64_STATUS_OK) return NTVDM64_STATUS_INVALID_STATE;
    session->retained_config.memory_bytes = memory_bytes;
    session->core_machine_config.memory_bytes = memory_bytes;
    vm_machine_debug_reset(&session->debug);
    vm_session_publish_display(session, 1);
    return NTVDM64_STATUS_OK;
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
        out_vector == STD_NULL) return NTVDM64_STATUS_INVALID_STATE;
    if (core_machine_capture_observation(session->core_machine, &observation) !=
        NTVDM64_STATUS_OK) return NTVDM64_STATUS_INVALID_STATE;
    out_vector->cs = observation.cpu.cs;
    out_vector->ip = (uint16_t)observation.cpu.eip;
    return NTVDM64_STATUS_OK;
}
