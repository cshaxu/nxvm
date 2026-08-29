#include "type.h"

#include "core/machine/machine.h"

static C_INT cpu_pic_binding_is_owned(const core_machine *machine)
{
    return machine == STD_NULL ||
        machine->executor_cpu_execution.cpu != &machine->executor_cpu ||
        machine->executor_cpu_execution.instructions !=
            &machine->executor_cpu_instructions ||
        machine->executor_cpu_execution.pic_master != &machine->shared_pic_master ||
        machine->executor_cpu_execution.pic_slave != &machine->shared_pic_slave;
}

C_INT main(C_VOID)
{
    core_machine_config config = { .memory_bytes = 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = 0;

    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= cpu_pic_binding_is_owned(machine);
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= cpu_pic_binding_is_owned(machine);
    if (machine != STD_NULL) {
        machine->shared_pic_master.data.irr = 0xffu;
        machine->shared_pic_slave.data.irr = 0xffu;
    }
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= cpu_pic_binding_is_owned(machine);
    failed |= machine == STD_NULL || machine->shared_pic_master.data.irr != 0u ||
        machine->shared_pic_slave.data.irr != 0u;

    core_machine_destroy(machine);
    if (failed != 0) return 1;
    STD_PRINTF("M5:T295:S3:CORE-CPU-PIC-LIFECYCLE:OK\n");
    return 0;
}
