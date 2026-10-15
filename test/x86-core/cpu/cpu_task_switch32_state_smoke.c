#include "support/cpu_instruction_fixture.h"
#include "x86-core/cpu/cpu_timing.h"
#include "lib/types/file.h"

#define TASK32_GDT_BASE 0x0300u
#define TASK32_IDT_BASE 0x0400u
#define TASK32_A_BASE 0x0600u
#define TASK32_B_BASE 0x0700u
#define TASK32_C_BASE 0x0800u
#define TASK32_CODE_BASE 0x2000u
#define TASK32_DATA_BASE 0x3000u

typedef struct task32_selector {
    lib_u16 selector;
    lib_u16 reserved;
} task32_selector;

typedef struct task32_state {
    lib_u32 cr3;
    lib_u32 eip;
    lib_u32 eflags;
    lib_u32 eax;
    lib_u32 ecx;
    lib_u32 edx;
    lib_u32 ebx;
    lib_u32 esp;
    lib_u32 ebp;
    lib_u32 esi;
    lib_u32 edi;
    task32_selector es;
    task32_selector cs;
    task32_selector ss;
    task32_selector ds;
    task32_selector fs;
    task32_selector gs;
    task32_selector ldtr;
} task32_state;

_Static_assert(sizeof(task32_state) == 0x48u,
    "TSS32 state image must retain the Intel saved-state span");

typedef enum task32_case {
    TASK32_DIRECT = 0,
    TASK32_OPERAND32,
    TASK32_INDIRECT16,
    TASK32_INDIRECT32,
    TASK32_INDIRECT_ADDRESS32,
    TASK32_INDIRECT_OPERAND_ADDRESS32,
    TASK32_INVALID_CODE,
    TASK32_TARGET_BUSY,
    TASK32_OLD_SHORT,
    TASK32_TARGET_SHORT,
    TASK32_STACK_LIMIT,
    TASK32_LDT_SUCCESS,
    TASK32_LDT_BAD_DESCRIPTOR,
    TASK32_LDT_NOT_PRESENT,
    TASK32_LDT_SHORT,
    TASK32_LDT_BAD_CODE,
    TASK32_LDT_BAD_DATA,
    TASK32_LOCK_DIRECT,
    TASK32_LOCK_INDIRECT,
    TASK32_DEBUG_TRAP,
    TASK32_NESTED_CALL,
    TASK32_NESTED_CALL_OPERAND32,
    TASK32_NESTED_CALL_INDIRECT,
    TASK32_NESTED_GATE_CALL,
    TASK32_GATE_JMP,
    TASK32_GATE_JMP_OPERAND32,
    TASK32_NESTED_RETURN,
    TASK32_NESTED_INVALID_CODE,
    TASK32_NESTED_TARGET_BUSY,
    TASK32_NESTED_TARGET_SHORT,
    TASK32_NESTED_STACK_LIMIT,
    TASK32_RING3_DIRECT,
    TASK32_RING3_SOURCE_DIRECT,
    TASK32_RING3_SOURCE_CALL,
    TASK32_RING3_SOURCE_GATE_JUMP,
    TASK32_RING3_SOURCE_GATE_CALL,
    TASK32_RING3_SOURCE_GATE_PRIVILEGE,
    TASK32_READABLE_CODE_DATA,
    TASK32_NULL_DATA,
    TASK32_CR3_RESERVED
} task32_case;

static void task32_set_fault_gate(lib_u8 *idt, lib_u8 vector)
{
    const lib_u16 index = (lib_u16)vector * 8u;

    idt[index] = 0x80u;
    idt[index + 1u] = 0x01u;
    idt[index + 2u] = 0x08u;
    idt[index + 5u] = 0x86u;
}

static lib_bool task32_is_post_switch_rejection(task32_case test_case);

static void task32_prepare(cpu_instruction_fixture *fixture,
    x86_core_cpu_profile profile, task32_case test_case)
{
    static const lib_u8 source_gprs[] = {
        0x66u,0xb8u,0x11u,0x11u,0x11u,0x11u,
        0x66u,0xb9u,0x22u,0x22u,0x22u,0x22u,
        0x66u,0xbau,0x33u,0x33u,0x33u,0x33u,
        0x66u,0xbbu,0x44u,0x44u,0x44u,0x44u,
        0x66u,0xbcu,0x00u,0x00u,0x55u,0x55u,
        0x66u,0xbdu,0x66u,0x66u,0x66u,0x66u,
        0x66u,0xbeu,0x77u,0x77u,0x77u,0x77u,
        0x66u,0xbfu,0x88u,0x88u,0x88u,0x88u
    };
    static const lib_u8 gdt_base[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x20u,0,0xfau,0,0,
        0xffu,0xffu,0,0x30u,0,0xf2u,0,0,
        0xffu,0,0,0x06u,0,0x89u,0,0,
        0xffu,0,0,0x07u,0,0x89u,0,0,
        0,0,0x30u,0,0,0x85u,0,0,
        0x17u,0,0,0x09u,0,0x82u,0,0,
        0x67u,0,0,0x08u,0,0x89u,0,0
    };
    static const lib_u8 ldt_base[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0
    };
    static const lib_u8 target_halt[] = {0xf4u};
    static const lib_u8 gdt_pointer[] = {0x4fu,0,0,0x03u,0,0};
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0u,0x01u,
        0xb8u,1u,0u,0x0fu,0x01u,0xf0u,
        0xb8u,0x28u,0u,0x0fu,0u,0xd8u,
        0xb8u,0x10u,0u,0x8eu,0xd0u,0x8eu,0xd8u,0x8eu,0xc0u,
        0x8eu,0xe0u,0x8eu,0xe8u,0xbcu,0u,0x80u,
        0xeau,0u,0u,0x08u,0u
    };
    lib_u8 gdt[sizeof(gdt_base)];
    lib_u8 idt[0x70u] = {0};
    lib_u8 ldt[sizeof(ldt_base)];
    lib_u8 source[128u];
    lib_size source_bytes = sizeof(source_gprs);
    lib_u8 fault_vector = 10u;
    task32_state target = {
        .cr3 = 0x00001000u, .eip = 0x100u, .eflags = 0x2u, .eax = 0xa1a12222u,
        .ecx = 0xc1c13333u, .edx = 0xd1d14444u, .ebx = 0xb1b15555u,
        .esp = 0x8000u, .ebp = 0xe1e16666u, .esi = 0xf1f17777u,
        .edi = 0x81818888u, .es = {0x10u,0u}, .cs = {0x08u,0u},
        .ss = {0x10u,0u}, .ds = {0x10u,0u}, .fs = {0x10u,0u},
        .gs = {0x10u,0u}
    };
    task32_state fault_target = {
        .cr3 = 0x00001000u, .eip = 0x180u, .eflags = 0x2u,
        .esp = 0x8000u, .es = {0x10u,0u}, .cs = {0x08u,0u},
        .ss = {0x10u,0u}, .ds = {0x10u,0u}, .fs = {0x10u,0u},
        .gs = {0x10u,0u}
    };
    task32_state ring3_source_target;
    t_cpu *const cpu = &fixture->cpu;

    cpu_instruction_prepare(fixture, profile);
    lib_memory_copy(gdt, gdt_base, sizeof(gdt));
    lib_memory_copy(ldt, ldt_base, sizeof(ldt));
    lib_memory_copy(source, source_gprs, sizeof(source_gprs));
    source[source_bytes++] = 0xeau;
    source[source_bytes++] = 0u;
    source[source_bytes++] = 0u;
    source[source_bytes++] = 0x30u;
    source[source_bytes++] = 0u;
    if (test_case >= TASK32_NESTED_CALL) {
        const lib_u8 opcode = test_case == TASK32_GATE_JMP ||
            test_case == TASK32_GATE_JMP_OPERAND32 ? 0xeau : 0x9au;

        source_bytes = sizeof(source_gprs);
        if (test_case == TASK32_NESTED_CALL_OPERAND32 ||
            test_case == TASK32_GATE_JMP_OPERAND32) {
            source[sizeof(source_gprs)] = 0x66u;
            source[sizeof(source_gprs) + 1u] = opcode;
            source_bytes = sizeof(source_gprs) + 2u;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0u;
        } else {
            source[source_bytes++] = opcode;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0u;
        }
        source[source_bytes++] = (test_case == TASK32_NESTED_GATE_CALL ||
            test_case == TASK32_GATE_JMP ||
            test_case == TASK32_GATE_JMP_OPERAND32) ? 0x38u : 0x30u;
        source[source_bytes++] = 0u;
        if (test_case == TASK32_NESTED_CALL_INDIRECT) {
            source_bytes = sizeof(source_gprs);
            source[source_bytes++] = 0xffu;
            source[source_bytes++] = 0x1eu;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0x52u;
            fixture->memory[0x8200u] = 0u;
            fixture->memory[0x8201u] = 0u;
            fixture->memory[0x8202u] = 0x30u;
            fixture->memory[0x8203u] = 0u;
        }
        if (test_case == TASK32_NESTED_RETURN) {
            source[26u] = 0u;
            source[27u] = 0x55u;
            source[28u] = 0u;
            source[29u] = 0u;
        }
    }
    if (test_case == TASK32_OPERAND32) {
        source[sizeof(source_gprs)] = 0x66u;
        source[sizeof(source_gprs) + 1u] = 0xeau;
        source[sizeof(source_gprs) + 2u] = 0u;
        source[sizeof(source_gprs) + 3u] = 0u;
        source[sizeof(source_gprs) + 4u] = 0u;
        source[sizeof(source_gprs) + 5u] = 0u;
        source[sizeof(source_gprs) + 6u] = 0x30u;
        source[sizeof(source_gprs) + 7u] = 0u;
        source_bytes = sizeof(source_gprs) + 8u;
    }
    if (test_case == TASK32_LOCK_DIRECT || test_case == TASK32_LOCK_INDIRECT) {
        source_bytes = sizeof(source_gprs);
        source[source_bytes++] = 0xf0u;
        source[source_bytes++] = test_case == TASK32_LOCK_DIRECT ? 0xeau : 0xffu;
        if (test_case == TASK32_LOCK_DIRECT) {
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0x30u;
            source[source_bytes++] = 0u;
        } else {
            source[source_bytes++] = 0x2eu;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0x52u;
            fixture->memory[0x8200u] = 0u;
            fixture->memory[0x8201u] = 0u;
            fixture->memory[0x8202u] = 0x30u;
            fixture->memory[0x8203u] = 0u;
        }
    }
    if (test_case == TASK32_INDIRECT16 || test_case == TASK32_INDIRECT32 ||
        test_case == TASK32_INDIRECT_ADDRESS32 ||
        test_case == TASK32_INDIRECT_OPERAND_ADDRESS32) {
        source_bytes = sizeof(source_gprs);
        if (test_case == TASK32_INDIRECT32 ||
            test_case == TASK32_INDIRECT_OPERAND_ADDRESS32) source[source_bytes++] = 0x66u;
        if (test_case == TASK32_INDIRECT_ADDRESS32 ||
            test_case == TASK32_INDIRECT_OPERAND_ADDRESS32) {
            source[source_bytes++] = 0x67u;
            source[source_bytes++] = 0xffu;
            source[source_bytes++] = 0x2du;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0x52u;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0u;
        } else {
            source[source_bytes++] = 0xffu;
            source[source_bytes++] = 0x2eu;
            source[source_bytes++] = 0u;
            source[source_bytes++] = 0x52u;
        }
        fixture->memory[0x8200u] = 0u;
        fixture->memory[0x8201u] = 0u;
        fixture->memory[0x8202u] = 0x30u;
        fixture->memory[0x8203u] = 0u;
        if (test_case == TASK32_INDIRECT32 ||
            test_case == TASK32_INDIRECT_OPERAND_ADDRESS32) {
            fixture->memory[0x8204u] = 0x30u;
            fixture->memory[0x8205u] = 0u;
        }
    }
    if (test_case == TASK32_INVALID_CODE) target.cs.selector = 0x10u;
    if (test_case == TASK32_RING3_DIRECT) {
        target.es.selector = 0x23u;
        target.cs.selector = 0x1bu;
        target.ss.selector = 0x23u;
        target.ds.selector = 0x23u;
        target.fs.selector = 0x23u;
        target.gs.selector = 0x23u;
        target.eflags = 0x3002u;
    }
    if (test_case == TASK32_RING3_SOURCE_DIRECT ||
        test_case == TASK32_RING3_SOURCE_CALL ||
        test_case == TASK32_RING3_SOURCE_GATE_JUMP ||
        test_case == TASK32_RING3_SOURCE_GATE_CALL ||
        test_case == TASK32_RING3_SOURCE_GATE_PRIVILEGE) {
        target.es.selector = 0x23u;
        target.cs.selector = 0x1bu;
        target.ss.selector = 0x23u;
        target.ds.selector = 0x23u;
        target.fs.selector = 0x23u;
        target.gs.selector = 0x23u;
        target.eflags = 0x3002u;
        ring3_source_target = target;
        ring3_source_target.eip = 0x1a0u;
        gdt[0x4du] = 0xe9u;
        if (test_case == TASK32_RING3_SOURCE_GATE_JUMP ||
            test_case == TASK32_RING3_SOURCE_GATE_CALL) {
            gdt[0x3au] = 0x48u;
            gdt[0x3du] = 0xe5u;
        }
    }
    if (test_case == TASK32_READABLE_CODE_DATA) {
        target.es.selector = 0x08u;
        target.ds.selector = 0x08u;
        target.fs.selector = 0x08u;
        target.gs.selector = 0x08u;
    }
    if (test_case == TASK32_NULL_DATA) {
        target.es.selector = 0u;
        target.ds.selector = 0u;
        target.fs.selector = 0u;
        target.gs.selector = 0u;
    }
    if (test_case == TASK32_CR3_RESERVED) target.cr3 = 0x00001003u;
    if (test_case == TASK32_NESTED_INVALID_CODE) {
        target.cs.selector = 0x10u;
        fault_vector = 10u;
    }
    if (test_case == TASK32_TARGET_BUSY) {
        gdt[0x35u] = 0x8bu;
        fault_vector = 13u;
    }
    if (test_case == TASK32_RING3_SOURCE_GATE_PRIVILEGE) fault_vector = 13u;
    if (test_case == TASK32_NESTED_TARGET_BUSY) {
        gdt[0x35u] = 0x8bu;
        fault_vector = 13u;
    }
    if (test_case == TASK32_TARGET_SHORT) gdt[0x30u] = 0x60u;
    if (test_case == TASK32_NESTED_TARGET_SHORT) gdt[0x30u] = 0x60u;
    if (test_case == TASK32_STACK_LIMIT) {
        gdt[0x20u] = 0u;
        gdt[0x21u] = 0u;
        gdt[0x22u] = 0u;
        gdt[0x23u] = 0x30u;
        gdt[0x25u] = 0x92u;
        target.ss.selector = 0x20u;
        fault_vector = 12u;
    }
    if (test_case == TASK32_NESTED_STACK_LIMIT) {
        gdt[0x20u] = 0u;
        gdt[0x21u] = 0u;
        gdt[0x22u] = 0u;
        gdt[0x23u] = 0x30u;
        gdt[0x25u] = 0x92u;
        target.ss.selector = 0x20u;
        fault_vector = 12u;
    }
    if (test_case >= TASK32_LDT_SUCCESS && test_case <= TASK32_LDT_BAD_DATA) {
        target.es.selector = 0x14u;
        target.cs.selector = 0x0cu;
        target.ss.selector = 0x14u;
        target.ds.selector = 0x14u;
        target.fs.selector = 0x14u;
        target.gs.selector = 0x14u;
        target.ldtr.selector = 0x40u;
        if (test_case == TASK32_LDT_BAD_DESCRIPTOR) gdt[0x45u] = 0x92u;
        if (test_case == TASK32_LDT_NOT_PRESENT) {
            gdt[0x45u] = 0x02u;
            fault_vector = 11u;
        }
        if (test_case == TASK32_LDT_SHORT) gdt[0x40u] = 0x0fu;
        if (test_case == TASK32_LDT_BAD_CODE) ldt[13u] = 0x92u;
        if (test_case == TASK32_LDT_BAD_DATA) ldt[21u] = 0x9au;
    }
    task32_set_fault_gate(idt, fault_vector);
    if (task32_is_post_switch_rejection(test_case)) {
        const lib_u8 fault_gate[] = {0,0,0x48u,0,0,0x85u,0,0};

        lib_memory_copy(idt + (lib_u16)fault_vector * 8u, fault_gate,
            sizeof(fault_gate));
        lib_memory_copy(fixture->memory + TASK32_C_BASE + 0x1cu,
            &fault_target, sizeof(fault_target));
    }
    fixture->memory[TASK32_CODE_BASE + 0x180u] = 0xf4u;
    lib_memory_copy(fixture->memory + 0x0100u, gdt_pointer, sizeof(gdt_pointer));
    lib_memory_copy(fixture->memory + TASK32_GDT_BASE, gdt, sizeof(gdt));
    lib_memory_copy(fixture->memory + TASK32_IDT_BASE, idt, sizeof(idt));
    lib_memory_copy(fixture->memory + TASK32_B_BASE + 0x1cu, &target,
        sizeof(target));
    if (test_case == TASK32_RING3_SOURCE_DIRECT ||
        test_case == TASK32_RING3_SOURCE_CALL ||
        test_case == TASK32_RING3_SOURCE_GATE_JUMP ||
        test_case == TASK32_RING3_SOURCE_GATE_CALL ||
        test_case == TASK32_RING3_SOURCE_GATE_PRIVILEGE)
        lib_memory_copy(fixture->memory + TASK32_C_BASE + 0x1cu,
            &ring3_source_target, sizeof(ring3_source_target));
    if (test_case == TASK32_RING3_SOURCE_GATE_PRIVILEGE) {
        const lib_u32 ring0_esp = 0x8000u;
        const lib_u16 ring0_ss = 0x10u;

        lib_memory_copy(fixture->memory + TASK32_B_BASE + 4u,
            &ring0_esp, sizeof(ring0_esp));
        lib_memory_copy(fixture->memory + TASK32_B_BASE + 8u,
            &ring0_ss, sizeof(ring0_ss));
    }
    lib_memory_copy(fixture->memory + TASK32_CODE_BASE + 0x100u, target_halt,
        sizeof(target_halt));
    if (test_case == TASK32_RING3_DIRECT) {
        fixture->memory[TASK32_CODE_BASE + 0x100u] = 0xebu;
        fixture->memory[TASK32_CODE_BASE + 0x101u] = 0xfeu;
    }
    if (test_case == TASK32_RING3_SOURCE_DIRECT ||
        test_case == TASK32_RING3_SOURCE_CALL ||
        test_case == TASK32_RING3_SOURCE_GATE_JUMP ||
        test_case == TASK32_RING3_SOURCE_GATE_CALL ||
        test_case == TASK32_RING3_SOURCE_GATE_PRIVILEGE) {
        const lib_u8 ring3_source[] = {
            (test_case == TASK32_RING3_SOURCE_DIRECT ||
             test_case == TASK32_RING3_SOURCE_GATE_JUMP) ? 0xeau : 0x9au,
            0xa0u,0x01u,
            (test_case == TASK32_RING3_SOURCE_GATE_JUMP ||
             test_case == TASK32_RING3_SOURCE_GATE_CALL ||
             test_case == TASK32_RING3_SOURCE_GATE_PRIVILEGE) ?
                (test_case == TASK32_RING3_SOURCE_GATE_PRIVILEGE ?
                    0x38u : 0x3bu) : 0x4bu,
            0u
        };

        lib_memory_copy(fixture->memory + TASK32_CODE_BASE + 0x100u,
            ring3_source, sizeof(ring3_source));
        fixture->memory[TASK32_CODE_BASE + 0x1a0u] = 0xebu;
        fixture->memory[TASK32_CODE_BASE + 0x1a1u] = 0xfeu;
    }
    if (test_case == TASK32_NESTED_RETURN) {
        fixture->memory[TASK32_CODE_BASE + 0x100u] = 0xcfu;
        fixture->memory[TASK32_CODE_BASE + source_bytes] = 0xf4u;
    }
    lib_memory_copy(fixture->memory + TASK32_CODE_BASE, source, source_bytes);
    lib_memory_copy(fixture->memory, bootstrap, sizeof(bootstrap));
    if (test_case >= TASK32_LDT_SUCCESS && test_case <= TASK32_LDT_BAD_DATA)
        lib_memory_copy(fixture->memory + 0x0900u, ldt, sizeof(ldt));
    if (test_case == TASK32_DEBUG_TRAP) {
        const lib_u16 debug_word = 1u;

        idt[8u] = 0x80u;
        idt[9u] = 0x01u;
        idt[10u] = 0x08u;
        idt[13u] = 0x8eu;
        fixture->memory[TASK32_CODE_BASE + 0x180u] = 0x40u;
        fixture->memory[TASK32_CODE_BASE + 0x181u] = 0xf4u;
        lib_memory_copy(fixture->memory + TASK32_IDT_BASE, idt, sizeof(idt));
        lib_memory_copy(fixture->memory + TASK32_B_BASE + 0x64u, &debug_word,
            sizeof(debug_word));
        cpu->data.dr0 = 0xffffffffu;
        cpu->data.dr1 = 0xffffffffu;
        cpu->data.dr2 = 0xffffffffu;
        cpu->data.dr3 = 0xffffffffu;
        cpu->data.dr7 = 0x000003ffu;
    }
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = TASK32_IDT_BASE;
    cpu->data.idtr.limit = (lib_u16)(sizeof(idt) - 1u);
}

static void task32_refresh(cpu_instruction_fixture *fixture, task32_case test_case)
{
    lib_u8 index;

    for (index = 0u; index < 13u; ++index)
        x86_core_cpu_execution_refresh(&fixture->execution);
    if (test_case == TASK32_OLD_SHORT) fixture->cpu.data.tr.limit = 0x60u;
    for (index = 0u; index < 20u; ++index)
        x86_core_cpu_execution_refresh(&fixture->execution);
}

static lib_bool task32_is_ldt(task32_case test_case)
{
    return test_case >= TASK32_LDT_SUCCESS && test_case <= TASK32_LDT_BAD_DATA;
}

static lib_bool task32_is_rejection(task32_case test_case)
{
    return (test_case >= TASK32_INVALID_CODE &&
        test_case <= TASK32_STACK_LIMIT) ||
        (test_case >= TASK32_LDT_BAD_DESCRIPTOR &&
        test_case <= TASK32_LDT_BAD_DATA) ||
        (test_case >= TASK32_NESTED_INVALID_CODE &&
        test_case <= TASK32_NESTED_STACK_LIMIT);
}

static lib_bool task32_is_post_switch_rejection(task32_case test_case)
{
    return test_case == TASK32_INVALID_CODE ||
        test_case == TASK32_STACK_LIMIT ||
        (test_case >= TASK32_LDT_BAD_DESCRIPTOR &&
            test_case <= TASK32_LDT_BAD_DATA) ||
        test_case == TASK32_NESTED_INVALID_CODE ||
        test_case == TASK32_NESTED_STACK_LIMIT;
}

static lib_bool task32_is_special(task32_case test_case)
{
    return test_case >= TASK32_LOCK_DIRECT && test_case <= TASK32_DEBUG_TRAP;
}

static lib_bool task32_is_nested(task32_case test_case)
{
    return test_case >= TASK32_NESTED_CALL &&
        test_case <= TASK32_NESTED_STACK_LIMIT &&
        test_case != TASK32_GATE_JMP &&
        test_case != TASK32_GATE_JMP_OPERAND32;
}

static lib_bool task32_expect(x86_core_cpu_profile profile, task32_case test_case)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    task32_state outgoing = {0};
    const x86_core_cpu_fault_snapshot *snapshot;
    const lib_bool ldt = task32_is_ldt(test_case);
    const lib_bool rejection = task32_is_rejection(test_case);
    const lib_bool nested = task32_is_nested(test_case);
    const lib_u32 expected_fault = test_case == TASK32_TARGET_BUSY ||
        test_case == TASK32_NESTED_TARGET_BUSY ?
        VCPUINS_EXCEPT_GP : test_case == TASK32_STACK_LIMIT ||
        test_case == TASK32_NESTED_STACK_LIMIT ?
        VCPUINS_EXCEPT_SS : test_case == TASK32_LDT_NOT_PRESENT ?
        VCPUINS_EXCEPT_NP : VCPUINS_EXCEPT_TS;

    task32_prepare(&fixture, profile, test_case);
    if (test_case == TASK32_RING3_SOURCE_GATE_PRIVILEGE) {
        lib_u8 step;

        for (step = 0u; step < 33u && !fixture.delivered_exception.valid;
            ++step)
            x86_core_cpu_execution_refresh(&fixture.execution);
        after = fixture.cpu;
        return !fixture.fault.valid && fixture.delivered_exception.valid &&
            (fixture.delivered_exception.exception_mask & VCPUINS_EXCEPT_GP) != 0u &&
            after.data.tr.selector == 0x30u &&
            fixture.memory[TASK32_GDT_BASE + 0x35u] == 0x8bu &&
            fixture.memory[TASK32_GDT_BASE + 0x4du] == 0xe9u;
    }
    task32_refresh(&fixture, test_case);
    after = fixture.cpu;
    snapshot = fixture.fault.valid ? &fixture.fault : &fixture.delivered_exception;
    lib_memory_copy(&outgoing, fixture.memory + (test_case == TASK32_NESTED_RETURN ?
        TASK32_B_BASE : TASK32_A_BASE) + 0x1cu,
        sizeof(outgoing));
    if (test_case == TASK32_LOCK_DIRECT || test_case == TASK32_LOCK_INDIRECT)
        /* UD/GP/DF entries are absent: the failed GP pair reaches DF. */
        return x86_core_cpu_is_shutdown(&fixture.execution) &&
            !fixture.fault.valid && fixture.delivered_exception.valid &&
            fixture.delivered_exception.exception_mask == VCPUINS_EXCEPT_SHUTDOWN &&
            fixture.delivered_exception.exception_code == 0u &&
            after.data.tr.selector == 0x28u && after.data.eax == 0x11111111u &&
            after.data.ecx == 0x22222222u && after.data.edx == 0x33333333u &&
            after.data.ebx == 0x44444444u && after.data.esp == 0x55550000u &&
            after.data.ebp == 0x66666666u && after.data.esi == 0x77777777u &&
            after.data.edi == 0x88888888u;
    if (test_case == TASK32_DEBUG_TRAP)
        return !fixture.fault.valid && fixture.delivered_exception.valid &&
            (fixture.delivered_exception.exception_mask & VCPUINS_EXCEPT_DB) != 0u &&
            after.data.tr.selector == 0x30u && after.data.eip == 0x182u &&
            after.data.eax == 0xa1a12223u && after.data.esp == 0x7ff4u &&
            (after.data.dr6 & 0x00008000u) != 0u &&
            (after.data.dr7 & 0x000003ffu) == 0x000002aau;
    if (test_case == TASK32_NESTED_RETURN && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && after.data.flagHalt &&
        after.data.tr.selector == 0x28u && after.data.eax == 0x11111111u &&
        outgoing.eip == 0x101u && outgoing.eax == 0xa1a12222u &&
        fixture.memory[TASK32_GDT_BASE + 0x2du] == 0x8bu &&
        fixture.memory[TASK32_GDT_BASE + 0x35u] == 0x89u) return LIB_TRUE;
    if (test_case == TASK32_RING3_DIRECT && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && !after.data.flagHalt &&
        after.data.tr.selector == 0x30u && after.data.cs.selector == 0x1bu &&
        after.data.cs.dpl == 3u && after.data.ss.selector == 0x23u &&
        after.data.ds.selector == 0x23u && after.data.es.selector == 0x23u &&
        after.data.fs.selector == 0x23u && after.data.gs.selector == 0x23u &&
        after.data.eax == 0xa1a12222u) return LIB_TRUE;
    if (test_case == TASK32_RING3_SOURCE_DIRECT && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && !after.data.flagHalt &&
        after.data.tr.selector == 0x4bu && after.data.cs.selector == 0x1bu &&
        after.data.cs.dpl == 3u && after.data.ss.selector == 0x23u &&
        after.data.ds.selector == 0x23u && after.data.es.selector == 0x23u &&
        after.data.fs.selector == 0x23u && after.data.gs.selector == 0x23u &&
        after.data.eip == 0x1a0u &&
        fixture.memory[TASK32_GDT_BASE + 0x35u] == 0x89u &&
        fixture.memory[TASK32_GDT_BASE + 0x4du] == 0xebu) return LIB_TRUE;
    if (test_case == TASK32_RING3_SOURCE_CALL && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && !after.data.flagHalt &&
        after.data.tr.selector == 0x4bu && after.data.cs.selector == 0x1bu &&
        after.data.cs.dpl == 3u && after.data.ss.selector == 0x23u &&
        after.data.ds.selector == 0x23u && after.data.es.selector == 0x23u &&
        after.data.fs.selector == 0x23u && after.data.gs.selector == 0x23u &&
        after.data.eip == 0x1a0u &&
        (after.data.eflags & VCPU_EFLAGS_NT) != 0u &&
        fixture.memory[TASK32_C_BASE] == 0x30u &&
        fixture.memory[TASK32_GDT_BASE + 0x35u] == 0x8bu &&
        fixture.memory[TASK32_GDT_BASE + 0x4du] == 0xebu) return LIB_TRUE;
    if (test_case == TASK32_RING3_SOURCE_GATE_CALL && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && !after.data.flagHalt &&
        after.data.tr.selector == 0x48u && after.data.cs.selector == 0x1bu &&
        after.data.cs.dpl == 3u && after.data.ss.selector == 0x23u &&
        after.data.ds.selector == 0x23u && after.data.es.selector == 0x23u &&
        after.data.fs.selector == 0x23u && after.data.gs.selector == 0x23u &&
        after.data.eip == 0x1a0u &&
        (after.data.eflags & VCPU_EFLAGS_NT) != 0u &&
        fixture.memory[TASK32_C_BASE] == 0x30u &&
        fixture.memory[TASK32_GDT_BASE + 0x35u] == 0x8bu &&
        fixture.memory[TASK32_GDT_BASE + 0x4du] == 0xebu) return LIB_TRUE;
    if (test_case == TASK32_RING3_SOURCE_GATE_JUMP && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && !after.data.flagHalt &&
        after.data.tr.selector == 0x48u && after.data.cs.selector == 0x1bu &&
        after.data.cs.dpl == 3u && after.data.ss.selector == 0x23u &&
        after.data.ds.selector == 0x23u && after.data.es.selector == 0x23u &&
        after.data.fs.selector == 0x23u && after.data.gs.selector == 0x23u &&
        after.data.eip == 0x1a0u &&
        (after.data.eflags & VCPU_EFLAGS_NT) == 0u &&
        fixture.memory[TASK32_GDT_BASE + 0x35u] == 0x89u &&
        fixture.memory[TASK32_GDT_BASE + 0x4du] == 0xebu) return LIB_TRUE;
    if (test_case == TASK32_READABLE_CODE_DATA && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && after.data.flagHalt &&
        after.data.tr.selector == 0x30u && after.data.es.selector == 0x08u &&
        after.data.ds.selector == 0x08u && after.data.fs.selector == 0x08u &&
        after.data.gs.selector == 0x08u) return LIB_TRUE;
    if (test_case == TASK32_NULL_DATA && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && after.data.flagHalt &&
        after.data.tr.selector == 0x30u && after.data.es.selector == 0u &&
        after.data.ds.selector == 0u && after.data.fs.selector == 0u &&
        after.data.gs.selector == 0u) return LIB_TRUE;
    if (test_case == TASK32_CR3_RESERVED && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && after.data.flagHalt &&
        after.data.tr.selector == 0x30u && after.data.cr3 ==
            (profile == X86_CORE_CPU_PROFILE_80486SX ? 0x00001000u : 0x00001003u))
        return LIB_TRUE;
    if (task32_is_post_switch_rejection(test_case))
        return !fixture.fault.valid && fixture.delivered_exception.valid &&
            (fixture.delivered_exception.exception_mask & expected_fault) != 0u &&
            after.data.tr.selector == 0x48u && after.data.flagHalt;
    if (nested && !rejection && !fixture.fault.valid &&
        !fixture.delivered_exception.valid && after.data.flagHalt &&
        after.data.tr.selector == 0x30u && after.data.eip == 0x101u &&
        (after.data.eflags & VCPU_EFLAGS_NT) != 0u &&
        fixture.memory[TASK32_B_BASE] == 0x28u &&
        fixture.memory[TASK32_GDT_BASE + 0x2du] == 0x8bu &&
        fixture.memory[TASK32_GDT_BASE + 0x35u] == 0x8bu) return LIB_TRUE;
    if (!rejection && !nested && !task32_is_special(test_case)) return !fixture.fault.valid &&
        !fixture.delivered_exception.valid && after.data.flagHalt &&
        after.data.tr.selector == 0x30u && after.data.eip == 0x101u &&
        after.data.eax == 0xa1a12222u && after.data.ecx == 0xc1c13333u &&
        after.data.edx == 0xd1d14444u && after.data.ebx == 0xb1b15555u &&
        after.data.esp == 0x8000u && after.data.ebp == 0xe1e16666u &&
        after.data.esi == 0xf1f17777u && after.data.edi == 0x81818888u &&
        after.data.cs.selector == (ldt ? 0x0cu : 0x08u) &&
        after.data.ss.selector == (ldt ? 0x14u : 0x10u) &&
        after.data.ldtr.flagValid == ldt &&
        (!ldt || (after.data.ldtr.selector == 0x40u &&
            after.data.ldtr.base == 0x0900u && after.data.ldtr.limit == 0x17u)) &&
        outgoing.eip != 0u && outgoing.eax == 0x11111111u &&
        fixture.memory[TASK32_GDT_BASE + 0x2du] == 0x89u &&
        fixture.memory[TASK32_GDT_BASE + 0x35u] == 0x8bu;
    if (snapshot->valid && (snapshot->exception_mask & expected_fault) != 0u &&
        after.data.tr.selector == 0x28u && after.data.eip == 0x181u &&
        outgoing.eip == 0u && outgoing.eax == 0u &&
        (!nested || (fixture.memory[TASK32_GDT_BASE + 0x2du] == 0x8bu &&
            fixture.memory[TASK32_GDT_BASE + 0x35u] ==
            (test_case == TASK32_NESTED_TARGET_BUSY ? 0x8bu : 0x89u))))
        return LIB_TRUE;
    lib_c_fprintf(lib_c_stderr, "task32 state case=%u halt=%u tr=%04x eip=%08x fault=%u/%x/%04x delivered=%u/%x/%04x outgoing=%08x busy=%02x/%02x\n",
        (unsigned)test_case, (unsigned)after.data.flagHalt, after.data.tr.selector,
        (unsigned)after.data.eip, (unsigned)fixture.fault.valid,
        (unsigned)fixture.fault.exception_mask, fixture.fault.exception_code,
        (unsigned)fixture.delivered_exception.valid,
        (unsigned)fixture.delivered_exception.exception_mask,
        fixture.delivered_exception.exception_code,
        (unsigned)outgoing.eip,
        fixture.memory[TASK32_GDT_BASE + 0x2du],
        fixture.memory[TASK32_GDT_BASE + 0x35u]);
    return LIB_FALSE;
}

static lib_bool task32_test_published_fault_context(x86_core_cpu_profile profile)
{
    cpu_instruction_fixture fixture;
    const lib_u8 gp_gate[] = {0x80u,1u,8u,0u,0u,0x86u,0u,0u};
    lib_u8 step;
    lib_u32 outgoing_ip = 0u;

    task32_prepare(&fixture, profile, TASK32_DEBUG_TRAP);
    lib_memory_set(fixture.memory + TASK32_IDT_BASE + 8u, 0, 8u);
    lib_memory_copy(fixture.memory + TASK32_IDT_BASE + 13u * 8u,
        gp_gate, sizeof(gp_gate));
    for (step = 0u; step < 21u; ++step)
        x86_core_cpu_execution_refresh(&fixture.execution);
    if (fixture.execution.stop_requested || fixture.cpu.data.tr.selector != 0x28u ||
        fixture.cpu.data.eip != 48u) return LIB_FALSE;
    x86_core_cpu_execution_refresh(&fixture.execution);
    const lib_u64 event_ticks = x86_core_cpu_capture_timing(&fixture.execution).event_ticks;
    lib_memory_copy(&outgoing_ip, fixture.memory + TASK32_A_BASE + 0x20u,
        sizeof(outgoing_ip));
    if (fixture.execution.stop_requested || fixture.fault.valid ||
        !fixture.delivered_exception.valid ||
        fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
        fixture.delivered_exception.exception_code != 0x0bu ||
        fixture.delivered_exception.point.eip != 0x100u ||
        fixture.delivered_exception.point.byte_count != 0u ||
        fixture.delivered_exception.eax != 0xa1a12222u ||
        fixture.delivered_exception.esp != 0x8000u ||
        fixture.cpu.data.tr.selector != 0x30u || fixture.cpu.data.eip != 0x180u ||
        fixture.cpu.data.esp != 0x7ff8u || outgoing_ip != 53u ||
        event_ticks != (profile == X86_CORE_CPU_PROFILE_80486SX ? 44u : 0u) ||
        (fixture.cpu.data.dr6 & 0x00008000u) == 0u) {
        lib_c_fprintf(lib_c_stderr, "Published task fault point=%x mask=%x code=%x tr=%x ip=%x sp=%x old-ip=%x event=%llu\n",
            fixture.delivered_exception.point.eip,
            fixture.delivered_exception.exception_mask,
            fixture.delivered_exception.exception_code,
            fixture.cpu.data.tr.selector, fixture.cpu.data.eip,
            fixture.cpu.data.esp, outgoing_ip, event_ticks);
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool task32_seed_translation(cpu_instruction_fixture *fixture)
{
    static const lib_u8 code[][3] = {{0x0fu, 0x26u, 0xfau}, {0x0fu, 0x26u, 0xf2u}};
    static const lib_u32 values[] = {0x30010u, 0x00400d40u};
    lib_u8 saved[8];
    const lib_u32 edx = fixture->cpu.data.edx;
    const lib_u32 eip = fixture->cpu.data.eip;
    const lib_u32 address = fixture->cpu.data.cs.base + eip;

    lib_memory_copy(saved, fixture->memory + address, sizeof(saved));
    for (lib_u8 command = 0u; command < 2u; ++command) {
        fixture->cpu.data.eip = eip;
        fixture->cpu.data.edx = values[command];
        lib_memory_copy(fixture->memory + address, code[command], sizeof(code[command]));
        x86_core_cpu_execution_invalidate_prefetch(&fixture->execution);
        x86_core_cpu_execution_refresh(&fixture->execution);
        if (fixture->execution.stop_requested || fixture->fault.valid ||
            fixture->delivered_exception.valid) return LIB_FALSE;
    }
    lib_memory_copy(fixture->memory + address, saved, sizeof(saved));
    fixture->cpu.data.edx = edx;
    fixture->cpu.data.eip = eip;
    x86_core_cpu_execution_invalidate_prefetch(&fixture->execution);
    return fixture->execution.tlb[0][0].valid &&
        fixture->execution.tlb[0][0].linear == 0x00400000u;
}

static lib_bool task32_test_completion_flags(void)
{
    static const task32_case cases[] = {TASK32_DIRECT, TASK32_NESTED_CALL,
        TASK32_NESTED_GATE_CALL, TASK32_GATE_JMP, TASK32_NESTED_CALL,
        TASK32_NESTED_CALL};
    /* E-13: TSS32 TS=162, with direct/gate JMP, CALL or IDT task gate. */
    static const lib_u16 task_clocks[] = {204u, 199u, 200u, 205u, 199u};
    lib_size index;
    lib_u16 bits;
    lib_u8 step;
    lib_u32 count = 0u, failures = 0u;

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    for (bits = 0u; bits < (profile == X86_CORE_CPU_PROFILE_80486SX ? 512u : 16u); ++bits) {
        cpu_instruction_fixture fixture;
        const lib_bool int1_vm86 = index == 5u;
        const lib_u32 incoming = ((bits & 256u) ? 0xffc08028u : 2u) |
            ((bits & 4u) ? VCPU_EFLAGS_RF : 0u) |
            ((bits & 8u) ? VCPU_EFLAGS_TF : 0u) |
            ((bits & 16u) ? VCPU_EFLAGS_AC : 0u) |
            ((bits & 32u) ? VCPU_EFLAGS_ID : 0u) |
            ((bits & 64u) ? VCPU_EFLAGS_VIF : 0u) |
            ((bits & 128u) ? VCPU_EFLAGS_VIP : 0u);
        const lib_u32 expected =
            (profile == X86_CORE_CPU_PROFILE_80486SX ?
                (incoming & 0x003f7fd5u) | 2u : incoming) |
            (index == 1u || index == 2u || index >= 4u ? VCPU_EFLAGS_NT : 0u);
        const lib_u32 cr3 = 0x1000u |
            (profile == X86_CORE_CPU_PROFILE_80486SX ?
                7u | ((lib_u32)(bits & 3u) << 3u) : 0u);

        if (int1_vm86 && profile != X86_CORE_CPU_PROFILE_80486SX) continue;
        task32_prepare(&fixture, profile, cases[index]);
        if (index >= 4u) {
            const lib_u8 gate[] = {0u,0u,0x30u,0u,0u,0x85u,0u,0u};

            fixture.memory[TASK32_CODE_BASE + 48u] = int1_vm86 ? 0xf1u : 0xcdu;
            fixture.memory[TASK32_CODE_BASE + 49u] = 0x20u;
            lib_memory_copy(fixture.memory + TASK32_IDT_BASE + (int1_vm86 ? 8u : 0x100u),
                gate, sizeof(gate));
            fixture.cpu.data.idtr.limit = 0x107u;
        }
        for (step = 0u; step < 21u; ++step)
            x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.cpu.data.eip != 48u ||
            fixture.cpu.data.tr.selector != 0x28u) return LIB_FALSE;
        if (profile == X86_CORE_CPU_PROFILE_80486SX &&
            !task32_seed_translation(&fixture)) return LIB_FALSE;
        fixture.cpu.data.eflags = 2u | ((bits & 1u) ? VCPU_EFLAGS_RF : 0u) |
            ((bits & 2u) ? VCPU_EFLAGS_TF : 0u) |
            (profile == X86_CORE_CPU_PROFILE_80486SX ?
                (VCPU_EFLAGS_AC | VCPU_EFLAGS_ID |
                    VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP) & ~incoming : 0u);
        fixture.cpu.data.cr4 = profile == X86_CORE_CPU_PROFILE_80486SX ?
            VCPU_CR4_VME : 0u;
        if (int1_vm86) {
            fixture.cpu.data.eflags = (fixture.cpu.data.eflags &
                ~(VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP)) | VCPU_EFLAGS_VM;
            fixture.cpu.data.cs.selector = 0x200u;
            fixture.cpu.data.cs.dpl = 3u;
            fixture.cpu.data.ss.selector = 0x300u;
            fixture.cpu.data.ss.dpl = 3u;
            fixture.cpu.data.ds.selector = 0x300u;
            fixture.cpu.data.ds.dpl = 3u;
            fixture.cpu.data.dr7 = 0x2400u;
        }
        const lib_u32 outgoing_flags = fixture.cpu.data.eflags;
        const lib_u32 outgoing_dr6 = fixture.cpu.data.dr6;
        lib_memory_copy(fixture.memory + TASK32_B_BASE + 0x24u,
            &incoming, sizeof(incoming));
        lib_memory_copy(fixture.memory + TASK32_B_BASE + 0x1cu,
            &cr3, sizeof(cr3));
        x86_core_cpu_execution_refresh(&fixture.execution);
        ++count;
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            (int1_vm86 ? (!fixture.delivered_exception.valid ||
                fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
                fixture.delivered_exception.exception_code != 0u ||
                fixture.delivered_exception.point.eip != 49u || fixture.cpu.data.dr7 != 0x400u ||
                fixture.cpu.data.dr6 != outgoing_dr6) :
                fixture.delivered_exception.valid) ||
            fixture.cpu.data.tr.selector != 0x30u || fixture.cpu.data.eip != 0x100u ||
            fixture.cpu.data.cr3 != (cr3 & ~7u) ||
            fixture.cpu.data.eflags != expected || fixture.execution.debug_trap_pending ||
            fixture.cpu.data.cr4 != (profile == X86_CORE_CPU_PROFILE_80486SX ?
                VCPU_CR4_VME : 0u)) {
            if (failures == 0u)
                lib_c_printf("Task completion mismatch profile=%u case=%zu bits=%u tr=%x ip=%x flags=%x expected=%x event=%x/%x point=%x\n",
                    (lib_u32)profile, index, (lib_u32)bits, fixture.cpu.data.tr.selector,
                    fixture.cpu.data.eip, fixture.cpu.data.eflags, expected,
                    fixture.delivered_exception.exception_mask,
                    fixture.delivered_exception.exception_code,
                    fixture.delivered_exception.point.eip);
            ++failures;
        }
        lib_u32 saved_flags;
        lib_memory_copy(&saved_flags, fixture.memory + TASK32_A_BASE + 0x24u,
            sizeof(saved_flags));
        if (saved_flags != (int1_vm86 ? outgoing_flags & ~VCPU_EFLAGS_RF : outgoing_flags))
            ++failures;
        if (profile == X86_CORE_CPU_PROFILE_80486SX && !int1_vm86) {
            lib_u64 ticks = 0u;

            if (!x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
                ticks != task_clocks[index]) {
                lib_c_printf("TSS32 timing: route=%zu expected=%u actual=%llu\n",
                    index, task_clocks[index], ticks);
                ++failures;
            }
        }
        if (int1_vm86) {
            lib_u32 saved_ip;

            lib_memory_copy(&saved_ip, fixture.memory + TASK32_A_BASE + 0x20u, sizeof(saved_ip));
            if (saved_ip != 49u) ++failures;
        }
        if (profile == X86_CORE_CPU_PROFILE_80486SX)
            for (lib_u8 set = 0u; set < X86_CPU_TLB_SETS; ++set)
                for (lib_u8 way = 0u; way < X86_CPU_TLB_WAYS; ++way)
                    if (fixture.execution.tlb[set][way].valid) ++failures;
    }
    lib_c_printf("Task completion flags cases=%u failures=%u\n",
        (unsigned)count, (unsigned)failures);
    return failures == 0u;
}

static lib_bool task32_test_vm86_entry(void)
{
    static const task32_case cases[] = {TASK32_DIRECT, TASK32_NESTED_CALL,
        TASK32_NESTED_GATE_CALL, TASK32_GATE_JMP, TASK32_NESTED_CALL,
        TASK32_NESTED_RETURN};
    /* E-13: incoming VM86 TS=140, including IDT task gate and IRET. */
    static const lib_u16 task_clocks[] = {182u, 177u, 178u, 183u, 177u, 172u};

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_size route = 0u; route < sizeof(cases) / sizeof(cases[0]); ++route)
    for (lib_u8 image = 0u; image < 8u; ++image)
    for (lib_u8 controls = 0u; controls < 4u; ++controls)
    for (lib_u8 ldt = 0u; ldt < 2u; ++ldt) {
        cpu_instruction_fixture fixture;
        task32_state target, outgoing;
        const lib_bool nested = route == 1u || route == 2u || route == 4u;
        const lib_bool returning = route == 5u;
        const lib_u32 target_base = returning ? TASK32_A_BASE : TASK32_B_BASE;
        const lib_u16 selectors[] = {
            image ? (lib_u16)(0x500u + image) : 0u,
            image ? (lib_u16)(0x600u + image) : 0u,
            image ? (lib_u16)(0xa00u + image) : 0u,
            image ? (lib_u16)(0x700u + image) : 0u,
            image ? (lib_u16)(0x800u + image) : 0u,
            image ? (lib_u16)(0x900u + image) : 0u
        };
        const lib_u8 read[] = {0xa0u, 0xf0u, 0x01u};

        task32_prepare(&fixture, profile, cases[route]);
        if (route == 4u) {
            const lib_u8 gate[] = {0u,0u,0x30u,0u,0u,0x85u,0u,0u};

            fixture.memory[TASK32_CODE_BASE + 48u] = 0xcdu;
            fixture.memory[TASK32_CODE_BASE + 49u] = 0x20u;
            lib_memory_copy(fixture.memory + TASK32_IDT_BASE + 0x100u, gate, sizeof(gate));
            fixture.cpu.data.idtr.limit = 0x107u;
        }
        if (returning) {
            for (lib_u8 step = 0u; step < 22u; ++step)
                x86_core_cpu_execution_refresh(&fixture.execution);
            if (fixture.execution.stop_requested || fixture.fault.valid ||
                fixture.delivered_exception.valid || fixture.cpu.data.tr.selector != 0x30u ||
                fixture.cpu.data.eip != 0x100u) return LIB_FALSE;
        }
        lib_memory_copy(&target, fixture.memory + target_base + 0x1cu, sizeof(target));
        target.eflags = 2u | VCPU_EFLAGS_VM |
            ((controls & 1u) ? VCPU_EFLAGS_TF : 0u) |
            ((controls & 2u) ? VCPU_EFLAGS_RF : 0u);
        target.eip = 0x180u;
        target.esp = 0x5678u;
        target.cs.selector = selectors[0];
        target.ss.selector = selectors[1];
        target.ds.selector = selectors[2];
        target.es.selector = selectors[3];
        target.fs.selector = selectors[4];
        target.gs.selector = selectors[5];
        target.ldtr.selector = ldt ? 0x40u : 0u;
        lib_memory_copy(fixture.memory + target_base + 0x1cu, &target, sizeof(target));
        lib_memory_copy(fixture.memory + ((lib_u32)selectors[0] << 4u) + target.eip,
            read, sizeof(read));
        fixture.memory[((lib_u32)selectors[2] << 4u) + 0x1f0u] = 0xabu;
        for (lib_u8 step = 0u; step < (returning ? 1u : 22u); ++step)
            x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            fixture.delivered_exception.valid ||
            fixture.cpu.data.tr.selector != (returning ? 0x28u : 0x30u) ||
            fixture.cpu.data.eip != target.eip || fixture.cpu.data.esp != target.esp ||
            fixture.cpu.data.eflags != (target.eflags | (nested ? VCPU_EFLAGS_NT : 0u)) ||
            fixture.cpu.data.eax != target.eax || fixture.cpu.data.ecx != target.ecx ||
            fixture.cpu.data.ldtr.selector != target.ldtr.selector ||
            fixture.cpu.data.ldtr.flagValid != (ldt != 0u) ||
            fixture.memory[TASK32_GDT_BASE + (returning ? 0x2du : 0x35u)] != 0x8bu) {
            lib_c_printf("VM86 task entry: profile=%u route=%zu image=%u controls=%u ldt=%u tr=%x ip=%x event=%x/%x\n",
                (lib_u32)profile, route, (lib_u32)image, (lib_u32)controls, (lib_u32)ldt,
                fixture.cpu.data.tr.selector, fixture.cpu.data.eip,
                fixture.delivered_exception.exception_mask, fixture.delivered_exception.exception_code);
            return LIB_FALSE;
        }
        if (profile == X86_CORE_CPU_PROFILE_80486SX) {
            lib_u64 ticks = 0u;

            if (!x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
                ticks != task_clocks[route]) {
                lib_c_printf("VM86 task timing: route=%zu expected=%u actual=%llu\n",
                    route, task_clocks[route], ticks);
                return LIB_FALSE;
            }
        }
        const t_cpu_data_sreg *segments[] = {&fixture.cpu.data.cs, &fixture.cpu.data.ss,
            &fixture.cpu.data.ds, &fixture.cpu.data.es, &fixture.cpu.data.fs, &fixture.cpu.data.gs};
        for (lib_u8 segment = 0u; segment < 6u; ++segment) {
            const t_cpu_data_sreg *cache = segments[segment];

            if (!cache->flagValid || cache->selector != selectors[segment] ||
                cache->base != (lib_u32)selectors[segment] << 4u ||
                cache->limit != 0xffffu || cache->dpl != 3u ||
                !cache->seg.accessed || cache->seg.executable != (segment == 0u) ||
                (segment == 0u ? (cache->seg.exec.defsize || cache->seg.exec.conform ||
                    !cache->seg.exec.readable) : (cache->seg.data.big ||
                    cache->seg.data.expdown || !cache->seg.data.writable)))
                return LIB_FALSE;
        }
        lib_memory_copy(&outgoing, fixture.memory +
            (returning ? TASK32_B_BASE : TASK32_A_BASE) + 0x1cu, sizeof(outgoing));
        if (outgoing.eip != (returning ? 0x101u : route == 4u ? 50u : 53u) ||
            (nested && fixture.memory[TASK32_B_BASE] != 0x28u)) return LIB_FALSE;
        /* Inspect actual VM86 instruction fetch and DS addressing without a TF trap. */
        fixture.cpu.data.eflags &= ~VCPU_EFLAGS_TF;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            fixture.delivered_exception.valid || fixture.cpu.data.al != 0xabu ||
            fixture.cpu.data.eip != target.eip + sizeof(read)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool task32_test_vm86_entry_faults(void)
{
    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 failure = 0u; failure < 2u; ++failure) {
        cpu_instruction_fixture fixture;
        task32_state target, fault_target, saved;
        const lib_u8 vector = failure ? 10u : 13u;
        const lib_u8 gate[] = {0u,0u,0x48u,0u,0u,0x85u,0u,0u};
        const lib_u32 error = failure ? 0x10u : 0u;

        task32_prepare(&fixture, profile, TASK32_DIRECT);
        lib_memory_copy(&target, fixture.memory + TASK32_B_BASE + 0x1cu, sizeof(target));
        fault_target = target;
        fault_target.eip = 0x180u;
        lib_memory_copy(fixture.memory + TASK32_C_BASE + 0x1cu,
            &fault_target, sizeof(fault_target));
        lib_memory_copy(fixture.memory + TASK32_IDT_BASE + vector * 8u, gate, sizeof(gate));
        target.eflags = 2u | VCPU_EFLAGS_VM;
        target.cs.selector = 0x523u;
        target.ss.selector = 0x600u;
        target.eip = failure ? 0x100u : 0x10000u;
        target.ldtr.selector = failure ? 0x10u : 0u;
        lib_memory_copy(fixture.memory + TASK32_B_BASE + 0x1cu, &target, sizeof(target));
        for (lib_u8 step = 0u; step < 22u; ++step)
            x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            !fixture.delivered_exception.valid ||
            fixture.delivered_exception.exception_mask !=
                (failure ? VCPUINS_EXCEPT_TS : VCPUINS_EXCEPT_GP) ||
            fixture.delivered_exception.exception_code != error ||
            fixture.delivered_exception.point.cs != target.cs.selector ||
            fixture.delivered_exception.point.eip != target.eip ||
            fixture.cpu.data.tr.selector != 0x48u || fixture.cpu.data.eip != 0x180u ||
            (fixture.cpu.data.eflags & VCPU_EFLAGS_VM) ||
            fixture.memory[TASK32_C_BASE] != 0x30u) {
            lib_c_printf("VM86 task fault: profile=%u failure=%u event=%x/%x point=%x:%x tr=%x ip=%x\n",
                (lib_u32)profile, (lib_u32)failure, fixture.delivered_exception.exception_mask,
                fixture.delivered_exception.exception_code, fixture.delivered_exception.point.cs,
                fixture.delivered_exception.point.eip, fixture.cpu.data.tr.selector,
                fixture.cpu.data.eip);
            return LIB_FALSE;
        }
        lib_memory_copy(&saved, fixture.memory + TASK32_B_BASE + 0x1cu, sizeof(saved));
        if (saved.eip != target.eip || saved.cs.selector != target.cs.selector ||
            !(saved.eflags & VCPU_EFLAGS_VM)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };
    if (!task32_test_completion_flags()) return 1;
    if (!task32_test_vm86_entry()) return 1;
    if (!task32_test_vm86_entry_faults()) return 1;
    static const task32_case cases[] = {
        TASK32_DIRECT, TASK32_OPERAND32, TASK32_INDIRECT16, TASK32_INDIRECT32,
        TASK32_INDIRECT_ADDRESS32, TASK32_INDIRECT_OPERAND_ADDRESS32,
        TASK32_INVALID_CODE, TASK32_TARGET_BUSY, TASK32_OLD_SHORT,
        TASK32_TARGET_SHORT, TASK32_STACK_LIMIT, TASK32_LDT_SUCCESS,
        TASK32_LDT_BAD_DESCRIPTOR, TASK32_LDT_NOT_PRESENT, TASK32_LDT_SHORT,
        TASK32_LDT_BAD_CODE, TASK32_LDT_BAD_DATA, TASK32_LOCK_DIRECT,
        TASK32_LOCK_INDIRECT, TASK32_DEBUG_TRAP, TASK32_NESTED_CALL,
        TASK32_NESTED_CALL_OPERAND32, TASK32_NESTED_CALL_INDIRECT,
        TASK32_NESTED_GATE_CALL, TASK32_GATE_JMP, TASK32_GATE_JMP_OPERAND32,
        TASK32_NESTED_RETURN, TASK32_NESTED_INVALID_CODE,
        TASK32_NESTED_TARGET_BUSY, TASK32_NESTED_TARGET_SHORT,
        TASK32_NESTED_STACK_LIMIT, TASK32_RING3_DIRECT,
        TASK32_RING3_SOURCE_DIRECT,
        TASK32_RING3_SOURCE_CALL,
        TASK32_RING3_SOURCE_GATE_JUMP,
        TASK32_RING3_SOURCE_GATE_CALL,
        TASK32_RING3_SOURCE_GATE_PRIVILEGE,
        TASK32_READABLE_CODE_DATA, TASK32_NULL_DATA, TASK32_CR3_RESERVED
    };
    lib_size index;

    for (lib_size profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        if (!task32_test_published_fault_context(profiles[profile])) return 1;
        for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
            if (!task32_expect(profiles[profile], cases[index])) {
                lib_c_fprintf(lib_c_stderr, "task32 failed profile=%u case=%u\n",
                    profiles[profile], (unsigned)cases[index]);
                lib_c_fprintf(lib_c_stderr, "%s", "TASK32-STATE:FAIL\n");
                return 1;
            }
    }
    lib_c_printf("%s\n", "TASK32-STATE:OK");
    lib_c_printf("%s\n", "TASK32-NESTING:OK");
    return 0;
}
