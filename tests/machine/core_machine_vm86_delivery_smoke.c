#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"
#include "../support/core_machine_cpu_fixture.h"

#define VM86_GDT_BASE 0x0300u
#define VM86_IDT_BASE 0x0400u
#define VM86_TSS_BASE 0x0600u
#define VM86_HANDLER_BASE 0x2100u
#define VM86_STACK_TOP 0x9000u
#define VM86_PAGE_DIRECTORY 0xa000u
#define VM86_PAGE_TABLE 0xb000u
#define VM86_PAGE_FLAGS 0x00000007u

typedef struct vm86_delivery_state { core_machine *machine; } vm86_delivery_state;

static C_VOID vm86_delivery_reset(C_VOID *opaque)
{
    vm86_delivery_state *state = (vm86_delivery_state *)opaque;
    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}
static const core_machine_execution_provider vm86_delivery_provider = {
    vm86_delivery_reset, STD_NULL
};

static C_INT vm86_delivery_write_u32(core_machine *machine,
    type_unsigned_32 address, type_unsigned_32 value)
{
    return core_machine_memory_write(machine, address, &value, sizeof(value)) ==
        TYPE_STATUS_OK;
}
static C_INT vm86_delivery_prepare(vm86_delivery_state *state, type_unsigned_8 vector)
{
    const core_machine_config config = {
        .memory_bytes = 0x100000u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .clock_plan.dma = { 1u, 1000000u, 0u },
        .clock_plan.pit = { 1u, 1000000u, 0u },
        .clock_plan.rtc = { 1u, 1000000u, 0u },
        .clock_plan.vadp = { 1u, 1000000u, 0u },
        .clock_plan.kbc = { 1u, 1000000u, 0u },
        .clock_plan.provider = { 1u, 1000000u, 0u }
    };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0, 0x67u,0,0,0x06u,0,0x8bu,0,0
    };
    type_unsigned_8 idt[0x108u] = {0u};
    type_unsigned_8 tss[12u] = {0u};
    type_unsigned_32 esp0 = VM86_STACK_TOP;
    type_unsigned_16 ss0 = 0x0010u;
    t_cpu *cpu;

    STD_MEMSET(state, 0, sizeof(*state));
    idt[vector * 8u] = 0u; idt[vector * 8u + 1u] = 0x01u;
    idt[vector * 8u + 2u] = 0x08u; idt[vector * 8u + 5u] = 0x8eu;
    idt[8u] = 0u; idt[9u] = 0x01u; idt[10u] = 0x08u; idt[13u] = 0x8eu;
    STD_MEMCPY(&tss[4u], &esp0, sizeof(esp0)); STD_MEMCPY(&tss[8u], &ss0, sizeof(ss0));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &vm86_delivery_provider, state) ||
        core_machine_memory_write(state->machine, VM86_GDT_BASE, gdt, sizeof(gdt)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, VM86_IDT_BASE, idt, sizeof(idt)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, VM86_TSS_BASE, tss, sizeof(tss)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, VM86_HANDLER_BASE,
            (const type_unsigned_8[]){0xf4u}, 1u) != TYPE_STATUS_OK) return 0;
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.eflags = VCPU_EFLAGS_VM | VCPU_EFLAGS_IF;
    cpu->data.idtr.flagValid = TYPE_TRUE; cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = VM86_IDT_BASE; cpu->data.idtr.limit = sizeof(idt) - 1u;
    cpu->data.gdtr.flagValid = TYPE_TRUE; cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = VM86_GDT_BASE; cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.tr.flagValid = TYPE_TRUE; cpu->data.tr.selector = 0x0018u; cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY; cpu->data.tr.base = VM86_TSS_BASE;
    cpu->data.tr.limit = sizeof(tss) - 1u; cpu->data.tr.dpl = 0u;
    cpu->data.cs.selector = 0x0200u; cpu->data.cs.base = 0x2000u; cpu->data.cs.limit = 0xffffu;
    cpu->data.ss.selector = 0x0300u; cpu->data.ss.base = 0x3000u; cpu->data.ss.limit = 0xffffu;
    cpu->data.ds.selector = 0x0400u; cpu->data.ds.base = 0x4000u; cpu->data.ds.limit = 0xffffu;
    cpu->data.es.selector = 0x0500u; cpu->data.es.base = 0x5000u; cpu->data.es.limit = 0xffffu;
    cpu->data.fs.selector = 0x0600u; cpu->data.fs.base = 0x6000u; cpu->data.fs.limit = 0xffffu;
    cpu->data.gs.selector = 0x0700u; cpu->data.gs.base = 0x7000u; cpu->data.gs.limit = 0xffffu;
    cpu->data.esp = 0x00001234u;
    cpu->data.cs.flagValid = TYPE_TRUE; cpu->data.cs.sregtype = SREG_CODE; cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.ss.flagValid = TYPE_TRUE; cpu->data.ss.sregtype = SREG_STACK; cpu->data.ss.dpl = 3u;
    cpu->data.ss.seg.data.writable = TYPE_TRUE;
    cpu->data.ds.flagValid = TYPE_TRUE; cpu->data.ds.sregtype = SREG_DATA; cpu->data.ds.dpl = 3u;
    cpu->data.es.flagValid = TYPE_TRUE; cpu->data.es.sregtype = SREG_DATA; cpu->data.es.dpl = 3u;
    cpu->data.fs.flagValid = TYPE_TRUE; cpu->data.fs.sregtype = SREG_DATA; cpu->data.fs.dpl = 3u;
    cpu->data.gs.flagValid = TYPE_TRUE; cpu->data.gs.sregtype = SREG_DATA; cpu->data.gs.dpl = 3u;
    return 1;
}
static C_INT vm86_delivery_fault(type_unsigned_8 vector, const type_unsigned_8 *code,
    STD_SIZE_T bytes, C_INT error_frame)
{
    vm86_delivery_state state; core_machine_run_result result; core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 frame[10u] = {0u}; C_INT failed = !vm86_delivery_prepare(&state, vector);
    if (!failed && vector == 7u) TYPE_SET_BIT(state.machine->executor_cpu.data.cr0, VCPU_CR0_EM);
    if (!failed) failed |= core_machine_memory_write(state.machine, 0x2000u, code, bytes) != TYPE_STATUS_OK ||
        core_machine_run(state.machine, (core_machine_run_budget){8u,0u}, &result) != TYPE_STATUS_OK ||
        core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
        diagnostic.last_delivered_exception.exception_mask != (1u << vector) ||
        TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_VM) ||
        TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) ||
        TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
        state.machine->executor_cpu.data.cs.selector != 0x0008u || state.machine->executor_cpu.data.ss.selector != 0x0010u ||
        state.machine->executor_cpu.data.eip != 0x101u || state.machine->executor_cpu.data.es.flagValid ||
        state.machine->executor_cpu.data.ds.flagValid || state.machine->executor_cpu.data.fs.flagValid ||
        state.machine->executor_cpu.data.gs.flagValid ||
        state.machine->executor_cpu.data.esp != VM86_STACK_TOP - (error_frame ? 40u : 36u) ||
        core_machine_memory_read_physical(&state.machine->executor_memory, VM86_STACK_TOP - (error_frame ? 40u : 36u),
            (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK || frame[0] != 0u ||
        frame[error_frame ? 1u : 0u] != 0u || frame[error_frame ? 2u : 1u] != 0x0200u ||
        frame[error_frame ? 3u : 2u] != (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF) ||
        frame[error_frame ? 4u : 3u] != 0x1234u || frame[error_frame ? 5u : 4u] != 0x0300u ||
        frame[error_frame ? 6u : 5u] != 0x0500u || frame[error_frame ? 7u : 6u] != 0x0400u ||
        frame[error_frame ? 8u : 7u] != 0x0600u || frame[error_frame ? 9u : 8u] != 0x0700u;
    core_machine_destroy(state.machine); return !failed;
}
static C_INT vm86_delivery_debug_tf(C_VOID)
{
    vm86_delivery_state state; core_machine_run_result result; core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 frame[9u] = {0u}; C_INT failed = !vm86_delivery_prepare(&state, 1u);
    if (!failed) {
        TYPE_SET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF);
        failed |= core_machine_memory_write(state.machine, 0x2000u,
            (const type_unsigned_8[]){0x90u}, 1u) != TYPE_STATUS_OK ||
            core_machine_run(state.machine, (core_machine_run_budget){4u,0u}, &result) != TYPE_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_VM) ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
            state.machine->executor_cpu.data.eip != 0x101u ||
            core_machine_memory_read_physical(&state.machine->executor_memory, VM86_STACK_TOP - 36u,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK || frame[0] != 1u ||
            frame[2u] != (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF);
    }
    core_machine_destroy(state.machine); return !failed;
}
static C_INT vm86_delivery_debug_breakpoint(C_VOID)
{
    vm86_delivery_state state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 frame[9u] = {0u};
    C_INT failed = !vm86_delivery_prepare(&state, 1u);

    if (!failed) {
        state.machine->executor_cpu.data.dr0 = 0x2000u;
        state.machine->executor_cpu.data.dr6 = 0u;
        state.machine->executor_cpu.data.dr7 = 0x00000001u;
        failed |= core_machine_memory_write(state.machine, 0x2000u,
            (const type_unsigned_8[]){0x90u}, 1u) != TYPE_STATUS_OK ||
            core_machine_run(state.machine, (core_machine_run_budget){4u,0u},
                &result) != TYPE_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask !=
                VCPUINS_EXCEPT_DB || state.machine->executor_cpu.data.eip !=
                0x101u || (state.machine->executor_cpu.data.dr6 & 1u) == 0u ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                VM86_STACK_TOP - 36u, (type_virtual_address)frame,
                sizeof(frame)) != TYPE_STATUS_OK || frame[0] != 0u ||
            frame[2u] != (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF | VCPU_EFLAGS_RF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}
static C_INT vm86_delivery_irq0(C_VOID)
{
    vm86_delivery_state state; core_machine_pic_irq_source irq; core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic; type_unsigned_32 frame[9u] = {0u}; C_INT failed = !vm86_delivery_prepare(&state, 0x20u);
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x2000u,
            (const type_unsigned_8[]){0x90u,0xf4u}, 2u) != TYPE_STATUS_OK;
        STD_MEMSET(&irq, 0, sizeof(irq)); state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u); core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){4u,0u}, &result) != TYPE_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
        state.machine->executor_cpu.data.eip != 0x101u || state.machine->executor_cpu.data.esp != VM86_STACK_TOP - 36u ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_VM) ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
            state.machine->executor_cpu.data.cs.selector != 0x0008u ||
            state.machine->executor_cpu.data.ss.selector != 0x0010u ||
            state.machine->executor_cpu.data.es.flagValid || state.machine->executor_cpu.data.ds.flagValid ||
            state.machine->executor_cpu.data.fs.flagValid || state.machine->executor_cpu.data.gs.flagValid ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            core_machine_memory_read_physical(&state.machine->executor_memory, VM86_STACK_TOP - 36u,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK || frame[0] != 1u ||
            frame[1u] != 0x0200u || frame[2u] != (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF) ||
            frame[3u] != 0x1234u || frame[4u] != 0x0300u || frame[5u] != 0x0500u ||
            frame[6u] != 0x0400u || frame[7u] != 0x0600u || frame[8u] != 0x0700u;
    }
    core_machine_destroy(state.machine); return !failed;
}
static C_INT vm86_delivery_irq0_iret_round_trip(C_VOID)
{
    vm86_delivery_state state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !vm86_delivery_prepare(&state, 0x20u);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x2000u,
            (const type_unsigned_8[]){ 0x90u, 0x90u }, 2u) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, VM86_HANDLER_BASE,
                (const type_unsigned_8[]){ 0xcfu }, 1u) != TYPE_STATUS_OK;
        STD_MEMSET(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 3u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_VM) ||
            state.machine->executor_cpu.data.eip != 2u ||
            state.machine->executor_cpu.data.cs.selector != 0x0200u ||
            state.machine->executor_cpu.data.ss.selector != 0x0300u ||
            state.machine->executor_cpu.data.esp != 0x00001234u ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT vm86_delivery_enable_paging(vm86_delivery_state *state,
    type_bool source_page_present)
{
    const type_unsigned_32 page_entry = VM86_PAGE_FLAGS;

    if (!(vm86_delivery_write_u32(state->machine, VM86_PAGE_DIRECTORY,
            VM86_PAGE_TABLE | VM86_PAGE_FLAGS) &&
        vm86_delivery_write_u32(state->machine, VM86_PAGE_TABLE,
            page_entry) &&
        vm86_delivery_write_u32(state->machine, VM86_PAGE_TABLE + 2u * 4u,
            0x2000u | VM86_PAGE_FLAGS) &&
        vm86_delivery_write_u32(state->machine, VM86_PAGE_TABLE + 4u * 4u,
            source_page_present ? 0x4000u | VM86_PAGE_FLAGS : 0u) &&
        vm86_delivery_write_u32(state->machine, VM86_PAGE_TABLE + 8u * 4u,
            0x8000u | VM86_PAGE_FLAGS) &&
        vm86_delivery_write_u32(state->machine, VM86_PAGE_TABLE + 9u * 4u,
            0x9000u | VM86_PAGE_FLAGS))) return 0;
    state->machine->executor_cpu.data.cr3 = VM86_PAGE_DIRECTORY;
    state->machine->executor_cpu.data.cr0 |= VCPU_CR0_PG;
    return 1;
}

static C_INT vm86_delivery_paging_composition(C_VOID)
{
    static const type_unsigned_8 ud[] = { 0x0fu, 0x0bu };
    static const type_unsigned_8 nop[] = { 0x90u };
    vm86_delivery_state state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 frame[10u] = { 0u };
    C_INT failed = !vm86_delivery_prepare(&state, 6u);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x2000u, ud,
                sizeof(ud)) != TYPE_STATUS_OK ||
            !vm86_delivery_enable_paging(&state, TYPE_TRUE) ||
            core_machine_run(state.machine, (core_machine_run_budget){ 8u, 0u },
                &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_UD) || state.machine->executor_cpu.data.eip !=
                0x101u || state.machine->executor_cpu.data.cr3 !=
                VM86_PAGE_DIRECTORY || state.machine->executor_cpu.data.esp !=
                VM86_STACK_TOP - 36u || core_machine_memory_read_physical(
                &state.machine->executor_memory, VM86_STACK_TOP - 36u,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
            frame[0] != 0u || frame[1] != 0x0200u || frame[2] !=
                (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !vm86_delivery_prepare(&state, 14u);
    if (!failed) {
        state.machine->executor_cpu.data.cs.selector = 0x0400u;
        state.machine->executor_cpu.data.cs.base = 0x4000u;
        failed |= core_machine_memory_write(state.machine, 0x4000u, nop,
                sizeof(nop)) != TYPE_STATUS_OK ||
            !vm86_delivery_enable_paging(&state, TYPE_FALSE) ||
            core_machine_run(state.machine, (core_machine_run_budget){ 8u, 0u },
                &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_PF) || diagnostic.last_delivered_exception.
                exception_code != 4u || state.machine->executor_cpu.data.cr2 !=
                0x4000u || state.machine->executor_cpu.data.eip != 0x101u ||
            state.machine->executor_cpu.data.esp != VM86_STACK_TOP - 40u ||
            state.machine->executor_cpu.data.cs.selector != 0x0008u ||
            state.machine->executor_cpu.data.ss.selector != 0x0010u ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                VM86_STACK_TOP - 40u, (type_virtual_address)frame,
                sizeof(frame)) != TYPE_STATUS_OK || frame[0] != 4u ||
            frame[1] != 0u || frame[2] != 0x0400u || frame[3] !=
                (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF) || frame[4] != 0x1234u ||
            frame[5] != 0x0300u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}
static C_INT vm86_delivery_expect_prepublication(vm86_delivery_state *state)
{
    core_machine_run_result result; core_machine_cpu_diagnostic diagnostic; t_cpu before, after;
    C_INT failed = core_machine_memory_write(state->machine, 0x2000u,
        (const type_unsigned_8[]){0x0fu,0x0bu}, 2u) != TYPE_STATUS_OK;
    before = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    (C_VOID)core_machine_run(state->machine, (core_machine_run_budget){2u,0u}, &result);
    after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    failed |= core_machine_get_cpu_diagnostic(state->machine, &diagnostic) != TYPE_STATUS_OK ||
        !diagnostic.first_fault.valid || STD_MEMCMP(&before, &after, sizeof(before)) != 0;
    return !failed;
}
static C_INT vm86_delivery_invalid_gate(C_VOID)
{
    vm86_delivery_state state; C_INT failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        type_unsigned_8 absent[8u] = {0u};
        failed |= core_machine_memory_write(state.machine, VM86_IDT_BASE + 6u * 8u, absent,
            sizeof(absent)) != TYPE_STATUS_OK || !vm86_delivery_expect_prepublication(&state);
    }
    core_machine_destroy(state.machine); return !failed;
}
static C_INT vm86_delivery_bad_gate_access(type_unsigned_8 access)
{
    vm86_delivery_state state; C_INT failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, VM86_IDT_BASE + 6u * 8u + 5u,
            &access, sizeof(access)) != TYPE_STATUS_OK ||
            !vm86_delivery_expect_prepublication(&state);
    }
    core_machine_destroy(state.machine); return !failed;
}
static C_INT vm86_delivery_invalid_tss(C_VOID)
{
    vm86_delivery_state state; C_INT failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        state.machine->executor_cpu.data.tr.flagValid = TYPE_FALSE;
        failed |= !vm86_delivery_expect_prepublication(&state);
    }
    core_machine_destroy(state.machine); return !failed;
}
static C_INT vm86_delivery_bad_tss(C_INT short_tss)
{
    vm86_delivery_state state; C_INT failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        if (short_tss) state.machine->executor_cpu.data.tr.limit = 7u;
        else state.machine->executor_cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_AVL;
        failed |= !vm86_delivery_expect_prepublication(&state);
    }
    core_machine_destroy(state.machine); return !failed;
}
static C_INT vm86_delivery_invalid_ss0(C_VOID)
{
    vm86_delivery_state state; type_unsigned_16 ss0 = 0u;
    C_INT failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, VM86_TSS_BASE + 8u, &ss0,
            sizeof(ss0)) != TYPE_STATUS_OK || !vm86_delivery_expect_prepublication(&state);
    }
    core_machine_destroy(state.machine); return !failed;
}
static C_INT vm86_delivery_bad_ss0(type_unsigned_16 ss0, type_unsigned_8 access,
    type_unsigned_32 esp0)
{
    vm86_delivery_state state; C_INT failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, VM86_TSS_BASE + 4u, &esp0,
            sizeof(esp0)) != TYPE_STATUS_OK || core_machine_memory_write(state.machine,
            VM86_TSS_BASE + 8u, &ss0, sizeof(ss0)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, VM86_GDT_BASE + 16u + 5u, &access,
            sizeof(access)) != TYPE_STATUS_OK || !vm86_delivery_expect_prepublication(&state);
    }
    core_machine_destroy(state.machine); return !failed;
}
static C_INT vm86_delivery_short_stack(C_VOID)
{
    vm86_delivery_state state; type_unsigned_8 limit_lo = 0x1fu;
    type_unsigned_8 limit_hi = 0u; type_unsigned_8 flags = 0x40u;
    type_unsigned_32 esp0 = 0x00000020u;
    C_INT failed = !vm86_delivery_prepare(&state, 6u);
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, VM86_TSS_BASE + 4u, &esp0,
            sizeof(esp0)) != TYPE_STATUS_OK || core_machine_memory_write(state.machine,
            VM86_GDT_BASE + 16u, &limit_lo, sizeof(limit_lo)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, VM86_GDT_BASE + 17u, &limit_hi,
            sizeof(limit_hi)) != TYPE_STATUS_OK || core_machine_memory_write(state.machine,
            VM86_GDT_BASE + 22u, &flags, sizeof(flags)) != TYPE_STATUS_OK ||
            !vm86_delivery_expect_prepublication(&state);
    }
    core_machine_destroy(state.machine); return !failed;
}
C_INT main(C_VOID)
{
    static const type_unsigned_8 ud[] = {0x0fu,0x0bu};
    static const type_unsigned_8 gp[] = {0xfau};
    static const type_unsigned_8 nm[] = {0xd8u,0xc0u};
    if (!vm86_delivery_fault(6u, ud, sizeof(ud), 0) ||
        !vm86_delivery_fault(13u, gp, sizeof(gp), 1) ||
        !vm86_delivery_fault(7u, nm, sizeof(nm), 0) || !vm86_delivery_debug_tf() ||
        !vm86_delivery_debug_breakpoint() ||
        !vm86_delivery_irq0() || !vm86_delivery_irq0_iret_round_trip() ||
        !vm86_delivery_paging_composition() ||
        !vm86_delivery_invalid_gate() || !vm86_delivery_bad_gate_access(0x0eu) ||
        !vm86_delivery_bad_gate_access(0x80u) || !vm86_delivery_invalid_tss() ||
        !vm86_delivery_bad_tss(0) || !vm86_delivery_bad_tss(1) ||
        !vm86_delivery_invalid_ss0() || !vm86_delivery_bad_ss0(0x0013u, 0x92u,
            VM86_STACK_TOP) || !vm86_delivery_bad_ss0(0x0010u, 0x12u, VM86_STACK_TOP) ||
        !vm86_delivery_bad_ss0(0x0010u, 0x90u, VM86_STACK_TOP) ||
        !vm86_delivery_short_stack()) return 1;
    STD_PRINTF("M5:T320:S1:VM86-DELIVERY:OK\n"); return 0;
}
