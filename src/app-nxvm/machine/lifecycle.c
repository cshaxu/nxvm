/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */
#include "lib/types/types_interface.h"

#include "type.h"

#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/control.h"

#include "app-nxvm/machine/machine_devices.h"

#include "app-nxvm/machine/machine_interface.h"


#include "lib/base/sync_interface.h"

#include "app-nxvm/machine/debug_adapter.h"

#include "app-nxvm/machine/debug.h"

#include "app-nxvm/devices/machine_interface.h"


#include "app-nxvm/machine/display.h"
#include "app-nxvm/machine/waiting.h"


#include "app-nxvm/machine/lifecycle.h"

static C_VOID vm_machine_execution_provider_reset(C_VOID *context)
{
    vm_machine_devices_reset((vm_machine *)context);
}

static const core_machine_execution_provider vm_machine_execution_provider = {
    vm_machine_execution_provider_reset,
    LIB_NULL
};

type_status vm_machine_bind_execution_provider(vm_machine *machine)
{
    type_status status;
    const core_machine_firmware_provider *firmware_provider;
    C_VOID *firmware_context;

    if (machine == LIB_NULL || machine->core_machine == LIB_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    firmware_provider = vm_profile_machine_plan_firmware_provider_get(
        machine->profile_plan);
    firmware_context = vm_profile_machine_plan_firmware_context_get(
        machine->profile_plan);
    if (firmware_provider == LIB_NULL || firmware_context == LIB_NULL) {
        return TYPE_STATUS_INVALID_STATE;
    }
    status = core_machine_bind_firmware_provider(machine->core_machine,
        firmware_provider, firmware_context);
    if (status != TYPE_STATUS_OK) return status;
    status = core_machine_bind_execution_provider(machine->core_machine,
        &vm_machine_execution_provider, machine);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_freeze_execution_providers(machine->core_machine);
}

/* Common owns the sole host worker and lifecycle queue.  NXVM only adapts its
 * bounded Core executor at the explicit callbacks below; it does not retain a
 * second task, request FIFO, or generation counter. */
static lib_bool vm_machine_driver_reset(C_VOID *context)
{
    vm_machine *machine = (vm_machine *)context;
    type_status status;

    if (machine == LIB_NULL) return LIB_FALSE;
    status = vm_machine_control_reset(&machine->control);
    return vm_machine_finish_reset(machine, status) == TYPE_STATUS_OK;
}

static lib_bool vm_machine_driver_run(C_VOID *context)
{
    vm_machine *machine = (vm_machine *)context;

    if (machine == LIB_NULL) return LIB_FALSE;
    machine->runner_failed = LIB_FALSE;
    vm_machine_control_start(&machine->control);
    return !machine->runner_failed;
}

static C_VOID vm_machine_driver_request_stop(C_VOID *context)
{ vm_machine_control_stop(&((vm_machine *)context)->control); }

static C_VOID vm_machine_driver_request_wake(C_VOID *context)
{ (C_VOID)context; }

static C_VOID vm_machine_driver_set_heartbeat(C_VOID *context, lib_bool enabled)
{ (C_VOID)context; (C_VOID)enabled; }

static C_VOID vm_machine_driver_set_executor_callback(C_VOID *context,
    common_machine_executor_callback callback, C_VOID *callback_context)
{
    vm_machine *machine = (vm_machine *)context;

    if (machine == LIB_NULL) return;
    machine->executor_callback = callback;
    machine->executor_callback_context = callback_context;
}

static C_VOID vm_machine_driver_deliver_input(C_VOID *context,
    const kvm_input_event *event)
{ (C_VOID)vm_machine_deliver_common_input((vm_machine *)context, event); }

static lib_status vm_machine_driver_copy_frame(C_VOID *context, common_machine_frame *frame)
{
    if (context == LIB_NULL || frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    /* Common clears staging validity before this call.  A Core display that
     * has not yet published is an ordinary no-frame result, not a machine
     * failure. */
    (C_VOID)vm_machine_copy_common_frame((vm_machine *)context, frame);
    return LIB_STATUS_OK;
}

static lib_bool vm_machine_driver_set_removable_media(C_VOID *context,
    const char *path, lib_storage_medium_mode mode)
{
    vm_machine *machine = (vm_machine *)context;

    if (machine == LIB_NULL) return LIB_FALSE;
    return vm_machine_set_common_media(machine, path, mode) == TYPE_STATUS_OK;
}

static lib_bool vm_machine_driver_take_debug_stop(C_VOID *context)
{
    vm_machine *machine = (vm_machine *)context;
    vm_machine_debug_stop_reason reason;

    return machine != LIB_NULL && vm_machine_debug_completion_pending(
        &machine->debug, &reason) ? LIB_TRUE : LIB_FALSE;
}

static C_VOID vm_machine_driver_cancel_debug(C_VOID *context)
{
    vm_machine *machine = (vm_machine *)context;
    if (machine != LIB_NULL) vm_machine_debug_reset(&machine->debug);
}

type_status vm_machine_finish_reset(vm_machine *machine, type_status status)
{
    if (machine == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (status != TYPE_STATUS_OK) return status;
    vm_machine_pacing_reset(machine);
    machine->runner_failed = LIB_FALSE;
    machine->display_snapshot_generation_valid = LIB_FALSE;
    machine->model40_fdc_terminal_observation_valid = LIB_FALSE;
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
    if (machine != LIB_NULL && (machine->executor == LIB_NULL ||
        common_machine_state_get(machine->executor) == COMMON_MACHINE_STOPPED)) {
        status = vm_machine_control_reset(&machine->control);
        return vm_machine_finish_reset(machine, status);
    }
    return machine != LIB_NULL && common_machine_reset(machine->executor) ?
        TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

C_VOID vm_machine_stop(vm_machine *machine) {
    if (machine == LIB_NULL) return;
    if (machine->executor != LIB_NULL) {
        (C_VOID)common_machine_stop(machine->executor);
        return;
    }
    /* An uncomposed machine has no Common lifecycle owner yet. This is the
     * matching construction/test state already handled by vm_machine_reset;
     * stopping it must still wake a direct bounded runner. */
    vm_machine_control_stop(&machine->control);
}

type_status vm_machine_request_pause(vm_machine *machine)
{
    return machine != LIB_NULL && machine->executor != LIB_NULL &&
        common_machine_pause(machine->executor) ?
        TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

type_status vm_machine_describe_common_driver(vm_machine *machine,
    common_machine_driver *out_driver)
{
    if (machine == LIB_NULL || out_driver == LIB_NULL || !machine->active)
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
        .execute_debug = vm_machine_debug_execute,
        .take_debug_stop = vm_machine_driver_take_debug_stop,
        .cancel_debug = vm_machine_driver_cancel_debug
    };
    return TYPE_STATUS_OK;
}

type_status vm_machine_bind_common_machine(vm_machine *machine,
    common_machine *common_machine)
{
    if (machine == LIB_NULL || !machine->active ||
        (machine->executor != LIB_NULL && common_machine != LIB_NULL))
        return TYPE_STATUS_INVALID_STATE;
    machine->executor = common_machine;
    return TYPE_STATUS_OK;
}

type_status vm_machine_resume(vm_machine *machine) {
    return machine != LIB_NULL && machine->executor != LIB_NULL &&
        (common_machine_state_get(machine->executor) == COMMON_MACHINE_STOPPED ?
            common_machine_start(machine->executor) : common_machine_resume(machine->executor)) ?
        TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

type_status vm_machine_initialize(vm_machine *machine) {
    type_status status;
    if (machine == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (machine->active) return TYPE_STATUS_INVALID_STATE;
    status = vm_machine_storage_initialize(machine);
    if (status != TYPE_STATUS_OK) return status;
    status = vm_machine_control_initialize(&machine->control, machine);
    if (status != TYPE_STATUS_OK) {
        vm_machine_finalize(machine);
        return status;
    }
    machine->active = 1;
    return TYPE_STATUS_OK;
}

C_VOID vm_machine_finalize(vm_machine *machine) {
    if (machine == LIB_NULL || machine->core_machine == LIB_NULL) return;
    vm_machine_stop(machine);
    machine->executor = LIB_NULL;
    machine->active = 0;
    vm_machine_control_finalize(&machine->control, machine);
    vm_machine_storage_finalize(machine);
}
