/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "type.h"

#include "vm/machine/runtime/machine_private.h"
#include "vm/machine/runtime/control.h"

#include "vm/machine/runtime/machine_devices.h"

#include "vm/machine/runtime/machine_interface.h"


#include "lib/host/sync_interface.h"

#include "vm/machine/runtime/debug_adapter.h"

#include "vm/machine/debug.h"

#include "core/machine/machine_interface.h"

#include "core/machine/guest_input_interface.h"

#include "vm/machine/runtime/display.h"
#include "vm/machine/runtime/waiting.h"


#include "vm/machine/runtime/lifecycle.h"

static C_VOID vm_machine_input_submit(C_VOID *context,
    const core_machine_guest_input_event *event)
{
    vm_machine *machine = (vm_machine *)context;
    common_machine_request request = {0};

    if (machine == STD_NULL || event == STD_NULL) return;
    if (event->kind == CORE_MACHINE_GUEST_INPUT_KEY) {
        request.kind = COMMON_MACHINE_REQUEST_INPUT;
        request.input.kind = COMMON_MACHINE_INPUT_KEY;
        request.input.value.key.scan_code = event->data.key.scan_code;
        request.input.value.key.virtual_key = event->data.key.virtual_key;
        request.input.value.key.pressed = event->data.key.pressed;
    } else if (event->kind == CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE) {
        request.kind = COMMON_MACHINE_REQUEST_INPUT;
        request.input.kind = COMMON_MACHINE_INPUT_RELATIVE_MOUSE;
        request.input.value.mouse.delta_x = event->data.relative_mouse.delta_x;
        request.input.value.mouse.delta_y = event->data.relative_mouse.delta_y;
        request.input.value.mouse.buttons = event->data.relative_mouse.buttons;
    } else {
        return;
    }
    (C_VOID)common_machine_submit(machine->executor, &request);
}

static C_VOID vm_machine_execution_provider_reset(C_VOID *context)
{
    vm_machine_devices_reset((vm_machine *)context);
}

static const core_machine_execution_provider vm_machine_execution_provider = {
    vm_machine_execution_provider_reset,
    STD_NULL
};

type_status vm_machine_bind_execution_provider(vm_machine *machine)
{
    type_status status;
    const core_machine_firmware_provider *firmware_provider;
    C_VOID *firmware_context;

    if (machine == STD_NULL || machine->core_machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (machine->firmware_kind == VM_MACHINE_FIRMWARE_MODEL40_BYOB) {
        firmware_provider = vm_profile_model40_firmware_provider();
        firmware_context = &machine->model40_rom;
    } else if (machine->firmware_kind == VM_MACHINE_FIRMWARE_XT_BYOB) {
        firmware_provider = vm_profile_xt_5160_268_firmware_provider();
        firmware_context = &machine->xt_rom;
    } else {
        firmware_provider = vm_profile_external_pc_at_rom_provider();
        firmware_context = &machine->pc_at_rom_context;
    }
    status = core_machine_bind_firmware_provider(machine->core_machine,
        firmware_provider, firmware_context);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_bind_execution_provider(machine->core_machine,
        &vm_machine_execution_provider, machine);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_freeze_execution_providers(machine->core_machine);
}

static const core_machine_guest_input_sink vm_machine_input_sink = {
    vm_machine_input_submit
};

static lib_bool vm_machine_common_is_paused(void *context)
{
    return vm_machine_control_is_paused(&((vm_machine *)context)->control) ?
        LIB_TRUE : LIB_FALSE;
}

static C_VOID vm_machine_observe_safe_point(C_VOID *context)
{
    (C_VOID)common_machine_observe_safe_point((common_machine *)context);
}

static C_VOID vm_machine_execution_task_main(C_VOID *opaque,
    const host_sync_task *task)
{
    vm_machine *machine = (vm_machine *)opaque;

    (C_VOID)task;
    if (machine == STD_NULL) return;
    host_sync_event_signal(machine->execution_started);
    vm_machine_control_start(&machine->control);
}

/* Composition owns the host task that executes its one Core machine.  It is
 * deliberately not a presenter or product-control resource. */
static C_VOID vm_machine_execution_join(vm_machine *machine)
{
    if (machine == STD_NULL || machine->execution_task == STD_NULL) return;
    host_sync_task_join(machine->execution_task);
    host_sync_task_destroy(machine->execution_task);
    machine->execution_task = STD_NULL;
}

static C_VOID vm_machine_execution_stop(vm_machine *machine)
{
    if (machine == STD_NULL) return;
    vm_machine_control_stop(&machine->control);
}

static type_status vm_machine_enqueue_executor_request(vm_machine *machine,
    common_machine_request_kind kind, vm_machine_pause_reason pause_reason)
{
    common_machine_request request = {0};
    type_status status;

    if (machine == STD_NULL || machine->executor == STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    request.kind = kind;
    request.pause_reason = (type_unsigned_8)pause_reason;
    status = (type_status)common_machine_submit(machine->executor, &request);
    return status;
}

/* Resume is an observable lifecycle request, not merely successful thread
 * creation.  The caller may immediately issue pause/input work, so wait until
 * the session state accepts it before reporting success. */
static C_INT vm_machine_execution_wait_until_active(vm_machine *machine)
{
    C_UINT elapsed;

    if (machine == STD_NULL) return TYPE_FALSE;
    for (elapsed = 0u; elapsed < 5000u; ++elapsed) {
        if (vm_machine_control_is_running(&machine->control) ||
            vm_machine_control_is_paused(&machine->control)) return TYPE_TRUE;
        if (machine->execution_task == STD_NULL) return TYPE_FALSE;
        host_sync_sleep_milliseconds(1u);
    }
    return TYPE_FALSE;
}

static C_VOID vm_machine_start_outcome_reset(vm_machine *machine)
{
    if (machine == STD_NULL) return;
    machine->start_outcome.status = TYPE_STATUS_OK;
    machine->start_outcome.valid = TYPE_FALSE;
}

static type_status vm_machine_start_outcome_record(vm_machine *machine,
    type_status status)
{
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    ++machine->start_outcome.sequence;
    machine->start_outcome.status = status;
    machine->start_outcome.valid = TYPE_TRUE;
    return status;
}

type_status vm_machine_start(vm_machine *machine) {
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = vm_machine_reset(machine);
    if (status != TYPE_STATUS_OK) return status;
    return vm_machine_resume(machine);
}

type_status vm_machine_finish_reset(vm_machine *machine, type_status status)
{
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (status != TYPE_STATUS_OK) return vm_machine_start_outcome_record(machine,
        status);
    vm_machine_pacing_reset(machine);
    machine->display_snapshot_generation_valid = TYPE_FALSE;
    machine->model40_fdc_terminal_observation_valid = TYPE_FALSE;
    vm_machine_start_outcome_reset(machine);
    if (!vm_machine_control_is_running(&machine->control)) {
        vm_machine_publish_display(machine, 1);
    }
    vm_machine_report_lifecycle(machine, VM_MACHINE_RESET);
    return TYPE_STATUS_OK;
}

type_status vm_machine_reset(vm_machine *machine) {
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (machine->execution_task != STD_NULL &&
        vm_machine_executor_state_is_active(machine->control.state)) {
        return vm_machine_enqueue_executor_request(machine,
            COMMON_MACHINE_REQUEST_RESET, VM_MACHINE_PAUSE_NONE);
    }
    if (machine->execution_task != STD_NULL) vm_machine_execution_join(machine);
    common_machine_debug_invalidate(machine->executor);
    status = vm_machine_control_reset(&machine->control);
    if (vm_machine_control_is_running(&machine->control)) return status;
    return vm_machine_finish_reset(machine, status);
}

C_VOID vm_machine_stop(vm_machine *machine) {
    if (machine == STD_NULL) return;
    common_machine_debug_invalidate(machine->executor);
    if (machine->execution_task != STD_NULL &&
        vm_machine_executor_state_is_active(machine->control.state)) {
        if (vm_machine_enqueue_executor_request(machine,
                COMMON_MACHINE_REQUEST_STOP, VM_MACHINE_PAUSE_NONE) != TYPE_STATUS_OK)
            vm_machine_execution_stop(machine);
    } else vm_machine_execution_stop(machine);
    vm_machine_execution_join(machine);
}

type_status vm_machine_request_pause(vm_machine *machine)
{
    return vm_machine_request_pause_reason(machine, VM_MACHINE_PAUSE_EXPLICIT);
}

common_machine *vm_machine_common_machine(vm_machine *machine)
{
    return machine == STD_NULL ? LIB_NULL : machine->executor;
}

void vm_machine_bind_debug_observer(vm_machine *machine,
    vm_machine_debug_observer observer, C_VOID *context)
{
    if (machine == STD_NULL) return;
    vm_machine_debug_bind_observer(&machine->debug, observer, context);
}

type_status vm_machine_pause_for_debug(vm_machine *machine,
    type_unsigned_32 timeout_milliseconds)
{
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_machine_control_is_running(&machine->control) &&
        vm_machine_request_pause(machine) != TYPE_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    return vm_machine_control_is_paused(&machine->control) ||
        vm_machine_control_wait_for_pause(&machine->control,
            timeout_milliseconds) ? TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

type_status vm_machine_request_pause_reason(vm_machine *machine,
    vm_machine_pause_reason reason)
{
    if (machine == STD_NULL || !vm_machine_control_is_running(&machine->control))
        return TYPE_STATUS_INVALID_STATE;
    return vm_machine_enqueue_executor_request(machine,
        COMMON_MACHINE_REQUEST_PAUSE, reason);
}

type_status vm_machine_request_step(vm_machine *machine)
{
    if (machine == STD_NULL || !vm_machine_control_is_paused(&machine->control))
        return TYPE_STATUS_INVALID_STATE;
    return vm_machine_enqueue_executor_request(machine, COMMON_MACHINE_REQUEST_STEP,
        VM_MACHINE_PAUSE_NONE);
}

C_INT vm_machine_is_running(const vm_machine *machine)
{
    return machine != STD_NULL && vm_machine_control_is_running(&machine->control);
}

void vm_machine_set_result_sink(vm_machine *machine,
    vm_machine_result_sink sink, C_VOID *context)
{
    if (machine == STD_NULL) return;
    machine->result_sink = sink;
    machine->result_sink_context = context;
}

void vm_machine_publish_result(vm_machine *machine,
    const vm_machine_result *result)
{
    if (machine != STD_NULL && machine->result_sink != STD_NULL &&
        result != STD_NULL) machine->result_sink(machine->result_sink_context,
            result);
}

void vm_machine_report_lifecycle(vm_machine *machine,
    vm_machine_lifecycle lifecycle)
{
    vm_machine_result result = {0};

    if (machine == STD_NULL) return;
    switch (lifecycle) {
    case VM_MACHINE_RUNNING: result.kind = VM_MACHINE_RESULT_RUNNING; break;
    case VM_MACHINE_PAUSED: result.kind = VM_MACHINE_RESULT_PAUSED; break;
    case VM_MACHINE_RESET: result.kind = VM_MACHINE_RESULT_RESET; break;
    default: result.kind = VM_MACHINE_RESULT_STOPPED; break;
    }
    result.status = TYPE_STATUS_OK;
    vm_machine_publish_result(machine, &result);
}

type_status vm_machine_resume(vm_machine *machine) {
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_machine_control_is_running(&machine->control)) {
        return vm_machine_start_outcome_record(machine, TYPE_STATUS_INVALID_STATE);
    }
    if (vm_machine_control_is_paused(&machine->control) &&
        machine->execution_task != STD_NULL) {
        status = vm_machine_enqueue_executor_request(machine,
            COMMON_MACHINE_REQUEST_RESUME, VM_MACHINE_PAUSE_NONE);
        return vm_machine_start_outcome_record(machine, status);
    }
    if (machine->execution_task != STD_NULL) return vm_machine_start_outcome_record(
        machine, TYPE_STATUS_INVALID_STATE);
    common_machine_debug_invalidate(machine->executor);
    host_sync_event_reset(machine->execution_started);
    if (host_sync_task_create(vm_machine_execution_task_main, machine,
            &machine->execution_task) != LIB_STATUS_OK ||
        host_sync_event_wait(machine->execution_started, 5000u) != HOST_SYNC_WAIT_SIGNALED ||
        !vm_machine_execution_wait_until_active(machine)) {
        vm_machine_execution_stop(machine);
        vm_machine_execution_join(machine);
        return vm_machine_start_outcome_record(machine, TYPE_STATUS_INVALID_STATE);
    }
    return vm_machine_start_outcome_record(machine, TYPE_STATUS_OK);
}

type_status vm_machine_initialize(vm_machine *machine) {
    type_status status;
    common_machine_driver driver;
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (machine->active) return TYPE_STATUS_INVALID_STATE;
    status = vm_machine_storage_initialize(machine);
    if (status != TYPE_STATUS_OK) return status;
    status = vm_machine_control_initialize(&machine->control, machine);
    if (status != TYPE_STATUS_OK) {
        vm_machine_finalize(machine);
        return status;
    }
    status = (type_status)common_machine_create(&machine->executor);
    if (status != TYPE_STATUS_OK) { vm_machine_finalize(machine); return status; }
    driver = (common_machine_driver) {
        vm_machine_consume_request, vm_machine_common_is_paused,
        vm_machine_common_debug_execute, machine
    };
    status = (type_status)common_machine_bind_driver(machine->executor, &driver);
    if (status != TYPE_STATUS_OK) { vm_machine_finalize(machine); return status; }
    status = core_machine_guest_input_source_create(&vm_machine_input_sink, machine,
        &machine->input_source);
    if (status != TYPE_STATUS_OK) {
        vm_machine_finalize(machine);
        return status;
    }
    if (host_sync_event_create(&machine->execution_started) != LIB_STATUS_OK) {
        vm_machine_finalize(machine); return TYPE_STATUS_NO_MEMORY;
    }
    vm_machine_start_outcome_reset(machine);
    vm_machine_control_bind_command_boundary(&machine->control,
        vm_machine_observe_safe_point, machine->executor);
    machine->active = 1;
    return TYPE_STATUS_OK;
}

C_VOID vm_machine_finalize(vm_machine *machine) {
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    vm_machine_execution_stop(machine);
    vm_machine_execution_join(machine);
    host_sync_event_destroy(machine->execution_started);
    machine->execution_started = STD_NULL;
    vm_machine_control_bind_command_boundary(&machine->control, STD_NULL, STD_NULL);
    core_machine_guest_input_source_destroy(machine->input_source);
    machine->input_source = STD_NULL;
    common_machine_close(machine->executor);
    common_machine_destroy(machine->executor);
    machine->executor = STD_NULL;
    machine->active = 0;
    vm_machine_control_finalize(&machine->control, machine);
    vm_machine_storage_finalize(machine);
}
