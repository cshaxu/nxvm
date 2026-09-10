/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "type.h"

#include "vm/composition/session/session_private.h"
#include "vm/composition/session/rom/external_pc_at.h"

#include "vm/composition/session/control.h"

#include "vm/composition/session/provider_lifecycle.h"

#include "vm/composition/session/session_interface.h"

#include "core/product/debug/debug_target.h"

#include "core/product/utils.h"
#include "core/utils/wait_provider.h"

#include "lib/host/sync_interface.h"

#include "vm/composition/session/debug_target.h"

#include "vm/machine/debug.h"

#include "core/machine/machine_interface.h"

#include "core/machine/guest_input_interface.h"

#include "vm/composition/session/display.h"
#include "vm/composition/session/waiting.h"


#include "vm/composition/session/lifecycle.h"

static C_VOID vm_session_wait(C_VOID *context, type_unsigned_32 milliseconds)
{
    (C_VOID)context;
    host_sync_sleep_milliseconds(milliseconds);
}

static type_status vm_session_debug_disassemble(C_VOID *context,
    C_CHAR *statement, STD_SIZE_T statement_capacity,
    const type_unsigned_8 *code, STD_SIZE_T code_bytes,
    STD_SIZE_T *out_code_bytes, C_INT flag32)
{
    (C_VOID)context;
    return core_product_utils_disassemble(code, code_bytes, statement,
        statement_capacity, out_code_bytes, flag32);
}

static C_VOID vm_session_input_submit(C_VOID *context,
    const core_machine_guest_input_event *event)
{
    vm_session *machine = (vm_session *)context;
    vm_session_request request;

    if (machine == STD_NULL || event == STD_NULL) return;
    if (event->kind == CORE_MACHINE_GUEST_INPUT_KEY) {
        request.kind = VM_SESSION_REQUEST_KEY_EVENT;
        request.data.key_event.scan_code = event->data.key.scan_code;
        request.data.key_event.virtual_key = event->data.key.virtual_key;
        request.data.key_event.pressed = event->data.key.pressed;
    } else if (event->kind == CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE) {
        request.kind = VM_SESSION_REQUEST_MOUSE_EVENT;
        request.data.mouse_event.delta_x = event->data.relative_mouse.delta_x;
        request.data.mouse_event.delta_y = event->data.relative_mouse.delta_y;
        request.data.mouse_event.buttons = event->data.relative_mouse.buttons;
    } else {
        return;
    }
    (C_VOID)vm_session_request_transport_enqueue_ingress(
        machine->request_transport, &request);
}

static C_VOID vm_session_execution_provider_reset(C_VOID *context)
{
    vm_session_provider_lifecycle_reset((vm_session *)context);
}

static const core_machine_execution_provider vm_session_execution_provider = {
    vm_session_execution_provider_reset,
    STD_NULL
};

type_status vm_session_bind_execution_provider(vm_session *machine)
{
    type_status status;
    const core_machine_firmware_provider *firmware_provider;
    C_VOID *firmware_context;

    if (machine == STD_NULL || machine->core_machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (machine->firmware_kind == VM_SESSION_FIRMWARE_MODEL40_BYOB) {
        firmware_provider = vm_profile_model40_firmware_provider();
        firmware_context = &machine->model40_rom;
    } else if (machine->firmware_kind == VM_SESSION_FIRMWARE_XT_BYOB) {
        firmware_provider = vm_profile_xt_5160_268_firmware_provider();
        firmware_context = &machine->xt_rom;
    } else {
        firmware_provider = vm_session_external_pc_at_rom_provider();
        firmware_context = &machine->pc_at_rom_context;
    }
    status = core_machine_bind_firmware_provider(machine->core_machine,
        firmware_provider, firmware_context);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_bind_execution_provider(machine->core_machine,
        &vm_session_execution_provider, machine);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_freeze_execution_providers(machine->core_machine);
}

static const core_machine_guest_input_sink vm_session_input_sink = {
    vm_session_input_submit
};

static C_VOID vm_session_debug_request_pause(C_VOID *context,
    vm_machine_debug_pause_reason reason)
{
    vm_session_control_request_pause(
        &((vm_session *)context)->control,
        reason == VM_MACHINE_DEBUG_PAUSE_TRACE ?
        VM_SESSION_PAUSE_TRACE : VM_SESSION_PAUSE_BREAKPOINT);
}

static C_VOID vm_session_execution_task_main(C_VOID *opaque,
    const host_sync_task *task)
{
    vm_session *machine = (vm_session *)opaque;

    (C_VOID)task;
    if (machine == STD_NULL) return;
    host_sync_event_signal(machine->execution_started);
    vm_session_control_start(&machine->control);
}

/* Composition owns the host task that executes its one Core machine.  It is
 * deliberately not a presenter or product-control resource. */
static C_VOID vm_session_execution_join(vm_session *machine)
{
    if (machine == STD_NULL || machine->execution_task == STD_NULL) return;
    host_sync_task_join(machine->execution_task);
    host_sync_task_destroy(machine->execution_task);
    machine->execution_task = STD_NULL;
}

static C_VOID vm_session_execution_stop(vm_session *machine)
{
    if (machine == STD_NULL) return;
    vm_session_control_stop(&machine->control);
}

/* Resume is an observable lifecycle request, not merely successful thread
 * creation.  The caller may immediately issue pause/input work, so wait until
 * the session state accepts it before reporting success. */
static C_INT vm_session_execution_wait_until_active(vm_session *machine)
{
    C_UINT elapsed;

    if (machine == STD_NULL) return TYPE_FALSE;
    for (elapsed = 0u; elapsed < 5000u; ++elapsed) {
        if (vm_session_control_is_running(&machine->control) ||
            vm_session_control_is_paused(&machine->control)) return TYPE_TRUE;
        if (machine->execution_task == STD_NULL) return TYPE_FALSE;
        host_sync_sleep_milliseconds(1u);
    }
    return TYPE_FALSE;
}

static C_VOID vm_session_start_outcome_reset(vm_session *machine)
{
    if (machine == STD_NULL) return;
    machine->start_outcome.status = TYPE_STATUS_OK;
    machine->start_outcome.valid = TYPE_FALSE;
}

static type_status vm_session_start_outcome_record(vm_session *machine,
    type_status status)
{
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ++machine->start_outcome.sequence;
    machine->start_outcome.status = status;
    machine->start_outcome.valid = TYPE_TRUE;
    return status;
}

type_status vm_session_start(vm_session *machine) {
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = vm_session_reset(machine);
    if (status != TYPE_STATUS_OK) return status;
    return vm_session_resume(machine);
}

type_status vm_session_finish_reset(vm_session *machine, type_status status)
{
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (status != TYPE_STATUS_OK) return vm_session_start_outcome_record(machine,
        status);
    vm_session_pacing_reset(machine);
    machine->display_snapshot_generation_valid = TYPE_FALSE;
    machine->model40_fdc_terminal_observation_valid = TYPE_FALSE;
    vm_session_start_outcome_reset(machine);
    if (!vm_session_control_is_running(&machine->control)) {
        vm_session_publish_display(machine, 1);
    }
    vm_session_report_lifecycle(machine, VM_SESSION_RESET);
    return TYPE_STATUS_OK;
}

type_status vm_session_reset(vm_session *machine) {
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (machine->execution_task != STD_NULL &&
        !vm_session_control_is_running(&machine->control)) vm_session_execution_join(machine);
    status = vm_session_control_reset(&machine->control);
    if (vm_session_control_is_running(&machine->control)) return status;
    return vm_session_finish_reset(machine, status);
}

C_VOID vm_session_stop(vm_session *machine) {
    if (machine == STD_NULL) return;
    vm_session_execution_stop(machine);
    vm_session_execution_join(machine);
}

type_status vm_session_request_pause(vm_session *machine)
{
    if (machine == STD_NULL || !vm_session_control_is_running(&machine->control))
        return TYPE_STATUS_INVALID_STATE;
    vm_session_control_request_pause(&machine->control, VM_SESSION_PAUSE_EXPLICIT);
    return TYPE_STATUS_OK;
}

C_INT vm_session_is_running(const vm_session *machine)
{
    return machine != STD_NULL && vm_session_control_is_running(&machine->control);
}

void vm_session_set_lifecycle_reporter(vm_session *machine,
    vm_session_lifecycle_reporter reporter, C_VOID *context)
{
    if (machine == STD_NULL) return;
    machine->lifecycle_reporter = reporter;
    machine->lifecycle_reporter_context = context;
}

void vm_session_set_display_reporter(vm_session *machine,
    vm_session_display_reporter reporter, C_VOID *context)
{
    if (machine == STD_NULL) return;
    machine->display_reporter = reporter;
    machine->display_reporter_context = context;
}

void vm_session_report_lifecycle(vm_session *machine,
    vm_session_lifecycle lifecycle)
{
    if (machine != STD_NULL && machine->lifecycle_reporter != STD_NULL) {
        machine->lifecycle_reporter(machine->lifecycle_reporter_context, lifecycle);
    }
}

type_status vm_session_resume(vm_session *machine) {
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_session_control_is_running(&machine->control)) {
        return vm_session_start_outcome_record(machine, TYPE_STATUS_INVALID_STATE);
    }
    if (vm_session_control_is_paused(&machine->control) &&
        machine->execution_task != STD_NULL) {
        vm_session_control_continue(&machine->control);
        return vm_session_start_outcome_record(machine, TYPE_STATUS_OK);
    }
    if (machine->execution_task != STD_NULL) return vm_session_start_outcome_record(
        machine, TYPE_STATUS_INVALID_STATE);
    host_sync_event_reset(machine->execution_started);
    if (host_sync_task_create(vm_session_execution_task_main, machine,
            &machine->execution_task) != LIB_STATUS_OK ||
        host_sync_event_wait(machine->execution_started, 5000u) != HOST_SYNC_WAIT_SIGNALED ||
        !vm_session_execution_wait_until_active(machine)) {
        vm_session_execution_stop(machine);
        vm_session_execution_join(machine);
        return vm_session_start_outcome_record(machine, TYPE_STATUS_INVALID_STATE);
    }
    return vm_session_start_outcome_record(machine, TYPE_STATUS_OK);
}

type_status vm_session_initialize(vm_session *machine) {
    type_status status;
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (machine->active) return TYPE_STATUS_INVALID_STATE;
    status = vm_session_storage_initialize(machine);
    if (status != TYPE_STATUS_OK) return status;
    core_utils_wait_scope_initialize(&machine->wait_scope,
        vm_session_wait, STD_NULL);
    status = vm_session_control_initialize(&machine->control, machine);
    if (status != TYPE_STATUS_OK) {
        vm_session_finalize(machine);
        return status;
    }
    vm_machine_debug_bind_pause(&machine->debug,
        vm_session_debug_request_pause, STD_NULL);
    vm_machine_debug_bind_disassembler(&machine->debug,
        vm_session_debug_disassemble, STD_NULL);
    status = vm_session_request_transport_create(&machine->request_transport);
    if (status != TYPE_STATUS_OK) { vm_session_finalize(machine); return status; }
    vm_session_request_transport_bind_consumer(machine->request_transport,
        vm_session_consume_request, machine);
    status = core_machine_guest_input_source_create(&vm_session_input_sink, machine,
        &machine->input_source);
    if (status != TYPE_STATUS_OK) {
        vm_session_finalize(machine);
        return status;
    }
    if (host_sync_event_create(&machine->execution_started) != LIB_STATUS_OK) {
        vm_session_finalize(machine); return TYPE_STATUS_NO_MEMORY;
    }
    vm_session_start_outcome_reset(machine);
    vm_session_control_bind_command_boundary(&machine->control,
        vm_session_request_transport_observe_execution_boundary,
        machine->request_transport);
    machine->active = 1;
    return TYPE_STATUS_OK;
}

C_VOID vm_session_finalize(vm_session *machine) {
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    vm_session_execution_stop(machine);
    vm_session_execution_join(machine);
    host_sync_event_destroy(machine->execution_started);
    machine->execution_started = STD_NULL;
    vm_session_control_bind_command_boundary(&machine->control, STD_NULL, STD_NULL);
    core_machine_guest_input_source_destroy(machine->input_source);
    machine->input_source = STD_NULL;
    vm_session_request_transport_close(machine->request_transport);
    vm_session_request_transport_destroy(machine->request_transport);
    machine->request_transport = STD_NULL;
    machine->active = 0;
    vm_session_control_finalize(&machine->control, machine);
    vm_machine_debug_bind_pause(&machine->debug, STD_NULL, STD_NULL);
    vm_machine_debug_bind_disassembler(&machine->debug, STD_NULL, STD_NULL);
    vm_session_debug_target_finalize(machine);
    vm_session_storage_finalize(machine);
}
