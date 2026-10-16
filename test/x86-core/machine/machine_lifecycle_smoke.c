#include "lib/types/types_interface.h"
#include "lib/types/file.h"



#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/machine.h"
#include "executor_fixture.h"

static lib_i32 expect_status(lib_status actual, lib_status expected)
{
    return actual == expected ? 0 : 1;
}

static lib_i32 expect_lifecycle(
    x86_core_machine *machine,
    x86_core_machine_lifecycle expected)
{
    x86_core_machine_lifecycle actual;

    if (x86_core_machine_get_lifecycle(machine, &actual) != LIB_STATUS_OK) {
        return 1;
    }

    return actual == expected ? 0 : 1;
}

static lib_i32 instance_owners_are_distinct(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    x86_core_machine *first = LIB_NULL;
    x86_core_machine *second = LIB_NULL;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &first) != LIB_STATUS_OK ||
        x86_core_machine_neutral_create(&config, &second) != LIB_STATUS_OK;

    if (!failed) failed = first == second ||
        first->executor_cpu_execution == second->executor_cpu_execution ||
        &first->executor_memory == &second->executor_memory ||
        &first->executor_port == &second->executor_port ||
        first->executor_memory.connect.backing == second->executor_memory.connect.backing;
    x86_core_machine_destroy(second);
    x86_core_machine_destroy(first);
    return failed;
}

static lib_status lifecycle_firmware_configure(void *owner,
    x86_core_machine_firmware_context *firmware)
{
    (void)owner;
    (void)firmware;
    return LIB_STATUS_OK;
}

static lib_status lifecycle_firmware_reset(void *owner,
    x86_core_machine_firmware_context *firmware)
{
    (void)firmware;
    return *(const lib_bool *)owner ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_OK;
}

static lib_i32 firmware_reset_failure(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    const x86_core_machine_firmware_provider firmware = {
        lifecycle_firmware_configure, lifecycle_firmware_reset, LIB_NULL
    };
    x86_core_machine *machine = LIB_NULL;
    lib_bool reject = LIB_FALSE;
    const lib_u8 value = 0x5au;
    lib_u8 observed = 0xffu;
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_core_machine_bind_firmware_provider(machine, &firmware, &reject) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(machine, 0x100u, &value, 1u) != LIB_STATUS_OK) goto done;
    reject = LIB_TRUE;
    failed = x86_core_machine_reset(machine) != LIB_STATUS_UNSUPPORTED ||
        expect_lifecycle(machine, X86_CORE_MACHINE_INITIALIZED) ||
        x86_core_machine_memory_inspect(machine, 0x100u, &observed, 1u) != LIB_STATUS_OK ||
        observed != 0u || machine->firmware_provider != &firmware;
    reject = LIB_FALSE;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        expect_lifecycle(machine, X86_CORE_MACHINE_STOPPED);
done:
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 port_latch_reset(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    for (lib_size index = 0u; !failed && index < 2u; ++index) {
        machine->executor_port.data.ioDWord = 0xa5a5a5a5u;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            machine->executor_port.data.ioDWord != 0u;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    x86_core_machine *machine = LIB_NULL;
    x86_core_cpu_execution_context *cpu_identity = LIB_NULL;
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result run_result;
    x86_core_cpu_state cpu;
    x86_core_machine_observation observation;
    lib_u8 halt = 0xf4u;
    lib_i32 result = instance_owners_are_distinct() || firmware_reset_failure() ||
        port_latch_reset();

    result |= expect_status(test_x86_core_machine_create_executor(16u * 1024u * 1024u, &machine),
                            LIB_STATUS_OK);
    if (machine == LIB_NULL) return 1;
    cpu_identity = machine->executor_cpu_execution;
    result |= cpu_identity == LIB_NULL;
    result |= expect_lifecycle(machine, X86_CORE_MACHINE_INITIALIZED);
    result |= expect_status(x86_core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_INVALID_STATE);

    result |= expect_status(x86_core_machine_freeze_execution_providers(machine),
                            LIB_STATUS_OK);

    result |= expect_status(x86_core_machine_reset(machine), LIB_STATUS_OK);
    result |= machine->executor_cpu_execution != cpu_identity;
    result |= expect_lifecycle(machine, X86_CORE_MACHINE_STOPPED);
    result |= expect_status(x86_core_machine_get_cpu_state(machine, &cpu),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_capture_observation(machine, &observation),
                            LIB_STATUS_OK);
    result |= observation.lifecycle != X86_CORE_MACHINE_STOPPED ||
              observation.cpu.cs != cpu.cs || observation.cpu.eip != cpu.eip;
    result |= cpu.cs != 0xf000u || cpu.eip != 0x0000fff0u;
    result |= expect_status(x86_core_machine_memory_write(machine, 0xffff0u, &halt, 1u),
                            LIB_STATUS_OK);
    budget.instructions = 0u;
    result |= expect_status(x86_core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_INVALID_ARGUMENT);

    budget.instructions = 1u;
    result |= expect_status(x86_core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_OK);
    result |= run_result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    result |= expect_lifecycle(machine, X86_CORE_MACHINE_PAUSED);

    result |= expect_status(x86_core_machine_request_stop(machine),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_OK);
    result |= run_result.reason != X86_CORE_MACHINE_STOP_REQUESTED;
    result |= expect_lifecycle(machine, X86_CORE_MACHINE_STOPPED);
    result |= expect_status(x86_core_machine_capture_observation(machine, &observation),
                            LIB_STATUS_OK);
    result |= expect_lifecycle(machine, X86_CORE_MACHINE_STOPPED);

    result |= expect_status(x86_core_machine_reset(machine), LIB_STATUS_OK);
    result |= machine->executor_cpu_execution != cpu_identity;
    result |= expect_status(x86_core_machine_report_fault(machine, 0x1234u),
                            LIB_STATUS_OK);
    result |= expect_lifecycle(machine, X86_CORE_MACHINE_FAULTED);
    result |= expect_status(x86_core_machine_capture_observation(machine, &observation),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_run(machine, budget, &run_result),
                            LIB_STATUS_INTERNAL_ERROR);
    result |= run_result.reason != X86_CORE_MACHINE_STOP_FAULT ||
              run_result.detail != 0x1234u ||
              run_result.linear_pc != 0xfffffff0u;

    x86_core_machine_destroy(machine);
    if (result != 0) {
        return 1;
    }

    lib_c_printf("LIFECYCLE:OK\n");
    return 0;
}
