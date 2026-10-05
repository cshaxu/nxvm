#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

#define TASK32_GDT_BASE 0x0300u
#define TASK32_IDT_BASE 0x0400u
#define TASK32_A_BASE 0x0600u
#define TASK32_B_BASE 0x0700u
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
    TASK32_NESTED_STACK_LIMIT
} task32_case;

static void task32_set_fault_gate(lib_u8 *idt, lib_u8 vector)
{
    const lib_u16 index = (lib_u16)vector * 8u;

    idt[index] = 0x80u;
    idt[index + 1u] = 0x01u;
    idt[index + 2u] = 0x08u;
    idt[index + 5u] = 0x86u;
}

static void task32_prepare(cpu_instruction_fixture *fixture, task32_case test_case)
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
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0xffu,0,0,0x06u,0,0x89u,0,0,
        0xffu,0,0,0x07u,0,0x89u,0,0,
        0,0,0x30u,0,0,0x85u,0,0,
        0x17u,0,0,0x09u,0,0x82u,0,0
    };
    static const lib_u8 ldt_base[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0
    };
    static const lib_u8 target_halt[] = {0xf4u};
    static const lib_u8 gdt_pointer[] = {0x47u,0,0,0x03u,0,0};
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
    t_cpu *const cpu = &fixture->cpu;

    cpu_instruction_prepare(fixture, CORE_MACHINE_CPU_PROFILE_80386);
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
    if (test_case == TASK32_NESTED_INVALID_CODE) {
        target.cs.selector = 0x10u;
        fault_vector = 10u;
    }
    if (test_case == TASK32_TARGET_BUSY) {
        gdt[0x35u] = 0x8bu;
        fault_vector = 13u;
    }
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
    fixture->memory[TASK32_CODE_BASE + 0x180u] = 0xf4u;
    lib_memory_copy(fixture->memory + 0x0100u, gdt_pointer, sizeof(gdt_pointer));
    lib_memory_copy(fixture->memory + TASK32_GDT_BASE, gdt, sizeof(gdt));
    lib_memory_copy(fixture->memory + TASK32_IDT_BASE, idt, sizeof(idt));
    lib_memory_copy(fixture->memory + TASK32_B_BASE + 0x1cu, &target,
        sizeof(target));
    lib_memory_copy(fixture->memory + TASK32_CODE_BASE + 0x100u, target_halt,
        sizeof(target_halt));
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
        core_machine_cpu_execution_refresh(&fixture->execution);
    if (test_case == TASK32_OLD_SHORT) fixture->cpu.data.tr.limit = 0x60u;
    for (index = 0u; index < 20u; ++index)
        core_machine_cpu_execution_refresh(&fixture->execution);
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

static lib_bool task32_expect(task32_case test_case)
{
    cpu_instruction_fixture fixture;
    t_cpu after = {0};
    task32_state outgoing = {0};
    const core_machine_cpu_fault_snapshot *snapshot;
    const lib_bool ldt = task32_is_ldt(test_case);
    const lib_bool rejection = task32_is_rejection(test_case);
    const lib_bool nested = task32_is_nested(test_case);
    const lib_u32 expected_fault = test_case == TASK32_TARGET_BUSY ||
        test_case == TASK32_NESTED_TARGET_BUSY ?
        VCPUINS_EXCEPT_GP : test_case == TASK32_STACK_LIMIT ||
        test_case == TASK32_NESTED_STACK_LIMIT ?
        VCPUINS_EXCEPT_SS : test_case == TASK32_LDT_NOT_PRESENT ?
        VCPUINS_EXCEPT_NP : VCPUINS_EXCEPT_TS;

    task32_prepare(&fixture, test_case);
    task32_refresh(&fixture, test_case);
    after = fixture.cpu;
    snapshot = fixture.fault.valid ? &fixture.fault : &fixture.delivered_exception;
    lib_memory_copy(&outgoing, fixture.memory + (test_case == TASK32_NESTED_RETURN ?
        TASK32_B_BASE : TASK32_A_BASE) + 0x1cu,
        sizeof(outgoing));
    if (test_case == TASK32_LOCK_DIRECT || test_case == TASK32_LOCK_INDIRECT)
        return fixture.fault.valid &&
            (fixture.fault.exception_mask & VCPUINS_EXCEPT_UD) != 0u &&
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

int main(void)
{
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
        TASK32_NESTED_STACK_LIMIT
    };
    lib_size index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
        if (!task32_expect(cases[index])) {
            lib_c_fprintf(lib_c_stderr, "task32 failed case=%u\n", (unsigned)cases[index]);
            lib_c_fprintf(lib_c_stderr, "%s", "M5:T539:S57:TASK32-STATE:FAIL\n");
            return 1;
        }
    lib_c_printf("%s\n", "M5:T539:S57:TASK32-STATE:OK");
    lib_c_printf("%s\n", "M5:T539:S60:TASK32-NESTING:OK");
    return 0;
}
