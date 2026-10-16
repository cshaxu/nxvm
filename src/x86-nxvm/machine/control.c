/* Copyright 2012-2014 Neko. */

/*
 * Adapt Core execution/reset to Common's sole worker and control queue.
 */
#include "lib/types/types_interface.h"


#include "x86-nxvm/machine/debug.h"

#include "x86-nxvm/machine/machine_devices.h"

#include "x86-nxvm/machine/fault.h"

#include "x86-core/machine/machine_interface.h"

#include "x86-nxvm/machine/control.h"

#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/runner.h"

#include "x86-nxvm/machine/machine_private.h"

#include "x86-nxvm/machine/display.h"

static lib_status x86_nxvm_machine_control_reset_machine(x86_nxvm_machine *machine)
{
    lib_status status;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    x86_nxvm_machine_debug_reset(&machine->debug);
    status = x86_core_machine_reset(machine->x86_core_machine);
    if (status != LIB_STATUS_OK) {
        x86_nxvm_machine_control_stop(&machine->control);
    } else {
        x86_nxvm_machine_fault_clear(machine);
    }
    return status;
}

static void x86_nxvm_machine_control_refresh_machine_debug(
    x86_nxvm_machine *machine)
{
    x86_core_cpu_debug_instruction_observation observation;

    if (machine == LIB_NULL || x86_core_machine_debug_capture_instruction_observation(
            machine->x86_core_machine, &observation) != LIB_STATUS_OK) return;
    x86_nxvm_machine_debug_refresh(&machine->debug, &observation);
}

void x86_nxvm_machine_control_start(x86_nxvm_machine_control_state *control) {
    x86_nxvm_machine *machine;

    if (control == LIB_NULL) return;
    machine = control->machine;
    if (machine == LIB_NULL || machine->x86_core_machine == LIB_NULL) return;
    x86_nxvm_machine_executor_state_start(&control->state);
    x86_nxvm_machine_runner_run(machine);
}

/* Request reset at the bounded executor boundary. */
lib_status x86_nxvm_machine_control_reset(x86_nxvm_machine_control_state *control) {
    if (control == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (x86_nxvm_machine_executor_state_is_active(&control->state)) {
        x86_nxvm_machine_executor_state_request_reset(&control->state);
        return LIB_STATUS_OK;
    } else {
        lib_status status = x86_nxvm_machine_control_reset_machine(control->machine);

        (void)x86_nxvm_machine_executor_state_take_reset(&control->state);
        return status;
    }
}

/* Cancel bounded execution and any pending reset. */
void x86_nxvm_machine_control_stop(x86_nxvm_machine_control_state *control)  {
    x86_nxvm_machine *machine;

    if (control == LIB_NULL) return;
    machine = control->machine;
    if (machine != LIB_NULL && machine->x86_core_machine != LIB_NULL) {
        x86_core_machine_request_stop(machine->x86_core_machine);
    }
    x86_nxvm_machine_executor_state_stop(&control->state);
}

void x86_nxvm_machine_control_end_run(x86_nxvm_machine_control_state *control)
{
    if (control == LIB_NULL) return;
    x86_nxvm_machine_executor_state_stop(&control->state);
}

lib_status x86_nxvm_machine_control_reset_at_boundary(x86_nxvm_machine_control_state *control)
{
    return control == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        x86_nxvm_machine_control_reset_machine(control->machine);
}

void x86_nxvm_machine_control_refresh_debug(x86_nxvm_machine_control_state *control)
{
    if (control != LIB_NULL) x86_nxvm_machine_control_refresh_machine_debug(control->machine);
}

/* Initializes devices */
lib_status x86_nxvm_machine_control_initialize(x86_nxvm_machine_control_state *control,
    x86_nxvm_machine *machine) {
    lib_status status;

    if (control == LIB_NULL || machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_set((void *)(control), 0u, sizeof(*control));
    lib_atomic_i32_initialize(&control->state.active, LIB_FALSE);
    lib_atomic_i32_initialize(&control->state.reset_requested, LIB_FALSE);
    control->machine = machine;
    x86_nxvm_machine_debug_initialize(&machine->debug);
    status = x86_nxvm_machine_devices_initialize_media(machine);
    if (status == LIB_STATUS_OK) status = x86_nxvm_machine_devices_bind_media(machine);
    if (status == LIB_STATUS_OK) {
        status = x86_nxvm_machine_bind_execution_provider(machine);
    }
    if (status != LIB_STATUS_OK) {
        x86_nxvm_machine_control_stop(control);
    }
    return status;
}

/* Finalizes devices */
void x86_nxvm_machine_control_finalize(x86_nxvm_machine_control_state *control,
    x86_nxvm_machine *machine) {
    if (control == LIB_NULL || machine == LIB_NULL) return;
    x86_nxvm_machine_devices_finalize(machine);
    x86_nxvm_machine_debug_finalize(&machine->debug);
}

lib_bool x86_nxvm_machine_control_is_running(const x86_nxvm_machine_control_state *control)
{
    return control != LIB_NULL && x86_nxvm_machine_executor_state_is_active(&control->state);
}
