#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

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
        fixture.execution.nmi_pending) return 1;
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (!core_machine_cpu_is_halted(&fixture.execution)) return 1;
    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_FALSE);
    if (core_machine_cpu_nmi_is_masked(&fixture.execution) ||
        fixture.execution.nmi_pending) return 1;
    if (!core_machine_cpu_request_nmi(&fixture.execution) ||
        !core_machine_cpu_request_nmi(&fixture.execution)) return 1;

    /* Masking after admission retains the single pending edge until delivery. */
    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_TRUE);
    core_machine_cpu_execution_refresh(&fixture.execution);
    if (!core_machine_cpu_is_halted(&fixture.execution) ||
        !fixture.execution.nmi_pending) return 1;
    core_machine_cpu_set_nmi_mask(&fixture.execution, LIB_FALSE);
    core_machine_cpu_execution_refresh(&fixture.execution);
    core_machine_cpu_capture_state(&fixture.execution, &state);
    if (state.halted || state.eip != 0x200u || fixture.execution.nmi_pending ||
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

static lib_bool cpu_signal_priority_matrix(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    const lib_u16 debug_vector[] = {0x0300u,0u};
    const lib_u16 nmi_vector[] = {0x0200u,0u};
    const lib_u16 irq_vector[] = {0x0400u,0u};
    lib_size profile;
    lib_u8 requests, halted;
    lib_u32 failures = 0u;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (halted = 0u; halted < 2u; ++halted)
    for (requests = 1u; requests < 8u; ++requests) {
        cpu_bus_fixture state;
        const lib_bool debug = (requests & 1u) != 0u;
        const lib_bool nmi = (requests & 2u) != 0u;
        const lib_bool irq = (requests & 4u) != 0u;
        const lib_bool early = profiles[profile] < CORE_MACHINE_CPU_PROFILE_80286;
        const lib_bool irq_entered = irq && !nmi && (!debug || early);
        const lib_bool nested = debug && (nmi || irq_entered);
        const lib_u16 expected_ip = debug ? (nmi && !early ? 0x0200u :
            0x0300u) : (nmi ? 0x0200u : 0x0400u);
        const lib_u16 expected_return = nested ? (early ?
            (nmi ? 0x0200u : 0x0400u) : 0x0300u) : 0x0101u;
        lib_u16 saved_ip = 0u;

        cpu_bus_prepare(&state, profiles[profile]);
        state.cpu.data.eflags = 2u | VCPU_EFLAGS_IF | (debug ? VCPU_EFLAGS_TF : 0u);
        state.memory[0x0100u] = halted ? 0xf4u : 0x90u;
        lib_memory_copy(state.memory + 4u, debug_vector, sizeof(debug_vector));
        lib_memory_copy(state.memory + 8u, nmi_vector, sizeof(nmi_vector));
        lib_memory_copy(state.memory + 0xc0u, irq_vector, sizeof(irq_vector));
        state.interrupt = irq;
        if (nmi) core_machine_cpu_request_nmi(&state.execution);
        core_machine_cpu_execution_refresh(&state.execution);
        if (halted && debug && !nmi && !irq) {
            if (state.execution.stop_requested || state.faults ||
                !state.cpu.data.flagHalt || !state.execution.debug_trap_pending ||
                state.cpu.data.eip != 0x0101u || state.cpu.data.sp != 0x0700u ||
                state.acknowledgements != 0u) ++failures;
            state.cpu.data.eflags &= ~VCPU_EFLAGS_IF;
            state.interrupt = LIB_TRUE;
            core_machine_cpu_execution_refresh(&state.execution);
            if (state.execution.stop_requested || !state.cpu.data.flagHalt ||
                !state.execution.debug_trap_pending || !state.interrupt ||
                state.acknowledgements != 0u || state.cpu.data.eip != 0x0101u)
                ++failures;
            state.cpu.data.eflags |= VCPU_EFLAGS_IF;
            core_machine_cpu_execution_refresh(&state.execution);
            lib_memory_copy(&saved_ip, state.memory + state.cpu.data.sp, sizeof(saved_ip));
            if (state.execution.stop_requested || state.faults || state.cpu.data.flagHalt ||
                state.execution.debug_trap_pending || state.cpu.data.eip != 0x0300u ||
                state.cpu.data.sp != (early ? 0x06f4u : 0x06fau) ||
                saved_ip != (early ? 0x0400u : 0x0101u) ||
                state.acknowledgements != (early ? 1u : 0u) || state.interrupt == early)
                ++failures;
            continue;
        }
        lib_memory_copy(&saved_ip, state.memory + state.cpu.data.sp, sizeof(saved_ip));
        if (state.execution.stop_requested || state.faults ||
            state.cpu.data.flagHalt ||
            state.cpu.data.eip != expected_ip ||
            state.cpu.data.sp != (nested ? 0x06f4u : 0x06fau) ||
            saved_ip != expected_return || state.acknowledgements != (irq_entered ? 1u : 0u) ||
            state.interrupt != (irq && !irq_entered) || state.execution.nmi_pending) {
            if (failures < 8u) lib_c_printf("Priority profile=%u requests=%u ip=%x/%x sp=%x return=%x/%x ack=%u\n",
                (unsigned)profiles[profile], requests, state.cpu.data.eip,
                expected_ip, state.cpu.data.sp, saved_ip, expected_return,
                (unsigned)state.acknowledgements);
            ++failures;
        }
    }
    lib_c_printf("Normal/HLT interrupt priority cases=70 failures=%u\n", (unsigned)failures);
    return failures == 0u;
}

static lib_bool cpu_signal_shadow_matrix(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const struct { lib_u8 code[5]; lib_u8 length; lib_bool stack; } forms[] = {
        {{0x8eu,0xc0u,0u,0u,0u},2u,LIB_FALSE},
        {{0x8eu,0xd0u,0u,0u,0u},2u,LIB_TRUE},
        {{0x8eu,0xd8u,0u,0u,0u},2u,LIB_FALSE},
        {{0x07u,0u,0u,0u,0u},1u,LIB_FALSE},
        {{0x17u,0u,0u,0u,0u},1u,LIB_TRUE},
        {{0x1fu,0u,0u,0u,0u},1u,LIB_FALSE},
        {{0xfbu,0u,0u,0u,0u},1u,LIB_FALSE},
        {{0x0fu,0xb2u,0x06u,0x84u,0u},5u,LIB_FALSE}
    };
    const lib_u16 nmi_vector[] = {0x0200u,0u};
    const lib_u16 irq_vector[] = {0x0400u,0u};
    const lib_u16 pointer[] = {0x0123u,0u};
    lib_size profile, form;
    lib_u8 old_if, requests;
    lib_u32 count = 0u, failures = 0u;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form < (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80386 ?
        8u : 7u); ++form)
    for (old_if = 0u; old_if < 2u; ++old_if)
    for (requests = 0u; requests < 4u; ++requests) {
        cpu_bus_fixture state;
        const lib_bool segment = form < 6u && (forms[form].stack ||
            profiles[profile] < CORE_MACHINE_CPU_PROFILE_80186);
        const lib_bool enabled = old_if || form == 6u;
        const lib_bool nmi = (requests & 1u) && !segment;
        const lib_bool irq = (requests & 2u) && enabled && !segment &&
            form != 6u && !nmi;
        const lib_u16 next_ip = 0x0100u + forms[form].length;

        cpu_bus_prepare(&state, profiles[profile]);
        state.cpu.data.ax = 0u;
        state.cpu.data.eflags = 2u | (old_if ? VCPU_EFLAGS_IF : 0u);
        lib_memory_copy(state.memory + 8u, nmi_vector, sizeof(nmi_vector));
        lib_memory_copy(state.memory + 0xc0u, irq_vector, sizeof(irq_vector));
        lib_memory_copy(state.memory + 0x84u, pointer, sizeof(pointer));
        lib_memory_copy(state.memory + 0x0100u, forms[form].code, forms[form].length);
        state.memory[next_ip] = 0x90u;
        state.interrupt = (requests & 2u) != 0u;
        if (requests & 1u) core_machine_cpu_request_nmi(&state.execution);
        core_machine_cpu_execution_refresh(&state.execution);
        ++count;
        if (state.execution.stop_requested || state.faults ||
            state.cpu.data.eip != (nmi ? 0x0200u : (irq ? 0x0400u : next_ip)) ||
            state.acknowledgements != (irq ? 1u : 0u) ||
            state.execution.nmi_pending != ((requests & 1u) && !nmi)) {
            ++failures;
            continue;
        }
        if (!nmi && !irq) {
            core_machine_cpu_execution_refresh(&state.execution);
            if (state.execution.stop_requested || state.faults ||
                state.cpu.data.eip != ((requests & 1u) ? 0x0200u :
                    (((requests & 2u) && enabled) ? 0x0400u : next_ip + 1u)))
                ++failures;
        }
    }
    lib_c_printf("Segment/STI/LSS external shadow cases=%u failures=%u\n",
        (unsigned)count, (unsigned)failures);
    return failures == 0u;
}

static lib_bool cpu_signal_sti_sequences(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 sequences[][3] = {
        {0xfbu,0x90u,0x90u}, {0xfbu,0xfbu,0x90u},
        {0xfbu,0xfau,0x90u}, {0xfbu,0xf4u,0x90u}
    };
    const lib_u16 irq_vector[] = {0x0400u,0u};
    lib_size profile, sequence;
    lib_u8 old_if;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (sequence = 0u; sequence < sizeof(sequences) / sizeof(sequences[0]); ++sequence)
    for (old_if = 0u; old_if < 2u; ++old_if) {
        cpu_bus_fixture state;
        lib_u16 saved_ip = 0u;
        cpu_bus_prepare(&state, profiles[profile]);
        state.cpu.data.eflags = 2u | (old_if ? VCPU_EFLAGS_IF : 0u);
        lib_memory_copy(state.memory + 0x100u, sequences[sequence], 3u);
        lib_memory_copy(state.memory + 0xc0u, irq_vector, sizeof(irq_vector));
        state.interrupt = LIB_TRUE;
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested || state.faults ||
            state.cpu.data.eip != 0x101u || state.acknowledgements != 0u ||
            !state.interrupt) return LIB_FALSE;
        core_machine_cpu_execution_refresh(&state.execution);
        if (sequence == 1u || sequence == 2u) {
            if (state.cpu.data.eip != 0x102u || state.acknowledgements != 0u ||
                !state.interrupt) return LIB_FALSE;
            core_machine_cpu_execution_refresh(&state.execution);
        }
        lib_memory_copy(&saved_ip, state.memory + state.cpu.data.sp, sizeof(saved_ip));
        if (state.execution.stop_requested || state.faults || state.cpu.data.flagHalt ||
            state.acknowledgements != (sequence == 2u ? 0u : 1u) ||
            state.interrupt != (sequence == 2u) ||
            state.cpu.data.eip != (sequence == 2u ? 0x103u : 0x400u) ||
            (sequence != 2u && saved_ip != (sequence == 1u ? 0x103u : 0x102u)))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_signal_rep_boundary(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    const lib_u16 irq_vector[] = {0x0400u,0u};
    const lib_u8 code[] = {0xf3u,0xa4u};
    lib_size profile;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        cpu_bus_fixture state;
        lib_u16 saved_ip = 0u;
        cpu_bus_prepare(&state, profiles[profile]);
        state.cpu.data.eflags = 2u | VCPU_EFLAGS_IF;
        state.cpu.data.cx = 3u;
        state.cpu.data.si = 0x500u;
        state.cpu.data.di = 0x600u;
        state.memory[0x500u] = 0x5au;
        lib_memory_copy(state.memory + 0x100u, code, sizeof(code));
        lib_memory_copy(state.memory + 0xc0u, irq_vector, sizeof(irq_vector));
        state.interrupt = LIB_TRUE;
        core_machine_cpu_execution_refresh(&state.execution);
        lib_memory_copy(&saved_ip, state.memory + state.cpu.data.sp, sizeof(saved_ip));
        if (state.execution.stop_requested || state.faults ||
            state.acknowledgements != 1u || state.interrupt ||
            state.cpu.data.eip != 0x400u || saved_ip != 0x100u ||
            state.cpu.data.cx != 2u || state.cpu.data.si != 0x501u ||
            state.cpu.data.di != 0x601u || state.memory[0x600u] != 0x5au ||
            state.memory[0x601u] != 0u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_signal_reset_lifetime(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_size profile;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        cpu_bus_fixture state;
        cpu_bus_prepare(&state, profiles[profile]);
        state.execution.nmi_pending = LIB_TRUE;
        state.execution.nmi_masked = LIB_TRUE;
        state.execution.nmi_in_service = LIB_TRUE;
        state.execution.interrupt_shadow = CPU_INTERRUPT_SHADOW_SEGMENT;
        state.execution.debug_segment_shadow_before = LIB_TRUE;
        state.execution.debug_trap_pending = LIB_TRUE;
        core_machine_cpu_state_reset(&state.execution);
        if (state.execution.nmi_pending || state.execution.nmi_masked ||
            state.execution.nmi_in_service || state.execution.debug_trap_pending ||
            state.execution.debug_segment_shadow_before ||
            state.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    if (!cpu_signal_sti_sequences() || !cpu_signal_rep_boundary()) return 1;
    if (!cpu_signal_reset_lifetime()) return 1;
    if (!cpu_signal_shadow_matrix()) return 1;
    if (!cpu_signal_priority_matrix()) return 1;
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

    lib_c_printf("%s\n", "M5:T539:S89:CPU-EXECUTION-SIGNAL-PREFETCH:OK");
    lib_c_printf("%s\n", "M5:T484:S3:XT-8088-QUEUE:OK");
    return 0;
}
