#include "lib/types/types_interface.h"
#ifndef TEST_CORE_MACHINE_CPU_FIXTURE_H
#define TEST_CORE_MACHINE_CPU_FIXTURE_H

/* Private prepared-state operations for CPU execution corpus fixtures. */
#include "core_machine_board_fixture.h"
#include "app-nxvm/devices/cpu_instructions.h"

/* This test-only borrow does not cross a production boundary: the board keeps
 * only the opaque execution owner, and tests use this fixture solely to build
 * historical instruction-state inputs. */
static inline t_cpu *test_core_machine_fixture_cpu(core_machine *machine)
{
    return machine == LIB_NULL || machine->executor_cpu_execution == LIB_NULL ?
        LIB_NULL : machine->executor_cpu_execution->cpu;
}

static inline const t_cpuins *test_core_machine_fixture_instructions(
    const core_machine *machine)
{
    return machine == LIB_NULL || machine->executor_cpu_execution == LIB_NULL ?
        LIB_NULL : machine->executor_cpu_execution->instructions;
}

static inline lib_i32 test_core_machine_fixture_reset_real_mode(core_machine *machine)
{
    t_cpu *cpu;
    core_machine_cpu_execution_context *execution;

    if (machine == LIB_NULL) return 0;
    cpu = test_core_machine_fixture_cpu(machine);
    execution = machine->executor_cpu_execution;
    if (cpu == LIB_NULL || execution == LIB_NULL) return 0;
    return core_machine_cpu_execution_load_segment(execution, &cpu->data.cs, 0u) == 0 &&
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ds, 0u) == 0 &&
        core_machine_cpu_execution_load_segment(execution, &cpu->data.es, 0u) == 0 &&
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ss, 0u) == 0 &&
        ((cpu->data.eip = 0u), 1);
}

static inline lib_i32 test_core_machine_fixture_set_control_zero(
    core_machine *machine, lib_u32 value)
{
    t_cpu *cpu = test_core_machine_fixture_cpu(machine);

    if (cpu == LIB_NULL) return 0;
    cpu->data.cr0 = value;
    return 1;
}

static inline t_cpu test_core_machine_fixture_capture_cpu_after_run(
    core_machine *machine)
{
    t_cpu observation = {0};

    t_cpu *cpu = test_core_machine_fixture_cpu(machine);

    if (cpu != LIB_NULL) observation = *cpu;
    return observation;
}


static inline lib_i32 test_core_machine_fixture_capture_instruction_exception(
    const core_machine *machine, lib_u32 *out_mask, lib_u32 *out_code)
{
    const t_cpuins *instructions = test_core_machine_fixture_instructions(machine);

    if (instructions == LIB_NULL || out_mask == LIB_NULL || out_code == LIB_NULL)
        return 0;
    *out_mask = instructions->data.except;
    *out_code = instructions->data.excode;
    return 1;
}

static inline lib_i32 test_core_machine_fixture_prepare_real_mode_execution(
    core_machine *machine, lib_u32 eip)
{
    if (!test_core_machine_fixture_reset_real_mode(machine)) return 0;
    test_core_machine_fixture_cpu(machine)->data.eip = eip;
    test_core_machine_fixture_cpu(machine)->data.flagHalt = LIB_FALSE;
    return 1;
}

/*
 * A negative real-mode #UD test that proves only producer rollback must make
 * vector 6 unavailable explicitly. Real hardware otherwise consumes the IVT
 * entry and publishes an interrupt frame, even when its contents are zero.
 * Call this immediately before the negative run; owners that prove delivery
 * instead install and validate a vector-6 handler themselves.
 */
static inline lib_i32 test_core_machine_fixture_preflight_real_ud_terminal(
    core_machine *machine)
{
    t_cpu *cpu = test_core_machine_fixture_cpu(machine);

    if (cpu == LIB_NULL) return 0;
    if (cpu->data.idtr.limit >= 0x18u) {
        cpu->data.idtr.limit = 0x17u;
    }
    return 1;
}

static inline void test_core_machine_fixture_resume_after_halt_at(
    core_machine *machine, lib_u32 eip)
{
    t_cpu *cpu = test_core_machine_fixture_cpu(machine);

    if (cpu == LIB_NULL) return;
    cpu->data.flagHalt = LIB_FALSE;
    cpu->data.eip = eip;
}

#ifdef CORE_MACHINE_TEST_CONTINUE_DELIVERED_FAULT
#define core_machine_run test_core_machine_fixture_run_after_delivery
#endif

static inline lib_i32 test_core_machine_fixture_read_linear(
    core_machine *machine, lib_u32 address, lib_uptr destination,
    lib_size bytes)
{
    return machine != LIB_NULL && core_machine_cpu_execution_read_linear(
        machine->executor_cpu_execution, address, destination, bytes) == 0;
}


#endif
