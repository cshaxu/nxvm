#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "core/x86/device_support_interface.h"

#include "core/x86/debug_interface.h"
#include "debug_fixture.h"

#define VM86_IRET_CODE 0x0100u
#define VM86_IRET_STACK 0x8000u
#define VM86_IRET_PAGE_DIRECTORY 0xa000u
#define VM86_IRET_PAGE_TABLE 0xb000u
#define VM86_IRET_PAGE_FLAGS 0x00000007u

typedef struct vm86_iret_state { core_machine *machine; } vm86_iret_state;

static core_machine_debug_cpu_snapshot vm86_iret_capture(const core_machine *machine)
{
    core_machine_debug_cpu_snapshot snapshot = {0};
    if (core_machine_debug_capture_cpu_snapshot(machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &snapshot) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return snapshot;
}

static lib_i32 vm86_iret_write_u32(core_machine *machine,
    lib_u32 address, lib_u32 value)
{
    return core_machine_memory_write(machine, address, &value, sizeof(value)) ==
        LIB_STATUS_OK;
}

static lib_i32 vm86_iret_prepare(vm86_iret_state *state,
    const lib_u8 *instruction, lib_u8 bytes,
    lib_u32 stack_limit)
{
    const core_machine_executor_config config = {
        .memory_bytes = 0x100000u,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u32 frame[9u] = {
        0x00000010u, 0xa5a50200u, CORE_MACHINE_DEBUG_EFLAGS_VM | CORE_MACHINE_DEBUG_EFLAGS_IF,
        0x00001234u, 0xb6b60300u, 0xc7c70500u, 0xd8d80400u,
        0xe9e90600u, 0xfafa0700u
    };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,1,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0x40u,0
    };
    static const lib_u8 pointer[] = {0x17u,0,0,3,0,0};
    static const lib_u8 setup[] = {
        0x0fu,0x01u,0x16u,0,1, 0xb8u,1,0, 0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0, 0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xbcu,0,0x80u, 0xeau,0,4,8,0
    };
    const core_machine_debug_register_patch real_entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x700u}
    };
    core_machine_run_result result;

    lib_memory_set(state, 0, sizeof(*state));
    gdt[16u] = (lib_u8)stack_limit;
    gdt[17u] = (lib_u8)(stack_limit >> 8u);
    if (core_machine_neutral_create(&config, &state->machine) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(state->machine, &real_entry) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x300u, gdt, sizeof(gdt)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x100u, pointer, sizeof(pointer)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x700u, setup, sizeof(setup)) != LIB_STATUS_OK ||
        core_machine_run(state->machine, (core_machine_run_budget){9u,0u}, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 9u ||
        core_machine_memory_write(state->machine, VM86_IRET_CODE, instruction, bytes) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x2010u,
            (const lib_u8[]){0x90u,0xf4u}, 2u) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, VM86_IRET_STACK, frame, sizeof(frame)) != LIB_STATUS_OK)
        return 0;
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EIP, 0u);
    test_core_machine_fixture_write_register(state->machine, CORE_MACHINE_DEBUG_EFLAGS,
        CORE_MACHINE_DEBUG_EFLAGS_IF);
    const core_machine_debug_cpu_snapshot snapshot = vm86_iret_capture(state->machine);
    return snapshot.cs.selector == 8u && snapshot.cs.base == VM86_IRET_CODE &&
        snapshot.cs.defsize && snapshot.ss.selector == 0x10u &&
        snapshot.ss.limit == stack_limit && snapshot.ss.big &&
        snapshot.esp == VM86_IRET_STACK;
}

static lib_i32 vm86_iret_cache(const core_machine_debug_segment_snapshot *sreg,
    lib_u16 selector, lib_bool code)
{
    return sreg->selector == selector && sreg->base == (lib_u32)selector << 4u &&
        sreg->limit == 0xffffu && sreg->dpl == 3u && !sreg->accessed &&
        sreg->executable == code && (code ? !sreg->defsize && !sreg->conform &&
            sreg->readable : !sreg->big && !sreg->expdown && sreg->writable);
}
static lib_i32 vm86_iret_success(const lib_u8 *instruction,
    lib_u8 bytes)
{
    vm86_iret_state state; core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot after;
    lib_i32 failed = !vm86_iret_prepare(
        &state, instruction, bytes, 0x0000ffffu);

    if (!failed) {
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            diagnostic.first_fault.valid;
        after = vm86_iret_capture(state.machine);
        failed |= after.eflags !=
                (CORE_MACHINE_DEBUG_EFLAGS_VM | CORE_MACHINE_DEBUG_EFLAGS_IF | 0x02u) ||
            after.eip != 0x0011u ||
            after.esp != 0x00001234u ||
            !vm86_iret_cache(&after.cs, 0x0200u, LIB_TRUE) ||
            !vm86_iret_cache(&after.ss, 0x0300u, LIB_FALSE) ||
            !vm86_iret_cache(&after.es, 0x0500u, LIB_FALSE) ||
            !vm86_iret_cache(&after.ds, 0x0400u, LIB_FALSE) ||
            !vm86_iret_cache(&after.fs, 0x0600u, LIB_FALSE) ||
            !vm86_iret_cache(&after.gs, 0x0700u, LIB_FALSE);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 vm86_iret_stack_atomic(void)
{
    vm86_iret_state state; core_machine_run_result result; core_machine_debug_cpu_snapshot before, after;
    lib_i32 failed = !vm86_iret_prepare(&state, (const lib_u8[]){ 0xcfu },
        1u, VM86_IRET_STACK + 31u);

    if (!failed) {
        before = vm86_iret_capture(state.machine);
        (void)core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        after = vm86_iret_capture(state.machine);
        failed |= lib_memory_compare(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 vm86_iret_paging_success(void)
{
    vm86_iret_state state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot after;
    lib_i32 failed = !vm86_iret_prepare(&state, (const lib_u8[]){ 0xcfu },
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
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_CR3, VM86_IRET_PAGE_DIRECTORY);
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_CR0,
            vm86_iret_capture(state.machine).cr0 | 0x80000000u);
        failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK || diagnostic.first_fault.valid;
        after = vm86_iret_capture(state.machine);
        failed |= after.cr3 != VM86_IRET_PAGE_DIRECTORY ||
            after.eip != 0x0011u ||
            after.esp != 0x00001234u ||
            !CORE_MACHINE_BIT_IS_SET(after.eflags,
                CORE_MACHINE_DEBUG_EFLAGS_VM) || !vm86_iret_cache(
                &after.cs, 0x0200u, LIB_TRUE) ||
            !vm86_iret_cache(&after.ss, 0x0300u,
                LIB_FALSE);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!vm86_iret_success((const lib_u8[]){ 0xcfu }, 1u) ||
        !vm86_iret_success((const lib_u8[]){ 0x67u, 0xcfu }, 2u) ||
        !vm86_iret_stack_atomic() || !vm86_iret_paging_success())
        return 1;
    lib_c_printf("VM86-IRET:OK\n");
    return 0;
}
