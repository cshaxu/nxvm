#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define VM86_IRET_CODE 0x0100u
#define VM86_IRET_STACK 0x8000u
#define VM86_IRET_PAGE_DIRECTORY 0xa000u
#define VM86_IRET_PAGE_TABLE 0xb000u
#define VM86_IRET_PAGE_FLAGS 0x00000007u

typedef struct vm86_iret_state { core_machine *machine; } vm86_iret_state;

static C_VOID vm86_iret_reset(C_VOID *opaque)
{
    vm86_iret_state *state = (vm86_iret_state *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider vm86_iret_provider = {
    vm86_iret_reset, STD_NULL
};

static C_INT vm86_iret_write_u32(core_machine *machine,
    type_unsigned_32 address, type_unsigned_32 value)
{
    return core_machine_memory_write(machine, address, &value, sizeof(value)) ==
        TYPE_STATUS_OK;
}

static C_INT vm86_iret_prepare(vm86_iret_state *state,
    const type_unsigned_8 *instruction, type_unsigned_8 bytes,
    type_unsigned_32 stack_limit)
{
    const core_machine_config config = {
        .memory_bytes = 0x100000u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const type_unsigned_32 frame[9u] = {
        0x00000010u, 0xa5a50200u, VCPU_EFLAGS_VM | VCPU_EFLAGS_IF,
        0x00001234u, 0xb6b60300u, 0xc7c70500u, 0xd8d80400u,
        0xe9e90600u, 0xfafa0700u
    };
    t_cpu *cpu;

    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &vm86_iret_provider, state) ||
        core_machine_memory_write(state->machine, VM86_IRET_CODE, instruction,
            bytes) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x2010u,
            (const type_unsigned_8[]){ 0x90u, 0xf4u }, 2u) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, VM86_IRET_STACK, frame,
            sizeof(frame)) != TYPE_STATUS_OK)
        return 0;
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.eflags = VCPU_EFLAGS_IF;
    cpu->data.cs.flagValid = TYPE_TRUE; cpu->data.cs.selector = 0x0008u;
    cpu->data.cs.sregtype = SREG_CODE; cpu->data.cs.base = VM86_IRET_CODE;
    cpu->data.cs.limit = 0x0000ffffu; cpu->data.cs.dpl = 0u;
    cpu->data.cs.seg.executable = TYPE_TRUE; cpu->data.cs.seg.exec.defsize = TYPE_TRUE;
    cpu->data.eip = 0u;
    cpu->data.ss.flagValid = TYPE_TRUE; cpu->data.ss.selector = 0x0010u;
    cpu->data.ss.sregtype = SREG_STACK; cpu->data.ss.base = 0u;
    cpu->data.ss.limit = stack_limit; cpu->data.ss.dpl = 0u;
    cpu->data.ss.seg.data.big = TYPE_TRUE; cpu->data.ss.seg.data.writable = TYPE_TRUE;
    cpu->data.esp = VM86_IRET_STACK;
    return 1;
}

static C_INT vm86_iret_cache(const t_cpu_data_sreg *sreg,
    type_unsigned_16 selector, type_unsigned_8 kind)
{
    return sreg->flagValid && sreg->selector == selector &&
        sreg->sregtype == kind && sreg->base == (type_unsigned_32)selector << 4u &&
        sreg->limit == 0x0000ffffu && sreg->dpl == 3u &&
        !sreg->seg.accessed && sreg->seg.executable == (kind == SREG_CODE) &&
        (kind == SREG_CODE ? !sreg->seg.exec.defsize && !sreg->seg.exec.conform &&
            sreg->seg.exec.readable : !sreg->seg.data.big &&
            !sreg->seg.data.expdown && sreg->seg.data.writable);
}

static C_INT vm86_iret_success(const type_unsigned_8 *instruction,
    type_unsigned_8 bytes)
{
    vm86_iret_state state; core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic; C_INT failed = !vm86_iret_prepare(
        &state, instruction, bytes, 0x0000ffffu);

    if (!failed) {
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid ||
            state.machine->executor_cpu.data.eflags !=
                (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF | 0x02u) ||
            state.machine->executor_cpu.data.eip != 0x0011u ||
            state.machine->executor_cpu.data.esp != 0x00001234u ||
            !vm86_iret_cache(&state.machine->executor_cpu.data.cs, 0x0200u, SREG_CODE) ||
            !vm86_iret_cache(&state.machine->executor_cpu.data.ss, 0x0300u, SREG_STACK) ||
            !vm86_iret_cache(&state.machine->executor_cpu.data.es, 0x0500u, SREG_DATA) ||
            !vm86_iret_cache(&state.machine->executor_cpu.data.ds, 0x0400u, SREG_DATA) ||
            !vm86_iret_cache(&state.machine->executor_cpu.data.fs, 0x0600u, SREG_DATA) ||
            !vm86_iret_cache(&state.machine->executor_cpu.data.gs, 0x0700u, SREG_DATA);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT vm86_iret_stack_atomic(C_VOID)
{
    vm86_iret_state state; core_machine_run_result result; t_cpu before, after;
    C_INT failed = !vm86_iret_prepare(&state, (const type_unsigned_8[]){ 0xcfu },
        1u, VM86_IRET_STACK + 31u);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        (C_VOID)core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT vm86_iret_paging_success(C_VOID)
{
    vm86_iret_state state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !vm86_iret_prepare(&state, (const type_unsigned_8[]){ 0xcfu },
        1u, 0x0000ffffu);

    if (!failed) {
        failed |= !vm86_iret_write_u32(state.machine, VM86_IRET_PAGE_DIRECTORY,
                VM86_IRET_PAGE_TABLE | VM86_IRET_PAGE_FLAGS) ||
            !vm86_iret_write_u32(state.machine, VM86_IRET_PAGE_TABLE,
                VM86_IRET_PAGE_FLAGS) ||
            !vm86_iret_write_u32(state.machine, VM86_IRET_PAGE_TABLE + 2u * 4u,
                0x2000u | VM86_IRET_PAGE_FLAGS) ||
            !vm86_iret_write_u32(state.machine, VM86_IRET_PAGE_TABLE + 8u * 4u,
                0x8000u | VM86_IRET_PAGE_FLAGS);
        state.machine->executor_cpu.data.cr3 = VM86_IRET_PAGE_DIRECTORY;
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_PG;
        failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            state.machine->executor_cpu.data.cr3 != VM86_IRET_PAGE_DIRECTORY ||
            state.machine->executor_cpu.data.eip != 0x0011u ||
            state.machine->executor_cpu.data.esp != 0x00001234u ||
            !TYPE_GET_BIT(state.machine->executor_cpu.data.eflags,
                VCPU_EFLAGS_VM) || !vm86_iret_cache(
                &state.machine->executor_cpu.data.cs, 0x0200u, SREG_CODE) ||
            !vm86_iret_cache(&state.machine->executor_cpu.data.ss, 0x0300u,
                SREG_STACK);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!vm86_iret_success((const type_unsigned_8[]){ 0xcfu }, 1u) ||
        !vm86_iret_success((const type_unsigned_8[]){ 0x67u, 0xcfu }, 2u) ||
        !vm86_iret_stack_atomic() || !vm86_iret_paging_success())
        return 1;
    STD_PRINTF("M5:T320:S2:VM86-IRET:OK\n");
    return 0;
}
