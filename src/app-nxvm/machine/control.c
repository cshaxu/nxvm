/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "type.h"

#include "app-nxvm/machine/debug.h"

#include "app-nxvm/machine/machine_devices.h"

#include "app-nxvm/machine/fault.h"

#include "app-nxvm/devices/machine_interface.h"

#include "app-nxvm/machine/control.h"

#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/runner.h"

#include "app-nxvm/machine/machine_private.h"

#include "app-nxvm/machine/display.h"

static type_status vm_machine_control_reset_machine(vm_machine *machine)
{
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    vm_machine_debug_reset(&machine->debug);
    status = core_machine_reset(machine->core_machine);
    if (status != TYPE_STATUS_OK) {
        vm_machine_control_stop(&machine->control);
    } else {
        vm_machine_fault_clear(machine);
    }
    return status;
}

static C_VOID vm_machine_control_refresh_machine_debug(
    vm_machine *machine)
{
    core_machine_debug_instruction_observation observation;

    if (machine == STD_NULL || core_machine_debug_capture_instruction_observation(
            machine->core_machine, &observation) != TYPE_STATUS_OK) return;
    vm_machine_debug_refresh(&machine->debug, &observation);
}

C_VOID vm_machine_control_start(vm_machine_control_state *control) {
    vm_machine *machine;

    if (control == STD_NULL) return;
    machine = control->machine;
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    vm_machine_executor_state_start(control->state);
    vm_machine_runner_run(machine);
}

/* Issues resetting signal to device thread */
type_status vm_machine_control_reset(vm_machine_control_state *control) {
    if (control == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_machine_executor_state_is_active(control->state)) {
        vm_machine_executor_state_request_reset(control->state);
        return TYPE_STATUS_OK;
    } else {
        type_status status = vm_machine_control_reset_machine(control->machine);

        (C_VOID)vm_machine_executor_state_take_reset(control->state);
        return status;
    }
}

/* Issues stopping signal to device thread */
C_VOID vm_machine_control_stop(vm_machine_control_state *control)  {
    vm_machine *machine;

    if (control == STD_NULL) return;
    machine = control->machine;
    if (machine != STD_NULL && machine->core_machine != STD_NULL) {
        core_machine_request_stop(machine->core_machine);
    }
    vm_machine_executor_state_stop(control->state);
}

C_VOID vm_machine_control_fault(vm_machine_control_state *control)
{
    if (control == STD_NULL) return;
    vm_machine_executor_state_stop(control->state);
}

type_status vm_machine_control_reset_at_boundary(vm_machine_control_state *control)
{
    return control == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        vm_machine_control_reset_machine(control->machine);
}

C_VOID vm_machine_control_refresh_debug(vm_machine_control_state *control)
{
    if (control != STD_NULL) vm_machine_control_refresh_machine_debug(control->machine);
}

/* Initializes devices */
type_status vm_machine_control_initialize(vm_machine_control_state *control,
    vm_machine *machine) {
    type_status status;

    if (control == STD_NULL || machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET((C_VOID *)(control), TYPE_ZERO_8, sizeof(*control));
    status = vm_machine_executor_state_create(&control->state);
    if (status != TYPE_STATUS_OK) return status;
    control->machine = machine;
    vm_machine_debug_initialize(&machine->debug);
    status = vm_machine_devices_initialize_media(machine);
    if (status == TYPE_STATUS_OK) status = vm_machine_devices_bind_media(machine);
    if (status == TYPE_STATUS_OK) {
        status = vm_machine_bind_execution_provider(machine);
    }
    if (status != TYPE_STATUS_OK) {
        vm_machine_control_stop(control);
    }
    return status;
}

/* Finalizes devices */
C_VOID vm_machine_control_finalize(vm_machine_control_state *control,
    vm_machine *machine) {
    if (control == STD_NULL || machine == STD_NULL) return;
    vm_machine_devices_finalize(machine);
    vm_machine_debug_finalize(&machine->debug);
    vm_machine_executor_state_destroy(control->state);
    control->state = STD_NULL;
}

C_INT vm_machine_control_is_running(const vm_machine_control_state *control)
{
    return control != STD_NULL && vm_machine_executor_state_is_active(control->state);
}
