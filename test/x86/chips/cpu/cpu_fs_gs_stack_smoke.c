#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* T337_REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
static lib_i32 fs_gs_test_real(void)
{
    static const lib_u8 opcodes[] = { 0xa0u, 0xa1u, 0xa8u, 0xa9u };
    lib_u8 opcode;
    lib_u8 size;
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
    for (size = 0u; size != 2u; ++size) {
        cpu_instruction_fixture state;

        t_cpu after;
        lib_status status;
        lib_u8 code[] = { 0x0fu, opcodes[opcode], 0u };
        lib_u32 image = 0u;
        lib_u32 before_esp = 0x8000u;
        lib_u16 selector = opcode < 2u ? 0x1234u : 0x5678u;
        lib_i32 pop = (opcodes[opcode] & 1u) != 0u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (!failed && size) {
            code[0] = 0x66u;
            code[1] = 0x0fu;
            code[2] = opcodes[opcode];
        }
        if (!failed) {
            state.cpu.data.esp = before_esp;
            state.cpu.data.eax = 0x11223344u;
            state.cpu.data.ecx = 0x55667788u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            state.cpu.data.fs.selector = 0x1111u;
            state.cpu.data.gs.selector = 0x2222u;
            if (!pop && opcode < 2u) state.cpu.data.fs.selector = selector;
            if (!pop && opcode >= 2u) state.cpu.data.gs.selector = selector;
            if (pop)
                failed |= cpu_instruction_write(&state, before_esp, &selector, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            status = cpu_instruction_run(&state, code, size ? 3u : 2u, &after);
            failed |= status != LIB_STATUS_OK || state.fault.valid ||
                after.data.eip != (size ? 3u : 2u) || after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                after.data.eax != 0x11223344u || after.data.ecx != 0x55667788u ||
                after.data.esp != (pop ? before_esp + (size ? 4u : 2u) :
                before_esp - (size ? 4u : 2u));
            if (!pop)
                failed |= cpu_instruction_read(&state, after.data.ss.base + after.data.esp, &image, size ? 4u : 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                    (size ? image != (lib_u32)selector : (image & 0xffffu) != selector);
            else if (opcode < 2u) failed |= after.data.fs.selector != selector;
            else failed |= after.data.gs.selector != selector;
        }

        if (failed) return 0;
    }
    }
    return 1;
}

static lib_i32 fs_gs_test_80286_reject(void)
{
    static const lib_u8 opcodes[] = { 0xa0u, 0xa1u, 0xa8u, 0xa9u };
    lib_u8 opcode;
    lib_u8 size;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (size = 0u; size != 2u; ++size) {
            cpu_instruction_fixture state;

            t_cpu before;
            t_cpu after;
            lib_status status;
            lib_u8 code[] = { 0x0fu, opcodes[opcode], 0u };
            lib_i32 failed = 0;
            cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);

            if (!failed && size) {
                code[0] = 0x66u;
                code[1] = 0x0fu;
                code[2] = opcodes[opcode];
            }
            if (!failed) {
                state.cpu.data.esp = 0x8000u;
                state.cpu.data.fs.selector = 0x1234u;
                state.cpu.data.gs.selector = 0x5678u;
                state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                state.cpu.data.idtr.limit = 0x17u;
                before = state.cpu;
                status = cpu_instruction_run(&state, code, size ? 3u : 2u, &after);
                failed |= status != LIB_STATUS_INTERNAL_ERROR ||
                    !state.fault.valid || !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
                    after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
                    after.data.fs.selector != before.data.fs.selector ||
                    after.data.gs.selector != before.data.gs.selector ||
                    after.data.eflags != before.data.eflags;
            }

            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 fs_gs_prepare_protected(cpu_instruction_fixture *state)
{
    static const lib_u8 pointer[] = { 0x1fu,0,0,0x03u,0,0 };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    lib_memory_copy(state->memory + 0x100u, pointer, sizeof(pointer));
    lib_memory_copy(state->memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(state->memory, bootstrap, sizeof(bootstrap));
    for (lib_u8 step = 0u; step != 10u; ++step) {
        core_machine_cpu_execution_refresh(&state->execution);
        if (state->execution.stop_requested || state->fault.valid) return 0;
    }
    return state->cpu.data.cs.selector == 8u && state->cpu.data.eip == 0u;
}

static lib_i32 fs_gs_test_protected_pop(void)
{
    static const lib_u8 opcodes[] = { 0xa1u, 0xa9u };
    lib_u8 opcode;
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;

        t_cpu after;
        lib_u16 selector = 0x0010u;
        lib_i32 failed = !fs_gs_prepare_protected(&state);
        if (!failed) {
            state.cpu.data.fs.selector = 0x1111u;
            state.cpu.data.gs.selector = 0x2222u;
            failed |= cpu_instruction_write(&state, 0xc000u, &selector, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
                cpu_instruction_write(&state, 0x2000u, (lib_u8[]){0x0fu,opcodes[opcode]}, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            state.cpu.data.eip = 0u;
            core_machine_cpu_execution_refresh(&state.execution);
            failed |= state.execution.stop_requested;
            after = state.cpu;
            failed |= after.data.eip != 2u || after.data.esp != 0x00008002u ||
                (opcode == 0u ? after.data.fs.selector : after.data.gs.selector) != selector;
        }

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 fs_gs_test_pop_stack_fault(void)
{
    static const lib_u8 opcodes[] = { 0xa1u, 0xa9u };
    lib_u8 opcode;
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u16 before_selector;
        lib_u32 before_flags;
        lib_i32 failed = !fs_gs_prepare_protected(&state);
        if (!failed) {
            state.cpu.data.ss.limit = 0x7fffu;
            state.cpu.data.esp = 0x8000u;
            state.cpu.data.fs.selector = 0x1111u;
            state.cpu.data.gs.selector = 0x2222u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            before_selector = opcode == 0u ? state.cpu.data.fs.selector :
                state.cpu.data.gs.selector;
            before_flags = state.cpu.data.eflags;
            failed |= cpu_instruction_write(&state, 0x2000u, (lib_u8[]){0x0fu,opcodes[opcode]}, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            state.cpu.data.eip = 0u;
            core_machine_cpu_execution_refresh(&state.execution);
            failed |= !state.execution.stop_requested;
            after = state.cpu;
            failed |= !state.fault.valid || !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || after.data.esp != 0x8000u ||
                after.data.eflags != before_flags ||
                (opcode == 0u ? after.data.fs.selector : after.data.gs.selector) !=
                    before_selector;
        }

        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!fs_gs_test_real()) { lib_c_printf("FS-GS stage=real\n"); return 1; }
    if (!fs_gs_test_80286_reject()) { lib_c_printf("FS-GS stage=reject\n"); return 1; }
    if (!fs_gs_test_protected_pop()) { lib_c_printf("FS-GS stage=protected\n"); return 1; }
    if (!fs_gs_test_pop_stack_fault()) return 1;
    lib_c_printf("M5:T539:S26:CPU-FS-GS-STACK:OK\n");
    return 0;
}
