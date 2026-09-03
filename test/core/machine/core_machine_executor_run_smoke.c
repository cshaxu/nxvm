#include "type.h"



#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

C_INT main(C_VOID)
{
    core_machine_config config = { .memory_bytes = 0u };
    core_machine_run_budget budget = {1u, 0u};
    const core_machine_run_budget alu_budget = {2u, 0u};
    core_machine_run_result result;
    core_machine_observation observation;
    type_status status;
    core_machine *machine = STD_NULL;
    const type_unsigned_8 program[] = {0x90u, 0xf4u};
    const type_unsigned_8 register_or_program[] = {
        0xbau, 0x00u, 0x03u, /* mov dx, 0300h */
        0x0au, 0xdfu,       /* or bh, bl */
        0xf4u
    };

    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    if (test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    if (core_machine_memory_write(machine, 0xfffffff0u, program, sizeof(program)) !=
        TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    status = core_machine_run(machine, budget, &result);
    if (status != TYPE_STATUS_OK || result.executed != 1u ||
        result.reason != CORE_MACHINE_STOP_BUDGET) {
        STD_FPRINTF(STD_STDERR,
            "M5:T198:S1:CORE-EXECUTOR-RUN:FAIL status=%d executed=%llu reason=%d\n",
            (C_INT)status,
            (unsigned long long)result.executed, (C_INT)result.reason);
        core_machine_destroy(machine);
        return 1;
    }
    if (core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0xfffffff0u, register_or_program,
            sizeof(register_or_program)) != TYPE_STATUS_OK ||
        core_machine_run(machine, alu_budget, &result) != TYPE_STATUS_OK ||
        result.executed != 2u || result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_capture_observation(machine, &observation) != TYPE_STATUS_OK ||
        observation.cpu.eip != 0x0000fff5u) {
        core_machine_destroy(machine);
        return 1;
    }
    status = core_machine_run(machine, budget, &result);
    if (status != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        STD_FPRINTF(STD_STDERR,
            "M5:T198:S1:CORE-EXECUTOR-WAIT:FAIL status=%d executed=%llu reason=%d\n",
            (C_INT)status, (unsigned long long)result.executed,
            (C_INT)result.reason);
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    puts("M5:T83:S3:CORE-EXECUTOR-RUN:OK");
    return 0;
}
