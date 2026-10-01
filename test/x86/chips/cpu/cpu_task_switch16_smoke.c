#include "support/cpu_task_switch16_fixture.h"
#include <stdio.h>

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
    fprintf(stderr, "task16 success profile=%u case=%u halt=%u tr=%04x ax=%04x marker=%04x fault=%u delivered=%u busy=%02x/%02x\n",
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
    snapshot = fixture.fault.valid ? &fixture.fault : &fixture.delivered_exception;
    if (snapshot->valid && (snapshot->exception_mask & mask) != 0u &&
        snapshot->exception_code == code && fixture.cpu.data.tr.selector == 0x28u)
        return LIB_TRUE;
    fprintf(stderr, "task16 fault profile=%u case=%u fault=%u/%x/%04x delivered=%u/%x/%04x expected=%x/%04x tr=%04x\n",
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
        after.data.eax == (profile == CORE_MACHINE_CPU_PROFILE_80386 ?
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
    fprintf(stderr, "task16 gate profile=%u case=%u fault=%u/%x delivered=%u/%x expected=%x tr=%04x ax=%04x\n",
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
    const lib_u32 expected = profile == CORE_MACHINE_CPU_PROFILE_80386 ?
        VCPUINS_EXCEPT_DF : VCPUINS_EXCEPT_SS;

    cpu_task16_prepare(&fixture, profile, CPU_TASK16_STACK_LIMIT);
    cpu_task16_refresh(&fixture, 4u);
    snapshot = fixture.fault.valid ? &fixture.fault : &fixture.delivered_exception;
    if (snapshot->valid && (snapshot->exception_mask & expected) != 0u &&
        snapshot->exception_code == 0u && fixture.cpu.data.tr.selector == 0x30u &&
        fixture.cpu.data.sp == 0u) return LIB_TRUE;
    fprintf(stderr, "task16 stack profile=%u fault=%u/%x delivered=%u/%x expected=%x tr=%04x sp=%04x\n",
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

static lib_bool cpu_task16_expect_double_fault_gate(void)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    lib_u16 marker = 0u;

    cpu_task16_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386,
        CPU_TASK16_DOUBLE_FAULT_GATE);
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

int main(void)
{
    lib_bool failed = LIB_FALSE;

    failed |= !cpu_task16_expect_success(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_DIRECT);
    failed |= !cpu_task16_expect_success(CORE_MACHINE_CPU_PROFILE_80386,
        CPU_TASK16_DIRECT);
    failed |= !cpu_task16_expect_success(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_INDIRECT);
    failed |= !cpu_task16_expect_success(CORE_MACHINE_CPU_PROFILE_80386,
        CPU_TASK16_INDIRECT);
    failed |= !cpu_task16_expect_success(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_CALL);
    failed |= !cpu_task16_expect_success(CORE_MACHINE_CPU_PROFILE_80386,
        CPU_TASK16_CALL);
    failed |= !cpu_task16_expect_success(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_GATE);
    failed |= !cpu_task16_expect_success(CORE_MACHINE_CPU_PROFILE_80386,
        CPU_TASK16_GATE);
    failed |= !cpu_task16_expect_ldt(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= !cpu_task16_expect_ldt(CORE_MACHINE_CPU_PROFILE_80386);
    failed |= !cpu_task16_expect_nested_return(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= !cpu_task16_expect_nested_return(CORE_MACHINE_CPU_PROFILE_80386);
    failed |= !cpu_task16_expect_gate_rejection(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_GATE_PRIVILEGE, VCPUINS_EXCEPT_GP);
    failed |= !cpu_task16_expect_gate_rejection(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_GATE_NOT_PRESENT, VCPUINS_EXCEPT_NP);
    failed |= !cpu_task16_expect_gate_rejection(CORE_MACHINE_CPU_PROFILE_80386,
        CPU_TASK16_GATE_NOT_PRESENT, VCPUINS_EXCEPT_DF);
    failed |= !cpu_task16_expect_stack_limit(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= !cpu_task16_expect_stack_limit(CORE_MACHINE_CPU_PROFILE_80386);
    failed |= !cpu_task16_expect_idt_gate(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= !cpu_task16_expect_idt_gate(CORE_MACHINE_CPU_PROFILE_80386);
    failed |= !cpu_task16_expect_double_fault_gate();
    failed |= !cpu_task16_expect_fault(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_INVALID, VCPUINS_EXCEPT_GP, 0x0040u);
    failed |= !cpu_task16_expect_fault(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_NOT_PRESENT, VCPUINS_EXCEPT_NP, 0x0030u);
    failed |= !cpu_task16_expect_fault(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_BUSY, VCPUINS_EXCEPT_GP, 0x0030u);
    failed |= !cpu_task16_expect_fault(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_SHORT, VCPUINS_EXCEPT_TS, 0x0030u);
    failed |= !cpu_task16_expect_fault(CORE_MACHINE_CPU_PROFILE_80286,
        CPU_TASK16_LDT_NOT_PRESENT, VCPUINS_EXCEPT_NP, 0x0040u);
    failed |= !cpu_task16_expect_fault(CORE_MACHINE_CPU_PROFILE_80386,
        CPU_TASK16_LOCK, VCPUINS_EXCEPT_UD, 0u);
    if (failed) {
        fputs("M5:T539:S55:TASK16:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T539:S55:TASK16:OK");
    return 0;
}
