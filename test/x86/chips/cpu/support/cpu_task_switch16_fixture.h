#ifndef TEST_CPU_TASK_SWITCH16_FIXTURE_H
#define TEST_CPU_TASK_SWITCH16_FIXTURE_H

#include "cpu_instruction_fixture.h"

#define CPU_TASK16_GDT_BASE 0x0300u
#define CPU_TASK16_IDT_BASE 0x0400u
#define CPU_TASK16_A_BASE 0x0600u
#define CPU_TASK16_B_BASE 0x0700u
#define CPU_TASK16_CODE_BASE 0x2000u
#define CPU_TASK16_DATA_BASE 0x3000u

typedef enum cpu_task16_case {
    CPU_TASK16_DIRECT = 0,
    CPU_TASK16_CALL,
    CPU_TASK16_GATE,
    CPU_TASK16_INVALID,
    CPU_TASK16_NOT_PRESENT,
    CPU_TASK16_BUSY,
    CPU_TASK16_SHORT,
    CPU_TASK16_LOCK,
    CPU_TASK16_LDT,
    CPU_TASK16_LDT_NOT_PRESENT,
    CPU_TASK16_NESTED_RETURN,
    CPU_TASK16_GATE_PRIVILEGE,
    CPU_TASK16_GATE_NOT_PRESENT,
    CPU_TASK16_STACK_LIMIT,
    CPU_TASK16_IDT_GATE,
    CPU_TASK16_DOUBLE_FAULT_GATE,
    CPU_TASK16_INDIRECT,
    CPU_TASK16_OPERAND32,
    CPU_TASK16_INDIRECT_OPERAND32,
    CPU_TASK16_INDIRECT_ADDRESS32,
    CPU_TASK16_INDIRECT_OPERAND_ADDRESS32,
    CPU_TASK16_RING3,
    CPU_TASK16_READABLE_CODE_DATA
} cpu_task16_case;

static inline void cpu_task16_set_gate(lib_u8 *idt, lib_u8 vector,
    lib_u16 offset)
{
    const lib_u16 index = (lib_u16)vector * 8u;

    idt[index] = (lib_u8)offset;
    idt[index + 1u] = (lib_u8)(offset >> 8u);
    idt[index + 2u] = 0x08u;
    idt[index + 5u] = 0x86u;
}

static inline void cpu_task16_configure(cpu_instruction_fixture *fixture,
    cpu_task16_case test_case)
{
    static const lib_u8 gdt_base[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0,0,0,0x30u,0,0x92u,0,0,
        0,0,0,0,0,0,0,0,
        0x2bu,0,0,0x06u,0,0x83u,0,0,
        0x2bu,0,0,0x07u,0,0x81u,0,0,
        0,0,0x30u,0,0,0x85u,0,0,
        0x17u,0,0,0x09u,0,0x82u,0,0,
        0x2bu,0,0,0x08u,0,0x81u,0,0,
        0xffu,0xffu,0,0x20u,0,0xfau,0,0,
        0xffu,0xffu,0,0x30u,0,0xf2u,0,0
    };
    static const lib_u8 target_state[] = {
        0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,
        0x00u,0x01u, 0x02u,0x00u, 0x22u,0x22u, 0,0, 0,0, 0,0,
        0x00u,0x80u, 0,0, 0,0, 0,0,
        0x10u,0, 0x08u,0, 0x10u,0, 0x10u,0, 0,0
    };
    static const lib_u8 target_code_base[] = {
        0xb8u,0x22u,0x22u,0xa3u,0u,0u,0xf4u
    };
    lib_u8 gdt[sizeof(gdt_base)];
    lib_u8 state[sizeof(target_state)];
    lib_u8 idt[0x70u] = {0};
    lib_u8 target_code[sizeof(target_code_base)];
    lib_u8 source[12u] = {0xb8u,0x11u,0x11u,0xeau,0,0,0x30u,0,0xf4u};
    lib_size source_bytes = 9u;
    t_cpu *const cpu = &fixture->cpu;

    lib_memory_copy(gdt, gdt_base, sizeof(gdt));
    lib_memory_copy(state, target_state, sizeof(state));
    lib_memory_copy(target_code, target_code_base, sizeof(target_code));
    if (test_case == CPU_TASK16_CALL) source[3] = 0x9au;
    if (test_case == CPU_TASK16_GATE) {
        source[3] = 0x9au;
        source[6] = 0x38u;
    }
    if (test_case == CPU_TASK16_GATE_PRIVILEGE) {
        source[3] = 0x9au;
        source[6] = 0x3bu;
    }
    if (test_case == CPU_TASK16_GATE_NOT_PRESENT) {
        source[3] = 0x9au;
        source[6] = 0x38u;
        gdt[7u * 8u + 5u] = 0x05u;
    }
    if (test_case == CPU_TASK16_STACK_LIMIT) {
        state[26u] = 0u;
        state[27u] = 0u;
        state[34u] = 0x18u;
        state[38u] = 0x18u;
        state[40u] = 0x18u;
        target_code[0] = 0x58u;
        target_code[1] = 0xf4u;
    }
    if (test_case == CPU_TASK16_IDT_GATE) source[3] = 0xccu;
    if (test_case == CPU_TASK16_DOUBLE_FAULT_GATE) {
        source[6] = 0x40u;
        lib_memory_set(gdt + 8u * 8u, 0, 8u);
    }
    if (test_case == CPU_TASK16_NESTED_RETURN) {
        source[3] = 0x9au;
        target_code[0] = 0xcfu;
        target_code[1] = 0xf4u;
    }
    if (test_case == CPU_TASK16_INDIRECT) {
        source[3] = 0xffu;
        source[4] = 0x2eu;
        source[5] = 0u;
        source[6] = 0x22u;
        source[7] = 0xf4u;
        fixture->memory[0x5200u] = 0u;
        fixture->memory[0x5201u] = 0u;
        fixture->memory[0x5202u] = 0x30u;
        fixture->memory[0x5203u] = 0u;
    }
    if (test_case == CPU_TASK16_OPERAND32) {
        const lib_u8 operand32[] = {
            0xb8u,0x11u,0x11u,0x66u,0xeau,0,0,0,0,0x30u,0u
        };
        lib_memory_copy(source, operand32, sizeof(operand32));
        source_bytes = sizeof(operand32);
    }
    if (test_case == CPU_TASK16_INDIRECT_OPERAND32) {
        const lib_u8 operand32[] = {
            0xb8u,0x11u,0x11u,0x66u,0xffu,0x2eu,0,0x22u
        };
        lib_memory_copy(source, operand32, sizeof(operand32));
        source_bytes = sizeof(operand32);
        fixture->memory[0x5200u] = 0u;
        fixture->memory[0x5201u] = 0u;
        fixture->memory[0x5202u] = 0u;
        fixture->memory[0x5203u] = 0u;
        fixture->memory[0x5204u] = 0x30u;
        fixture->memory[0x5205u] = 0u;
    }
    if (test_case == CPU_TASK16_INDIRECT_ADDRESS32 ||
        test_case == CPU_TASK16_INDIRECT_OPERAND_ADDRESS32) {
        const lib_u8 address32[] = {
            0xb8u,0x11u,0x11u,0x67u,0xffu,0x2du,0,0x22u,0,0
        };
        const lib_u8 operand_address32[] = {
            0xb8u,0x11u,0x11u,0x66u,0x67u,0xffu,0x2du,0,0x22u,0,0
        };
        const lib_u8 *const encoded = test_case == CPU_TASK16_INDIRECT_ADDRESS32 ?
            address32 : operand_address32;

        source_bytes = test_case == CPU_TASK16_INDIRECT_ADDRESS32 ?
            sizeof(address32) : sizeof(operand_address32);
        lib_memory_copy(source, encoded, source_bytes);
        fixture->memory[0x5200u] = 0u;
        fixture->memory[0x5201u] = 0u;
        fixture->memory[0x5202u] = test_case ==
            CPU_TASK16_INDIRECT_OPERAND_ADDRESS32 ? 0u : 0x30u;
        fixture->memory[0x5203u] = 0u;
        if (test_case == CPU_TASK16_INDIRECT_OPERAND_ADDRESS32) {
            fixture->memory[0x5204u] = 0x30u;
            fixture->memory[0x5205u] = 0u;
        }
    }
    if (test_case == CPU_TASK16_INVALID) source[6] = 0x40u;
    if (test_case == CPU_TASK16_NOT_PRESENT) gdt[6u * 8u + 5u] = 0x01u;
    if (test_case == CPU_TASK16_BUSY) gdt[6u * 8u + 5u] = 0x83u;
    if (test_case == CPU_TASK16_SHORT) gdt[6u * 8u] = 0x10u;
    if (test_case == CPU_TASK16_LDT || test_case == CPU_TASK16_LDT_NOT_PRESENT) {
        state[34u] = 0x14u;
        state[36u] = 0x0cu;
        state[38u] = 0x14u;
        state[40u] = 0x14u;
        state[42u] = 0x40u;
        if (test_case == CPU_TASK16_LDT_NOT_PRESENT) gdt[8u * 8u + 5u] = 0x02u;
    }
    if (test_case == CPU_TASK16_RING3) {
        state[34u] = 0x5bu;
        state[36u] = 0x53u;
        state[38u] = 0x5bu;
        state[40u] = 0x5bu;
        target_code[0] = 0xebu;
        target_code[1] = 0xfeu;
    }
    if (test_case == CPU_TASK16_READABLE_CODE_DATA) {
        state[34u] = 0x08u;
        state[40u] = 0x08u;
        target_code[0] = 0xf4u;
    }
    if (test_case == CPU_TASK16_LOCK) {
        source[3] = 0xf0u;
        source[4] = 0xeau;
        source[5] = 0u;
        source[6] = 0u;
        source[7] = 0x30u;
    }
    cpu_task16_set_gate(idt, 8u, 0x0180u);
    cpu_task16_set_gate(idt, 10u, 0x0180u);
    cpu_task16_set_gate(idt, 11u, 0x0180u);
    cpu_task16_set_gate(idt, 12u, 0x0180u);
    cpu_task16_set_gate(idt, 13u, 0x0180u);
    if (test_case == CPU_TASK16_IDT_GATE) {
        const lib_u8 task_gate[] = {0,0,0x30u,0,0,0x85u,0,0};
        lib_memory_copy(idt + 3u * 8u, task_gate, sizeof(task_gate));
    }
    if (test_case == CPU_TASK16_DOUBLE_FAULT_GATE) {
        const lib_u8 fault_gate[] = {0,0,0x40u,0,0,0x85u,0,0};
        const lib_u8 double_fault_gate[] = {0,0,0x30u,0,0,0x85u,0,0};
        lib_memory_copy(idt + 8u * 8u, double_fault_gate,
            sizeof(double_fault_gate));
        lib_memory_copy(idt + 13u * 8u, fault_gate, sizeof(fault_gate));
    }
    if (test_case == CPU_TASK16_LDT_NOT_PRESENT) {
        const lib_u8 fault_gate[] = {0,0,0x48u,0,0,0x85u,0,0};
        lib_u8 fault_state[sizeof(state)];

        lib_memory_copy(fault_state, target_state, sizeof(fault_state));
        fault_state[14u] = 0x80u;
        fault_state[15u] = 0x01u;
        lib_memory_copy(idt + 11u * 8u, fault_gate, sizeof(fault_gate));
        lib_memory_copy(fixture->memory + 0x0800u, fault_state,
            sizeof(fault_state));
    }
    if (test_case == CPU_TASK16_GATE_NOT_PRESENT)
        lib_memory_set(idt + 11u * 8u, 0, 8u);
    if (test_case == CPU_TASK16_STACK_LIMIT)
        lib_memory_set(idt + 12u * 8u, 0, 8u);
    fixture->memory[CPU_TASK16_CODE_BASE + 0x180u] = 0xf4u;
    lib_memory_copy(fixture->memory + CPU_TASK16_GDT_BASE, gdt, sizeof(gdt));
    lib_memory_copy(fixture->memory + CPU_TASK16_IDT_BASE, idt, sizeof(idt));
    lib_memory_copy(fixture->memory + CPU_TASK16_B_BASE, state, sizeof(state));
    if (test_case == CPU_TASK16_LDT || test_case == CPU_TASK16_LDT_NOT_PRESENT) {
        static const lib_u8 ldt[] = {
            0,0,0,0,0,0,0,0,
            0xffu,0xffu,0,0x20u,0,0x9au,0,0,
            0xffu,0xffu,0,0x30u,0,0x92u,0,0
        };
        lib_memory_copy(fixture->memory + 0x0900u, ldt, sizeof(ldt));
    }
    lib_memory_copy(fixture->memory + CPU_TASK16_CODE_BASE + 0x100u,
        target_code, sizeof(target_code));
    lib_memory_copy(fixture->memory + CPU_TASK16_CODE_BASE, source,
        source_bytes);
    cpu->data.cr0 |= VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = CPU_TASK16_GDT_BASE;
    cpu->data.gdtr.limit = (lib_u16)(sizeof(gdt) - 1u);
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = CPU_TASK16_IDT_BASE;
    cpu->data.idtr.limit = (lib_u16)(sizeof(idt) - 1u);
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x08u;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = CPU_TASK16_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x10u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = CPU_TASK16_DATA_BASE;
    cpu->data.ss.limit = 0xffffu;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.ds = cpu->data.ss;
    cpu->data.es = cpu->data.ss;
    cpu->data.tr.flagValid = LIB_TRUE;
    cpu->data.tr.selector = 0x28u;
    cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.base = CPU_TASK16_A_BASE;
    cpu->data.tr.limit = 0x2bu;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    cpu->data.sp = 0x8000u;
    cpu->data.eflags = 0x02u;
}

static inline void cpu_task16_prepare(cpu_instruction_fixture *fixture,
    core_machine_cpu_profile profile, cpu_task16_case test_case)
{
    cpu_instruction_prepare(fixture, profile);
    cpu_task16_configure(fixture, test_case);
}

static inline void cpu_task16_refresh(cpu_instruction_fixture *fixture,
    lib_u8 count)
{
    lib_u8 index;

    for (index = 0u; index < count; ++index)
        core_machine_cpu_execution_refresh(&fixture->execution);
}

#endif
