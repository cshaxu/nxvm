#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static void sign_extend_set_registers(t_cpu *cpu)
{
    cpu->data.eax = 0xaabb0000u;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55660000u;
    cpu->data.ebx = 0x778899aau;
    cpu->data.esp = 0xbbbb8000u;
    cpu->data.ebp = 0xccccddddu;
    cpu->data.esi = 0xeeeeffffu;
    cpu->data.edi = 0x10203040u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
}

static lib_i32 sign_extend_nonparticipants_equal(const t_cpu *before,
    const t_cpu *after, lib_u8 opcode)
{
    return before->data.ecx == after->data.ecx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags &&
        (opcode == 0x98u || before->data.eax == after->data.eax) &&
        (opcode == 0x99u || before->data.edx == after->data.edx);
}

static lib_i32 sign_extend_test_default(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 opcodes[] = {0x98u, 0x99u};
    lib_u8 profile, opcode, sign;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (opcode = 0u; opcode < sizeof(opcodes); ++opcode)
    for (sign = 0u; sign < 2u; ++sign) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        const lib_u8 code[] = {opcodes[opcode]};
        lib_u32 expected_eax, expected_edx;

        cpu_instruction_prepare(&state, profiles[profile]);
        sign_extend_set_registers(&state.cpu);
        state.cpu.data.eax = opcodes[opcode] == 0x98u ?
            (sign == 0u ? 0xaabb007fu : 0xaabb0080u) :
            (sign == 0u ? 0xaabb007fu : 0xaabb8000u);
        before = state.cpu;
        expected_eax = opcodes[opcode] == 0x98u ?
            (sign == 0u ? 0xaabb007fu : 0xaabbff80u) : before.data.eax;
        expected_edx = opcodes[opcode] == 0x99u ?
            (sign == 0u ? 0x55660000u : 0x5566ffffu) : before.data.edx;
        if (cpu_instruction_run(&state, code, sizeof(code), &after) !=
                LIB_STATUS_OK || state.fault.valid || after.data.eip != 1u ||
            after.data.eax != expected_eax || after.data.edx != expected_edx ||
            !sign_extend_nonparticipants_equal(&before, &after,
                opcodes[opcode])) return 0;
    }
    return 1;
}

static lib_i32 sign_extend_test_operand32(void)
{
    static const lib_u8 opcodes[] = {0x98u, 0x99u};
    lib_u8 opcode, sign;

    for (opcode = 0u; opcode < sizeof(opcodes); ++opcode)
    for (sign = 0u; sign < 2u; ++sign) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        const lib_u8 code[] = {0x66u, opcodes[opcode]};
        lib_u32 expected_eax, expected_edx;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        sign_extend_set_registers(&state.cpu);
        state.cpu.data.eax = opcodes[opcode] == 0x98u ?
            (sign == 0u ? 0xabcd7f00u : 0xabcd8000u) :
            (sign == 0u ? 0x12347f00u : 0xabcd8000u);
        before = state.cpu;
        expected_eax = opcodes[opcode] == 0x98u ?
            (sign == 0u ? 0x00007f00u : 0xffff8000u) : before.data.eax;
        expected_edx = opcodes[opcode] == 0x99u ?
            (sign == 0u ? 0u : 0xffffffffu) : before.data.edx;
        if (cpu_instruction_run(&state, code, sizeof(code), &after) !=
                LIB_STATUS_OK || state.fault.valid || after.data.eip != 2u ||
            after.data.eax != expected_eax || after.data.edx != expected_edx ||
            !sign_extend_nonparticipants_equal(&before, &after,
                opcodes[opcode])) return 0;
    }
    return 1;
}

static lib_i32 sign_extend_test_address_prefix(void)
{
    static const lib_u8 opcodes[] = {0x98u, 0x99u};
    lib_u8 opcode;

    for (opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        const lib_u8 code[] = {0x67u, opcodes[opcode]};

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        sign_extend_set_registers(&state.cpu);
        state.cpu.data.eax = opcodes[opcode] == 0x98u ?
            0xaabb0080u : 0xaabb8000u;
        before = state.cpu;
        if (cpu_instruction_run(&state, code, sizeof(code), &after) !=
                LIB_STATUS_OK || state.fault.valid || after.data.eip != 2u ||
            after.data.eax != (opcodes[opcode] == 0x98u ?
                0xaabbff80u : before.data.eax) ||
            after.data.edx != (opcodes[opcode] == 0x99u ?
                0x5566ffffu : before.data.edx) ||
            !sign_extend_nonparticipants_equal(&before, &after,
                opcodes[opcode])) return 0;
    }
    return 1;
}

/* T337_REAL_UD_TERMINAL_CPU_OWNER: prefix/LOCK #UD belongs to the CPU. */
static lib_i32 sign_extend_test_prefix_reject(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 prefixes[] = {0x66u, 0x67u};
    static const lib_u8 opcodes[] = {0x98u, 0x99u};
    lib_u8 profile, prefix, opcode;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (prefix = 0u; prefix < sizeof(prefixes); ++prefix)
    for (opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        const lib_u8 code[] = {prefixes[prefix], opcodes[opcode]};

        cpu_instruction_prepare(&state, profiles[profile]);
        sign_extend_set_registers(&state.cpu);
        if (!cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u)) return 0;
    }
    return 1;
}

static lib_i32 sign_extend_test_lock_diagnostic(void)
{
    static const lib_u8 opcodes[] = {0x98u, 0x99u};
    lib_u8 opcode;

    for (opcode = 0u; opcode < sizeof(opcodes); ++opcode) {
        cpu_instruction_fixture state;
        const lib_u8 code[] = {0xf0u, opcodes[opcode]};

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        sign_extend_set_registers(&state.cpu);
        state.cpu.data.eax = 0xaabb0080u;
        if (!cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u)) {
            lib_c_fprintf(lib_c_stderr, "SIGN-EXT lock opcode=%02x fault=%08x\n",
                opcodes[opcode], state.fault.exception_mask);
            return 0;
        }
    }
    return 1;
}

lib_i32 main(void)
{
    if (!sign_extend_test_default() || !sign_extend_test_operand32() ||
        !sign_extend_test_address_prefix() ||
        !sign_extend_test_prefix_reject() ||
        !sign_extend_test_lock_diagnostic()) return 1;
    lib_c_printf("M5:T539:S30:CPU-SIGN-EXTEND:OK\n");
    return 0;
}
