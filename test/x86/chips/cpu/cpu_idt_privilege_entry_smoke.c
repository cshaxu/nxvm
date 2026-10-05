#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

#define IDT_GDT_BASE 0x0300u
#define IDT_IDT_BASE 0x0400u
#define IDT_TSS_BASE 0x0600u
#define IDT_KERNEL_CODE_BASE 0x2000u
#define IDT_USER_CODE_BASE 0x3000u
#define IDT_HANDLER_OFFSET 0x0100u
#define IDT_VECTOR 0x30u

/* CPU cache rollback and NMI latches stay here; PIC delivery and public
 * Core diagnostics stay with the board receiver. */
static void idt_prepare(cpu_instruction_fixture *state, lib_u8 gate_access,
    lib_u8 stack_access, lib_bool stack_big)
{
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,(lib_u8)(stack_big ? 0xcfu : 0x8fu),0,
        0xffu,0xffu,0,0x30u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0xf2u,0xcfu,0,
        0x67u,0,0,0x06u,0,0x8bu,0,0
    };
    lib_u8 idt[IDT_VECTOR * 8u + 8u] = {0};
    lib_u8 tss[10] = {0};
    const lib_u8 program[] = {0xcdu,IDT_VECTOR};
    const lib_u8 handler[] = {0xf4u};
    const lib_u32 esp0 = 0x00009000u;
    const lib_u16 ss0 = 0x0010u;
    t_cpu *const cpu = &state->cpu;

    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    gdt[21u] = stack_access;
    idt[IDT_VECTOR * 8u] = IDT_HANDLER_OFFSET & 0xffu;
    idt[IDT_VECTOR * 8u + 1u] = IDT_HANDLER_OFFSET >> 8u;
    idt[IDT_VECTOR * 8u + 2u] = 0x08u;
    idt[IDT_VECTOR * 8u + 5u] = gate_access;
    lib_memory_copy(tss + 4u, &esp0, sizeof(esp0));
    lib_memory_copy(tss + 8u, &ss0, sizeof(ss0));
    lib_memory_copy(state->memory + IDT_GDT_BASE, gdt, sizeof(gdt));
    lib_memory_copy(state->memory + IDT_IDT_BASE, idt, sizeof(idt));
    lib_memory_copy(state->memory + IDT_TSS_BASE, tss, sizeof(tss));
    lib_memory_copy(state->memory + IDT_USER_CODE_BASE, program, sizeof(program));
    lib_memory_copy(state->memory + IDT_KERNEL_CODE_BASE + IDT_HANDLER_OFFSET,
        handler, sizeof(handler));
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = IDT_GDT_BASE;
    cpu->data.gdtr.limit = (lib_u16)(sizeof(gdt) - 1u);
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = IDT_IDT_BASE;
    cpu->data.idtr.limit = (lib_u16)(sizeof(idt) - 1u);
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x001bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = IDT_USER_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.defsize = LIB_TRUE;
    cpu->data.cs.seg.exec.conform = LIB_FALSE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x0023u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffffffu;
    cpu->data.ss.dpl = 3u;
    cpu->data.ss.seg.data.big = LIB_TRUE;
    cpu->data.ss.seg.data.expdown = LIB_FALSE;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.tr.flagValid = LIB_TRUE;
    cpu->data.tr.selector = 0x0028u;
    cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.base = IDT_TSS_BASE;
    cpu->data.tr.limit = 0x67u;
    cpu->data.tr.dpl = 0u;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    cpu->data.eip = 0u;
    cpu->data.esp = 0x00008800u;
    cpu->data.eflags = 0x00000302u;
    cpu->data.flagHalt = LIB_FALSE;
}

static lib_bool idt_run(cpu_instruction_fixture *state, t_cpu *after)
{
    core_machine_cpu_execution_refresh(&state->execution);
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return !state->execution.stop_requested && !state->fault.valid;
}

static lib_bool idt_fault(cpu_instruction_fixture *state, t_cpu *after)
{
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return state->execution.stop_requested && state->fault.valid &&
        (state->fault.exception_mask & VCPUINS_EXCEPT_DF) != 0u &&
        state->fault.exception_code == 0u;
}

static lib_bool idt_test_success(lib_u8 gate_access, lib_bool expect_if)
{
    cpu_instruction_fixture state;
    t_cpu after;
    lib_u32 frame[5] = {0};

    idt_prepare(&state, gate_access, 0x92u, LIB_TRUE);
    if (!idt_run(&state, &after) || after.data.cs.selector != 0x0008u ||
        after.data.cs.dpl != 0u || after.data.eip != IDT_HANDLER_OFFSET + 1u ||
        after.data.ss.selector != 0x0010u || after.data.ss.dpl != 0u ||
        after.data.esp != 0x00008fecu ||
        (after.data.eflags & VCPU_EFLAGS_TF) != 0u ||
        ((after.data.eflags & VCPU_EFLAGS_IF) != 0u) != expect_if)
        return LIB_FALSE;
    lib_memory_copy(frame, state.memory + 0x00008fecu, sizeof(frame));
    return frame[0] == 2u && frame[1] == 0x0000001bu &&
        frame[2] == 0x00000302u && frame[3] == 0x00008800u &&
        frame[4] == 0x00000023u && state.memory[IDT_GDT_BASE + 13u] == 0x9bu &&
        state.memory[IDT_GDT_BASE + 21u] == 0x93u;
}

static lib_bool idt_test_16bit_stack(void)
{
    cpu_instruction_fixture state;
    t_cpu after;
    lib_u32 frame[5] = {0};

    idt_prepare(&state, 0xeeu, 0x92u, LIB_FALSE);
    if (!idt_run(&state, &after) || after.data.ss.seg.data.big ||
        after.data.esp != 0x00008fecu) return LIB_FALSE;
    lib_memory_copy(frame, state.memory + 0x00008fecu, sizeof(frame));
    return frame[0] == 2u && frame[1] == 0x0000001bu &&
        frame[2] == 0x00000302u && frame[3] == 0x00008800u &&
        frame[4] == 0x00000023u;
}

static lib_bool idt_test_atomic(lib_u8 gate_access, lib_u8 stack_access,
    lib_bool corrupt_code)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 cs_before;
    lib_u8 ss_before;

    idt_prepare(&state, gate_access, stack_access, LIB_TRUE);
    if (corrupt_code) state.memory[IDT_GDT_BASE + 13u] = 0x1au;
    before = state.cpu;
    cs_before = state.memory[IDT_GDT_BASE + 13u];
    ss_before = state.memory[IDT_GDT_BASE + 21u];
    return idt_fault(&state, &after) && after.data.eip == before.data.eip &&
        after.data.esp == before.data.esp && after.data.eflags == before.data.eflags &&
        after.data.cs.selector == before.data.cs.selector &&
        after.data.ss.selector == before.data.ss.selector &&
        state.memory[IDT_GDT_BASE + 13u] == cs_before &&
        state.memory[IDT_GDT_BASE + 21u] == ss_before;
}

static lib_bool idt_test_nmi(lib_bool invalid_gate, lib_bool user_source)
{
    cpu_instruction_fixture state;
    t_cpu before;
    lib_u8 gate[] = {IDT_HANDLER_OFFSET & 0xffu, IDT_HANDLER_OFFSET >> 8u,
        8u,0,0,0x8eu,0,0};
    const lib_u8 loop[] = {0xebu,0xfeu};
    const lib_u32 code_base = user_source ? IDT_USER_CODE_BASE : IDT_KERNEL_CODE_BASE;
    const lib_u16 code_selector = user_source ? 0x1bu : 8u;

    idt_prepare(&state, 0xeeu, 0x92u, LIB_TRUE);
    state.cpu.data.cs.selector = code_selector;
    state.cpu.data.cs.base = code_base;
    state.cpu.data.cs.dpl = user_source ? 3u : 0u;
    state.cpu.data.ss.selector = user_source ? 0x23u : 0x10u;
    state.cpu.data.ss.dpl = user_source ? 3u : 0u;
    state.cpu.data.esp = 0x8000u;
    state.cpu.data.eflags = 0x202u;
    state.cpu.data.flagNMI = LIB_TRUE;
    gate[2] = (lib_u8)code_selector;
    if (invalid_gate) gate[5] = 0x80u;
    lib_memory_copy(state.memory + IDT_IDT_BASE + 2u * 8u, gate, sizeof(gate));
    lib_memory_copy(state.memory + code_base, loop, sizeof(loop));
    lib_memory_copy(state.memory + code_base + IDT_HANDLER_OFFSET,
        user_source ? loop : (const lib_u8[]){0xf4u},
        user_source ? sizeof(loop) : 1u);
    before = state.cpu;
    core_machine_cpu_execution_refresh(&state.execution);
    if (invalid_gate)
        return state.execution.stop_requested && state.fault.valid &&
            (state.fault.exception_mask & VCPUINS_EXCEPT_DF) != 0u &&
            state.cpu.data.flagNMI && state.cpu.data.eip == before.data.eip &&
            state.cpu.data.esp == before.data.esp &&
            state.cpu.data.eflags == before.data.eflags &&
            lib_memory_compare(&state.cpu.data.cs, &before.data.cs,
                sizeof(before.data.cs)) == 0 &&
            lib_memory_compare(&state.cpu.data.ss, &before.data.ss,
                sizeof(before.data.ss)) == 0;
    if (state.execution.stop_requested || state.fault.valid ||
        state.cpu.data.flagNMI || state.cpu.data.esp != 0x7ff4u ||
        state.cpu.data.cs.selector != code_selector ||
        state.cpu.data.cs.dpl != (user_source ? 3u : 0u) ||
        state.cpu.data.eip != IDT_HANDLER_OFFSET) return LIB_FALSE;
    core_machine_cpu_execution_refresh(&state.execution);
    return !state.execution.stop_requested && !state.cpu.data.flagNMI &&
        state.cpu.data.esp == 0x7ff4u &&
        (user_source ? !state.cpu.data.flagHalt &&
            state.cpu.data.eip == IDT_HANDLER_OFFSET : state.cpu.data.flagHalt);
}

static lib_bool idt_test_delivery_cache_rollback(lib_u8 failure)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 gate[] = {IDT_HANDLER_OFFSET & 0xffu, IDT_HANDLER_OFFSET >> 8u,
        0x1bu,0,0,0x8eu,0,0};
    const lib_u8 code[] = {0x0fu,0x01u,0xf0u};
    lib_u32 stack_before[4], stack_after[4];
    lib_u8 access;

    idt_prepare(&state, 0xeeu, 0x92u, LIB_TRUE);
    if (failure == 0u) gate[5] = 0x80u;
    if (failure == 1u) gate[5] = 0x0eu;
    if (failure == 2u) state.memory[IDT_GDT_BASE + 29u] = 0x7au;
    if (failure == 3u) state.cpu.data.ss.limit = state.cpu.data.esp - 2u;
    lib_memory_copy(state.memory + IDT_IDT_BASE + 0x0du * 8u, gate, sizeof(gate));
    lib_memory_copy(state.memory + IDT_USER_CODE_BASE, code, sizeof(code));
    before = state.cpu;
    access = state.memory[IDT_GDT_BASE + 29u];
    lib_memory_copy(stack_before, state.memory + before.data.esp - 16u,
        sizeof(stack_before));
    if (!idt_fault(&state, &after)) return LIB_FALSE;
    lib_memory_copy(stack_after, state.memory + before.data.esp - 16u,
        sizeof(stack_after));
    return !state.delivered_exception.valid &&
        after.data.eip == before.data.eip && after.data.esp == before.data.esp &&
        after.data.eflags == before.data.eflags &&
        lib_memory_compare(&after.data.cs, &before.data.cs, sizeof(before.data.cs)) == 0 &&
        lib_memory_compare(&after.data.ss, &before.data.ss, sizeof(before.data.ss)) == 0 &&
        state.memory[IDT_GDT_BASE + 29u] == access &&
        lib_memory_compare(stack_before, stack_after, sizeof(stack_before)) == 0;
}

static lib_bool idt_test_software_full_rollback(lib_u8 negative)
{
    static const lib_u8 forms[][2] = {{0xccu,0}, {0xcdu,IDT_VECTOR}, {0xceu,0}};
    static const lib_u8 vectors[] = {3u,IDT_VECTOR,4u};
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 gate[] = {IDT_HANDLER_OFFSET & 0xffu, IDT_HANDLER_OFFSET >> 8u,
        8u,0,0,0x8eu,0,0};
    idt_prepare(&state, 0xeeu, 0x92u, LIB_TRUE);
    if (negative < 3u) {
        lib_memory_copy(state.memory + IDT_IDT_BASE + vectors[negative] * 8u,
            gate, sizeof(gate));
        lib_memory_copy(state.memory + IDT_USER_CODE_BASE, forms[negative], 2u);
        state.cpu.data.eflags |= VCPU_EFLAGS_OF;
    } else {
        state.cpu.data.cs.selector = 8u;
        state.cpu.data.cs.base = IDT_KERNEL_CODE_BASE;
        state.cpu.data.cs.dpl = 0u;
        state.cpu.data.ss.selector = 0x10u;
        state.cpu.data.ss.dpl = 0u;
        lib_memory_copy(state.memory + IDT_KERNEL_CODE_BASE, forms[1], 2u);
        if (negative == 3u) state.cpu.data.idtr.limit = 7u;
        if (negative == 4u) state.memory[IDT_GDT_BASE + 13u] = 0x92u;
        if (negative == 5u) {
            state.cpu.data.eflags = VCPU_EFLAGS_VM | VCPU_EFLAGS_CF | 2u;
            core_machine_cpu_execution_load_segment(&state.execution, &state.cpu.data.cs, 0u);
            core_machine_cpu_execution_load_segment(&state.execution, &state.cpu.data.ss, 0u);
            lib_memory_copy(state.memory, forms[1], 2u);
        }
    }
    before = state.cpu;
    return idt_fault(&state, &after) &&
        lib_memory_compare(&before, &after, sizeof(before)) == 0;
}

int main(void)
{
    if (!idt_test_success(0xeeu, LIB_FALSE) ||
        !idt_test_success(0xefu, LIB_TRUE) || !idt_test_16bit_stack() ||
        !idt_test_atomic(0x8eu, 0x92u, LIB_FALSE) ||
        !idt_test_atomic(0xeeu, 0x12u, LIB_FALSE) ||
        !idt_test_atomic(0xeeu, 0x92u, LIB_TRUE) ||
        !idt_test_nmi(LIB_FALSE, LIB_FALSE) || !idt_test_nmi(LIB_TRUE, LIB_FALSE) ||
        !idt_test_nmi(LIB_FALSE, LIB_TRUE) || !idt_test_nmi(LIB_TRUE, LIB_TRUE)) return 1;
    for (lib_u8 failure = 0u; failure < 4u; ++failure)
        if (!idt_test_delivery_cache_rollback(failure)) return 1;
    for (lib_u8 negative = 0u; negative < 6u; ++negative)
        if (!idt_test_software_full_rollback(negative)) return 1;
    lib_c_printf("%s\n", "M5:T307:IDT-PRIVILEGE-ENTRY:CPU:OK");
    return 0;
}
