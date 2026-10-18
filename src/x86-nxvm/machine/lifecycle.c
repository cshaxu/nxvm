/* Copyright 2012-2014 Neko. */

/* Adapt machine lifecycle to the Common owner. */
#include "lib/types/types_interface.h"


#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/control.h"

#include "x86-nxvm/machine/machine_devices.h"

#include "x86-nxvm/machine/machine_interface.h"


#include "lib/base/sync_interface.h"

#include "x86-nxvm/machine/debug_adapter.h"

#include "x86-nxvm/machine/debug.h"

#include "x86-core/machine/machine_interface.h"


#include "x86-nxvm/machine/display.h"
#include "x86-nxvm/machine/waiting.h"


#include "x86-nxvm/machine/lifecycle.h"

static void x86_nxvm_machine_execution_provider_reset(void *context)
{
    x86_nxvm_machine_devices_reset((x86_nxvm_machine *)context);
}

static const x86_core_machine_execution_provider x86_nxvm_machine_execution_provider = {
    x86_nxvm_machine_execution_provider_reset,
    LIB_NULL
};

lib_status x86_nxvm_machine_bind_execution_provider(x86_nxvm_machine *machine)
{
    lib_status status;
    const x86_core_machine_firmware_provider *firmware_provider;
    void *firmware_context;

    if (machine == LIB_NULL || machine->x86_core_machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    firmware_provider = machine->construction.firmware_provider;
    firmware_context = machine->construction.firmware_context;
    if (firmware_provider == LIB_NULL || firmware_context == LIB_NULL) {
        return LIB_STATUS_INVALID_STATE;
    }
    status = x86_core_machine_bind_firmware_provider(machine->x86_core_machine,
        firmware_provider, firmware_context);
    if (status != LIB_STATUS_OK) return status;
    status = x86_core_machine_bind_execution_provider(machine->x86_core_machine,
        &x86_nxvm_machine_execution_provider, machine);
    if (status != LIB_STATUS_OK) return status;
    return x86_core_machine_freeze_execution_providers(machine->x86_core_machine);
}

/* Common owns the sole host worker and lifecycle queue.  NXVM only adapts its
 * bounded Core executor at the explicit callbacks below; it does not retain a
 * second task, request FIFO, or generation counter. */
static lib_bool x86_nxvm_machine_driver_reset(void *context)
{
    x86_nxvm_machine *machine = (x86_nxvm_machine *)context;
    lib_status status;

    if (machine == LIB_NULL) return LIB_FALSE;
    status = x86_nxvm_machine_control_reset(&machine->control);
    return x86_nxvm_machine_finish_reset(machine, status) == LIB_STATUS_OK;
}

static lib_bool x86_nxvm_machine_driver_run(void *context)
{
    x86_nxvm_machine *machine = (x86_nxvm_machine *)context;

    if (machine == LIB_NULL) return LIB_FALSE;
    machine->runner_failed = LIB_FALSE;
    x86_nxvm_machine_control_start(&machine->control);
    return !machine->runner_failed;
}

static void x86_nxvm_machine_driver_request_stop(void *context)
{ x86_nxvm_machine_control_stop(&((x86_nxvm_machine *)context)->control); }

static void x86_nxvm_machine_driver_request_wake(void *context)
{ (void)context; }

static void x86_nxvm_machine_driver_set_heartbeat(void *context, lib_bool enabled)
{ (void)context; (void)enabled; }

static void x86_nxvm_machine_driver_set_executor_callback(void *context,
    emulator_machine_executor_callback callback, void *callback_context)
{
    x86_nxvm_machine *machine = (x86_nxvm_machine *)context;

    if (machine == LIB_NULL) return;
    machine->executor_callback = callback;
    machine->executor_callback_context = callback_context;
}

static void x86_nxvm_machine_driver_deliver_input(void *context,
    const kvm_input_event *event)
{ (void)x86_nxvm_machine_deliver_emulator_input((x86_nxvm_machine *)context, event); }

static lib_status x86_nxvm_machine_driver_copy_frame(void *context,
    kvm_window_frame_storage *frame, kvm_console_text_frame *console, lib_bool *produced)
{
    if (context == LIB_NULL || frame == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    /* The publisher clears readiness before this call. A Core display that
     * has not yet published is an ordinary no-frame result, not a machine
     * failure. */
    return x86_nxvm_machine_copy_emulator_frame((x86_nxvm_machine *)context,
        frame, console, produced);
}

static lib_bool x86_nxvm_machine_driver_set_removable_media(void *context,
    const char *path, lib_storage_medium_mode mode)
{
    x86_nxvm_machine *machine = (x86_nxvm_machine *)context;

    if (machine == LIB_NULL) return LIB_FALSE;
    return x86_nxvm_machine_set_emulator_media(machine, path, mode) == LIB_STATUS_OK;
}

static lib_bool x86_nxvm_machine_driver_take_debug_stop(void *context)
{
    x86_nxvm_machine *machine = (x86_nxvm_machine *)context;
    x86_nxvm_machine_debug_stop_reason reason;

    return machine != LIB_NULL && x86_nxvm_machine_debug_completion_pending(
        &machine->debug, &reason) ? LIB_TRUE : LIB_FALSE;
}

static void x86_nxvm_machine_driver_cancel_debug(void *context)
{
    x86_nxvm_machine *machine = (x86_nxvm_machine *)context;
    if (machine != LIB_NULL) x86_nxvm_machine_debug_reset(&machine->debug);
}

lib_status x86_nxvm_machine_finish_reset(x86_nxvm_machine *machine, lib_status status)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (status != LIB_STATUS_OK) return status;
    x86_nxvm_machine_pacing_reset(machine);
    machine->runner_failed = LIB_FALSE;
    machine->display_snapshot_generation_valid = LIB_FALSE;
    if (machine->construction.profile.notify != LIB_NULL)
        machine->construction.profile.notify(machine->construction.profile.context,
            X86_NXVM_MACHINE_PROFILE_RESET_COMPLETED);
    if (!x86_nxvm_machine_control_is_running(&machine->control)) {
        return x86_nxvm_machine_publish_display(machine, LIB_TRUE);
    }
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_reset(x86_nxvm_machine *machine) {
    lib_status status;

    /* Construction and repository-only owner tests reset an inactive machine
     * before it is composed into Common.  There is no worker or Common run to
     * rendezvous with in that state, so reset the VM-owned Core directly.
     * Once a Common run exists, lifecycle remains exclusively Common-owned. */
    if (machine != LIB_NULL && (machine->executor == LIB_NULL ||
        emulator_machine_state_get(machine->executor) == EMULATOR_MACHINE_STOPPED)) {
        status = x86_nxvm_machine_control_reset(&machine->control);
        return x86_nxvm_machine_finish_reset(machine, status);
    }
    return machine != LIB_NULL && emulator_machine_reset(machine->executor) ?
        LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
}

void x86_nxvm_machine_stop(x86_nxvm_machine *machine) {
    if (machine == LIB_NULL) return;
    if (machine->executor != LIB_NULL) {
        (void)emulator_machine_stop(machine->executor);
        return;
    }
    /* An uncomposed machine has no Common lifecycle owner yet. This is the
     * matching construction/test state already handled by x86_nxvm_machine_reset;
     * stopping it must still wake a direct bounded runner. */
    x86_nxvm_machine_control_stop(&machine->control);
}

lib_status x86_nxvm_machine_request_pause(x86_nxvm_machine *machine)
{
    return machine != LIB_NULL && machine->executor != LIB_NULL &&
        emulator_machine_pause(machine->executor) ?
        LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
}

lib_status x86_nxvm_machine_describe_emulator_driver(x86_nxvm_machine *machine,
    emulator_machine_driver *out_driver)
{
    if (machine == LIB_NULL || out_driver == LIB_NULL || !machine->active)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_driver = (emulator_machine_driver) {
        .context = machine,
        .reset = x86_nxvm_machine_driver_reset,
        .run = x86_nxvm_machine_driver_run,
        .request_stop = x86_nxvm_machine_driver_request_stop,
        .request_wake = x86_nxvm_machine_driver_request_wake,
        .set_heartbeat = x86_nxvm_machine_driver_set_heartbeat,
        .set_executor_callback = x86_nxvm_machine_driver_set_executor_callback,
        .deliver_input = x86_nxvm_machine_driver_deliver_input,
        .copy_frame = x86_nxvm_machine_driver_copy_frame,
        .set_removable_media = x86_nxvm_machine_driver_set_removable_media,
        .execute_debug = x86_nxvm_machine_debug_execute,
        .take_debug_stop = x86_nxvm_machine_driver_take_debug_stop,
        .cancel_debug = x86_nxvm_machine_driver_cancel_debug
    };
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_bind_emulator_machine(x86_nxvm_machine *machine,
    emulator_machine *emulator_machine)
{
    if (machine == LIB_NULL || !machine->active ||
        (machine->executor != LIB_NULL && emulator_machine != LIB_NULL))
        return LIB_STATUS_INVALID_STATE;
    machine->executor = emulator_machine;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_resume(x86_nxvm_machine *machine) {
    return machine != LIB_NULL && machine->executor != LIB_NULL &&
        (emulator_machine_state_get(machine->executor) == EMULATOR_MACHINE_STOPPED ?
            emulator_machine_start(machine->executor) : emulator_machine_resume(machine->executor)) ?
        LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
}

lib_status x86_nxvm_machine_initialize(x86_nxvm_machine *machine) {
    lib_status status;
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (machine->active) return LIB_STATUS_INVALID_STATE;
    status = x86_nxvm_machine_storage_initialize(machine);
    if (status != LIB_STATUS_OK) return status;
    status = x86_nxvm_machine_control_initialize(&machine->control, machine);
    if (status != LIB_STATUS_OK) {
        x86_nxvm_machine_finalize(machine);
        return status;
    }
    machine->active = LIB_TRUE;
    return LIB_STATUS_OK;
}

void x86_nxvm_machine_finalize(x86_nxvm_machine *machine) {
    if (machine == LIB_NULL) return;
    x86_nxvm_machine_stop(machine);
    machine->executor = LIB_NULL;
    machine->active = LIB_FALSE;
    /* Revoke Core routes before freeing their borrowed provider contexts. */
    x86_nxvm_machine_storage_finalize(machine);
    x86_nxvm_machine_control_finalize(&machine->control, machine);
}
