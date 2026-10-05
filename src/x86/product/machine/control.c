/* Copyright 2012-2014 Neko. */

/*
 * Adapt Core execution/reset to Common's sole worker and control queue.
 */
#include "lib/types/types_interface.h"


#include "x86/product/machine/debug.h"

#include "x86/product/machine/machine_devices.h"

#include "x86/product/machine/fault.h"

#include "x86/core/machine_interface.h"

#include "x86/product/machine/control.h"

#include "x86/product/machine/lifecycle.h"
#include "x86/product/machine/runner.h"

#include "x86/product/machine/machine_private.h"

#include "x86/product/machine/display.h"

static lib_status vm_machine_control_reset_machine(vm_machine *machine)
{
    lib_status status;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    vm_machine_debug_reset(&machine->debug);
    status = core_machine_reset(machine->core_machine);
    if (status != LIB_STATUS_OK) {
        vm_machine_control_stop(&machine->control);
    } else {
        vm_machine_fault_clear(machine);
    }
    return status;
}

static void vm_machine_control_refresh_machine_debug(
    vm_machine *machine)
{
    core_machine_debug_instruction_observation observation;

    if (machine == LIB_NULL || core_machine_debug_capture_instruction_observation(
            machine->core_machine, &observation) != LIB_STATUS_OK) return;
    vm_machine_debug_refresh(&machine->debug, &observation);
}

void vm_machine_control_start(vm_machine_control_state *control) {
    vm_machine *machine;

    if (control == LIB_NULL) return;
    machine = control->machine;
    if (machine == LIB_NULL || machine->core_machine == LIB_NULL) return;
    vm_machine_executor_state_start(control->state);
    vm_machine_runner_run(machine);
}

/* Request reset at the bounded executor boundary. */
lib_status vm_machine_control_reset(vm_machine_control_state *control) {
    if (control == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (vm_machine_executor_state_is_active(control->state)) {
        vm_machine_executor_state_request_reset(control->state);
        return LIB_STATUS_OK;
    } else {
        lib_status status = vm_machine_control_reset_machine(control->machine);

        (void)vm_machine_executor_state_take_reset(control->state);
        return status;
    }
}

/* Cancel bounded execution and any pending reset. */
void vm_machine_control_stop(vm_machine_control_state *control)  {
    vm_machine *machine;

    if (control == LIB_NULL) return;
    machine = control->machine;
    if (machine != LIB_NULL && machine->core_machine != LIB_NULL) {
        core_machine_request_stop(machine->core_machine);
    }
    vm_machine_executor_state_stop(control->state);
}

void vm_machine_control_fault(vm_machine_control_state *control)
{
    if (control == LIB_NULL) return;
    vm_machine_executor_state_stop(control->state);
}

lib_status vm_machine_control_reset_at_boundary(vm_machine_control_state *control)
{
    return control == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        vm_machine_control_reset_machine(control->machine);
}

void vm_machine_control_refresh_debug(vm_machine_control_state *control)
{
    if (control != LIB_NULL) vm_machine_control_refresh_machine_debug(control->machine);
}

/* Initializes devices */
lib_status vm_machine_control_initialize(vm_machine_control_state *control,
    vm_machine *machine) {
    lib_status status;

    if (control == LIB_NULL || machine == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_set((void *)(control), 0u, sizeof(*control));
    status = vm_machine_executor_state_create(&control->state);
    if (status != LIB_STATUS_OK) return status;
    control->machine = machine;
    vm_machine_debug_initialize(&machine->debug);
    status = vm_machine_devices_initialize_media(machine);
    if (status == LIB_STATUS_OK) status = vm_machine_devices_bind_media(machine);
    if (status == LIB_STATUS_OK) {
        status = vm_machine_bind_execution_provider(machine);
    }
    if (status != LIB_STATUS_OK) {
        vm_machine_control_stop(control);
    }
    return status;
}

/* Finalizes devices */
void vm_machine_control_finalize(vm_machine_control_state *control,
    vm_machine *machine) {
    if (control == LIB_NULL || machine == LIB_NULL) return;
    vm_machine_devices_finalize(machine);
    vm_machine_debug_finalize(&machine->debug);
    vm_machine_executor_state_destroy(control->state);
    control->state = LIB_NULL;
}

lib_bool vm_machine_control_is_running(const vm_machine_control_state *control)
{
    return control != LIB_NULL && vm_machine_executor_state_is_active(control->state);
}
