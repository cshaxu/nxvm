#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
static lib_i32 lld_test_real(void)
{
    static const lib_u8 opcodes[] = { 0xc4u, 0xc5u };
    static const core_machine_cpu_profile legacy_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 pointer16[] = { 0x44u, 0x33u, 0x34u, 0x12u };
    static const lib_u8 pointer32[] = {
        0x44u, 0x33u, 0x22u, 0x11u, 0x34u, 0x12u
    };
    lib_u8 opcode;
    lib_u8 profile;
    lib_u8 operand32;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (profile = 0u; profile != sizeof(legacy_profiles) /
                sizeof(legacy_profiles[0]); ++profile) {
            for (operand32 = 0u; operand32 != 2u; ++operand32) {
                if (operand32 && profile != 2u)
                    continue;
                cpu_instruction_fixture state;
                t_cpu after;
                lib_status status;
                lib_u8 code[] = { opcodes[opcode], 0x06u, 0x00u, 0x10u, 0u };
                const lib_u8 *pointer = operand32 ? pointer32 : pointer16;
                lib_u8 code_bytes = operand32 ? 5u : 4u;
                lib_u8 pointer_bytes = operand32 ? 6u : 4u;
                lib_i32 failed = 0;
                cpu_instruction_prepare(&state, legacy_profiles[profile]);

                if (operand32) {
                    code[0] = 0x66u;
                    code[1] = opcodes[opcode];
                    code[2] = 0x06u;
                    code[3] = 0x00u;
                    code[4] = 0x10u;
                }
                if (!failed) {
                    state.cpu.data.eflags =
                        VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                    state.cpu.data.es.selector = 0x1111u;
                    state.cpu.data.ds.selector = 0x2222u;
                    failed |= cpu_instruction_write(&state, 0x1000u, pointer, pointer_bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
                        (status = cpu_instruction_run(&state, code, code_bytes, &after)) != LIB_STATUS_OK ||
                        state.fault.valid || after.data.eip != code_bytes ||
                        after.data.eflags !=
                            (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                        (operand32 ? after.data.eax : after.data.ax) !=
                            (operand32 ? 0x11223344u : 0x3344u) ||
                        (opcode == 0u ? after.data.es.selector :
                            after.data.ds.selector) != 0x1234u;
                }

                if (failed)
                    return 0;
            }
        }
    }
    return 1;
}

static lib_i32 lld_test_reg_direct_ud(void)
{
    static const lib_u8 opcodes[] = { 0xc4u, 0xc5u };
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        lib_u8 code[] = { opcodes[opcode], 0xc0u };
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            state.cpu.data.es.selector = 0x1111u;
            state.cpu.data.ds.selector = 0x2222u;
            failed |= !cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u);
        }

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 lld_test_80286_operand32_ud(void)
{
    static const lib_u8 opcodes[] = { 0xc4u, 0xc5u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 opcode;
    lib_u8 profile;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
                ++profile) {
            cpu_instruction_fixture state;
            lib_u8 code[] = { 0x66u, opcodes[opcode], 0x06u, 0x00u, 0x10u };
            lib_i32 failed = 0;
            cpu_instruction_prepare(&state, profiles[profile]);

            if (!failed) {
                state.cpu.data.es.selector = 0x1111u;
                state.cpu.data.ds.selector = 0x2222u;
                failed |= !cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u);
            }

            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 lld_prepare_protected(cpu_instruction_fixture *state)
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

static lib_i32 lld_test_protected(void)
{
    static const lib_u8 opcodes[] = { 0xc4u, 0xc5u };
    static const lib_u8 pointer16[] = { 0x44u, 0x33u, 0x10u, 0x00u };
    static const lib_u8 pointer32[] = {
        0x44u, 0x33u, 0x22u, 0x11u, 0x10u, 0x00u
    };
    lib_u8 opcode;
    lib_u8 operand32;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (operand32 = 0u; operand32 != 2u; ++operand32) {
            cpu_instruction_fixture state;
            t_cpu after;
            lib_u8 code[] = { opcodes[opcode], 0x06u, 0x00u, 0x10u, 0u };
            const lib_u8 *pointer = operand32 ? pointer32 : pointer16;
            lib_u8 code_bytes = operand32 ? 5u : 4u;
            lib_u8 pointer_bytes = operand32 ? 6u : 4u;
            lib_i32 failed = !lld_prepare_protected(&state);

            if (operand32) {
                code[0] = 0x66u;
                code[1] = opcodes[opcode];
                code[2] = 0x06u;
                code[3] = 0x00u;
                code[4] = 0x10u;
            }
            if (!failed) {
                state.cpu.data.es.selector = 0x1111u;
                state.cpu.data.ds.selector = 0x2222u;
                state.cpu.data.eflags =
                    VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                failed |= cpu_instruction_write(&state, 0x1000u, pointer, pointer_bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
                    cpu_instruction_write(&state, 0x2000u, code, code_bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
                state.cpu.data.eip = 0u;
                core_machine_cpu_execution_refresh(&state.execution);
                failed |= state.execution.stop_requested;
                after = state.cpu;
                failed |= after.data.eip != code_bytes ||
                    (operand32 ? after.data.eax : after.data.ax) !=
                        (operand32 ? 0x11223344u : 0x3344u) ||
                    after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                    (opcode == 0u ? after.data.es.selector :
                        after.data.ds.selector) != 0x0010u;
            }

            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 lld_test_source_fault_atomicity(void)
{
    static const lib_u8 opcodes[] = { 0xc4u, 0xc5u };
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u8 code[] = { opcodes[opcode], 0x06u, 0x00u, 0x10u };
        lib_i32 failed = !lld_prepare_protected(&state);

        if (!failed) {
            state.cpu.data.ds.limit = 0x1001u;
            state.cpu.data.eax = 0x55557777u;
            state.cpu.data.es.selector = 0x1111u;
            state.cpu.data.ds.selector = 0x2222u;
            state.cpu.data.eflags =
                VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            failed |= cpu_instruction_write(&state, 0x2000u, code, sizeof(code), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            state.cpu.data.eip = 0u;
            before = state.cpu;
            core_machine_cpu_execution_refresh(&state.execution);
            failed |= !core_machine_cpu_is_shutdown(&state.execution) || state.execution.stop_requested;
            after = state.cpu;
            failed |= state.fault.valid || !state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
                after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags ||
                (opcode == 0u ? after.data.es.selector : after.data.ds.selector) !=
                    (opcode == 0u ? before.data.es.selector :
                        before.data.ds.selector);
        }

        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!lld_test_real() || !lld_test_reg_direct_ud() ||
            !lld_test_80286_operand32_ud() || !lld_test_protected() ||
            !lld_test_source_fault_atomicity())
        return 1;
    lib_c_printf("LES-LDS:OK\n");
    return 0;
}
