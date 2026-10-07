#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* T337_REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
static lib_i32 lfg_test_real(void)
{
    static const lib_u8 op[] = { 0xb2u, 0xb4u, 0xb5u };
    lib_u8 i;
    lib_u8 z;

    for (i = 0u; i < 3u; ++i) {
        for (z = 0u; z < 2u; ++z) {
            cpu_instruction_fixture s;
            t_cpu a = {0};
            lib_status st = LIB_STATUS_INVALID_STATE;
            lib_u8 c[] = { 0x0fu, op[i], 0x06u, 0, 0x10u, 0 };
            lib_u8 p16[] = { 0x44u, 0x33u, 0x34u, 0x12u };
            lib_u8 p32[] = { 0x44u, 0x33u, 0x22u, 0x11u, 0x34u, 0x12u };
            lib_i32 f = 0;
            cpu_instruction_prepare(&s, CORE_MACHINE_CPU_PROFILE_80386);

            if (z) {
                c[0] = 0x66u;
                c[1] = 0x0fu;
                c[2] = op[i];
                c[3] = 0x06u;
                c[4] = 0u;
                c[5] = 0x10u;
            }
            if (!f) {
                s.cpu.data.eflags =
                    VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                f |= cpu_instruction_write(&s, 0x1000u, z ? p32 :
                        p16, z ? 6u : 4u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
                    (st = cpu_instruction_run(&s, c, z ? 6u : 5u, &a)) != LIB_STATUS_OK || s.fault.valid ||
                    a.data.eip != (z ? 6u : 5u) ||
                    a.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                    (z ? a.data.eax : a.data.ax) !=
                        (z ? 0x11223344u : 0x3344u);
                if (i == 0u)
                    f |= a.data.ss.selector != 0x1234u;
                else if (i == 1u)
                    f |= a.data.fs.selector != 0x1234u;
                else
                    f |= a.data.gs.selector != 0x1234u;
            }
            if (f) {
                lib_c_printf("LFG real op=%02x size=%u status=%d eip=%08x eax=%08x ss=%04x fs=%04x gs=%04x flags=%08x first=%u mask=%08x\n",
                    op[i], z, st, a.data.eip, a.data.eax, a.data.ss.selector,
                    a.data.fs.selector, a.data.gs.selector, a.data.eflags,
                    s.fault.valid, s.fault.exception_mask);

                return 0;
            }

        }
    }
    return 1;
}
static lib_i32 lfg_test_reg_direct_80386(void)
{
    static const lib_u8 op[] = { 0xb2u, 0xb4u, 0xb5u };
    lib_u8 i;

    for (i = 0u; i < 3u; ++i) {
        cpu_instruction_fixture s;
        lib_u8 c[] = { 0x0fu, op[i], 0xc0u };
        lib_i32 f = 0;
        cpu_instruction_prepare(&s, CORE_MACHINE_CPU_PROFILE_80386);

        if (!f) {
            s.cpu.data.ss.selector = 0x0018u;
            s.cpu.data.fs.selector = 0x1111u;
            s.cpu.data.gs.selector = 0x2222u;
            f |= !cpu_instruction_expect_real_fault(&s, c, 3u, 6u);
        }

        if (f)
            return 0;
    }
    return 1;
}

static lib_i32 lfg_test_80286_memory(void)
{
    static const lib_u8 op[] = { 0xb2u, 0xb4u, 0xb5u };
    lib_u8 i;
    lib_u8 z;

    for (i = 0u; i < 3u; ++i) {
        for (z = 0u; z < 2u; ++z) {
            cpu_instruction_fixture s;
            lib_u8 c[] = { 0x0fu, op[i], 0x06u, 0, 0x10u, 0 };
            lib_i32 f = 0;
            cpu_instruction_prepare(&s, CORE_MACHINE_CPU_PROFILE_80286);

            if (z) {
                c[0] = 0x66u;
                c[1] = 0x0fu;
                c[2] = op[i];
                c[3] = 0x06u;
                c[4] = 0u;
                c[5] = 0x10u;
            }
            if (!f) {
                s.cpu.data.ss.selector = 0x0018u;
                s.cpu.data.fs.selector = 0x1111u;
                s.cpu.data.gs.selector = 0x2222u;
                f |= !cpu_instruction_expect_real_fault(&s, c, z ? 6u : 5u, 6u);
            }

            if (f)
                return 0;
        }
    }
    return 1;
}
static lib_i32 lfg_prepare_protected(cpu_instruction_fixture *state)
{
    static const lib_u8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const lib_u8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0xffu, 0xffu, 0, 0, 0, 0x92u, 0, 0, 0xffu, 0xffu, 0, 0x40u, 0, 0x92u,
        0, 0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u,
        0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u, 0xb8u, 0x18u,
        0x00u, 0x8eu, 0xd0u, 0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u,
        0x00u
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
static lib_i32 lfg_test_protected(void)
{
    static const lib_u8 opcodes[] = { 0xb2u, 0xb4u, 0xb5u };
    static const lib_u8 pointer16[] = { 0x44u, 0x33u, 0x10u, 0x00u };
    static const lib_u8 pointer32[] = { 0x44u, 0x33u, 0x22u, 0x11u, 0x10u, 0x00u };
    lib_u8 opcode;
    lib_u8 operand32;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (operand32 = 0u; operand32 != 2u; ++operand32) {
            cpu_instruction_fixture state;
            t_cpu after;
            lib_u8 code[] = { 0x0fu, opcodes[opcode], 0x06u, 0x00u, 0x10u, 0u };
            const lib_u8 *pointer = operand32 ? pointer32 : pointer16;
            lib_u8 code_bytes = operand32 ? 6u : 5u;
            lib_u8 pointer_bytes = operand32 ? 6u : 4u;
            lib_u32 expected_offset = operand32 ? 0x11223344u : 0x3344u;
            lib_i32 failed = !lfg_prepare_protected(&state);

            if (!failed && operand32) {
                code[0] = 0x66u;
                code[1] = 0x0fu;
                code[2] = opcodes[opcode];
                code[3] = 0x06u;
                code[4] = 0x00u;
                code[5] = 0x10u;
            }
            if (!failed) {
                state.cpu.data.ss.selector = 0x0018u;
                state.cpu.data.fs.selector = 0x1111u;
                state.cpu.data.gs.selector = 0x2222u;
                state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                failed |= cpu_instruction_write(&state, 0x1000u, pointer, pointer_bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
                    cpu_instruction_write(&state, 0x2000u, code, code_bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
                state.cpu.data.eip = 0u;
                core_machine_cpu_execution_refresh(&state.execution);
                failed |= state.execution.stop_requested;
                after = state.cpu;
                failed |= after.data.eip != code_bytes ||
                    (operand32 ? after.data.eax : after.data.ax) != expected_offset ||
                    after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                    (opcode == 0u ? after.data.ss.selector : opcode == 1u ?
                        after.data.fs.selector : after.data.gs.selector) != 0x0010u;
            }

            if (failed)
                return 0;
        }
    }
    return 1;
}
static lib_i32 lfg_test_source_fault_atomicity(void)
{
    static const lib_u8 opcodes[] = { 0xb2u, 0xb4u, 0xb5u };
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        t_cpu before = {0};
        t_cpu after = {0};
        lib_u8 code[] = { 0x0fu, opcodes[opcode], 0x06u, 0x00u, 0x10u };
        lib_i32 failed = !lfg_prepare_protected(&state);

        if (!failed) {
            state.cpu.data.ds.limit = 0x1001u;
            state.cpu.data.eax = 0x55557777u;
            state.cpu.data.ss.selector = 0x0018u;
            state.cpu.data.fs.selector = 0x1111u;
            state.cpu.data.gs.selector = 0x2222u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            failed |= cpu_instruction_write(&state, 0x2000u, code, sizeof(code), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            state.cpu.data.eip = 0u;
            before = state.cpu;
            core_machine_cpu_execution_refresh(&state.execution);
            failed |= !core_machine_cpu_is_shutdown(&state.execution) || state.execution.stop_requested;
            after = state.cpu;
            failed |= state.fault.valid || !state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
                after.data.eip != before.data.eip || after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags ||
                (opcode == 0u ? after.data.ss.selector : opcode == 1u ?
                    after.data.fs.selector : after.data.gs.selector) !=
                    (opcode == 0u ? before.data.ss.selector : opcode == 1u ?
                        before.data.fs.selector : before.data.gs.selector);
        }
        if (failed)
            lib_c_printf("LFG source-fault op=%02x reason=%d first=%u mask=%08x eip=%08x/%08x eax=%08x/%08x flags=%08x/%08x\n",
                opcodes[opcode], state.execution.stop_requested, state.fault.valid,
                state.fault.exception_mask, before.data.eip, after.data.eip,
                before.data.eax, after.data.eax, before.data.eflags, after.data.eflags);

        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!lfg_test_real()) {
        lib_c_printf("LFG stage=real\n");
        return 1;
    }
    if (!lfg_test_reg_direct_80386()) {
        lib_c_printf("LFG stage=regdirect\n");
        return 1;
    }
    if (!lfg_test_80286_memory()) {
        lib_c_printf("LFG stage=80286\n");
        return 1;
    }
    if (!lfg_test_protected()) {
        lib_c_printf("LFG stage=protected\n");
        return 1;
    }
    if (!lfg_test_source_fault_atomicity())
        return 1;
    lib_c_printf("M5:T316:S24:LSS-LFS-LGS:OK\n");
    return 0;
}
