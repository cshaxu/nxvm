#include "support/cpu_instruction_fixture.h"
#include "app-nxvm/devices/device_support.h"
#include <stdio.h>

#define IDT_GDT_BASE 0x0300u
#define IDT_IDT_BASE 0x0400u
#define IDT_TSS_BASE 0x0600u
#define IDT_KERNEL_CODE_BASE 0x2000u
#define IDT_USER_CODE_BASE 0x3000u
#define IDT_HANDLER_OFFSET 0x0100u
#define IDT_VECTOR 0x30u

/* This receiver owns only CPU-local software INT privilege transfer.  The
 * corresponding PIC delivery case remains a board receiver. */
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
        CORE_MACHINE_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_TF) ||
        CORE_MACHINE_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_IF) != expect_if)
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

int main(void)
{
    if (!idt_test_success(0xeeu, LIB_FALSE) ||
        !idt_test_success(0xefu, LIB_TRUE) || !idt_test_16bit_stack() ||
        !idt_test_atomic(0x8eu, 0x92u, LIB_FALSE) ||
        !idt_test_atomic(0xeeu, 0x12u, LIB_FALSE) ||
        !idt_test_atomic(0xeeu, 0x92u, LIB_TRUE)) return 1;
    puts("M5:T307:IDT-PRIVILEGE-ENTRY:CPU:OK");
    return 0;
}
