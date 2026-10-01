#ifndef TEST_CPU_PROTECTED_FIXTURE_H
#define TEST_CPU_PROTECTED_FIXTURE_H

#include "cpu_instruction_fixture.h"

/* A protected-mode CPU fixture owns only CPU state and its flat test memory.
 * Board routing, PIC delivery and public-machine lifecycle belong elsewhere. */
#define CPU_PROTECTED_GDT_BASE 0x0300u
#define CPU_PROTECTED_CODE_BASE 0x2000u
#define CPU_PROTECTED_DATA_BASE 0x3000u
#define CPU_PROTECTED_STACK_BASE 0x4000u
#define CPU_PROTECTED_ES_BASE 0x5000u

static inline void cpu_protected_set_code(t_cpu_data_sreg *segment,
    lib_u16 selector, lib_u32 base, lib_u8 dpl, lib_bool conform)
{
    segment->selector = selector;
    segment->base = base;
    segment->limit = 0xffffu;
    segment->dpl = dpl;
    segment->flagValid = LIB_TRUE;
    segment->sregtype = SREG_CODE;
    segment->seg.executable = LIB_TRUE;
    segment->seg.exec.defsize = LIB_FALSE;
    segment->seg.exec.readable = LIB_TRUE;
    segment->seg.exec.conform = conform;
}

static inline void cpu_protected_set_data(t_cpu_data_sreg *segment,
    lib_u16 selector, lib_u32 base, lib_u8 dpl, lib_u8 type)
{
    segment->selector = selector;
    segment->base = base;
    segment->limit = 0xffffu;
    segment->dpl = dpl;
    segment->flagValid = LIB_TRUE;
    segment->sregtype = type;
    segment->seg.executable = LIB_FALSE;
    segment->seg.data.writable = LIB_TRUE;
    segment->seg.data.expdown = LIB_FALSE;
    segment->seg.data.big = LIB_FALSE;
}

static inline void cpu_protected_prepare_far(cpu_instruction_fixture *fixture,
    core_machine_cpu_profile profile)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x9au,0,0,
        0xffu,0xffu,0,0x50u,0,0x9eu,0,0,
        0xffu,0xffu,0,0x60u,0,0xfau,0,0,
        0xffu,0xffu,0,0x70u,0,0x1au,0,0,
        0xffu,0xffu,0,0x80u,0,0x92u,0,0
    };

    cpu_instruction_prepare(fixture, profile);
    lib_memory_copy(fixture->memory + CPU_PROTECTED_GDT_BASE, gdt, sizeof(gdt));
    fixture->cpu.data.cr0 |= VCPU_CR0_PE;
    fixture->cpu.data.gdtr.flagValid = LIB_TRUE;
    fixture->cpu.data.gdtr.sregtype = SREG_GDTR;
    fixture->cpu.data.gdtr.base = CPU_PROTECTED_GDT_BASE;
    fixture->cpu.data.gdtr.limit = (lib_u16)(sizeof(gdt) - 1u);
    cpu_protected_set_code(&fixture->cpu.data.cs, 0x0008u,
        CPU_PROTECTED_CODE_BASE, 0u, LIB_FALSE);
    cpu_protected_set_data(&fixture->cpu.data.ds, 0x0010u,
        CPU_PROTECTED_DATA_BASE, 0u, SREG_DATA);
    fixture->cpu.data.es = fixture->cpu.data.ds;
    cpu_protected_set_data(&fixture->cpu.data.ss, 0x0010u,
        CPU_PROTECTED_DATA_BASE, 0u, SREG_STACK);
    fixture->cpu.data.esp = 0x8000u;
}

static inline void cpu_protected_prepare_data(cpu_instruction_fixture *fixture,
    core_machine_cpu_profile profile)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };

    cpu_instruction_prepare(fixture, profile);
    lib_memory_copy(fixture->memory + CPU_PROTECTED_GDT_BASE, gdt, sizeof(gdt));
    fixture->cpu.data.cr0 |= VCPU_CR0_PE;
    fixture->cpu.data.gdtr.flagValid = LIB_TRUE;
    fixture->cpu.data.gdtr.sregtype = SREG_GDTR;
    fixture->cpu.data.gdtr.base = CPU_PROTECTED_GDT_BASE;
    fixture->cpu.data.gdtr.limit = (lib_u16)(sizeof(gdt) - 1u);
    cpu_protected_set_code(&fixture->cpu.data.cs, 0x0008u,
        CPU_PROTECTED_CODE_BASE, 0u, LIB_FALSE);
    cpu_protected_set_data(&fixture->cpu.data.ds, 0x0010u,
        CPU_PROTECTED_DATA_BASE, 0u, SREG_DATA);
    cpu_protected_set_data(&fixture->cpu.data.es, 0x0010u,
        CPU_PROTECTED_ES_BASE, 0u, SREG_DATA);
    cpu_protected_set_data(&fixture->cpu.data.ss, 0x0018u,
        CPU_PROTECTED_STACK_BASE, 0u, SREG_STACK);
    fixture->cpu.data.esp = 0x8000u;
}

static inline lib_bool cpu_protected_step(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    fixture->cpu.data.eip = 0u;
    lib_memory_copy(fixture->memory + fixture->cpu.data.cs.base, code, bytes);
    core_machine_cpu_execution_refresh(&fixture->execution);
    *after = fixture->cpu;
    return !fixture->execution.stop_requested && !fixture->fault.valid;
}

static inline lib_bool cpu_protected_fault(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, lib_u32 exception, const t_cpu *before)
{
    t_cpu after;

    fixture->cpu.data.idtr.limit = 0x17u;
    if (cpu_protected_step(fixture, code, bytes, &after)) return LIB_FALSE;
    return fixture->execution.stop_requested && fixture->fault.valid &&
        (fixture->fault.exception_mask & exception) != 0u &&
        after.data.eip == before->data.eip && after.data.esp == before->data.esp &&
        after.data.eflags == before->data.eflags;
}

#endif
