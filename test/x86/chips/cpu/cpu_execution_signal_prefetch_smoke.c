#include "support/cpu_bus_fixture.h"
#include <stdio.h>

static lib_i32 cpu_signal_case(core_machine_cpu_profile profile)
{
    cpu_bus_fixture fixture;
    core_machine_cpu_state state;

    cpu_bus_prepare(&fixture, profile);
    fixture.memory[0x100u] = 0xf4u;
    fixture.memory[0x09u] = 0x02u;
    core_machine_cpu_capture_state(&fixture.execution, &state);
    if (state.cs != 0u || state.cs_base != 0u || state.eip != 0x100u ||
        state.eflags != fixture.cpu.data.eflags || state.halted ||
        core_machine_cpu_linear_pc(&fixture.execution) != 0x100u) return 1;

    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_TRUE);
    if (!core_machine_cpu_nmi_is_masked(&fixture.execution) ||
        core_machine_cpu_request_nmi(&fixture.execution) ||
        fixture.cpu.data.flagNMI) return 1;
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (!core_machine_cpu_is_halted(&fixture.execution)) return 1;
    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_FALSE);
    if (core_machine_cpu_nmi_is_masked(&fixture.execution) ||
        fixture.cpu.data.flagNMI) return 1;
    if (!core_machine_cpu_request_nmi(&fixture.execution) ||
        !core_machine_cpu_request_nmi(&fixture.execution)) return 1;

    /* Masking after admission retains the single pending edge until delivery. */
    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_TRUE);
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (!core_machine_cpu_is_halted(&fixture.execution) ||
        !fixture.cpu.data.flagNMI) return 1;
    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_FALSE);
    core_machine_cpu_execution_refresh(&fixture.execution);
    core_machine_cpu_capture_state(&fixture.execution, &state);
    if (state.halted || state.eip != 0x200u || fixture.cpu.data.flagNMI ||
        fixture.cpu.data.sp != 0x6fau || fixture.acknowledgements != 0u ||
        fixture.instructions.data.except != 0u) return 1;
    fixture.cpu.data.eip = 0x345u;
    return state.eip != 0x200u;
}

static lib_i32 cpu_prefetch_case(core_machine_cpu_profile profile)
{
    cpu_bus_fixture fixture;
    core_machine_cpu_execution_context *cpu = &fixture.execution;
    lib_bool byte_queue = profile == CORE_MACHINE_CPU_PROFILE_8088;
    lib_u8 count = byte_queue ? 4u : 15u;
    lib_u32 next = byte_queue ? 0x11u : 0x17u;
    lib_i32 failed = 0;

    cpu_bus_prepare(&fixture, profile);
    cpu->prefetch_linear = 0x10u;
    cpu->prefetch_count = count;
    cpu->prefetch_valid = LIB_TRUE;
    cpu->prefetch_expected_linear = next;
    cpu->prefetch_expected_valid = LIB_TRUE;
    core_machine_cpu_execution_reserve_prefetch(cpu);
    failed |= !cpu->prefetch_reservation_valid ||
        cpu->prefetch_reservation_linear != (byte_queue ? 0x14u : next) ||
        cpu->prefetch_reservation_count != (byte_queue ? 1u : count);
    core_machine_cpu_execution_advance_prefetch_reservation(cpu);
    failed |= cpu->prefetch_reservation_valid || !cpu->prefetch_valid ||
        cpu->prefetch_linear != (byte_queue ? next : 0x10u) ||
        cpu->prefetch_count != count;
    if (byte_queue) cpu->prefetch_expected_linear = 0x12u;
    core_machine_cpu_execution_reserve_prefetch(cpu);
    failed |= !cpu->prefetch_reservation_valid;
    core_machine_cpu_execution_invalidate_prefetch(cpu);
    failed |= cpu->prefetch_reservation_valid || cpu->prefetch_valid ||
        cpu->prefetch_expected_valid;
    cpu->prefetch_linear = 0x10u;
    cpu->prefetch_count = count;
    cpu->prefetch_valid = LIB_TRUE;
    cpu->prefetch_expected_linear = next;
    cpu->prefetch_expected_valid = LIB_TRUE;
    core_machine_cpu_execution_reserve_prefetch(cpu);
    failed |= !cpu->prefetch_reservation_valid;
    core_machine_cpu_state_reset(cpu);
    failed |= cpu->prefetch_reservation_valid || cpu->prefetch_valid ||
        cpu->prefetch_expected_valid;
    return failed;
}

static lib_i32 run_8088_prefetch_capacity(void)
{
    static const lib_u8 program[] = {
        0xbbu, 0x07u, 0x00u, /* mov bx, 7 */
        0xc6u, 0x07u, 0xccu, /* mov byte ptr [bx], 0cch */
        0x90u, 0x90u
    };
    cpu_bus_fixture state;
    core_machine_cpu_execution_context *execution;
    lib_i32 failed = 0;

    cpu_bus_prepare(&state, CORE_MACHINE_CPU_PROFILE_8088);
    state.cpu.data.eip = 0u;

    lib_memory_copy(state.memory, program, sizeof(program));
    core_machine_cpu_execution_refresh(&state.execution);
    failed |= state.faults != 0u || state.instructions.data.except != 0u;
    execution = &state.execution;
    failed |= execution->prefetch_capacity != 4u ||
        execution->prefetch_count != 4u;
    core_machine_cpu_execution_reserve_prefetch(execution);
    failed |= !execution->prefetch_reservation_valid;
    core_machine_cpu_execution_advance_prefetch_reservation(execution);
    failed |= execution->prefetch_reservation_valid ||
        execution->prefetch_count != 2u || execution->prefetch_bytes[1] != 0x07u;
    core_machine_cpu_execution_invalidate_prefetch(execution);
    failed |= execution->prefetch_valid || execution->prefetch_count != 0u;
    return failed;
}

static lib_i32 run_8088_prefetch_control_and_self_modify(void)
{
    static const lib_u8 self_modifying[] = {
        0xbbu, 0x07u, 0x00u, /* mov bx, 7 */
        0xc6u, 0x07u, 0xccu, /* queued mov byte ptr [bx], 0cch */
        0x90u
    };
    const lib_u8 replacement = 0x90u;
    cpu_bus_fixture state;
    core_machine_cpu_execution_context *execution;
    lib_i32 failed = 0;

    cpu_bus_prepare(&state, CORE_MACHINE_CPU_PROFILE_8088);
    state.cpu.data.eip = 0u;

    lib_memory_copy(state.memory, self_modifying, sizeof(self_modifying));
    core_machine_cpu_execution_refresh(&state.execution);
    failed |= state.faults != 0u || state.instructions.data.except != 0u;
    execution = &state.execution;
    failed |= execution->prefetch_count != 4u || execution->prefetch_bytes[3] !=
        0xc6u;
    /* A byte already owned by the 8088 queue remains stale after the
     * write; this is not a second VM-side instruction cache. */
    state.memory[3u] = replacement;
    failed |= execution->prefetch_bytes[3] != 0xc6u;
    /* The same CPU-owned flush called by control transfers drops the old
     * queue before the target may be fetched. */
    core_machine_cpu_execution_invalidate_prefetch(execution);
    failed |= execution->prefetch_valid || execution->prefetch_count != 0u;
    return failed;
}

lib_i32 main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_i32 result = 0;

    for (lib_size index = 0u;
         index < sizeof(profiles) / sizeof(profiles[0]); ++index) {
        result |= cpu_signal_case(profiles[index]);
        result |= cpu_prefetch_case(profiles[index]);
    }
    result |= run_8088_prefetch_capacity();
    result |= run_8088_prefetch_control_and_self_modify();
    if (result != 0) return 1;

    puts("M5:T539:S89:CPU-EXECUTION-SIGNAL-PREFETCH:OK");
    puts("M5:T484:S3:XT-8088-QUEUE:OK");
    return 0;
}
