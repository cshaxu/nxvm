#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-core/machine/machine_interface.h"
#include "executor_fixture.h"

static lib_i32 expect_lifecycle(x86_core_machine *machine,
    x86_core_machine_lifecycle expected)
{
    x86_core_machine_lifecycle actual;

    return x86_core_machine_get_lifecycle(machine, &actual) != LIB_STATUS_OK ||
        actual != expected;
}

static lib_i32 expect_reset_vector(x86_core_machine *machine)
{
    x86_core_cpu_state cpu;

    return x86_core_machine_get_cpu_state(machine, &cpu) != LIB_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0x0000fff0u;
}

lib_i32 main(void)
{
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result;
    lib_i32 failed = 0;

    failed |= test_x86_core_machine_create_executor(16u * 1024u * 1024u, &machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= expect_lifecycle(machine, X86_CORE_MACHINE_STOPPED);
    failed |= expect_reset_vector(machine);
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != X86_CORE_MACHINE_STOP_BUDGET;
    failed |= expect_lifecycle(machine, X86_CORE_MACHINE_PAUSED);
    failed |= x86_core_machine_request_stop(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != X86_CORE_MACHINE_STOP_REQUESTED;
    failed |= expect_lifecycle(machine, X86_CORE_MACHINE_STOPPED);
    failed |= expect_reset_vector(machine);

    x86_core_machine_destroy(machine);
    if (failed != 0) return 1;
    lib_c_printf("STOPPED-LIFECYCLE:OK\n");
    return 0;
}
