#include "lib/types/types_interface.h"
#ifndef TEST_CPU_OUTER_RETURN_FIXTURE_H
#define TEST_CPU_OUTER_RETURN_FIXTURE_H

#include "cpu_instruction_fixture.h"

#define CPU_OUTER_GDT_BASE 0x0300u
#define CPU_OUTER_IDT_BASE 0x0400u
#define CPU_OUTER_KERNEL_BASE 0x2000u
#define CPU_OUTER_KERNEL_STACK_BASE 0x3000u
#define CPU_OUTER_USER_CODE_BASE 0x4000u
#define CPU_OUTER_USER_STACK_BASE 0x5000u
#define CPU_OUTER_TSS_BASE 0x0600u

static inline void cpu_outer_set_gate(lib_u8 *idt, lib_u8 vector, lib_u16 offset)
{
    const lib_u16 index = (lib_u16)vector * 8u;

    idt[index] = (lib_u8)offset;
    idt[index + 1u] = (lib_u8)(offset >> 8u);
    idt[index + 2u] = 0x08u;
    idt[index + 5u] = 0x86u;
}

/* CPU-only protected layout for outer RETF/IRET and their fault handlers. */
static inline void cpu_outer_return_configure(cpu_instruction_fixture *fixture)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0xfau,0,0,
        0xffu,0xffu,0,0x50u,0,0xf2u,0,0,
        0xffu,0xffu,0,0x40u,0,0x7au,0,0,
        0xffu,0xffu,0,0x50u,0,0x72u,0,0,
        0xffu,0xffu,0,0x60u,0,0xf2u,0,0,
        0xffu,0xffu,0,0x70u,0,0xfeu,0,0,
        0x67u,0x00u,0x00u,0x06u,0x00u,0x8bu,0x00u,0x00u
    };
    lib_u8 idt[0x70u] = {0};
    lib_u8 tss[10] = {0};
    lib_u8 handlers[0x21u] = {0};
    const lib_u32 esp0 = 0x00009000u;
    const lib_u16 ss0 = 0x0010u;
    t_cpu *const cpu = &fixture->cpu;

    cpu_outer_set_gate(idt, 11u, 0x0110u);
    cpu_outer_set_gate(idt, 12u, 0x0120u);
    cpu_outer_set_gate(idt, 13u, 0x0100u);
    handlers[0u] = handlers[0x10u] = handlers[0x20u] = 0xf4u;
    lib_memory_copy(tss + 4u, &esp0, sizeof(esp0));
    lib_memory_copy(tss + 8u, &ss0, sizeof(ss0));
    lib_memory_copy(fixture->memory + CPU_OUTER_GDT_BASE, gdt, sizeof(gdt));
    lib_memory_copy(fixture->memory + CPU_OUTER_IDT_BASE, idt, sizeof(idt));
    lib_memory_copy(fixture->memory + CPU_OUTER_TSS_BASE, tss, sizeof(tss));
    lib_memory_copy(fixture->memory + CPU_OUTER_KERNEL_BASE + 0x100u, handlers,
        sizeof(handlers));
    cpu->data.cr0 |= VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = CPU_OUTER_GDT_BASE;
    cpu->data.gdtr.limit = (lib_u16)(sizeof(gdt) - 1u);
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = CPU_OUTER_IDT_BASE;
    cpu->data.idtr.limit = (lib_u16)(sizeof(idt) - 1u);
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x0008u;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = CPU_OUTER_KERNEL_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 0u;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x0010u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = CPU_OUTER_KERNEL_STACK_BASE;
    cpu->data.ss.limit = 0xffffu;
    cpu->data.ss.dpl = 0u;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.tr.flagValid = LIB_TRUE;
    cpu->data.tr.selector = 0x0048u;
    cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.base = CPU_OUTER_TSS_BASE;
    cpu->data.tr.limit = 0x0067u;
    cpu->data.tr.dpl = 0u;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    cpu->data.esp = 0x8000u;
    cpu->data.eflags = 0x00000002u;
}

static inline void cpu_outer_return_prepare(cpu_instruction_fixture *fixture,
    core_machine_cpu_profile profile)
{
    cpu_instruction_prepare(fixture, profile);
    cpu_outer_return_configure(fixture);
}

static inline lib_bool cpu_outer_return_step(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    fixture->cpu.data.eip = 0u;
    lib_memory_copy(fixture->memory + fixture->cpu.data.cs.base, code, bytes);
    core_machine_cpu_execution_refresh(&fixture->execution);
    *after = fixture->cpu;
    return !fixture->execution.stop_requested && !fixture->fault.valid;
}

#endif
