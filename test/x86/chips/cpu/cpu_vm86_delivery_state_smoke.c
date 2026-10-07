#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

#define VM86_GDT_BASE 0x0300u
#define VM86_IDT_BASE 0x0400u
#define VM86_TSS_BASE 0x0600u
#define VM86_HANDLER_BASE 0x2100u
#define VM86_STACK_TOP 0x9000u
#define VM86_PAGE_DIRECTORY 0xa000u
#define VM86_PAGE_TABLE 0xb000u
#define VM86_PAGE_FLAGS 0x00000007u


/* Full cached CPU rollback belongs to the CPU owner, not the PC board. */
static lib_i32 vm86_delivery_prepare(cpu_instruction_fixture *state, lib_u8 vector)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0, 0x67u,0,0,0x06u,0,0x8bu,0,0
    };
    lib_u8 idt[0x108u] = {0u};
    lib_u8 tss[12u] = {0u};
    lib_u32 esp0 = VM86_STACK_TOP;
    lib_u16 ss0 = 0x0010u;
    t_cpu *cpu;

    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    idt[vector * 8u] = 0u; idt[vector * 8u + 1u] = 0x01u;
    idt[vector * 8u + 2u] = 0x08u; idt[vector * 8u + 5u] = 0x8eu;
    idt[8u] = 0u; idt[9u] = 0x01u; idt[10u] = 0x08u; idt[13u] = 0x8eu;
    lib_memory_copy(&tss[4u], &esp0, sizeof(esp0)); lib_memory_copy(&tss[8u], &ss0, sizeof(ss0));
    lib_memory_copy(state->memory + VM86_GDT_BASE, gdt, sizeof(gdt));
    lib_memory_copy(state->memory + VM86_IDT_BASE, idt, sizeof(idt));
    lib_memory_copy(state->memory + VM86_TSS_BASE, tss, sizeof(tss));
    state->memory[VM86_HANDLER_BASE] = 0xf4u;
    cpu = &state->cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.eflags = CORE_MACHINE_DEBUG_EFLAGS_VM | CORE_MACHINE_DEBUG_EFLAGS_IF;
    cpu->data.idtr.flagValid = LIB_TRUE; cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = VM86_IDT_BASE; cpu->data.idtr.limit = sizeof(idt) - 1u;
    cpu->data.gdtr.flagValid = LIB_TRUE; cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = VM86_GDT_BASE; cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.tr.flagValid = LIB_TRUE; cpu->data.tr.selector = 0x0018u; cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY; cpu->data.tr.base = VM86_TSS_BASE;
    cpu->data.tr.limit = sizeof(tss) - 1u; cpu->data.tr.dpl = 0u;
    cpu->data.cs.selector = 0x0200u; cpu->data.cs.base = 0x2000u; cpu->data.cs.limit = 0xffffu;
    cpu->data.ss.selector = 0x0300u; cpu->data.ss.base = 0x3000u; cpu->data.ss.limit = 0xffffu;
    cpu->data.ds.selector = 0x0400u; cpu->data.ds.base = 0x4000u; cpu->data.ds.limit = 0xffffu;
    cpu->data.es.selector = 0x0500u; cpu->data.es.base = 0x5000u; cpu->data.es.limit = 0xffffu;
    cpu->data.fs.selector = 0x0600u; cpu->data.fs.base = 0x6000u; cpu->data.fs.limit = 0xffffu;
    cpu->data.gs.selector = 0x0700u; cpu->data.gs.base = 0x7000u; cpu->data.gs.limit = 0xffffu;
    cpu->data.esp = 0x00001234u;
    cpu->data.cs.flagValid = LIB_TRUE; cpu->data.cs.sregtype = SREG_CODE; cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE; cpu->data.ss.sregtype = SREG_STACK; cpu->data.ss.dpl = 3u;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.ds.flagValid = LIB_TRUE; cpu->data.ds.sregtype = SREG_DATA; cpu->data.ds.dpl = 3u;
    cpu->data.es.flagValid = LIB_TRUE; cpu->data.es.sregtype = SREG_DATA; cpu->data.es.dpl = 3u;
    cpu->data.fs.flagValid = LIB_TRUE; cpu->data.fs.sregtype = SREG_DATA; cpu->data.fs.dpl = 3u;
    cpu->data.gs.flagValid = LIB_TRUE; cpu->data.gs.sregtype = SREG_DATA; cpu->data.gs.dpl = 3u;
    return 1;
}

static lib_status vm86_state_write(cpu_instruction_fixture *state,
    lib_u32 address, const void *data, lib_size bytes)
{
    if (address > sizeof(state->memory) ||
        bytes > sizeof(state->memory) - address) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(state->memory + address, data, bytes);
    return LIB_STATUS_OK;
}
static lib_i32 vm86_delivery_expect_prepublication(cpu_instruction_fixture *state)
{
    const lib_u8 ud[] = {0x0fu,0x0bu};
    t_cpu before, after;
    lib_u8 stack_before[40], stack_after[40];
    lib_memory_copy(state->memory + 0x2000u, ud, sizeof(ud));
    before = state->cpu;
    lib_memory_copy(stack_before, state->memory + VM86_STACK_TOP - 40u, sizeof(stack_before));
    core_machine_cpu_execution_refresh(&state->execution);
    after = state->cpu;
    lib_memory_copy(stack_after, state->memory + VM86_STACK_TOP - 40u, sizeof(stack_after));
    return state->execution.stop_requested && state->fault.valid &&
        lib_memory_compare(&before, &after, sizeof(before)) == 0 &&
        lib_memory_compare(stack_before, stack_after, sizeof(stack_before)) == 0;
}
static lib_i32 vm86_delivery_invalid_gate(void)
{
    cpu_instruction_fixture state; lib_i32 failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        lib_u8 absent[8u] = {0u};
        failed |= vm86_state_write(&state, VM86_IDT_BASE + 6u * 8u, absent,
            sizeof(absent)) != LIB_STATUS_OK || !vm86_delivery_expect_prepublication(&state);
    }
    return !failed;
}
static lib_i32 vm86_delivery_bad_gate_access(lib_u8 access)
{
    cpu_instruction_fixture state; lib_i32 failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        failed |= vm86_state_write(&state, VM86_IDT_BASE + 6u * 8u + 5u,
            &access, sizeof(access)) != LIB_STATUS_OK ||
            !vm86_delivery_expect_prepublication(&state);
    }
    return !failed;
}
static lib_i32 vm86_delivery_invalid_tss(void)
{
    cpu_instruction_fixture state; lib_i32 failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        state.cpu.data.tr.flagValid = LIB_FALSE;
        failed |= !vm86_delivery_expect_prepublication(&state);
    }
    return !failed;
}
static lib_i32 vm86_delivery_bad_tss(lib_i32 short_tss)
{
    cpu_instruction_fixture state; lib_i32 failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        if (short_tss) state.cpu.data.tr.limit = 7u;
        else state.cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_AVL;
        failed |= !vm86_delivery_expect_prepublication(&state);
    }
    return !failed;
}
static lib_i32 vm86_delivery_invalid_ss0(void)
{
    cpu_instruction_fixture state; lib_u16 ss0 = 0u;
    lib_i32 failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        failed |= vm86_state_write(&state, VM86_TSS_BASE + 8u, &ss0,
            sizeof(ss0)) != LIB_STATUS_OK || !vm86_delivery_expect_prepublication(&state);
    }
    return !failed;
}
static lib_i32 vm86_delivery_bad_ss0(lib_u16 ss0, lib_u8 access,
    lib_u32 esp0)
{
    cpu_instruction_fixture state; lib_i32 failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        failed |= vm86_state_write(&state, VM86_TSS_BASE + 4u, &esp0,
            sizeof(esp0)) != LIB_STATUS_OK || vm86_state_write(&state,
            VM86_TSS_BASE + 8u, &ss0, sizeof(ss0)) != LIB_STATUS_OK ||
            vm86_state_write(&state, VM86_GDT_BASE + 16u + 5u, &access,
            sizeof(access)) != LIB_STATUS_OK || !vm86_delivery_expect_prepublication(&state);
    }
    return !failed;
}
static lib_i32 vm86_delivery_short_stack(void)
{
    cpu_instruction_fixture state; lib_u8 limit_lo = 0x1fu;
    lib_u8 limit_hi = 0u; lib_u8 flags = 0x40u;
    lib_u32 esp0 = 0x00000020u;
    lib_i32 failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        failed |= vm86_state_write(&state, VM86_TSS_BASE + 4u, &esp0,
            sizeof(esp0)) != LIB_STATUS_OK || vm86_state_write(&state,
            VM86_GDT_BASE + 16u, &limit_lo, sizeof(limit_lo)) != LIB_STATUS_OK ||
            vm86_state_write(&state, VM86_GDT_BASE + 17u, &limit_hi,
            sizeof(limit_hi)) != LIB_STATUS_OK || vm86_state_write(&state,
            VM86_GDT_BASE + 22u, &flags, sizeof(flags)) != LIB_STATUS_OK ||
            !vm86_delivery_expect_prepublication(&state);
    }
    return !failed;
}

static lib_bool vm86_state_delivery(lib_u8 vector, const lib_u8 *code,
    lib_u8 bytes, lib_bool trap, lib_bool breakpoint)
{
    cpu_instruction_fixture state;
    lib_u32 frame[10] = {0};
    const lib_bool error_frame = vector == 13u;
    if (!vm86_delivery_prepare(&state, vector)) return LIB_FALSE;
    lib_memory_copy(state.memory + 0x2000u, code, bytes);
    if (vector == 7u) state.cpu.data.cr0 |= VCPU_CR0_EM;
    if (trap) state.cpu.data.eflags |= VCPU_EFLAGS_TF;
    if (breakpoint) {
        state.cpu.data.dr0 = 0x2000u;
        state.cpu.data.dr6 = 0u;
        state.cpu.data.dr7 = 1u;
    }
    core_machine_cpu_execution_refresh(&state.execution);
    core_machine_cpu_execution_refresh(&state.execution);
    lib_memory_copy(frame, state.memory + VM86_STACK_TOP -
        (error_frame ? 40u : 36u), sizeof(frame));
    return !state.execution.stop_requested && !state.fault.valid &&
        state.delivered_exception.valid &&
        state.delivered_exception.exception_mask == (1u << vector) &&
        state.cpu.data.eip == 0x101u && state.cpu.data.cs.selector == 8u &&
        state.cpu.data.ss.selector == 0x10u &&
        !state.cpu.data.es.flagValid && !state.cpu.data.ds.flagValid &&
        !state.cpu.data.fs.flagValid && !state.cpu.data.gs.flagValid &&
        state.cpu.data.esp == VM86_STACK_TOP - (error_frame ? 40u : 36u) &&
        (state.cpu.data.eflags & (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF)) == 0u &&
        (!breakpoint || (state.cpu.data.dr6 & 1u) != 0u) &&
        frame[error_frame ? 1u : 0u] == (trap ? 1u : 0u) &&
        frame[error_frame ? 2u : 1u] == 0x200u &&
        frame[error_frame ? 3u : 2u] == (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF |
            (trap ? VCPU_EFLAGS_TF : VCPU_EFLAGS_RF)) &&
        frame[error_frame ? 4u : 3u] == 0x1234u &&
        frame[error_frame ? 5u : 4u] == 0x300u &&
        frame[error_frame ? 6u : 5u] == 0x500u &&
        frame[error_frame ? 7u : 6u] == 0x400u &&
        frame[error_frame ? 8u : 7u] == 0x600u &&
        frame[error_frame ? 9u : 8u] == 0x700u;
}

static lib_bool vm86_state_nmi(lib_bool masked)
{
    cpu_instruction_fixture state;
    const lib_u8 nop = 0x90u;
    if (!vm86_delivery_prepare(&state, 2u)) return LIB_FALSE;
    state.memory[0x2000u] = nop;
    state.execution.nmi_pending = LIB_TRUE;
    state.execution.nmi_masked = masked;
    core_machine_cpu_execution_refresh(&state.execution);
    if (!masked) core_machine_cpu_execution_refresh(&state.execution);
    return !state.execution.stop_requested && !state.fault.valid &&
        state.execution.nmi_pending == masked &&
        state.cpu.data.eip == (masked ? 1u : 0x101u) &&
        state.cpu.data.cs.selector == (masked ? 0x200u : 8u) &&
        state.cpu.data.esp == (masked ? 0x1234u : VM86_STACK_TOP - 36u);
}

typedef struct vm86_irq_state {
    cpu_instruction_fixture fixture;
    lib_bool pending;
} vm86_irq_state;

static lib_bool vm86_irq_pending(void *opaque)
{
    return ((vm86_irq_state *)opaque)->pending;
}

static lib_status vm86_irq_acknowledge(void *opaque, lib_u8 *vector)
{
    ((vm86_irq_state *)opaque)->pending = LIB_FALSE;
    *vector = 0x20u;
    return LIB_STATUS_OK;
}

static lib_bool vm86_state_irq(void)
{
    vm86_irq_state state;
    const core_machine_instruction_timing timing = {.base_ticks = 1u};
    const core_machine_cpu_bus_provider bus = {
        .read_memory = cpu_instruction_read,
        .write_memory = cpu_instruction_write,
        .interrupt_pending = vm86_irq_pending,
        .acknowledge_interrupt = vm86_irq_acknowledge
    };
    cpu_instruction_fixture *const fixture = &state.fixture;
    lib_u32 frame[9] = {0};
    if (!vm86_delivery_prepare(fixture, 0x20u)) return LIB_FALSE;
    fixture->memory[0x2000u] = 0x90u;
    state.pending = LIB_TRUE;
    /* The first member is the CPU fixture consumed by its memory callbacks.
     * The bus owns the synthetic IRQ line; no PIC state is mirrored here. */
    core_machine_cpu_execution_context_initialize(&fixture->execution,
        &fixture->cpu, &fixture->instructions, &bus, &state);
    core_machine_cpu_execution_context_bind_profiles(&fixture->execution,
        CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_execution_context_bind_diagnostic_provider(&fixture->execution,
        &cpu_instruction_diagnostics, fixture);
    core_machine_cpu_execution_refresh(&fixture->execution);
    core_machine_cpu_execution_refresh(&fixture->execution);
    lib_memory_copy(frame, fixture->memory + VM86_STACK_TOP - 36u, sizeof(frame));
    return !fixture->execution.stop_requested && !fixture->fault.valid &&
        !state.pending && !fixture->cpu.data.es.flagValid &&
        !fixture->cpu.data.ds.flagValid && !fixture->cpu.data.fs.flagValid &&
        !fixture->cpu.data.gs.flagValid && fixture->cpu.data.eip == 0x101u &&
        fixture->cpu.data.esp == VM86_STACK_TOP - 36u &&
        frame[0] == 1u && frame[1] == 0x200u &&
        frame[2] == (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF) &&
        frame[3] == 0x1234u && frame[4] == 0x300u && frame[5] == 0x500u &&
        frame[6] == 0x400u && frame[7] == 0x600u && frame[8] == 0x700u;
}

lib_i32 main(void)
{
    static const lib_u8 ud[] = {0x0fu,0x0bu};
    static const lib_u8 gp[] = {0xfau};
    static const lib_u8 nm[] = {0xd8u,0xc0u};
    static const lib_u8 nop[] = {0x90u};
    if (!vm86_state_irq() || !vm86_state_nmi(LIB_FALSE) || !vm86_state_nmi(LIB_TRUE) ||
        !vm86_state_delivery(6u, ud, sizeof(ud), LIB_FALSE, LIB_FALSE) ||
        !vm86_state_delivery(13u, gp, sizeof(gp), LIB_FALSE, LIB_FALSE) ||
        !vm86_state_delivery(7u, nm, sizeof(nm), LIB_FALSE, LIB_FALSE) ||
        !vm86_state_delivery(1u, nop, sizeof(nop), LIB_TRUE, LIB_FALSE) ||
        !vm86_state_delivery(1u, nop, sizeof(nop), LIB_FALSE, LIB_TRUE)) return 1;
    if (!vm86_delivery_invalid_gate() ||
        !vm86_delivery_bad_gate_access(0x0eu) ||
        !vm86_delivery_bad_gate_access(0x80u) ||
        !vm86_delivery_invalid_tss() || !vm86_delivery_bad_tss(0) ||
        !vm86_delivery_bad_tss(1) || !vm86_delivery_invalid_ss0() ||
        !vm86_delivery_bad_ss0(0x0013u, 0x92u, VM86_STACK_TOP) ||
        !vm86_delivery_bad_ss0(0x0010u, 0x12u, VM86_STACK_TOP) ||
        !vm86_delivery_bad_ss0(0x0010u, 0x90u, VM86_STACK_TOP) ||
        !vm86_delivery_short_stack()) return 1;
    lib_c_printf("M5:T540:S93:VM86-FULL-ROLLBACK:OK\n");
    return 0;
}
