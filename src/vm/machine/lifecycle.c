/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "type.h"

#include "vm/machine/machine_private.h"
#include "vm/machine/control.h"

#include "vm/machine/machine_devices.h"

#include "vm/machine/machine_interface.h"


#include "lib/host/sync_interface.h"

#include "vm/machine/debug_adapter.h"

#include "vm/machine/debug.h"

#include "core/machine/machine_interface.h"

#include "core/machine/guest_input_interface.h"

#include "vm/machine/display.h"
#include "vm/machine/waiting.h"


#include "vm/machine/lifecycle.h"

static C_VOID vm_machine_input_submit(C_VOID *context,
    const core_machine_guest_input_event *event)
{
    vm_machine *machine = (vm_machine *)context;
    kvm_input_event input = {0};

    if (machine == STD_NULL || event == STD_NULL) return;
    if (event->kind == CORE_MACHINE_GUEST_INPUT_KEY) {
        input.type = KVM_EVENT_KEY;
        input.data.key.scan_code = event->data.key.scan_code;
        input.data.key.key = event->data.key.virtual_key;
        input.data.key.pressed = event->data.key.pressed;
    } else if (event->kind == CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE) {
        input.type = KVM_EVENT_MOUSE;
        input.data.mouse.delta_x = event->data.relative_mouse.delta_x;
        input.data.mouse.delta_y = event->data.relative_mouse.delta_y;
        input.data.mouse.buttons = event->data.relative_mouse.buttons;
    } else {
        return;
    }
    (C_VOID)vm_machine_deliver_common_input(machine, &input);
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

/* Common owns the sole host worker and lifecycle queue.  NXVM only adapts its
 * bounded Core executor at the explicit callbacks below; it does not retain a
 * second task, request FIFO, or generation counter. */
static lib_bool vm_machine_driver_reset(void *context)
{
    vm_machine *machine = (vm_machine *)context;
    type_status status;

    if (machine == STD_NULL) return LIB_FALSE;
    status = vm_machine_control_reset(&machine->control);
    return vm_machine_finish_reset(machine, status) == TYPE_STATUS_OK;
}

static lib_bool vm_machine_driver_run(void *context)
{
    vm_machine *machine = (vm_machine *)context;

    if (machine == STD_NULL) return LIB_FALSE;
    vm_machine_control_start(&machine->control);
    return machine->fault_outcome.valid ? LIB_FALSE : LIB_TRUE;
}

static void vm_machine_driver_request_stop(void *context)
{ vm_machine_control_stop(&((vm_machine *)context)->control); }

static void vm_machine_driver_request_wake(void *context)
{ (C_VOID)context; }

static void vm_machine_driver_set_heartbeat(void *context, lib_bool enabled)
{ (C_VOID)context; (C_VOID)enabled; }

static void vm_machine_driver_set_executor_callback(void *context,
    common_machine_executor_callback callback, void *callback_context)
{
    vm_machine *machine = (vm_machine *)context;

    if (machine == STD_NULL) return;
    machine->executor_callback = callback;
    machine->executor_callback_context = callback_context;
}

static void vm_machine_driver_deliver_input(void *context,
    const kvm_input_event *event)
{ (C_VOID)vm_machine_deliver_common_input((vm_machine *)context, event); }

static lib_bool vm_machine_driver_copy_frame(void *context, kvm_frame *frame)
{ return vm_machine_copy_common_frame((vm_machine *)context, frame) ? LIB_TRUE : LIB_FALSE; }

static lib_bool vm_machine_driver_set_removable_media(void *context,
    const char *path)
{
    vm_machine *machine = (vm_machine *)context;

    if (machine == STD_NULL) return LIB_FALSE;
    return vm_machine_set_common_media(machine, path) == TYPE_STATUS_OK;
}

static lib_bool vm_machine_driver_take_debug_stop(void *context)
{
    vm_machine *machine = (vm_machine *)context;
    vm_machine_pause_reason reason;

    return machine != STD_NULL && vm_machine_debug_completion_pending(
        &machine->debug, &reason) ? LIB_TRUE : LIB_FALSE;
}

static void vm_machine_driver_cancel_debug(void *context)
{
    vm_machine *machine = (vm_machine *)context;
    if (machine != STD_NULL) vm_machine_debug_reset(&machine->debug);
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
    return machine != STD_NULL && machine->executor != LIB_NULL &&
        common_machine_start(machine->executor) ?
        TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
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
    return TYPE_STATUS_OK;
}

type_status vm_machine_reset(vm_machine *machine) {
    type_status status;

    /* Construction and repository-only owner tests reset an inactive machine
     * before it is composed into Common.  There is no worker or Common run to
     * rendezvous with in that state, so reset the VM-owned Core directly.
     * Once a Common run exists, lifecycle remains exclusively Common-owned. */
    if (machine != STD_NULL && (machine->executor == LIB_NULL ||
        common_machine_state_get(machine->executor) == COMMON_MACHINE_STOPPED)) {
        status = vm_machine_control_reset(&machine->control);
        return vm_machine_finish_reset(machine, status);
    }
    return machine != STD_NULL && common_machine_reset(machine->executor) ?
        TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

C_VOID vm_machine_stop(vm_machine *machine) {
    if (machine != STD_NULL && machine->executor != LIB_NULL)
        (C_VOID)common_machine_stop(machine->executor);
}

type_status vm_machine_request_pause(vm_machine *machine)
{
    return vm_machine_request_pause_reason(machine, VM_MACHINE_PAUSE_EXPLICIT);
}

type_status vm_machine_describe_common_driver(vm_machine *machine,
    common_machine_driver *out_driver)
{
    if (machine == STD_NULL || out_driver == LIB_NULL || !machine->active)
        return TYPE_STATUS_INVALID_ARGUMENT;
    *out_driver = (common_machine_driver) {
        .context = machine,
        .reset = vm_machine_driver_reset,
        .run = vm_machine_driver_run,
        .request_stop = vm_machine_driver_request_stop,
        .request_wake = vm_machine_driver_request_wake,
        .set_heartbeat = vm_machine_driver_set_heartbeat,
        .set_executor_callback = vm_machine_driver_set_executor_callback,
        .deliver_input = vm_machine_driver_deliver_input,
        .copy_frame = vm_machine_driver_copy_frame,
        .set_removable_media = vm_machine_driver_set_removable_media,
        .execute_debug = vm_machine_common_debug_execute,
        .take_debug_stop = vm_machine_driver_take_debug_stop,
        .cancel_debug = vm_machine_driver_cancel_debug
    };
    return TYPE_STATUS_OK;
}

type_status vm_machine_bind_common_machine(vm_machine *machine,
    common_machine *common_machine)
{
    if (machine == STD_NULL || !machine->active ||
        (machine->executor != LIB_NULL && common_machine != LIB_NULL))
        return TYPE_STATUS_INVALID_STATE;
    machine->executor = common_machine;
    return TYPE_STATUS_OK;
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
    if (machine == STD_NULL || machine->executor == LIB_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (common_machine_state_get(machine->executor) == COMMON_MACHINE_RUNNING &&
        !common_machine_pause(machine->executor))
        return TYPE_STATUS_INVALID_STATE;
    while (timeout_milliseconds-- != 0u) {
        if (common_machine_state_get(machine->executor) == COMMON_MACHINE_PAUSED)
            return TYPE_STATUS_OK;
        host_sync_sleep_milliseconds(1u);
    }
    return TYPE_STATUS_INVALID_STATE;
}

type_status vm_machine_request_pause_reason(vm_machine *machine,
    vm_machine_pause_reason reason)
{
    (C_VOID)reason;
    return machine != STD_NULL && machine->executor != LIB_NULL &&
        common_machine_pause(machine->executor) ?
        TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

type_status vm_machine_request_step(vm_machine *machine)
{
    if (machine == STD_NULL || machine->executor == LIB_NULL ||
        common_machine_state_get(machine->executor) !=
        COMMON_MACHINE_PAUSED) return TYPE_STATUS_INVALID_STATE;
    return TYPE_STATUS_UNSUPPORTED;
}

C_INT vm_machine_is_running(const vm_machine *machine)
{
    return machine != STD_NULL && machine->executor != LIB_NULL &&
        common_machine_state_get(machine->executor) ==
        COMMON_MACHINE_RUNNING;
}

type_status vm_machine_resume(vm_machine *machine) {
    return machine != STD_NULL && machine->executor != LIB_NULL &&
        (common_machine_state_get(machine->executor) == COMMON_MACHINE_STOPPED ?
            common_machine_start(machine->executor) : common_machine_resume(machine->executor)) ?
        TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

type_status vm_machine_initialize(vm_machine *machine) {
    type_status status;
    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (machine->active) return TYPE_STATUS_INVALID_STATE;
    status = vm_machine_storage_initialize(machine);
    if (status != TYPE_STATUS_OK) return status;
    status = vm_machine_control_initialize(&machine->control, machine);
    if (status != TYPE_STATUS_OK) {
        vm_machine_finalize(machine);
        return status;
    }
    status = core_machine_guest_input_source_create(&vm_machine_input_sink, machine,
        &machine->input_source);
    if (status != TYPE_STATUS_OK) {
        vm_machine_finalize(machine);
        return status;
    }
    vm_machine_start_outcome_reset(machine);
    machine->active = 1;
    return TYPE_STATUS_OK;
}

C_VOID vm_machine_finalize(vm_machine *machine) {
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    vm_machine_stop(machine);
    core_machine_guest_input_source_destroy(machine->input_source);
    machine->input_source = STD_NULL;
    machine->executor = STD_NULL;
    machine->active = 0;
    vm_machine_control_finalize(&machine->control, machine);
    vm_machine_storage_finalize(machine);
}
