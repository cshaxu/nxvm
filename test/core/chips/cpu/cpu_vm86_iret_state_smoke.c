#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "core/x86/device_support_interface.h"

#include "support/cpu_instruction_fixture.h"

#define VM86_IRET_CODE 0x0100u
#define VM86_IRET_STACK 0x8000u
#define VM86_IRET_PAGE_DIRECTORY 0xa000u
#define VM86_IRET_PAGE_TABLE 0xb000u
#define VM86_IRET_PAGE_FLAGS 0x00000007u

typedef cpu_instruction_fixture vm86_iret_state;

static lib_status vm86_iret_write(vm86_iret_state *state,
    lib_u32 address, const void *data, lib_size bytes)
{
    if (address > sizeof(state->memory) ||
        bytes > sizeof(state->memory) - address) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(state->memory + address, data, bytes);
    return LIB_STATUS_OK;
}
static lib_i32 vm86_iret_write_u32(vm86_iret_state *state,
    lib_u32 address, lib_u32 value)
{
    return vm86_iret_write(state, address, &value, sizeof(value)) == LIB_STATUS_OK;
}
static lib_status vm86_iret_run(vm86_iret_state *state, lib_u32 instructions)
{
    while (instructions-- != 0u && !state->execution.stop_requested)
        core_machine_cpu_execution_refresh(&state->execution);
    return state->execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

static lib_i32 vm86_iret_prepare(vm86_iret_state *state,
    const lib_u8 *instruction, lib_u8 bytes,
    lib_u32 stack_limit)
{
    const lib_u32 frame[9u] = {
        0x00000010u, 0xa5a50200u, CORE_MACHINE_DEBUG_EFLAGS_VM | CORE_MACHINE_DEBUG_EFLAGS_IF,
        0x00001234u, 0xb6b60300u, 0xc7c70500u, 0xd8d80400u,
        0xe9e90600u, 0xfafa0700u
    };
    t_cpu *cpu;

    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    if (vm86_iret_write(state, VM86_IRET_CODE, instruction,
            bytes) != LIB_STATUS_OK ||
        vm86_iret_write(state, 0x2010u,
            (const lib_u8[]){ 0x90u, 0xf4u }, 2u) != LIB_STATUS_OK ||
        vm86_iret_write(state, VM86_IRET_STACK, frame,
            sizeof(frame)) != LIB_STATUS_OK)
        return 0;
    cpu = &state->cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.eflags = CORE_MACHINE_DEBUG_EFLAGS_IF;
    cpu->data.cs.flagValid = LIB_TRUE; cpu->data.cs.selector = 0x0008u;
    cpu->data.cs.sregtype = SREG_CODE; cpu->data.cs.base = VM86_IRET_CODE;
    cpu->data.cs.limit = 0x0000ffffu; cpu->data.cs.dpl = 0u;
    cpu->data.cs.seg.executable = LIB_TRUE; cpu->data.cs.seg.exec.defsize = LIB_TRUE;
    cpu->data.eip = 0u;
    cpu->data.ss.flagValid = LIB_TRUE; cpu->data.ss.selector = 0x0010u;
    cpu->data.ss.sregtype = SREG_STACK; cpu->data.ss.base = 0u;
    cpu->data.ss.limit = stack_limit; cpu->data.ss.dpl = 0u;
    cpu->data.ss.seg.data.big = LIB_TRUE; cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.esp = VM86_IRET_STACK;
    return 1;
}

static lib_i32 vm86_iret_cache(const t_cpu_data_sreg *sreg,
    lib_u16 selector, lib_u8 kind)
{
    return sreg->flagValid && sreg->selector == selector &&
        sreg->sregtype == kind && sreg->base == (lib_u32)selector << 4u &&
        sreg->limit == 0x0000ffffu && sreg->dpl == 3u &&
        !sreg->seg.accessed && sreg->seg.executable == (kind == SREG_CODE) &&
        (kind == SREG_CODE ? !sreg->seg.exec.defsize && !sreg->seg.exec.conform &&
            sreg->seg.exec.readable : !sreg->seg.data.big &&
            !sreg->seg.data.expdown && sreg->seg.data.writable);
}

static lib_i32 vm86_iret_success(const lib_u8 *instruction,
    lib_u8 bytes)
{
    vm86_iret_state state;
    lib_i32 failed = !vm86_iret_prepare(
        &state, instruction, bytes, 0x0000ffffu);

    if (!failed) {
        failed |= vm86_iret_run(&state, 2u) != LIB_STATUS_OK ||
            state.fault.valid ||
            state.cpu.data.eflags !=
                (CORE_MACHINE_DEBUG_EFLAGS_VM | CORE_MACHINE_DEBUG_EFLAGS_IF | 0x02u) ||
            state.cpu.data.eip != 0x0011u ||
            state.cpu.data.esp != 0x00001234u ||
            !vm86_iret_cache(&state.cpu.data.cs, 0x0200u, SREG_CODE) ||
            !vm86_iret_cache(&state.cpu.data.ss, 0x0300u, SREG_STACK) ||
            !vm86_iret_cache(&state.cpu.data.es, 0x0500u, SREG_DATA) ||
            !vm86_iret_cache(&state.cpu.data.ds, 0x0400u, SREG_DATA) ||
            !vm86_iret_cache(&state.cpu.data.fs, 0x0600u, SREG_DATA) ||
            !vm86_iret_cache(&state.cpu.data.gs, 0x0700u, SREG_DATA);
    }
    return !failed;
}

static lib_i32 vm86_iret_stack_atomic(void)
{
    vm86_iret_state state; t_cpu before, after;
    lib_i32 failed = !vm86_iret_prepare(&state, (const lib_u8[]){ 0xcfu },
        1u, VM86_IRET_STACK + 31u);

    if (!failed) {
        before = state.cpu;
        (void)vm86_iret_run(&state, 1u);
        after = state.cpu;
        failed |= lib_memory_compare(&before, &after, sizeof(before)) != 0;
    }
    return !failed;
}

static lib_i32 vm86_iret_paging_success(void)
{
    vm86_iret_state state;
    lib_i32 failed = !vm86_iret_prepare(&state, (const lib_u8[]){ 0xcfu },
        1u, 0x0000ffffu);

    if (!failed) {
        failed |= !vm86_iret_write_u32(&state, VM86_IRET_PAGE_DIRECTORY,
                VM86_IRET_PAGE_TABLE | VM86_IRET_PAGE_FLAGS) ||
            !vm86_iret_write_u32(&state, VM86_IRET_PAGE_TABLE,
                VM86_IRET_PAGE_FLAGS) ||
            !vm86_iret_write_u32(&state, VM86_IRET_PAGE_TABLE + 2u * 4u,
                0x2000u | VM86_IRET_PAGE_FLAGS) ||
            !vm86_iret_write_u32(&state, VM86_IRET_PAGE_TABLE + 8u * 4u,
                0x8000u | VM86_IRET_PAGE_FLAGS);
        state.cpu.data.cr3 = VM86_IRET_PAGE_DIRECTORY;
        state.cpu.data.cr0 |= VCPU_CR0_PG;
        failed |= vm86_iret_run(&state, 2u) != LIB_STATUS_OK ||
            state.fault.valid ||
            state.cpu.data.cr3 != VM86_IRET_PAGE_DIRECTORY ||
            state.cpu.data.eip != 0x0011u ||
            state.cpu.data.esp != 0x00001234u ||
            !CORE_MACHINE_BIT_IS_SET(state.cpu.data.eflags,
                CORE_MACHINE_DEBUG_EFLAGS_VM) || !vm86_iret_cache(
                &state.cpu.data.cs, 0x0200u, SREG_CODE) ||
            !vm86_iret_cache(&state.cpu.data.ss, 0x0300u,
                SREG_STACK);
    }
    return !failed;
}

static lib_bool vm86_iret_flags_matrix(void)
{
    lib_u8 wide, old_rf, new_rf;

    for (wide = 0u; wide < 2u; ++wide)
    for (old_rf = 0u; old_rf < 2u; ++old_rf)
    for (new_rf = 0u; new_rf < 2u; ++new_rf) {
        vm86_iret_state state;
        const lib_u8 code[] = {0x66u,0xcfu};
        const lib_u32 image = 3u | (new_rf ? VCPU_EFLAGS_RF : 0u);
        const lib_u32 frame32[] = {0x0100u,0x0200u,image};
        const lib_u16 frame16[] = {0x0100u,0x0200u,(lib_u16)image};
        const lib_u32 expected = 3u | VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL |
            ((wide ? new_rf : old_rf) ? VCPU_EFLAGS_RF : 0u);

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cr0 |= VCPU_CR0_PE;
        state.cpu.data.cs.selector = 0x0200u;
        state.cpu.data.cs.base = 0x2000u;
        state.cpu.data.cs.dpl = 3u;
        state.cpu.data.ss.selector = 0x0300u;
        state.cpu.data.ss.base = 0x3000u;
        state.cpu.data.ss.dpl = 3u;
        state.cpu.data.sp = 0x8000u;
        state.cpu.data.eflags = 2u | VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL |
            (old_rf ? VCPU_EFLAGS_RF : 0u);
        lib_memory_copy(state.memory + 0x2000u, wide ? code : code + 1u,
            wide ? 2u : 1u);
        lib_memory_copy(state.memory + 0xb000u,
            wide ? (const void *)frame32 : (const void *)frame16,
            wide ? sizeof(frame32) : sizeof(frame16));
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested || state.fault.valid ||
            state.delivered_exception.valid || state.cpu.data.eip != 0x0100u ||
            state.cpu.data.sp != 0x8000u + (wide ? 12u : 6u) ||
            state.cpu.data.eflags != expected) return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    if (!vm86_iret_flags_matrix()) return 1;
    if (!vm86_iret_success((const lib_u8[]){ 0xcfu }, 1u) ||
        !vm86_iret_success((const lib_u8[]){ 0x67u, 0xcfu }, 2u) ||
        !vm86_iret_stack_atomic() || !vm86_iret_paging_success())
        return 1;
    lib_c_printf("VM86-IRET-CPU-STATE:OK\n");
    return 0;
}
