#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_executor_fixture.h"

typedef struct reset_probe {
    lib_u32 reset_count;
} reset_probe;

static void reset_probe_reset(void *opaque)
{
    reset_probe *probe = (reset_probe *)opaque;
    if (probe != LIB_NULL) ++probe->reset_count;
}

static const core_machine_execution_provider reset_probe_provider = {
    reset_probe_reset,
    LIB_NULL
};

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    reset_probe probe = { 0u };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result run_result;
    lib_u8 value = 0xa5u;
    lib_u8 observed = 0xffu;
    lib_size memory_bytes = 0u;
    lib_i32 failed = 0;

    failed |= test_core_machine_create_executor(2u * 1024u * 1024u, &machine) !=
        LIB_STATUS_OK;
    if (failed) return 1;
    failed |= core_machine_bind_execution_provider(machine, &reset_probe_provider,
        &probe) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= probe.reset_count != 1u;
    failed |= core_machine_memory_write(machine, 0u, &value, sizeof(value)) !=
        LIB_STATUS_OK;
    failed |= core_machine_reconfigure_memory(machine, 4u * 1024u * 1024u) !=
        LIB_STATUS_OK;
    failed |= probe.reset_count != 2u;
    failed |= core_machine_get_memory_bytes(machine, &memory_bytes) != LIB_STATUS_OK ||
        memory_bytes != 4u * 1024u * 1024u;
    failed |= core_machine_memory_read(machine, 0u, &observed, sizeof(observed)) !=
        LIB_STATUS_OK || observed != 0u;
    failed |= core_machine_run(machine, budget, &run_result) != LIB_STATUS_OK;
    failed |= run_result.reason != CORE_MACHINE_STOP_BUDGET;
    failed |= core_machine_reconfigure_memory(machine, 8u * 1024u * 1024u) !=
        LIB_STATUS_INVALID_STATE;
    failed |= core_machine_request_stop(machine) != LIB_STATUS_OK;
    failed |= core_machine_run(machine, budget, &run_result) != LIB_STATUS_OK;
    failed |= core_machine_reconfigure_memory(machine, 1u * 1024u * 1024u) !=
        LIB_STATUS_INVALID_STATE;
    failed |= core_machine_report_fault(machine, 0x172u) != LIB_STATUS_OK;
    failed |= core_machine_reconfigure_memory(machine, 8u * 1024u * 1024u) !=
        LIB_STATUS_INVALID_STATE;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;

    core_machine_destroy(machine);
    if (failed) return 1;
    puts("M5:T172:S1:MEMORY-RECONFIGURE:OK");
    return 0;
}
