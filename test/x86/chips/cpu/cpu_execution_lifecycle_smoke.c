#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

typedef struct cpu_lifecycle_reset_case {
    core_machine_cpu_profile profile;
    lib_u32 code_base;
    lib_u32 first_fetch;
    lib_u16 selector;
    lib_u16 offset;
} cpu_lifecycle_reset_case;

static lib_i32 cpu_timing_case(core_machine_cpu_profile profile)
{
    cpu_bus_fixture first;
    cpu_bus_fixture second;
    core_machine_cpu_timing_result first_result;
    core_machine_cpu_timing_result second_result;

    cpu_bus_prepare(&first, profile);
    cpu_bus_prepare(&second, profile);
    first.memory[0x100u] = second.memory[0x100u] = 0xf3u;
    first.memory[0x101u] = second.memory[0x101u] = 0xa4u;
    first.cpu.data.cx = second.cpu.data.cx = 2u;
    first.cpu.data.si = second.cpu.data.si = 0x400u;
    first.cpu.data.di = second.cpu.data.di = 0x500u;
    core_machine_cpu_execution_refresh(&first.execution);
    if (!core_machine_cpu_timing_select(&first.execution, &first_result) ||
        first_result.source_timing_unallocated || first_result.ticks == 0u ||
        first_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST)
        return 1;
    core_machine_cpu_execution_refresh(&second.execution);
    if (!core_machine_cpu_timing_select(&second.execution, &second_result) ||
        second_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST ||
        first_result.ticks != second_result.ticks) return 1;
    core_machine_cpu_execution_refresh(&first.execution);
    if (!core_machine_cpu_timing_select(&first.execution, &first_result) ||
        first_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION ||
        first.cpu.data.cx != 0u || second.cpu.data.cx != 1u) return 1;
    second_result = core_machine_cpu_capture_timing(&second.execution);
    if (second_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST ||
        second_result.form_id != first_result.form_id ||
        second_result.key_id != first_result.key_id) return 1;
    core_machine_cpu_state_reset(&second.execution);
    return second.execution.source_repeat_active ||
        second_result.repeat_phase != CORE_MACHINE_RETIREMENT_REPEAT_FIRST;
}

static lib_i32 cpu_execution_context_reset_case(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    const cpu_lifecycle_reset_case *test_case)
{
    const core_machine_instruction_timing timing = { .base_ticks = 1u };

    if (context == LIB_NULL || cpu == LIB_NULL || test_case == LIB_NULL) return 1;
    const lib_u32 flags_mask = test_case->profile < CORE_MACHINE_CPU_PROFILE_80286 ?
        0x0fffu : test_case->profile == CORE_MACHINE_CPU_PROFILE_80286 ? 0xffffu : 0x3ffffu;
    core_machine_cpu_execution_context_bind_profiles(context, test_case->profile,
        X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_state_reset(context);
    return cpu->data.cs.selector != test_case->selector || cpu->data.eip != test_case->offset ||
        cpu->data.cs.base != test_case->code_base ||
        cpu->data.cs.limit != 0xffffu || (cpu->data.eflags & flags_mask) != 0x02u ||
        (cpu->data.cr0 & (VCPU_CR0_PE | VCPU_CR0_MP | VCPU_CR0_EM |
            VCPU_CR0_TS | VCPU_CR0_PG)) != 0u ||
        cpu->data.cs.base + cpu->data.eip != test_case->first_fetch ||
        (test_case->profile == CORE_MACHINE_CPU_PROFILE_80386 &&
         cpu->data.edx != 0x00000300u);
}

static lib_i32 cpu_instance_case(const cpu_lifecycle_reset_case *test_case)
{
    core_machine_cpu_execution_context *first = LIB_NULL;
    core_machine_cpu_execution_context *second = LIB_NULL;
    const core_machine_instruction_timing timing = { .base_ticks = 1u };
    core_machine_cpu_state first_state;
    core_machine_cpu_state second_state;
    core_machine_cpu_prepared_entry *entry = LIB_NULL;
    core_machine_entry_plan_state entry_state = {
        .cs = 0x1234u, .ip = 0x5678u, .sp = 0x700u, .eax = 0x12345678u
    };
    lib_i32 failed = 0;

    if (core_machine_cpu_create(&cpu_bus_provider, LIB_NULL, &first) !=
            LIB_STATUS_OK ||
        core_machine_cpu_create(&cpu_bus_provider, LIB_NULL, &second) !=
            LIB_STATUS_OK) {
        core_machine_cpu_destroy(second);
        core_machine_cpu_destroy(first);
        return 1;
    }
    core_machine_cpu_execution_context_bind_profiles(first, test_case->profile,
        X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_execution_context_bind_profiles(second, test_case->profile,
        X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_state_initialize(first);
    core_machine_cpu_state_initialize(second);
    core_machine_cpu_state_reset(first);
    core_machine_cpu_state_reset(second);
    core_machine_cpu_capture_state(first, &first_state);
    core_machine_cpu_capture_state(second, &second_state);
    failed |= first_state.cs != test_case->selector || second_state.cs != test_case->selector ||
        first_state.eip != test_case->offset || second_state.eip != test_case->offset ||
        first_state.cs_base != test_case->code_base ||
        second_state.cs_base != test_case->code_base ||
        core_machine_cpu_linear_pc(first) != test_case->first_fetch ||
        core_machine_cpu_linear_pc(second) != test_case->first_fetch;
    core_machine_cpu_execution_request_stop(first);
    failed |= core_machine_cpu_execution_consume_stop_request(second) ||
        !core_machine_cpu_execution_consume_stop_request(first);
    failed |= core_machine_cpu_prepare_entry(first, &entry_state, &entry) !=
        LIB_STATUS_OK;
    failed |= core_machine_cpu_linear_pc(first) != test_case->first_fetch;
    core_machine_cpu_finish_entry(entry, LIB_FALSE);
    failed |= core_machine_cpu_linear_pc(first) != test_case->first_fetch;
    entry = LIB_NULL;
    failed |= core_machine_cpu_prepare_entry(first, &entry_state, &entry) !=
        LIB_STATUS_OK;
    core_machine_cpu_finish_entry(entry, LIB_TRUE);
    failed |= core_machine_cpu_linear_pc(first) != 0x179b8u ||
        core_machine_cpu_linear_pc(second) != test_case->first_fetch;
    entry_state.eflags = 0x80000000u;
    entry = LIB_NULL;
    failed |= core_machine_cpu_prepare_entry(first, &entry_state, &entry) !=
        LIB_STATUS_INVALID_ARGUMENT || entry != LIB_NULL ||
        core_machine_cpu_linear_pc(first) != 0x179b8u;
    core_machine_cpu_destroy(first);
    core_machine_cpu_state_reset(second);
    failed |= core_machine_cpu_linear_pc(second) != test_case->first_fetch;
    core_machine_cpu_destroy(second);
    core_machine_cpu_destroy(LIB_NULL);
    return failed;
}

static lib_i32 cpu_context_isolation_case(void)
{
    t_cpu first_cpu = {0};
    t_cpu second_cpu = {0};
    t_cpuins first_instructions = {0};
    t_cpuins second_instructions = {0};
    core_machine_cpu_execution_context first = {0};
    core_machine_cpu_execution_context second = {0};
    lib_i32 result = 0;

    core_machine_cpu_execution_context_initialize(
        &first, &first_cpu, &first_instructions, LIB_NULL, LIB_NULL);
    core_machine_cpu_execution_context_initialize(
        &second, &second_cpu, &second_instructions, LIB_NULL, LIB_NULL);
    core_machine_cpu_state_initialize(&first);
    core_machine_cpu_state_initialize(&second);
    core_machine_cpu_state_reset(&first);
    core_machine_cpu_state_reset(&second);
    result |= first_cpu.data.cs.selector != 0xf000u;
    result |= first_cpu.data.eip != 0x0000fff0u;
    result |= second_cpu.data.cs.selector != 0xf000u;
    result |= second_cpu.data.eip != 0x0000fff0u;
    first_cpu.data.eip = 0x12345678u;
    first_instructions.data.except = VCPUINS_EXCEPT_UD;
    core_machine_cpu_execution_request_stop(&first);
    core_machine_cpu_execution_request_reset(&first);
    result |= second_cpu.data.eip != 0x0000fff0u;
    result |= second_instructions.data.except != 0u;
    result |= core_machine_cpu_execution_consume_stop_request(&second);
    result |= core_machine_cpu_execution_consume_reset_request(&second);
    result |= !core_machine_cpu_execution_consume_stop_request(&first);
    result |= !core_machine_cpu_execution_consume_reset_request(&first);
    core_machine_cpu_execution_finalize(&second);
    core_machine_cpu_execution_finalize(&first);
    return result;
}

lib_i32 main(void)
{
    static const cpu_lifecycle_reset_case reset_cases[] = {
        {CORE_MACHINE_CPU_PROFILE_8086, 0x000ffff0u, 0x000ffff0u, 0xffffu, 0u},
        {CORE_MACHINE_CPU_PROFILE_8088, 0x000ffff0u, 0x000ffff0u, 0xffffu, 0u},
        {CORE_MACHINE_CPU_PROFILE_80186, 0x000ffff0u, 0x000ffff0u, 0xffffu, 0u},
        {CORE_MACHINE_CPU_PROFILE_80286, 0x00ff0000u, 0x00fffff0u, 0xf000u, 0xfff0u},
        {CORE_MACHINE_CPU_PROFILE_80386, 0xffff0000u, 0xfffffff0u, 0xf000u, 0xfff0u}
    };
    t_cpu cpu = {0};
    t_cpuins instructions = {0};
    core_machine_cpu_execution_context context = {0};
    lib_i32 result = cpu_context_isolation_case();

    core_machine_cpu_execution_context_initialize(
        &context, &cpu, &instructions, LIB_NULL, LIB_NULL);
    core_machine_cpu_state_initialize(&context);
    for (lib_size index = 0u;
         index < sizeof(reset_cases) / sizeof(reset_cases[0]); ++index) {
        result |= cpu_execution_context_reset_case(&context, &cpu,
            &reset_cases[index]);
        result |= cpu_timing_case(reset_cases[index].profile);
        result |= cpu_instance_case(&reset_cases[index]);
    }
    core_machine_cpu_execution_finalize(&context);
    if (result != 0) return 1;

    lib_c_printf("%s\n", "CPU-EXECUTION-LIFECYCLE:OK");
    return 0;
}
