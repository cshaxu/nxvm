#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct fs_gs_machine { core_machine *machine; } fs_gs_machine;

static C_VOID fs_gs_reset(C_VOID *opaque)
{
    fs_gs_machine *state = (fs_gs_machine *)opaque;
    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider fs_gs_provider = {
    fs_gs_reset, STD_NULL
};

static C_INT fs_gs_prepare(core_machine_cpu_profile profile, fs_gs_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &fs_gs_provider, state, &state->machine);
}

static C_INT fs_gs_run(fs_gs_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes,
    t_cpu *after, core_machine_cpu_diagnostic *diagnostic, type_status *status)
{
    core_machine_run_result result;
    if (!test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u }, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK &&
        result.reason == (*status == TYPE_STATUS_OK ? CORE_MACHINE_STOP_BUDGET :
            CORE_MACHINE_STOP_FAULT);
}

static C_INT fs_gs_test_real(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xa0u, 0xa1u, 0xa8u, 0xa9u };
    type_unsigned_8 opcode;
    type_unsigned_8 size;
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
    for (size = 0u; size != 2u; ++size) {
        fs_gs_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu after;
        type_status status;
        type_unsigned_8 code[] = { 0x0fu, opcodes[opcode], 0u };
        type_unsigned_32 image = 0u;
        type_unsigned_32 before_esp = 0x8000u;
        type_unsigned_16 selector = opcode < 2u ? 0x1234u : 0x5678u;
        C_INT pop = (opcodes[opcode] & 1u) != 0u;
        C_INT failed = !fs_gs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed && size) {
            code[0] = 0x66u;
            code[1] = 0x0fu;
            code[2] = opcodes[opcode];
        }
        if (!failed) {
            state.machine->executor_cpu.data.esp = before_esp;
            state.machine->executor_cpu.data.eax = 0x11223344u;
            state.machine->executor_cpu.data.ecx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            state.machine->executor_cpu.data.fs.selector = 0x1111u;
            state.machine->executor_cpu.data.gs.selector = 0x2222u;
            if (!pop && opcode < 2u) state.machine->executor_cpu.data.fs.selector = selector;
            if (!pop && opcode >= 2u) state.machine->executor_cpu.data.gs.selector = selector;
            if (pop)
                failed |= core_machine_memory_write(state.machine, before_esp, &selector, 2u) != TYPE_STATUS_OK;
            failed |= !fs_gs_run(&state, code, size ? 3u : 2u, &after, &diagnostic, &status) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != (size ? 3u : 2u) || after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                after.data.eax != 0x11223344u || after.data.ecx != 0x55667788u ||
                after.data.esp != (pop ? before_esp + (size ? 4u : 2u) :
                before_esp - (size ? 4u : 2u));
            if (!pop)
                failed |= core_machine_memory_read(state.machine, after.data.ss.base + after.data.esp,
                    &image, size ? 4u : 2u) != TYPE_STATUS_OK ||
                    (size ? image != (type_unsigned_32)selector : (image & 0xffffu) != selector);
            else if (opcode < 2u) failed |= after.data.fs.selector != selector;
            else failed |= after.data.gs.selector != selector;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    }
    return 1;
}

static C_INT fs_gs_test_80286_reject(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xa0u, 0xa1u, 0xa8u, 0xa9u };
    type_unsigned_8 opcode;
    type_unsigned_8 size;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (size = 0u; size != 2u; ++size) {
            fs_gs_machine state;
            core_machine_cpu_diagnostic diagnostic;
            t_cpu before;
            t_cpu after;
            type_status status;
            type_unsigned_8 code[] = { 0x0fu, opcodes[opcode], 0u };
            C_INT failed = !fs_gs_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

            if (!failed && size) {
                code[0] = 0x66u;
                code[1] = 0x0fu;
                code[2] = opcodes[opcode];
            }
            if (!failed) {
                state.machine->executor_cpu.data.esp = 0x8000u;
                state.machine->executor_cpu.data.fs.selector = 0x1234u;
                state.machine->executor_cpu.data.gs.selector = 0x5678u;
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !fs_gs_run(&state, code, size ? 3u : 2u, &after,
                        &diagnostic, &status) || status != TYPE_STATUS_FAULT ||
                    !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                        diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
                    after.data.fs.selector != before.data.fs.selector ||
                    after.data.gs.selector != before.data.gs.selector ||
                    after.data.eflags != before.data.eflags;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT fs_gs_prepare_protected(fs_gs_machine *state)
{
    static const type_unsigned_8 pointer[] = { 0x1fu,0,0,0x03u,0,0 };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;
    return fs_gs_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer, sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap, sizeof(bootstrap)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, halt, sizeof(halt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 96u,0u }, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT fs_gs_test_protected_pop(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xa1u, 0xa9u };
    type_unsigned_8 opcode;
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        fs_gs_machine state;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_16 selector = 0x0010u;
        C_INT failed = !fs_gs_prepare_protected(&state);
        if (!failed) {
            state.machine->executor_cpu.data.fs.selector = 0x1111u;
            state.machine->executor_cpu.data.gs.selector = 0x2222u;
            failed |= core_machine_memory_write(state.machine, 0xc000u, &selector, 2u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, (type_unsigned_8[]){0x0fu,opcodes[opcode]}, 2u) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= after.data.eip != 2u || after.data.esp != 0x00008002u ||
                (opcode == 0u ? after.data.fs.selector : after.data.gs.selector) != selector;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT fs_gs_test_pop_stack_fault(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xa1u, 0xa9u };
    type_unsigned_8 opcode;
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        fs_gs_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu after;
        type_unsigned_16 before_selector;
        type_unsigned_32 before_flags;
        C_INT failed = !fs_gs_prepare_protected(&state);
        if (!failed) {
            state.machine->executor_cpu.data.ss.limit = 0x7fffu;
            state.machine->executor_cpu.data.esp = 0x8000u;
            state.machine->executor_cpu.data.fs.selector = 0x1111u;
            state.machine->executor_cpu.data.gs.selector = 0x2222u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            before_selector = opcode == 0u ? state.machine->executor_cpu.data.fs.selector :
                state.machine->executor_cpu.data.gs.selector;
            before_flags = state.machine->executor_cpu.data.eflags;
            failed |= core_machine_memory_write(state.machine, 0x2000u,
                    (type_unsigned_8[]){0x0fu,opcodes[opcode]}, 2u) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
                    &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || after.data.esp != 0x8000u ||
                after.data.eflags != before_flags ||
                (opcode == 0u ? after.data.fs.selector : after.data.gs.selector) !=
                    before_selector;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!fs_gs_test_real()) { STD_PRINTF("FS-GS stage=real\n"); return 1; }
    if (!fs_gs_test_80286_reject()) { STD_PRINTF("FS-GS stage=reject\n"); return 1; }
    if (!fs_gs_test_protected_pop()) { STD_PRINTF("FS-GS stage=protected\n"); return 1; }
    if (!fs_gs_test_pop_stack_fault()) return 1;
    STD_PRINTF("M5:T316:S23:FS-GS-STACK:OK\n");
    return 0;
}
