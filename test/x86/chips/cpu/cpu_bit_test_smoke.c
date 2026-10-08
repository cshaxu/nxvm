#include "support/cpu_operand_probe_fixture.h"
#include "lib/types/file.h"

static lib_i32 bit_test_register_forms(void)
{
    static const lib_u8 opcodes[] = {0xa3u, 0xabu, 0xb3u, 0xbbu};
    static const lib_u32 expected[] = {2u, 2u, 0u, 0u};
    lib_u8 index;

    for (index = 0u; index < 4u; ++index) {
        lib_u8 operand32;
        for (operand32 = 0u; operand32 < 2u; ++operand32) {
            lib_u8 code[] = {0x66u, 0x0fu, opcodes[index], 0xc8u};
            cpu_instruction_fixture state;
            t_cpu after;
            lib_u8 bytes = operand32 ? 4u : 3u;

            if (!operand32) {
                code[0] = 0x0fu;
                code[1] = opcodes[index];
                code[2] = 0xc8u;
            }
            cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
            state.cpu.data.eax = operand32 ? 2u : 0xaabb0002u;
            state.cpu.data.ecx = 1u;
            state.cpu.data.eflags = VCPU_EFLAGS_ZF | VCPU_EFLAGS_OF;
            if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
                state.fault.valid ||
                after.data.eax != (operand32 ? expected[index] :
                    (0xaabb0000u | (expected[index] & 0xffffu))) ||
                !X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF)) return 0;
        }
    }
    return 1;
}

static lib_i32 bit_test_immediate_and_memory(void)
{
    static const lib_u8 groups[] = {4u, 5u, 6u, 7u};
    lib_u8 group;

    for (group = 0u; group < 4u; ++group) {
        lib_u8 code[] = {0x0fu, 0xbau,
            (lib_u8)(0xe1u + (groups[group] - 4u) * 8u), 1u};
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.ecx = 2u;
        state.cpu.data.eflags = VCPU_EFLAGS_ZF;
        if (cpu_instruction_run(&state, code, sizeof(code), &after) !=
                LIB_STATUS_OK || state.fault.valid ||
            after.data.ecx != (group >= 2u ? 0u : 2u) ||
            !X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF)) return 0;
    }
    {
        static const lib_u8 signed_code[] = {0x0fu, 0xabu, 0x0eu, 0x02u, 0x40u};
        static const lib_u8 immediate_code[] = {0x0fu, 0xbau, 0x2eu,
            0x00u, 0x40u, 0x10u};
        static const lib_u8 immediate32_code[] = {0x66u, 0x0fu, 0xbau,
            0x2eu, 0x08u, 0x40u, 0x21u};
        lib_u16 first = 0u, second = 0u, read = 0u;
        lib_u32 third = 0u, fourth = 0u;
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.ecx = 0xffffu;
        state.cpu.data.eflags = VCPU_EFLAGS_ZF;
        if (cpu_instruction_write(&state, 0x4000u, &first, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x4002u, &second, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, signed_code, sizeof(signed_code),
                &after) != LIB_STATUS_OK || state.fault.valid ||
            cpu_instruction_read(&state, 0x4000u, &read, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || read != 0x8000u ||
            X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF)) return 0;

        state.cpu.data.eip = 0u;
        state.cpu.data.eflags = VCPU_EFLAGS_ZF;
        if (cpu_instruction_run(&state, immediate_code, sizeof(immediate_code),
                &after) != LIB_STATUS_OK || state.fault.valid ||
            cpu_instruction_read(&state, 0x4002u, &read, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || read != 0u ||
            cpu_instruction_read(&state, 0x4000u, &read, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || read != 0x8001u ||
            X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF)) return 0;

        state.cpu.data.eip = 0u;
        state.cpu.data.eflags = VCPU_EFLAGS_ZF;
        if (cpu_instruction_write(&state, 0x4008u, &third, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x400cu, &fourth, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, immediate32_code,
                sizeof(immediate32_code), &after) != LIB_STATUS_OK ||
            state.fault.valid ||
            cpu_instruction_read(&state, 0x400cu, &fourth, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || fourth != 0u ||
            cpu_instruction_read(&state, 0x4008u, &third, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || third != 2u ||
            X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF)) return 0;
    }
    {
        static const lib_u8 memory_bt[] = {0x0fu, 0xa3u, 0x0eu, 0x00u, 0x40u};
        static const lib_u8 address32_bts[] = {0x67u, 0x66u, 0x0fu, 0xabu, 0x0eu};
        lib_u16 word = 2u, read16 = 0u;
        lib_u32 dword = 0u, read32 = 0u;
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.ecx = 1u;
        if (cpu_instruction_write(&state, 0x4000u, &word, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, memory_bt, sizeof(memory_bt), &after) !=
                LIB_STATUS_OK || state.fault.valid ||
            cpu_instruction_read(&state, 0x4000u, &read16, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || read16 != word ||
            !X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF)) return 0;

        state.cpu.data.eip = 0u;
        state.cpu.data.esi = 0x4004u;
        state.cpu.data.ecx = 1u;
        if (cpu_instruction_write(&state, 0x4004u, &dword, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, address32_bts,
                sizeof(address32_bts), &after) != LIB_STATUS_OK ||
            state.fault.valid ||
            cpu_instruction_read(&state, 0x4004u, &read32, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || read32 != 2u ||
            X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF)) return 0;
    }
    return 1;
}

static lib_i32 bit_test_memory_destination_forms(void)
{
    static const lib_u8 opcodes[] = {0xa3u, 0xabu, 0xb3u, 0xbbu};
    lib_u8 form;

    for (form = 0u; form < 4u; ++form) {
        lib_u8 indexed[] = {0x0fu, opcodes[form], 0x0eu, 0x00u, 0x40u};
        lib_u8 immediate[] = {0x0fu, 0xbau,
            (lib_u8)(0x26u + form * 8u), 0x00u, 0x40u, 1u};
        lib_u16 value = 2u, read = 0u;
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.ecx = 1u;
        if (cpu_instruction_write(&state, 0x4000u, &value, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, indexed, sizeof(indexed), &after) !=
                LIB_STATUS_OK || state.fault.valid ||
            cpu_instruction_read(&state, 0x4000u, &read, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || read != (form >= 2u ? 0u : 2u) ||
            !X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF)) return 0;

        state.cpu.data.eip = 0u;
        value = 2u;
        if (cpu_instruction_write(&state, 0x4000u, &value, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, immediate, sizeof(immediate), &after) !=
                LIB_STATUS_OK || state.fault.valid ||
            cpu_instruction_read(&state, 0x4000u, &read, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || read != (form >= 2u ? 0u : 2u) ||
            !X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF)) return 0;
    }
    return 1;
}

/* REAL_UD_TERMINAL_CPU_OWNER: illegal group and older profiles. */
static lib_i32 bit_test_rejection(void)
{
    static const lib_u8 invalid_ba[] = {0x0fu, 0xbau, 0x06u, 0x00u, 0x50u, 0u};
    static const lib_u8 bt_memory[] = {0x0fu, 0xa3u, 0x0eu, 0x00u, 0x50u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 index;

    for (index = 0u; index < 3u; ++index) {
        cpu_operand_probe_fixture fixture;
        cpu_instruction_fixture *state = &fixture.instruction;
        const lib_u8 *code = index == 2u ? invalid_ba : bt_memory;
        lib_u8 bytes = index == 2u ? sizeof(invalid_ba) : sizeof(bt_memory);

        cpu_operand_probe_prepare(&fixture, profiles[index]);
        state->cpu.data.eflags = VCPU_EFLAGS_ZF;
        state->cpu.data.ecx = 0xaabbccddu;
        if (!cpu_instruction_expect_real_fault(state, code, bytes, 6u) ||
            fixture.operand_reads != 0u || fixture.operand_writes != 0u) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!bit_test_register_forms() || !bit_test_immediate_and_memory() ||
        !bit_test_memory_destination_forms() || !bit_test_rejection()) return 1;
    lib_c_printf("CPU-BIT-TEST:OK\n");
    return 0;
}
