#include "support/cpu_task_switch16_fixture.h"
#include "lib/types/file.h"

static lib_bool cpu_task16_expect_success(core_machine_cpu_profile profile,
    cpu_task16_case test_case)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    lib_u16 marker = 0u;

    cpu_task16_prepare(&fixture, profile, test_case);
    cpu_task16_refresh(&fixture, 8u);
    after = fixture.cpu;
    lib_memory_copy(&marker, fixture.memory + CPU_TASK16_DATA_BASE,
        sizeof(marker));
    if (!fixture.fault.valid && !fixture.delivered_exception.valid &&
        after.data.flagHalt && after.data.tr.selector == 0x30u &&
        after.data.ax == 0x2222u && marker == 0x2222u &&
        (fixture.memory[CPU_TASK16_GDT_BASE + 0x2du] ==
            ((test_case == CPU_TASK16_CALL || test_case == CPU_TASK16_GATE) ?
                0x83u : 0x81u)) &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x35u] == 0x83u &&
        ((test_case != CPU_TASK16_CALL && test_case != CPU_TASK16_GATE) ||
            (fixture.memory[CPU_TASK16_B_BASE] == 0x28u &&
                fixture.memory[CPU_TASK16_B_BASE + 1u] == 0u &&
                (after.data.eflags & VCPU_EFLAGS_NT) != 0u))) return LIB_TRUE;
    lib_c_fprintf(lib_c_stderr, "task16 success profile=%u case=%u halt=%u tr=%04x ax=%04x marker=%04x fault=%u delivered=%u busy=%02x/%02x\n",
        (unsigned)profile, (unsigned)test_case, (unsigned)after.data.flagHalt,
        after.data.tr.selector, after.data.ax, marker, (unsigned)fixture.fault.valid,
        (unsigned)fixture.delivered_exception.valid,
        fixture.memory[CPU_TASK16_GDT_BASE + 0x2du],
        fixture.memory[CPU_TASK16_GDT_BASE + 0x35u]);
    return LIB_FALSE;
}

static lib_bool cpu_task16_expect_fault(core_machine_cpu_profile profile,
    cpu_task16_case test_case, lib_u32 mask, lib_u16 code)
{
    cpu_instruction_fixture fixture;
    const core_machine_cpu_fault_snapshot *snapshot;

    cpu_task16_prepare(&fixture, profile, test_case);
    cpu_task16_refresh(&fixture, 4u);
    if (test_case == CPU_TASK16_LDT_NOT_PRESENT) {
        if (!fixture.fault.valid && fixture.delivered_exception.valid &&
            (fixture.delivered_exception.exception_mask & VCPUINS_EXCEPT_NP) != 0u &&
            fixture.cpu.data.tr.selector == 0x48u) return LIB_TRUE;
        lib_c_fprintf(lib_c_stderr, "task16 late LDT fault=%u/%x delivered=%u/%x tr=%04x ip=%04x\n",
            (unsigned)fixture.fault.valid, (unsigned)fixture.fault.exception_mask,
            (unsigned)fixture.delivered_exception.valid,
            (unsigned)fixture.delivered_exception.exception_mask,
            fixture.cpu.data.tr.selector, fixture.cpu.data.ip);
        return LIB_FALSE;
    }
    snapshot = fixture.fault.valid ? &fixture.fault : &fixture.delivered_exception;
    if (snapshot->valid && (snapshot->exception_mask & mask) != 0u &&
        snapshot->exception_code == code && fixture.cpu.data.tr.selector == 0x28u)
        return LIB_TRUE;
    lib_c_fprintf(lib_c_stderr, "task16 fault profile=%u case=%u fault=%u/%x/%04x delivered=%u/%x/%04x expected=%x/%04x tr=%04x\n",
        (unsigned)profile, (unsigned)test_case, (unsigned)fixture.fault.valid,
        (unsigned)fixture.fault.exception_mask, fixture.fault.exception_code,
        (unsigned)fixture.delivered_exception.valid,
        (unsigned)fixture.delivered_exception.exception_mask,
        fixture.delivered_exception.exception_code, (unsigned)mask, code,
        fixture.cpu.data.tr.selector);
    return LIB_FALSE;
}

static lib_bool cpu_task16_expect_ldt(core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_LDT);
    cpu_task16_refresh(&fixture, 8u);
    after = fixture.cpu;
    return !fixture.fault.valid && after.data.flagHalt &&
        after.data.ldtr.flagValid && after.data.ldtr.selector == 0x40u &&
        after.data.ldtr.base == 0x0900u && after.data.ldtr.limit == 0x17u &&
        after.data.cs.selector == 0x0cu && after.data.ss.selector == 0x14u &&
        after.data.ds.selector == 0x14u && after.data.es.selector == 0x14u;
}

static lib_bool cpu_task16_expect_ring3(core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_RING3);
    cpu_task16_refresh(&fixture, 8u);
    after = fixture.cpu;
    return !fixture.fault.valid && !fixture.delivered_exception.valid &&
        !after.data.flagHalt && after.data.tr.selector == 0x30u &&
        after.data.cs.selector == 0x53u && after.data.cs.dpl == 3u &&
        after.data.ss.selector == 0x5bu && after.data.ds.selector == 0x5bu &&
        after.data.es.selector == 0x5bu && after.data.ax == 0x2222u;
}

static lib_bool cpu_task16_expect_ring3_source_direct(
    core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_RING3_SOURCE_DIRECT);
    cpu_task16_refresh(&fixture, 10u);
    after = fixture.cpu;
    return !fixture.fault.valid && !fixture.delivered_exception.valid &&
        !after.data.flagHalt && after.data.tr.selector == 0x4bu &&
        after.data.cs.selector == 0x53u && after.data.cs.dpl == 3u &&
        after.data.ss.selector == 0x5bu && after.data.ds.selector == 0x5bu &&
        after.data.es.selector == 0x5bu && after.data.eip == 0x1a0u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x35u] == 0x81u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x4du] == 0xe3u;
}

static lib_bool cpu_task16_expect_ring3_source_call(
    core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    lib_u16 backlink = 0u;

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_RING3_SOURCE_CALL);
    cpu_task16_refresh(&fixture, 10u);
    after = fixture.cpu;
    lib_memory_copy(&backlink, fixture.memory + 0x0800u, sizeof(backlink));
    return !fixture.fault.valid && !fixture.delivered_exception.valid &&
        !after.data.flagHalt && after.data.tr.selector == 0x4bu &&
        after.data.cs.selector == 0x53u && after.data.cs.dpl == 3u &&
        after.data.ss.selector == 0x5bu && after.data.ds.selector == 0x5bu &&
        after.data.es.selector == 0x5bu && after.data.eip == 0x1a0u &&
        (after.data.eflags & VCPU_EFLAGS_NT) != 0u && backlink == 0x30u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x35u] == 0x83u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x4du] == 0xe3u;
}

static lib_bool cpu_task16_expect_ring3_source_gate_call(
    core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    lib_u16 backlink = 0u;

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_RING3_SOURCE_GATE_CALL);
    cpu_task16_refresh(&fixture, 10u);
    after = fixture.cpu;
    lib_memory_copy(&backlink, fixture.memory + 0x0800u, sizeof(backlink));
    return !fixture.fault.valid && !fixture.delivered_exception.valid &&
        !after.data.flagHalt && after.data.tr.selector == 0x48u &&
        after.data.cs.selector == 0x53u && after.data.cs.dpl == 3u &&
        after.data.ss.selector == 0x5bu && after.data.ds.selector == 0x5bu &&
        after.data.es.selector == 0x5bu && after.data.eip == 0x1a0u &&
        (after.data.eflags & VCPU_EFLAGS_NT) != 0u && backlink == 0x30u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x35u] == 0x83u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x4du] == 0xe3u;
}

static lib_bool cpu_task16_expect_ring3_source_gate_jump(
    core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_RING3_SOURCE_GATE_JUMP);
    cpu_task16_refresh(&fixture, 10u);
    after = fixture.cpu;
    return !fixture.fault.valid && !fixture.delivered_exception.valid &&
        !after.data.flagHalt && after.data.tr.selector == 0x48u &&
        after.data.cs.selector == 0x53u && after.data.cs.dpl == 3u &&
        after.data.ss.selector == 0x5bu && after.data.ds.selector == 0x5bu &&
        after.data.es.selector == 0x5bu && after.data.eip == 0x1a0u &&
        (after.data.eflags & VCPU_EFLAGS_NT) == 0u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x35u] == 0x81u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x4du] == 0xe3u;
}

static lib_bool cpu_task16_expect_ring3_source_gate_privilege(
    core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    lib_u8 step;

    cpu_task16_prepare(&fixture, profile,
        CPU_TASK16_RING3_SOURCE_GATE_PRIVILEGE);
    for (step = 0u; step < 10u && !fixture.delivered_exception.valid; ++step)
        cpu_task16_refresh(&fixture, 1u);
    after = fixture.cpu;
    return !fixture.fault.valid && fixture.delivered_exception.valid &&
        (fixture.delivered_exception.exception_mask & VCPUINS_EXCEPT_GP) != 0u &&
        after.data.tr.selector == 0x30u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x35u] == 0x83u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x4du] == 0xe1u;
}

static lib_bool cpu_task16_expect_readable_code_data(
    core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_READABLE_CODE_DATA);
    cpu_task16_refresh(&fixture, 8u);
    after = fixture.cpu;
    return !fixture.fault.valid && !fixture.delivered_exception.valid &&
        after.data.flagHalt && after.data.tr.selector == 0x30u &&
        after.data.es.selector == 0x08u && after.data.ds.selector == 0x08u;
}

static lib_bool cpu_task16_expect_nested_return(core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_NESTED_RETURN);
    cpu_task16_refresh(&fixture, 8u);
    after = fixture.cpu;
    return !fixture.fault.valid && !fixture.delivered_exception.valid &&
        after.data.flagHalt && after.data.eip == 9u && after.data.tr.selector == 0x28u &&
        (after.data.eflags & VCPU_EFLAGS_NT) == 0u &&
        after.data.eax == (profile >= CORE_MACHINE_CPU_PROFILE_80386 ?
            0xffff1111u : 0x1111u) &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x2du] == 0x83u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x35u] == 0x81u;
}

static lib_bool cpu_task16_expect_gate_rejection(
    core_machine_cpu_profile profile, cpu_task16_case test_case, lib_u32 mask)
{
    cpu_instruction_fixture fixture;
    const core_machine_cpu_fault_snapshot *snapshot;

    cpu_task16_prepare(&fixture, profile, test_case);
    cpu_task16_refresh(&fixture, 4u);
    snapshot = fixture.fault.valid ? &fixture.fault : &fixture.delivered_exception;
    if (snapshot->valid && (snapshot->exception_mask & mask) != 0u &&
        fixture.cpu.data.tr.selector == 0x28u && fixture.cpu.data.ax == 0x1111u)
        return LIB_TRUE;
    lib_c_fprintf(lib_c_stderr, "task16 gate profile=%u case=%u fault=%u/%x delivered=%u/%x expected=%x tr=%04x ax=%04x\n",
        (unsigned)profile, (unsigned)test_case, (unsigned)fixture.fault.valid,
        (unsigned)fixture.fault.exception_mask,
        (unsigned)fixture.delivered_exception.valid,
        (unsigned)fixture.delivered_exception.exception_mask, (unsigned)mask,
        fixture.cpu.data.tr.selector, fixture.cpu.data.ax);
    return LIB_FALSE;
}

static lib_bool cpu_task16_expect_stack_limit(core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    const core_machine_cpu_fault_snapshot *snapshot;
    /* SS entry is absent and the same invalid stack cannot enter DF. */
    const lib_u32 expected = VCPUINS_EXCEPT_SHUTDOWN;

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_STACK_LIMIT);
    cpu_task16_refresh(&fixture, 4u);
    snapshot = fixture.fault.valid ? &fixture.fault : &fixture.delivered_exception;
    if (core_machine_cpu_is_shutdown(&fixture.execution) && !fixture.fault.valid &&
        snapshot->valid && (snapshot->exception_mask & expected) != 0u &&
        snapshot->exception_code == 0u && fixture.cpu.data.tr.selector == 0x30u &&
        fixture.cpu.data.sp == 0u) return LIB_TRUE;
    lib_c_fprintf(lib_c_stderr, "task16 stack profile=%u fault=%u/%x delivered=%u/%x expected=%x tr=%04x sp=%04x\n",
        (unsigned)profile, (unsigned)fixture.fault.valid,
        (unsigned)fixture.fault.exception_mask,
        (unsigned)fixture.delivered_exception.valid,
        (unsigned)fixture.delivered_exception.exception_mask, (unsigned)expected,
        fixture.cpu.data.tr.selector, fixture.cpu.data.sp);
    return LIB_FALSE;
}

static lib_bool cpu_task16_expect_idt_gate(core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    lib_u16 marker = 0u;

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_IDT_GATE);
    cpu_task16_refresh(&fixture, 8u);
    after = fixture.cpu;
    lib_memory_copy(&marker, fixture.memory + CPU_TASK16_DATA_BASE,
        sizeof(marker));
    return !fixture.fault.valid && !fixture.delivered_exception.valid &&
        after.data.flagHalt && after.data.eip == 0x107u &&
        after.data.tr.selector == 0x30u && marker == 0x2222u &&
        (after.data.eflags & VCPU_EFLAGS_NT) != 0u &&
        fixture.memory[CPU_TASK16_B_BASE] == 0x28u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x2du] == 0x83u &&
        fixture.memory[CPU_TASK16_GDT_BASE + 0x35u] == 0x83u;
}

static lib_bool cpu_task16_expect_double_fault_gate(core_machine_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    lib_u16 marker = 0u;

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_DOUBLE_FAULT_GATE);
    cpu_task16_refresh(&fixture, 8u);
    after = fixture.cpu;
    lib_memory_copy(&marker, fixture.memory + CPU_TASK16_DATA_BASE,
        sizeof(marker));
    return !fixture.fault.valid && fixture.delivered_exception.valid &&
        fixture.delivered_exception.exception_mask == VCPUINS_EXCEPT_DF &&
        after.data.flagHalt && after.data.eip == 0x107u &&
        after.data.tr.selector == 0x30u && marker == 0x2222u &&
        (after.data.eflags & VCPU_EFLAGS_NT) != 0u &&
        fixture.memory[CPU_TASK16_B_BASE] == 0x28u;
}

static lib_bool cpu_task16_completion_flags(void)
{
    static const cpu_task16_case cases[] = {CPU_TASK16_DIRECT, CPU_TASK16_CALL,
        CPU_TASK16_GATE, CPU_TASK16_IDT_GATE};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_CPU_PROFILE_80486SX
    };
    lib_size index, profile;
    lib_u8 old_tf, new_tf, old_rf;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    for (old_tf = 0u; old_tf < 2u; ++old_tf)
    for (new_tf = 0u; new_tf < 2u; ++new_tf)
    for (old_rf = 0u; old_rf <= (profiles[profile] >= CORE_MACHINE_CPU_PROFILE_80386);
         ++old_rf)
    for (lib_u8 upper = 0u; upper <
        (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80486SX ? 16u : 1u); ++upper)
    for (lib_u8 reserved = 0u; reserved <
        (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80486SX ? 2u : 1u); ++reserved) {
        cpu_instruction_fixture fixture;
        const lib_u16 incoming = (lib_u16)((reserved ? 0x8028u : 2u) |
            (new_tf ? VCPU_EFLAGS_TF : 0u));
        const lib_u32 high_flags = ((upper & 1u) ? VCPU_EFLAGS_AC : 0u) |
            ((upper & 2u) ? VCPU_EFLAGS_ID : 0u) |
            ((upper & 4u) ? VCPU_EFLAGS_VIF : 0u) |
            ((upper & 8u) ? VCPU_EFLAGS_VIP : 0u);
        lib_u16 saved_flags;

        cpu_task16_prepare(&fixture, profiles[profile], cases[index]);
        cpu_task16_refresh(&fixture, 1u);
        fixture.cpu.data.eflags = 2u | (old_tf ? VCPU_EFLAGS_TF : 0u) |
            (old_rf ? VCPU_EFLAGS_RF : 0u) | high_flags;
        fixture.cpu.data.cr4 = profiles[profile] == CORE_MACHINE_CPU_PROFILE_80486SX ?
            VCPU_CR4_VME : 0u;
        lib_memory_copy(fixture.memory + CPU_TASK16_B_BASE + 0x10u,
            &incoming, sizeof(incoming));
        cpu_task16_refresh(&fixture, 1u);
        lib_memory_copy(&saved_flags, fixture.memory + CPU_TASK16_A_BASE + 0x10u,
            sizeof(saved_flags));
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            fixture.delivered_exception.valid || fixture.execution.debug_trap_pending ||
            fixture.cpu.data.tr.selector != 0x30u || fixture.cpu.data.eip != 0x100u ||
            fixture.cpu.data.eflags != (lib_u32)((incoming & 0x7fd5u) | 2u |
                (index ? VCPU_EFLAGS_NT : 0u)) ||
            fixture.cpu.data.cr4 != (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80486SX ?
                VCPU_CR4_VME : 0u) ||
            saved_flags != (lib_u16)(2u | (old_tf ? VCPU_EFLAGS_TF : 0u))) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool cpu_task16_debug_local_enables(core_machine_cpu_profile profile)
{
    static const struct {
        cpu_task16_case test_case;
        lib_u16 task;
        lib_bool switched;
    } cases[] = {
        {CPU_TASK16_DIRECT, 0x30u, LIB_TRUE},
        {CPU_TASK16_INDIRECT, 0x30u, LIB_TRUE},
        {CPU_TASK16_CALL, 0x30u, LIB_TRUE},
        {CPU_TASK16_GATE, 0x30u, LIB_TRUE},
        {CPU_TASK16_NESTED_RETURN, 0x28u, LIB_TRUE},
        {CPU_TASK16_IDT_GATE, 0x30u, LIB_TRUE},
        {CPU_TASK16_LDT_NOT_PRESENT, 0x48u, LIB_TRUE},
        {CPU_TASK16_INVALID, 0x28u, LIB_FALSE},
        {CPU_TASK16_NOT_PRESENT, 0x28u, LIB_FALSE},
        {CPU_TASK16_BUSY, 0x28u, LIB_FALSE},
        {CPU_TASK16_SHORT, 0x28u, LIB_FALSE}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        cpu_instruction_fixture fixture;
        const lib_u32 expected = cases[index].switched ? 0x4aau : 0x4ffu;

        cpu_task16_prepare(&fixture, profile, cases[index].test_case);
        fixture.cpu.data.dr0 = 0xfffffff0u;
        fixture.cpu.data.dr1 = 0xfffffff4u;
        fixture.cpu.data.dr2 = 0xfffffff8u;
        fixture.cpu.data.dr3 = 0xfffffffcu;
        fixture.cpu.data.dr7 = 0x4ffu;
        if (cases[index].test_case == CPU_TASK16_NESTED_RETURN) {
            cpu_task16_refresh(&fixture, 2u);
            if (fixture.cpu.data.tr.selector != 0x30u) return LIB_FALSE;
            fixture.cpu.data.dr7 = 0x4ffu;
        }
        cpu_task16_refresh(&fixture, 8u);
        if (fixture.cpu.data.tr.selector != cases[index].task ||
            fixture.cpu.data.dr7 != expected ||
            fixture.cpu.data.dr0 != 0xfffffff0u ||
            fixture.cpu.data.dr1 != 0xfffffff4u ||
            fixture.cpu.data.dr2 != 0xfffffff8u ||
            fixture.cpu.data.dr3 != 0xfffffffcu) {
            lib_c_fprintf(lib_c_stderr,
                "task16 debug profile=%u case=%u tr=%04x dr7=%08x expected=%08x\n",
                (unsigned)profile, (unsigned)cases[index].test_case,
                fixture.cpu.data.tr.selector, (unsigned)fixture.cpu.data.dr7,
                (unsigned)expected);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

int main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_CPU_PROFILE_80486SX
    };
    static const cpu_task16_case success[] = {
        CPU_TASK16_DIRECT, CPU_TASK16_INDIRECT, CPU_TASK16_CALL, CPU_TASK16_GATE
    };
    static const struct {
        cpu_task16_case test_case;
        lib_u32 exception;
        lib_u16 error;
    } faults[] = {
        {CPU_TASK16_INVALID,VCPUINS_EXCEPT_GP,0x0040u},
        {CPU_TASK16_NOT_PRESENT,VCPUINS_EXCEPT_NP,0x0030u},
        {CPU_TASK16_BUSY,VCPUINS_EXCEPT_GP,0x0030u},
        {CPU_TASK16_SHORT,VCPUINS_EXCEPT_TS,0x0030u},
        {CPU_TASK16_LDT_NOT_PRESENT,VCPUINS_EXCEPT_NP,0x0040u}
    };
    lib_bool failed = !cpu_task16_completion_flags();

    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]); ++index) {
        const core_machine_cpu_profile profile = profiles[index];

        for (lib_size form = 0u; form < sizeof(success) / sizeof(success[0]); ++form)
            failed |= !cpu_task16_expect_success(profile, success[form]);
        failed |= !cpu_task16_expect_ldt(profile);
        failed |= !cpu_task16_expect_ring3(profile);
        failed |= !cpu_task16_expect_ring3_source_direct(profile);
        failed |= !cpu_task16_expect_ring3_source_call(profile);
        failed |= !cpu_task16_expect_ring3_source_gate_jump(profile);
        failed |= !cpu_task16_expect_ring3_source_gate_privilege(profile);
        failed |= !cpu_task16_expect_ring3_source_gate_call(profile);
        failed |= !cpu_task16_expect_readable_code_data(profile);
        failed |= !cpu_task16_expect_nested_return(profile);
        failed |= !cpu_task16_expect_gate_rejection(profile,
            CPU_TASK16_GATE_PRIVILEGE, VCPUINS_EXCEPT_GP);
        failed |= !cpu_task16_expect_gate_rejection(profile,
            CPU_TASK16_GATE_NOT_PRESENT, VCPUINS_EXCEPT_DF);
        failed |= !cpu_task16_expect_stack_limit(profile);
        failed |= !cpu_task16_expect_idt_gate(profile);
        for (lib_size form = 0u; form < sizeof(faults) / sizeof(faults[0]); ++form)
            failed |= !cpu_task16_expect_fault(profile, faults[form].test_case,
                faults[form].exception, faults[form].error);
        if (profile >= CORE_MACHINE_CPU_PROFILE_80386) {
            failed |= !cpu_task16_debug_local_enables(profile);
            failed |= !cpu_task16_expect_double_fault_gate(profile);
            failed |= !cpu_task16_expect_fault(profile,
                CPU_TASK16_LOCK, VCPUINS_EXCEPT_GP, 0x0033u);
        }
    }
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "%s", "TASK16:FAIL\n");
        return 1;
    }
    lib_c_printf("%s\n", "TASK16:OK");
    return 0;
}
