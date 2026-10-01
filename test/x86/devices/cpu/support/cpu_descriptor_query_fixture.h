#ifndef TEST_CPU_DESCRIPTOR_QUERY_FIXTURE_H
#define TEST_CPU_DESCRIPTOR_QUERY_FIXTURE_H

#include "cpu_instruction_fixture.h"

#define CPU_DESCRIPTOR_QUERY_GDT_ADDRESS 0x0300u
#define CPU_DESCRIPTOR_QUERY_CODE_ADDRESS 0x2000u
#define CPU_DESCRIPTOR_QUERY_BIT_IS_SET(state, flag) (((state) & (flag)) != 0u)

static inline lib_i32 cpu_descriptor_query_execute(
    cpu_instruction_fixture *fixture, lib_u32 budget)
{
    lib_u32 step;

    for (step = 0u; step != budget; ++step) {
        if (fixture->cpu.data.flagHalt || fixture->execution.stop_requested) break;
        core_machine_cpu_execution_refresh(&fixture->execution);
    }
    return !fixture->execution.stop_requested;
}

static inline lib_i32 cpu_descriptor_query_boot_protected(
    cpu_instruction_fixture *fixture, core_machine_cpu_profile profile)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const lib_u8 gdt_pointer[] = {0x1fu,0,0,0x03u,0,0};
    static const lib_u8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const lib_u8 halt[] = {0xf4u};

    cpu_instruction_prepare(fixture, profile);
    lib_memory_copy(fixture->memory + 0x0100u, gdt_pointer,
        sizeof(gdt_pointer));
    lib_memory_copy(fixture->memory + CPU_DESCRIPTOR_QUERY_GDT_ADDRESS, gdt,
        sizeof(gdt));
    lib_memory_copy(fixture->memory, real_code, sizeof(real_code));
    lib_memory_copy(fixture->memory + CPU_DESCRIPTOR_QUERY_CODE_ADDRESS, halt,
        sizeof(halt));
    return cpu_descriptor_query_execute(fixture, 96u) &&
        fixture->cpu.data.flagHalt;
}

static inline void cpu_descriptor_query_enter_protected(
    cpu_instruction_fixture *fixture, lib_u8 cpl)
{
    t_cpu *cpu = &fixture->cpu;

    cpu->data.cr0 |= VCPU_CR0_PE;
    cpu->data.cs.selector = (lib_u16)(0x0008u | cpl);
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.ds.base = 0u;
    cpu->data.ds.limit = 0xffffu;
    cpu->data.ds.selector = (lib_u16)(0x0010u | cpl);
    cpu->data.ds.flagValid = LIB_TRUE;
    cpu->data.ds.sregtype = SREG_DATA;
    cpu->data.ds.seg.data.writable = LIB_TRUE;
    cpu->data.ds.dpl = cpl;
}

static inline void cpu_descriptor_query_install_gdt(
    cpu_instruction_fixture *fixture)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0,0,0x9au,0,0,
        /* Query source selector 10h is the already-loaded data segment from
         * the protected execution precondition, so its accessed bit is set. */
        0xffu,0xffu,0,0,0,0x93u,0,0,
        0xffu,0xffu,0,0,0,0x12u,0,0,
        0xffu,0xffu,0,0,0,0x92u,0xc0u,0,
        0xffu,0xffu,0,0,0,0xf2u,0,0,
        0xffu,0xffu,0,0,0,0xfau,0,0
    };

    fixture->cpu.data.gdtr.base = CPU_DESCRIPTOR_QUERY_GDT_ADDRESS;
    fixture->cpu.data.gdtr.limit = (lib_u16)(sizeof(gdt) - 1u);
    lib_memory_copy(fixture->memory + CPU_DESCRIPTOR_QUERY_GDT_ADDRESS,
        gdt, sizeof(gdt));
}

static inline lib_i32 cpu_descriptor_query_run(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, t_cpu *out_cpu)
{
    lib_memory_copy(fixture->memory, code, bytes);
    core_machine_cpu_execution_refresh(&fixture->execution);
    *out_cpu = fixture->cpu;
    return !fixture->execution.stop_requested && !fixture->fault.valid;
}

static inline lib_i32 cpu_descriptor_query_fault(
    cpu_instruction_fixture *fixture, const lib_u8 *code, lib_u8 bytes,
    lib_u32 exception)
{
    t_cpu after;

    /* The CPU-local fixture deliberately has no real IDT.  A short IDTR
     * makes rejected opcodes terminal at the CPU boundary, as in the other
     * instruction receivers, instead of asking the absent board to deliver
     * their exception. */
    fixture->cpu.data.idtr.limit = 0x0017u;
    (void)cpu_instruction_run(fixture, code, bytes, &after);
    return fixture->execution.stop_requested && fixture->fault.valid &&
        (fixture->fault.exception_mask & exception) != 0u;
}

#endif
