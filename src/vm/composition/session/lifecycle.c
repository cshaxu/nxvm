/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "type.h"

#include "vm/composition/session/session_private.h"
#include "vm/composition/session/profile_firmware.h"

#include "vm/composition/session/control.h"

#include "vm/composition/session/provider_lifecycle.h"

#include "vm/composition/session/session_interface.h"

#include "core/product/debug/debug_target.h"

#include "core/product/utils.h"
#include "core/utils/wait_provider.h"

#include "core/platform/sleep.h"

#include "vm/composition/session/debug_target.h"

#include "vm/machine/debug.h"

#include "vm/profile/default_profile/firmware/qdcga.h"

#include "vm/profile/default_profile/firmware/qdkeyb.h"


#include "core/machine/machine_interface.h"

#include "vm/platform/execution.h"

#include "core/platform/input_interface.h"

#include "vm/platform/platform.h"


#include "vm/composition/session/display.h"
#include "vm/composition/session/waiting.h"


#include "vm/composition/session/lifecycle.h"

static C_VOID vm_session_wait(C_VOID *context, type_unsigned_32 milliseconds)
{
    (C_VOID)context;
    core_platform_sleep_milliseconds(milliseconds);
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
    const core_platform_input_event *event)
{
    vm_session *machine = (vm_session *)context;
    vm_platform_request request;

    if (machine == STD_NULL || event == STD_NULL) return;
    if (event->kind == CORE_PLATFORM_INPUT_KEY) {
        request.kind = VM_PLATFORM_REQUEST_KEY_EVENT;
        request.data.key_event.scan_code = event->data.key.scan_code;
        request.data.key_event.virtual_key = event->data.key.virtual_key;
        request.data.key_event.pressed = event->data.key.pressed;
    } else if (event->kind == CORE_PLATFORM_INPUT_RELATIVE_MOUSE) {
        request.kind = VM_PLATFORM_REQUEST_MOUSE_EVENT;
        request.data.mouse_event.delta_x = event->data.relative_mouse.delta_x;
        request.data.mouse_event.delta_y = event->data.relative_mouse.delta_y;
        request.data.mouse_event.buttons = event->data.relative_mouse.buttons;
    } else {
        return;
    }
    (C_VOID)vm_platform_request_transport_enqueue_ingress(
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
        firmware_provider = vm_session_profile_firmware_provider();
        firmware_context = &machine->default_profile_context;
    }
    status = core_machine_bind_firmware_provider(machine->core_machine,
        firmware_provider, firmware_context);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_bind_execution_provider(machine->core_machine,
        &vm_session_execution_provider, machine);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_freeze_execution_providers(machine->core_machine);
}

static const core_platform_input_sink vm_session_input_sink = {
    vm_session_input_submit
};

static type_status vm_session_host_input_submit(C_VOID *context,
    const core_platform_input_event *event)
{
    return vm_session_submit_host_input((vm_session *)context, event);
}

static const vm_platform_host_input_sink vm_session_host_input_sink = {
    vm_session_host_input_submit,
    STD_NULL
};

static C_INT vm_session_execution_is_running(C_VOID *context)
{
    return vm_session_control_is_running(
        &((vm_session *)context)->control);
}

static C_INT vm_session_execution_get_flip(C_VOID *context)
{
    return vm_session_control_get_flip(
        &((vm_session *)context)->control);
}

static C_VOID vm_session_execution_start(C_VOID *context)
{
    vm_session_control_start(&((vm_session *)context)->control);
}

static C_VOID vm_session_execution_stop(C_VOID *context)
{
    vm_session_control_stop(&((vm_session *)context)->control);
}

static const vm_platform_execution_sink vm_session_execution_sink = {
    vm_session_execution_is_running,
    vm_session_execution_get_flip,
    vm_session_execution_start,
    vm_session_execution_stop
};

static C_VOID vm_session_debug_request_pause(C_VOID *context,
    vm_machine_debug_pause_reason reason)
{
    vm_session_control_request_pause(
        &((vm_session *)context)->control,
        reason == VM_MACHINE_DEBUG_PAUSE_TRACE ?
        VM_SESSION_PAUSE_TRACE : VM_SESSION_PAUSE_BREAKPOINT);
}

/* Composition is the only owner allowed to tear down its live run handle. */
static C_VOID vm_session_platform_join_and_finalize(vm_session *machine)
{
    if (machine == STD_NULL || !vm_platform_run_handle_is_active(
            machine->platform_run_handle)) return;
    vm_platform_run_handle_join(machine->platform_run_handle);
    vm_platform_run_handle_finalize(machine->platform_run_handle);
}

static C_VOID vm_session_platform_request_stop(vm_session *machine)
{
    if (machine == STD_NULL) return;
    if (!vm_platform_run_handle_is_active(machine->platform_run_handle)) {
        vm_session_control_stop(&machine->control);
        return;
    }
    vm_platform_run_handle_request_stop(machine->platform_run_handle);
}

static C_VOID vm_session_start_outcome_clear(vm_session *machine)
{
    if (machine == STD_NULL) return;
    machine->start_outcome.valid = 0;
    machine->start_outcome.status = TYPE_STATUS_OK;
}

static type_status vm_session_start_outcome_record(vm_session *machine,
    type_status status)
{
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    machine->start_outcome.valid = 1;
    machine->start_outcome.status = status;
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
    machine->model40_fdc_terminal_observation_valid = TYPE_FALSE;
    vm_session_start_outcome_clear(machine);
    if (!vm_session_control_is_running(&machine->control)) {
        vm_session_publish_display(machine, 1);
    }
    return TYPE_STATUS_OK;
}

type_status vm_session_reset(vm_session *machine) {
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_platform_run_handle_is_active(machine->platform_run_handle) &&
        !vm_session_control_is_running(&machine->control)) {
        vm_session_platform_join_and_finalize(machine);
    }
    status = vm_session_control_reset(&machine->control);
    if (vm_session_control_is_running(&machine->control)) return status;
    return vm_session_finish_reset(machine, status);
}

C_VOID vm_session_stop(vm_session *machine) {
    if (machine == STD_NULL) return;
    vm_session_platform_request_stop(machine);
    /* Console runs synchronously in vm_session_resume(), which remains its
     * sole joiner. Window runs return here and need their async teardown. */
    if (vm_platform_run_handle_is_window_display(machine->platform_run_handle)) {
        vm_session_platform_join_and_finalize(machine);
    }
}

type_status vm_session_resume(vm_session *machine) {
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_session_control_is_paused(&machine->control)) {
        vm_session_control_continue(&machine->control);
        return vm_session_start_outcome_record(machine, TYPE_STATUS_OK);
    } else {
        do {
            status = vm_platform_start(machine->platform_run_context,
                machine->platform_run_handle);
            if (status != TYPE_STATUS_OK) {
                return vm_session_start_outcome_record(machine, status);
            }
            if (vm_platform_run_handle_is_window_display(
                    machine->platform_run_handle)) {
                break;
            }
            vm_session_platform_join_and_finalize(machine);
        } while (vm_platform_run_context_take_auto_promotion(
            machine->platform_run_context));
    }
    return vm_session_start_outcome_record(machine, TYPE_STATUS_OK);
}

type_status vm_session_initialize(vm_session *machine) {
    type_status status;
    vm_platform_host_input_sink host_input_sink = vm_session_host_input_sink;

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
    status = vm_platform_execution_transport_create(&vm_session_execution_sink,
        machine, &machine->execution_transport);
    if (status != TYPE_STATUS_OK) { vm_session_finalize(machine); return status; }
    status = vm_platform_request_transport_create(&machine->request_transport);
    if (status != TYPE_STATUS_OK) { vm_session_finalize(machine); return status; }
    vm_platform_request_transport_bind_consumer(machine->request_transport,
        vm_session_consume_request, machine);
    status = core_platform_input_source_create(&vm_session_input_sink, machine,
        &machine->input_source);
    if (status != TYPE_STATUS_OK) {
        vm_session_finalize(machine);
        return status;
    }
    host_input_sink.context = machine;
    status = vm_platform_run_context_create(
        machine->execution_transport, &host_input_sink,
        machine->presentation_mailbox, &machine->wait_scope,
        &machine->platform_run_context);
    if (status != TYPE_STATUS_OK) { vm_session_finalize(machine); return status; }
    status = vm_platform_run_handle_create(&machine->platform_run_handle);
    if (status != TYPE_STATUS_OK) { vm_session_finalize(machine); return status; }
    vm_session_start_outcome_clear(machine);
    vm_session_control_bind_command_boundary(&machine->control,
        vm_platform_request_transport_observe_execution_boundary,
        machine->request_transport);
    machine->active = 1;
    return TYPE_STATUS_OK;
}

C_VOID vm_session_finalize(vm_session *machine) {
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    vm_session_platform_request_stop(machine);
    vm_session_platform_join_and_finalize(machine);
    vm_platform_run_handle_destroy(machine->platform_run_handle);
    machine->platform_run_handle = STD_NULL;
    vm_platform_run_context_destroy(machine->platform_run_context);
    machine->platform_run_context = STD_NULL;
    vm_platform_execution_transport_destroy(machine->execution_transport);
    machine->execution_transport = STD_NULL;
    vm_session_control_bind_command_boundary(&machine->control, STD_NULL, STD_NULL);
    core_platform_input_source_destroy(machine->input_source);
    machine->input_source = STD_NULL;
    vm_platform_request_transport_close(machine->request_transport);
    vm_platform_request_transport_destroy(machine->request_transport);
    machine->request_transport = STD_NULL;
    machine->active = 0;
    vm_session_control_finalize(&machine->control, machine);
    vm_machine_debug_bind_pause(&machine->debug, STD_NULL, STD_NULL);
    vm_machine_debug_bind_disassembler(&machine->debug, STD_NULL, STD_NULL);
    vm_session_debug_target_finalize(machine);
    vm_session_storage_finalize(machine);
}
