#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

typedef struct cpu_lifecycle_reset_case {
    x86_core_cpu_profile profile;
    lib_u32 code_base;
    lib_u32 first_fetch;
    lib_u16 selector;
    lib_u16 offset;
} cpu_lifecycle_reset_case;

static lib_i32 cpu_timing_case(x86_core_cpu_profile profile)
{
    cpu_bus_fixture first;
    cpu_bus_fixture second;
    x86_core_cpu_timing_result first_result;
    x86_core_cpu_timing_result second_result;

    cpu_bus_prepare(&first, profile);
    cpu_bus_prepare(&second, profile);
    first.memory[0x100u] = second.memory[0x100u] = 0xf3u;
    first.memory[0x101u] = second.memory[0x101u] = 0xa4u;
    first.cpu.data.cx = second.cpu.data.cx = 2u;
    first.cpu.data.si = second.cpu.data.si = 0x400u;
    first.cpu.data.di = second.cpu.data.di = 0x500u;
    x86_core_cpu_execution_refresh(&first.execution);
    first_result = x86_core_cpu_capture_timing(&first.execution);
    if (first_result.source_timing_unallocated || first_result.ticks == 0u ||
        first_result.repeat_phase != X86_CORE_CPU_RETIREMENT_REPEAT_FIRST)
        return 1;
    x86_core_cpu_execution_refresh(&second.execution);
    second_result = x86_core_cpu_capture_timing(&second.execution);
    if (second_result.repeat_phase != X86_CORE_CPU_RETIREMENT_REPEAT_FIRST ||
        first_result.ticks != second_result.ticks) return 1;
    x86_core_cpu_execution_refresh(&first.execution);
    first_result = x86_core_cpu_capture_timing(&first.execution);
    if (first_result.repeat_phase != X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION ||
        first.cpu.data.cx != 0u || second.cpu.data.cx != 1u) return 1;
    second_result = x86_core_cpu_capture_timing(&second.execution);
    if (second_result.repeat_phase != X86_CORE_CPU_RETIREMENT_REPEAT_FIRST ||
        second_result.form_id != first_result.form_id ||
        second_result.key_id != first_result.key_id) return 1;
    x86_core_cpu_state_reset(&second.execution);
    return second.execution.source_repeat_active ||
        second_result.repeat_phase != X86_CORE_CPU_RETIREMENT_REPEAT_FIRST;
}

static lib_i32 cpu_interrupt_timing_case(x86_core_cpu_profile profile)
{
    for (lib_u8 kind = 0u; kind < 3u; ++kind) {
        cpu_bus_fixture state;
        x86_core_cpu_timing_result before, after;
        const lib_u8 vector = kind == 0u ? 0x30u : kind == 1u ? 2u : 1u;

        cpu_bus_prepare(&state, profile);
        lib_memory_set(state.memory + 0x100u, 0x90, 14u);
        /* The final byte of this prefetch is only a segment prefix. Interrupt
         * entry must not replace the completed NOP's timing with that byte. */
        state.memory[0x10eu] = 0x2eu;
        state.memory[0x10fu] = 0x90u;
        state.memory[vector * 4u + 1u] = 2u;
        state.cpu.data.eflags |= VCPU_EFLAGS_IF;
        for (lib_u8 index = 0u; index < 13u; ++index)
            x86_core_cpu_execution_refresh(&state.execution);
        before = x86_core_cpu_capture_timing(&state.execution);
        if (kind == 0u) state.interrupt = LIB_TRUE;
        else if (kind == 1u) (void)x86_core_cpu_request_nmi(&state.execution);
        else state.cpu.data.eflags |= VCPU_EFLAGS_TF;
        x86_core_cpu_execution_refresh(&state.execution);
        after = x86_core_cpu_capture_timing(&state.execution);
        if (state.fault.valid || state.execution.stop_requested ||
            state.cpu.data.eip != 0x200u || !before.ticks ||
            after.ticks != before.ticks || after.form_id != before.form_id ||
            after.retirement_origin != before.retirement_origin ||
            after.source_timing_unallocated ||
            x86_core_cpu_execution_consume_instruction_fault_delivery(
                &state.execution)) {
            lib_c_printf("CPU-INTERRUPT-TIMING:profile=%u:kind=%u:failed\n",
                profile, kind);
            return 1;
        }
    }
    return 0;
}

static lib_i32 cpu_execution_context_reset_case(
    x86_core_cpu_execution_context *context, t_cpu *cpu,
    const cpu_lifecycle_reset_case *test_case)
{
    const x86_core_cpu_instruction_timing timing = { .base_ticks = 1u };

    if (context == LIB_NULL || cpu == LIB_NULL || test_case == LIB_NULL) return 1;
    const lib_u32 flags_mask = test_case->profile < X86_CORE_CPU_PROFILE_80286 ?
        0x0fffu : test_case->profile == X86_CORE_CPU_PROFILE_80286 ? 0xffffu : 0x3ffffu;
    x86_core_cpu_execution_context_bind_profiles(context, test_case->profile,
        X86_FPU_PROFILE_NONE, LIB_FALSE, X86_CORE_CPU_PROTECTED_DATA_RELOAD_STRICT,
        &timing);
    x86_core_cpu_state_reset(context);
    return cpu->data.cs.selector != test_case->selector || cpu->data.eip != test_case->offset ||
        cpu->data.cs.base != test_case->code_base ||
        cpu->data.cs.limit != 0xffffu || (cpu->data.eflags & flags_mask) != 0x02u ||
        (cpu->data.cr0 & (VCPU_CR0_PE | VCPU_CR0_MP | VCPU_CR0_EM |
            VCPU_CR0_TS | VCPU_CR0_PG)) != 0u ||
        cpu->data.cs.base + cpu->data.eip != test_case->first_fetch ||
        (test_case->profile == X86_CORE_CPU_PROFILE_80386 &&
         cpu->data.edx != 0x00000300u);
}

static lib_i32 cpu_instance_case(const cpu_lifecycle_reset_case *test_case)
{
    x86_core_cpu_execution_context *first = LIB_NULL;
    x86_core_cpu_execution_context *second = LIB_NULL;
    const x86_core_cpu_instruction_timing timing = { .base_ticks = 1u };
    x86_core_cpu_state first_state;
    x86_core_cpu_state second_state;
    x86_core_cpu_prepared_entry *entry = LIB_NULL;
    x86_core_cpu_entry_plan_state entry_state = {
        .cs = 0x1234u, .ip = 0x5678u, .sp = 0x700u, .eax = 0x12345678u
    };
    lib_i32 failed = 0;

    if (x86_core_cpu_create(&cpu_bus_provider, LIB_NULL, &first) !=
            LIB_STATUS_OK ||
        x86_core_cpu_create(&cpu_bus_provider, LIB_NULL, &second) !=
            LIB_STATUS_OK) {
        x86_core_cpu_destroy(second);
        x86_core_cpu_destroy(first);
        return 1;
    }
    x86_core_cpu_execution_context_bind_profiles(first, test_case->profile,
        X86_FPU_PROFILE_NONE, LIB_FALSE, X86_CORE_CPU_PROTECTED_DATA_RELOAD_STRICT,
        &timing);
    x86_core_cpu_execution_context_bind_profiles(second, test_case->profile,
        X86_FPU_PROFILE_NONE, LIB_FALSE, X86_CORE_CPU_PROTECTED_DATA_RELOAD_STRICT,
        &timing);
    x86_core_cpu_state_initialize(first);
    x86_core_cpu_state_initialize(second);
    x86_core_cpu_state_reset(first);
    x86_core_cpu_state_reset(second);
    x86_core_cpu_capture_state(first, &first_state);
    x86_core_cpu_capture_state(second, &second_state);
    failed |= first_state.cs != test_case->selector || second_state.cs != test_case->selector ||
        first_state.eip != test_case->offset || second_state.eip != test_case->offset ||
        first_state.cs_base != test_case->code_base ||
        second_state.cs_base != test_case->code_base ||
        x86_core_cpu_linear_pc(first) != test_case->first_fetch ||
        x86_core_cpu_linear_pc(second) != test_case->first_fetch;
    x86_core_cpu_execution_request_stop(first);
    failed |= x86_core_cpu_execution_consume_stop_request(second) ||
        !x86_core_cpu_execution_consume_stop_request(first);
    failed |= x86_core_cpu_prepare_entry(first, &entry_state, &entry) !=
        LIB_STATUS_OK;
    failed |= x86_core_cpu_linear_pc(first) != test_case->first_fetch;
    x86_core_cpu_finish_entry(entry, LIB_FALSE);
    failed |= x86_core_cpu_linear_pc(first) != test_case->first_fetch;
    entry = LIB_NULL;
    failed |= x86_core_cpu_prepare_entry(first, &entry_state, &entry) !=
        LIB_STATUS_OK;
    x86_core_cpu_finish_entry(entry, LIB_TRUE);
    failed |= x86_core_cpu_linear_pc(first) != 0x179b8u ||
        x86_core_cpu_linear_pc(second) != test_case->first_fetch;
    entry_state.eflags = 0x80000000u;
    entry = LIB_NULL;
    failed |= x86_core_cpu_prepare_entry(first, &entry_state, &entry) !=
        LIB_STATUS_INVALID_ARGUMENT || entry != LIB_NULL ||
        x86_core_cpu_linear_pc(first) != 0x179b8u;
    x86_core_cpu_destroy(first);
    x86_core_cpu_state_reset(second);
    failed |= x86_core_cpu_linear_pc(second) != test_case->first_fetch;
    x86_core_cpu_destroy(second);
    x86_core_cpu_destroy(LIB_NULL);
    return failed;
}

static lib_i32 cpu_soft_reset_case(void)
{
    const x86_core_cpu_execution_context empty = {0};
    cpu_bus_fixture fixture;
    x86_core_cpu_execution_context *context = &fixture.execution;
    t_cpu before;

    cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    context->smbase = 0x00180000u;
    fixture.cpu.data.eax = 0x12345678u;
    fixture.cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG | VCPU_CR0_WP;
    fixture.cpu.data.cr2 = 0x87654321u;
    fixture.cpu.data.cr3 = 0x00123000u;
    fixture.cpu.data.cr4 = VCPU_CR4_VME;
    fixture.cpu.data.flagHalt = LIB_TRUE;
    before = fixture.cpu;
    if (x86_core_cpu_execution_request_soft_reset(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_cpu_state_soft_reset(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_cpu_execution_consume_reset_request(LIB_NULL) != X86_CORE_CPU_RESET_NONE ||
        x86_core_cpu_execution_request_soft_reset(context) != LIB_STATUS_OK ||
        x86_core_cpu_execution_request_soft_reset(context) != LIB_STATUS_OK ||
        context->smbase != 0x00180000u || lib_memory_compare(&before, &fixture.cpu, sizeof(before)) ||
        x86_core_cpu_execution_consume_reset_request(context) != X86_CORE_CPU_RESET_SOFT ||
        x86_core_cpu_execution_consume_reset_request(context) != X86_CORE_CPU_RESET_NONE)
        return 1;

    lib_memory_set(context->cache, 0x5au, sizeof(context->cache));
    lib_memory_set(context->cache_lru, 0x5au, sizeof(context->cache_lru));
    lib_memory_set(context->cache_fill, 0x5au, sizeof(context->cache_fill));
    lib_memory_set(context->cache_read, 0x5au, sizeof(context->cache_read));
    lib_memory_set(context->tlb, 0x5au, sizeof(context->tlb));
    lib_memory_set(context->tlb_lru, 0x5au, sizeof(context->tlb_lru));
    context->prefetch_valid = context->prefetch_expected_valid = LIB_TRUE;
    context->prefetch_reservation_valid = LIB_TRUE;
    context->source_repeat_active = LIB_TRUE;
    context->debug_trap_pending = context->debug_tf_before = LIB_TRUE;
    context->debug_rf_before = LIB_TRUE;
    context->debug_trap_cause = 0x00004000u;
    context->interrupt_shadow = CPU_INTERRUPT_SHADOW_SEGMENT;
    context->nmi_in_service = context->nmi_pending = LIB_TRUE;
    context->nmi_masked = LIB_TRUE;
    context->wait_state = CPU_WAIT_STATE_TEST;
    context->wait_poll_remaining_ticks = 5u;
    x86_core_cpu_execution_request_shutdown(context);
    if (x86_core_cpu_state_soft_reset(context) != LIB_STATUS_OK ||
        context->smbase != 0x00180000u || fixture.cpu.data.eax != 0u ||
        fixture.cpu.data.edx != VCPU_486SX_SIGNATURE || fixture.cpu.data.eip != 0xfff0u ||
        fixture.cpu.data.cs.base != 0xffff0000u || fixture.cpu.data.cs.selector != 0xf000u ||
        fixture.cpu.data.cs.limit != 0xffffu || fixture.cpu.data.idtr.limit != 0x3ffu ||
        fixture.cpu.data.cr0 != (VCPU_CR0_CD | VCPU_CR0_NW | VCPU_CR0_ET) ||
        fixture.cpu.data.cr2 != 0u || fixture.cpu.data.cr3 != 0u || fixture.cpu.data.cr4 != 0u ||
        fixture.cpu.data.eflags != 2u || fixture.cpu.data.dr6 != 0xffff0ff0u ||
        fixture.cpu.data.flagHalt || x86_core_cpu_is_shutdown(context) ||
        context->prefetch_valid || context->prefetch_expected_valid ||
        context->prefetch_reservation_valid || context->source_repeat_active ||
        context->debug_trap_pending || context->debug_tf_before || context->debug_rf_before ||
        context->debug_trap_cause || context->interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
        context->nmi_in_service || context->nmi_pending || context->nmi_masked ||
        context->wait_state != CPU_WAIT_STATE_NONE || context->wait_poll_remaining_ticks ||
        lib_memory_compare(context->cache, empty.cache, sizeof(empty.cache)) ||
        lib_memory_compare(context->cache_lru, empty.cache_lru, sizeof(empty.cache_lru)) ||
        lib_memory_compare(context->cache_fill, empty.cache_fill, sizeof(empty.cache_fill)) ||
        lib_memory_compare(context->cache_read, empty.cache_read, sizeof(empty.cache_read)) ||
        lib_memory_compare(context->tlb, empty.tlb, sizeof(empty.tlb)) ||
        lib_memory_compare(context->tlb_lru, empty.tlb_lru, sizeof(empty.tlb_lru)))
        return 1;

    if (x86_core_cpu_execution_request_soft_reset(context) != LIB_STATUS_OK) return 1;
    x86_core_cpu_execution_request_reset(context);
    if (x86_core_cpu_execution_request_soft_reset(context) != LIB_STATUS_OK ||
        x86_core_cpu_execution_consume_reset_request(context) != X86_CORE_CPU_RESET_HARD)
        return 1;
    x86_core_cpu_state_reset(context);
    if (context->smbase != X86_CPU_DEFAULT_SMBASE) return 1;
    x86_core_cpu_execution_request_reset(context);
    if (x86_core_cpu_execution_request_soft_reset(context) != LIB_STATUS_OK ||
        x86_core_cpu_execution_consume_reset_request(context) != X86_CORE_CPU_RESET_HARD)
        return 1;

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_8086;
         profile < X86_CORE_CPU_PROFILE_80486SX; ++profile) {
        context->cpu_profile = profile;
        context->smbase = 0x00200000u;
        fixture.cpu.data.eax = 0x89abcdefu;
        before = fixture.cpu;
        if (x86_core_cpu_execution_request_soft_reset(context) != LIB_STATUS_UNSUPPORTED ||
            x86_core_cpu_state_soft_reset(context) != LIB_STATUS_UNSUPPORTED ||
            x86_core_cpu_execution_consume_reset_request(context) != X86_CORE_CPU_RESET_NONE ||
            context->smbase != 0x00200000u ||
            lib_memory_compare(&before, &fixture.cpu, sizeof(before))) return 1;
    }
    return 0;
}

static lib_i32 cpu_context_isolation_case(void)
{
    t_cpu first_cpu = {0};
    t_cpu second_cpu = {0};
    t_cpuins first_instructions = {0};
    t_cpuins second_instructions = {0};
    x86_core_cpu_execution_context first = {0};
    x86_core_cpu_execution_context second = {0};
    lib_i32 result = 0;

    x86_core_cpu_execution_context_initialize(
        &first, &first_cpu, &first_instructions, LIB_NULL, LIB_NULL);
    x86_core_cpu_execution_context_initialize(
        &second, &second_cpu, &second_instructions, LIB_NULL, LIB_NULL);
    x86_core_cpu_state_initialize(&first);
    x86_core_cpu_state_initialize(&second);
    x86_core_cpu_state_reset(&first);
    x86_core_cpu_state_reset(&second);
    result |= first_cpu.data.cs.selector != 0xf000u;
    result |= first_cpu.data.eip != 0x0000fff0u;
    result |= second_cpu.data.cs.selector != 0xf000u;
    result |= second_cpu.data.eip != 0x0000fff0u;
    first_cpu.data.eip = 0x12345678u;
    first_instructions.data.except = VCPUINS_EXCEPT_UD;
    x86_core_cpu_execution_request_stop(&first);
    x86_core_cpu_execution_request_reset(&first);
    result |= second_cpu.data.eip != 0x0000fff0u;
    result |= second_instructions.data.except != 0u;
    result |= x86_core_cpu_execution_consume_stop_request(&second);
    result |= x86_core_cpu_execution_consume_reset_request(&second);
    result |= !x86_core_cpu_execution_consume_stop_request(&first);
    result |= !x86_core_cpu_execution_consume_reset_request(&first);
    x86_core_cpu_execution_finalize(&second);
    x86_core_cpu_execution_finalize(&first);
    return result;
}

lib_i32 main(void)
{
    static const cpu_lifecycle_reset_case reset_cases[] = {
        {X86_CORE_CPU_PROFILE_8086, 0x000ffff0u, 0x000ffff0u, 0xffffu, 0u},
        {X86_CORE_CPU_PROFILE_8088, 0x000ffff0u, 0x000ffff0u, 0xffffu, 0u},
        {X86_CORE_CPU_PROFILE_80186, 0x000ffff0u, 0x000ffff0u, 0xffffu, 0u},
        {X86_CORE_CPU_PROFILE_80286, 0x00ff0000u, 0x00fffff0u, 0xf000u, 0xfff0u},
        {X86_CORE_CPU_PROFILE_80386, 0xffff0000u, 0xfffffff0u, 0xf000u, 0xfff0u}
    };
    t_cpu cpu = {0};
    t_cpuins instructions = {0};
    x86_core_cpu_execution_context context = {0};
    lib_i32 result = cpu_context_isolation_case() | cpu_soft_reset_case();

    x86_core_cpu_execution_context_initialize(
        &context, &cpu, &instructions, LIB_NULL, LIB_NULL);
    x86_core_cpu_state_initialize(&context);
    for (lib_size index = 0u;
         index < sizeof(reset_cases) / sizeof(reset_cases[0]); ++index) {
        result |= cpu_execution_context_reset_case(&context, &cpu,
            &reset_cases[index]);
        result |= cpu_timing_case(reset_cases[index].profile);
        result |= cpu_interrupt_timing_case(reset_cases[index].profile);
        result |= cpu_instance_case(&reset_cases[index]);
    }
    x86_core_cpu_execution_finalize(&context);
    if (result != 0) return 1;

    lib_c_printf("%s\n", "CPU-EXECUTION-LIFECYCLE:OK");
    return 0;
}
