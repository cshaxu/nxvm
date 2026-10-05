#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/core/device_support_interface.h"

#include "x86/core/machine.h"
#include "../../ibmpc/board-common/core_machine_board_fixture.h"
#include "exception_fixture.h"

typedef struct fpu_interface_s65_machine {
    core_machine *machine;
} fpu_interface_s65_machine;

#define FPU_S65_GDT_POINTER 0x0100u
#define FPU_S65_GDT_BASE 0x0300u
#define FPU_S65_IDT_BASE 0x0400u
#define FPU_S65_CODE_BASE 0x2000u

static lib_i32 fpu_interface_s65_prepare(core_machine_cpu_profile profile,
    x86_fpu_profile fpu_profile, fpu_interface_s65_machine *state)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = fpu_profile
    };

    if (state == LIB_NULL) {
        return 0;
    }
    lib_memory_set(state, 0, sizeof(*state));
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {0}
    };
    /* T337_REAL_UD_TERMINAL_IVT_REJECT: producer failures cannot deliver vector 6. */
    return core_machine_neutral_create(&config, &state->machine) == LIB_STATUS_OK &&
        test_core_exception_block_vector(state->machine, 6u, state) == LIB_STATUS_OK &&
        test_core_machine_fixture_bind_freeze_reset(state->machine, LIB_NULL, LIB_NULL) &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static core_machine_debug_cpu_snapshot fpu_interface_s65_capture(core_machine *machine)
{
    core_machine_debug_cpu_snapshot snapshot = {0};
    if (core_machine_debug_capture_cpu_snapshot(machine, CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
            &snapshot) != LIB_STATUS_OK) exit(EXIT_FAILURE);
    return snapshot;
}

static lib_i32 fpu_interface_s65_run(fpu_interface_s65_machine *state,
    const lib_u8 *code, lib_size size, core_machine_debug_cpu_snapshot *after,
    core_machine_cpu_diagnostic *diagnostic, lib_status *status)
{
    core_machine_run_result result;

    if (state == LIB_NULL || state->machine == LIB_NULL || code == LIB_NULL ||
        core_machine_memory_write(state->machine, 0u, code, size) !=
            LIB_STATUS_OK) {
        return 0;
    }
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, &result);
    *after = fpu_interface_s65_capture(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        LIB_STATUS_OK;
}

static lib_i32 fpu_interface_s65_same(const core_machine_debug_cpu_snapshot *before, const core_machine_debug_cpu_snapshot *after)
{
    return lib_memory_compare(before, after, sizeof(*before)) == 0;
}

static lib_i32 fpu_interface_s65_success(const lib_u8 *code, lib_size size,
    core_machine_cpu_profile profile, x86_fpu_profile fpu_profile,
    lib_u32 cr0)
{
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_status status;
    lib_i32 failed = !fpu_interface_s65_prepare(profile, fpu_profile, &state);

    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_CR0, cr0);
        before = fpu_interface_s65_capture(state.machine);
        failed |= !fpu_interface_s65_run(&state, code, size, &after,
            &diagnostic, &status) || status != LIB_STATUS_OK ||
            diagnostic.first_fault.valid || after.eip != size ||
            after.eax != before.eax ||
            after.ebx != before.ebx ||
            after.ecx != before.ecx ||
            after.edx != before.edx ||
            after.esp != before.esp ||
            after.ebp != before.ebp ||
            after.esi != before.esi ||
            after.edi != before.edi ||
            after.eflags != before.eflags ||
            lib_memory_compare(&after.es, &before.es,
                sizeof(after.es)) != 0 ||
            lib_memory_compare(&after.cs, &before.cs,
                sizeof(after.cs)) != 0 ||
            lib_memory_compare(&after.ss, &before.ss,
                sizeof(after.ss)) != 0 ||
            lib_memory_compare(&after.ds, &before.ds,
                sizeof(after.ds)) != 0 ||
            lib_memory_compare(&after.fs, &before.fs,
                sizeof(after.fs)) != 0 ||
            lib_memory_compare(&after.gs, &before.gs,
                sizeof(after.gs)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 fpu_interface_s65_mf(void)
{
    static const lib_u8 wait[] = { 0x9bu };
    static const lib_u8 handler[] = { 0xf4u };
    const lib_u16 handler_offset = 0x0100u;
    const lib_u16 handler_segment = 0u;
    lib_u16 frame[3] = { 0u, 0u, 0u };
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_status status;
    lib_i32 failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_80387, &state);

    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_ESP, 0x00008000u);
        failed |= core_machine_memory_write(state.machine, 0x0040u,
            &handler_offset, sizeof(handler_offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0042u, &handler_segment,
                sizeof(handler_segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, handler_offset, handler,
                sizeof(handler)) != LIB_STATUS_OK;
        x86_fpu_load_control_word(state.machine->fpu, 0x037eu);
        failed |= x86_fpu_store_m32(state.machine->fpu, &(lib_u32){0}) !=
            X86_FPU_EXECUTE_COMPLETED || !x86_fpu_wait_pending(state.machine->fpu);
        before = fpu_interface_s65_capture(state.machine);
        failed |= !fpu_interface_s65_run(&state, wait, sizeof(wait), &after,
            &diagnostic, &status) || status != LIB_STATUS_OK ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_MF) || after.eip != handler_offset ||
            after.esp != ((before.esp & 0xffff0000u) |
                (lib_u16)(before.esp - 6u)) ||
            core_machine_debug_read_linear(state.machine,
                after.ss.base + (lib_u16)after.esp, frame, sizeof(frame)) != LIB_STATUS_OK ||
            frame[0] != 0u || frame[1] != before.cs.selector ||
            frame[2] != (lib_u16)before.eflags ||
            after.eax != before.eax || after.ebx != before.ebx ||
            after.ecx != before.ecx || after.edx != before.edx ||
            after.ebp != before.ebp || after.esi != before.esi ||
            after.edi != before.edi ||
            lib_memory_compare(&after.es, &before.es, sizeof(after.es)) != 0 ||
            lib_memory_compare(&after.ds, &before.ds, sizeof(after.ds)) != 0 ||
            lib_memory_compare(&after.fs, &before.fs, sizeof(after.fs)) != 0 ||
            lib_memory_compare(&after.gs, &before.gs, sizeof(after.gs)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 fpu_interface_s65_reject(const lib_u8 *code, lib_size size,
    core_machine_cpu_profile profile)
{
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_status status;
    lib_i32 failed = !fpu_interface_s65_prepare(profile,
        X86_FPU_PROFILE_NONE, &state);

    if (!failed) {
        before = fpu_interface_s65_capture(state.machine);
        failed |= !fpu_interface_s65_run(&state, code, size, &after,
            &diagnostic, &status) || status != LIB_STATUS_INTERNAL_ERROR ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            !fpu_interface_s65_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 fpu_interface_s65_handoff(core_machine_cpu_profile cpu,
    x86_fpu_profile profile)
{
    static const lib_u8 fadd_wait[] = { 0xd8u, 0xc0u, 0x9bu };
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_status status;
    lib_i32 failed = !fpu_interface_s65_prepare(cpu, profile, &state);

    if (!failed) {
        before = fpu_interface_s65_capture(state.machine);
        failed |= core_machine_memory_write(state.machine, 0u, fadd_wait,
            sizeof(fadd_wait)) != LIB_STATUS_OK;
        status = core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &(core_machine_run_result){ 0 });
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        after = fpu_interface_s65_capture(state.machine);
        if (!failed) failed |= status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
            after.eip != sizeof(fadd_wait) - 1u ||
            after.eax != before.eax || after.ebx != before.ebx ||
            x86_fpu_ticks_until_completion(state.machine->fpu,
                &(lib_u64){0}) != LIB_STATUS_OK ||
            state.machine->transaction.address != fadd_wait[0] ||
            state.machine->transaction.value != fadd_wait[1] ||
            state.machine->transaction.owner != CORE_MACHINE_TRANSACTION_OWNER_NONE ||
            state.machine->transaction.kind != CORE_MACHINE_TRANSACTION_CPU_FPU_COMMAND;
        failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &(core_machine_run_result){ 0 }) != LIB_STATUS_OK ||
            x86_fpu_ticks_until_completion(state.machine->fpu,
                &(lib_u64){0}) != LIB_STATUS_INVALID_STATE;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 fpu_interface_s65_deadline(core_machine_cpu_profile cpu,
    x86_fpu_profile profile)
{
    static const lib_u8 fadd[] = { 0xd8u, 0xc0u };
    fpu_interface_s65_machine state;
    core_machine_time_observation observation;
    lib_u8 advanced = LIB_FALSE;
    lib_i32 failed = !fpu_interface_s65_prepare(cpu, profile, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, fadd,
            sizeof(fadd)) != LIB_STATUS_OK ||
            core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
                &(core_machine_run_result){0}) != LIB_STATUS_OK ||
            x86_fpu_ticks_until_completion(state.machine->fpu,
                &(lib_u64){0}) != LIB_STATUS_OK ||
            core_machine_capture_time_observation(state.machine, &observation) !=
                LIB_STATUS_OK || !observation.next_deadline_valid ||
            observation.next_deadline_tick <= observation.elapsed_ticks ||
            core_machine_advance_to_next_deadline(state.machine, &advanced) !=
                LIB_STATUS_OK || !advanced ||
            x86_fpu_ticks_until_completion(state.machine->fpu,
                &(lib_u64){0}) != LIB_STATUS_INVALID_STATE;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 fpu_interface_s65_incompatible(void)
{
    static const lib_u8 fninit[] = { 0xdbu, 0xe3u };
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_status status;
    lib_i32 failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_8087, &state);

    if (!failed) {
        before = fpu_interface_s65_capture(state.machine);
        failed |= !fpu_interface_s65_run(&state, fninit, sizeof(fninit), &after,
            &diagnostic, &status) || status != LIB_STATUS_INTERNAL_ERROR ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_FPU_UNSUPPORTED) || !fpu_interface_s65_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 fpu_interface_s65_nm_delivery(const lib_u8 *code,
    lib_size code_size, lib_u32 cr0)
{
    static const lib_u8 hlt = 0xf4u;
    const lib_u16 offset = 0x0100u;
    const lib_u16 segment = 0u;
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot after;
    lib_u16 frame_ip = 0u;
    lib_i32 failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_NONE, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, code,
            code_size) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x1cu, &offset,
            sizeof(offset)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x1eu, &segment,
            sizeof(segment)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, offset, &hlt,
            sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_CR0, cr0);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
        after = fpu_interface_s65_capture(state.machine);
        if (!failed) failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_NM) || after.eip != offset ||
            core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)CORE_MACHINE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != LIB_STATUS_OK ||
            frame_ip != 0u;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = fpu_interface_s65_capture(state.machine);
        failed |= after.eip != offset + 1u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 fpu_interface_s65_vm86(void)
{
    static const lib_u8 esc[] = { 0xd8u, 0xc0u };
    fpu_interface_s65_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_status status;
    lib_i32 failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_NONE, &state);

    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_CR0, VCPU_CR0_PE);
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS,
            CORE_MACHINE_DEBUG_EFLAGS_VM | 0x00003000u |
            CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_CF);
        const core_machine_debug_register_patch segments = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS),
            .values = {0}
        };
        failed |= core_machine_debug_patch_registers(state.machine, &segments) != LIB_STATUS_OK;
        before = fpu_interface_s65_capture(state.machine);
        failed |= core_machine_memory_write(state.machine, 0u, esc,
            sizeof(esc)) != LIB_STATUS_OK;
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        failed |= status != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
        after = fpu_interface_s65_capture(state.machine);
        failed |= after.eip != sizeof(esc) || after.eax != before.eax ||
            after.ecx != before.ecx || after.edx != before.edx ||
            after.ebx != before.ebx || after.esp != before.esp ||
            after.ebp != before.ebp || after.esi != before.esi ||
            after.edi != before.edi || after.eflags != before.eflags ||
            lib_memory_compare(&after.es, &before.es, sizeof(after.es)) != 0 ||
            lib_memory_compare(&after.cs, &before.cs, sizeof(after.cs)) != 0 ||
            lib_memory_compare(&after.ss, &before.ss, sizeof(after.ss)) != 0 ||
            lib_memory_compare(&after.ds, &before.ds, sizeof(after.ds)) != 0 ||
            lib_memory_compare(&after.fs, &before.fs, sizeof(after.fs)) != 0 ||
            lib_memory_compare(&after.gs, &before.gs, sizeof(after.gs)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 fpu_interface_s65_protected_nm(void)
{
    static const lib_u8 gdt_pointer[] = { 0x1fu,0u,0u,0x03u,0u,0u };
    static const lib_u8 idt_pointer[] = { 0xffu,0u,0u,0x04u,0u,0u };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const lib_u8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u, 0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0,0x0fu,0x01u,0xf0u, 0xb8u,0x10u,0,0x8eu,0xd8u,
        0xb8u,0x18u,0,0x8eu,0xd0u, 0xeau,0,0,0x08u,0
    };
    static const lib_u8 esc[] = { 0xd8u,0xc0u };
    static const lib_u8 hlt = 0xf4u;
    lib_u8 idt[0x100u] = { 0u };
    fpu_interface_s65_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_u16 frame_ip = 0u;
    lib_i32 failed = !fpu_interface_s65_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_NONE, &state);

    idt[7u * 8u + 1u] = 0x01u;
    idt[7u * 8u + 2u] = 0x08u;
    idt[7u * 8u + 5u] = 0x86u;
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, FPU_S65_GDT_POINTER,
            gdt_pointer, sizeof(gdt_pointer)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, FPU_S65_GDT_BASE, gdt,
            sizeof(gdt)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x0110u, idt_pointer, sizeof(idt_pointer)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, FPU_S65_IDT_BASE, idt,
            sizeof(idt)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x0200u, real_code, sizeof(real_code)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, FPU_S65_CODE_BASE + 0x100u,
            &hlt, sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EIP, 0x0200u);
        lib_status boot_status = core_machine_run(state.machine,
            (core_machine_run_budget){9u,0u}, &result);
        failed |= boot_status != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 9u;
    }
    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_CR0,
            test_core_machine_fixture_read_register(state.machine, CORE_MACHINE_DEBUG_CR0) | VCPU_CR0_EM);
        before = fpu_interface_s65_capture(state.machine);
        failed |= core_machine_memory_write(state.machine, FPU_S65_CODE_BASE, esc,
            sizeof(esc)) != LIB_STATUS_OK || core_machine_run(state.machine,
            (core_machine_run_budget){64u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
        after = fpu_interface_s65_capture(state.machine);
        if (!failed) failed |= core_machine_memory_read(state.machine,
            after.ss.base + (lib_u16)after.esp, (void *)CORE_MACHINE_REFERENCE_OF(frame_ip),
            sizeof(frame_ip)) != LIB_STATUS_OK;
        if (!failed) failed |= !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
            diagnostic.last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_NM) || result.executed != 0u || result.ticks != 0u ||
            after.eip != 0x100u || frame_ip != 0u ||
            after.eax != before.eax || after.ecx != before.ecx ||
            after.edx != before.edx || after.ebx != before.ebx ||
            after.ebp != before.ebp || after.esi != before.esi ||
            after.edi != before.edi ||
            lib_memory_compare(&after.ds, &before.ds, sizeof(after.ds)) != 0 ||
            lib_memory_compare(&after.es, &before.es, sizeof(after.es)) != 0 ||
            lib_memory_compare(&after.fs, &before.fs, sizeof(after.fs)) != 0 ||
            lib_memory_compare(&after.gs, &before.gs, sizeof(after.gs)) != 0;
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = fpu_interface_s65_capture(state.machine);
        failed |= after.eip != 0x101u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    static const lib_u8 wait[] = { 0x9bu };
    static const lib_u8 fninit[] = { 0xdbu, 0xe3u };
    static const lib_u8 escapes[][2] = {
        { 0xd8u, 0xc0u }, { 0xd9u, 0xc0u }, { 0xdau, 0xc0u },
        { 0xdbu, 0xe3u }, { 0xdcu, 0xc0u }, { 0xddu, 0xc0u },
        { 0xdeu, 0xc0u }, { 0xdfu, 0xc0u }
    };
    static const lib_u8 attr_wait_66[] = { 0x66u, 0x9bu };
    static const lib_u8 attr_wait_67[] = { 0x67u, 0x9bu };
    static const lib_u8 attr_wait[] = { 0x66u, 0x67u, 0x9bu };
    static const lib_u8 attr_esc_66[] = { 0x66u, 0xdbu, 0xe3u };
    static const lib_u8 attr_esc_67[] = { 0x67u, 0xdbu, 0xe3u };
    static const lib_u8 attr_esc[] = { 0x66u, 0x67u, 0xdbu, 0xe3u };
    static const lib_u8 *const legacy_attributes[] = {
        attr_wait_66, attr_wait_67, attr_wait, attr_esc_66, attr_esc_67,
        attr_esc
    };
    static const lib_u8 legacy_attribute_sizes[] = { 2u, 2u, 3u, 3u, 3u, 4u };
    static const lib_u8 lock_forms[][5] = {
        { 0xf0u, 0x9bu }, { 0xf0u, 0x66u, 0x9bu },
        { 0xf0u, 0x67u, 0x9bu }, { 0xf0u, 0x66u, 0x67u, 0x9bu },
        { 0xf0u, 0xdbu, 0xe3u }, { 0xf0u, 0x66u, 0xdbu, 0xe3u },
        { 0xf0u, 0x67u, 0xdbu, 0xe3u },
        { 0xf0u, 0x66u, 0x67u, 0xdbu, 0xe3u }
    };
    static const lib_u8 lock_sizes[] = { 2u, 3u, 3u, 4u, 3u, 4u, 4u, 5u };
    core_machine_cpu_profile profile;
    lib_u8 index;
    lib_i32 failed = 0;

    for (profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        failed |= !fpu_interface_s65_success(wait, sizeof(wait), profile,
            X86_FPU_PROFILE_NONE, 0u);
        failed |= !fpu_interface_s65_success(fninit, sizeof(fninit), profile,
            X86_FPU_PROFILE_NONE, 0u);
        for (index = 0u; index != sizeof(escapes) / sizeof(escapes[0]);
            ++index) {
            failed |= !fpu_interface_s65_success(escapes[index],
                sizeof(escapes[index]), profile, X86_FPU_PROFILE_NONE,
                0u);
        }
    }
    failed |= !fpu_interface_s65_success(fninit, sizeof(fninit),
        CORE_MACHINE_CPU_PROFILE_8086, X86_FPU_PROFILE_8087, 0u);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_8086,
        X86_FPU_PROFILE_8087);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_80186,
        X86_FPU_PROFILE_8087);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_80286,
        X86_FPU_PROFILE_80287);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_80287);
    failed |= !fpu_interface_s65_handoff(CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_80387);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_8086,
        X86_FPU_PROFILE_8087);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_80186,
        X86_FPU_PROFILE_8087);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_80286,
        X86_FPU_PROFILE_80287);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_80287);
    failed |= !fpu_interface_s65_deadline(CORE_MACHINE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_80387);
    failed |= !fpu_interface_s65_incompatible();
    failed |= !fpu_interface_s65_success(attr_wait, sizeof(attr_wait),
        CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_wait_66, sizeof(attr_wait_66),
        CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_wait_67, sizeof(attr_wait_67),
        CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_esc, sizeof(attr_esc),
        CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_esc_66, sizeof(attr_esc_66),
        CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_success(attr_esc_67, sizeof(attr_esc_67),
        CORE_MACHINE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, 0u);
    failed |= !fpu_interface_s65_mf();
    for (profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80286; ++profile) {
        for (index = 0u; index != sizeof(legacy_attributes) /
            sizeof(legacy_attributes[0]); ++index) {
            failed |= !fpu_interface_s65_reject(legacy_attributes[index],
                legacy_attribute_sizes[index], profile);
        }
    }
    for (index = 0u; index != sizeof(lock_forms) / sizeof(lock_forms[0]);
        ++index) {
        failed |= !fpu_interface_s65_reject(lock_forms[index], lock_sizes[index],
            CORE_MACHINE_CPU_PROFILE_80386);
    }
    if (!fpu_interface_s65_nm_delivery(wait, sizeof(wait),
        VCPU_CR0_TS | VCPU_CR0_MP) || !fpu_interface_s65_nm_delivery(fninit,
        sizeof(fninit), VCPU_CR0_EM) || !fpu_interface_s65_nm_delivery(fninit,
        sizeof(fninit), VCPU_CR0_TS)) {
        fprintf(stderr, "S65 stage=nm-delivery\n");
        failed = 1;
    }
    for (index = 0u; index != sizeof(escapes) / sizeof(escapes[0]); ++index) {
        if (!fpu_interface_s65_nm_delivery(escapes[index],
            sizeof(escapes[index]), VCPU_CR0_EM)) {
            fprintf(stderr, "S65 stage=escape-nm index=%u\n", index);
            failed = 1;
        }
        if (!fpu_interface_s65_nm_delivery(escapes[index],
            sizeof(escapes[index]), VCPU_CR0_TS)) {
            fprintf(stderr, "S65 stage=escape-ts index=%u\n", index);
            failed = 1;
        }
    }
    if (!fpu_interface_s65_vm86()) {
        fprintf(stderr, "S65 stage=vm86\n");
        failed = 1;
    }
    if (!fpu_interface_s65_protected_nm()) {
        fprintf(stderr, "S65 stage=protected-nm\n");
        failed = 1;
    }
    if (failed) {
        return 1;
    }
    printf("M5:T316:S65:FPU-INTERFACE:OK\n");
    printf("M5:T437:S3:X87-CROSS-PROFILE-INTERFACE:PASS\n");
    return 0;
}
