#ifndef TEST_VM_COMMON_MACHINE_FIXTURE_H
#define TEST_VM_COMMON_MACHINE_FIXTURE_H

#include "vm/machine/runtime/machine_private.h"
#include "vm/machine/runtime/machine_interface.h"

static type_status vm_test_common_machine_bind(vm_machine *machine)
{
    common_machine_driver driver;
    common_machine *common_machine = LIB_NULL;

    if (vm_machine_describe_common_driver(machine, &driver) != TYPE_STATUS_OK ||
        common_machine_create(&common_machine, &driver) != LIB_STATUS_OK ||
        vm_machine_bind_common_machine(machine, common_machine) != TYPE_STATUS_OK) {
        common_machine_destroy(common_machine);
        return TYPE_STATUS_INVALID_STATE;
    }
    return TYPE_STATUS_OK;
}

static C_VOID vm_test_common_machine_unbind(vm_machine *machine)
{
    common_machine *common_machine;

    if (machine == STD_NULL) return;
    common_machine = machine->executor;
    common_machine_destroy(common_machine);
    (C_VOID)vm_machine_bind_common_machine(machine, LIB_NULL);
}

#endif
