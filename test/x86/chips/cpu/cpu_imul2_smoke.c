#include "support/cpu_operand_probe_fixture.h"
#include "lib/types/file.h"

static lib_i32 imul_forms(void)
{
    static const lib_i32 left[] = { -2, 0x7fff, -2, 0x7fffffff };
    static const lib_i32 right[] = { 3, 2, 0x40000000, 2 };
    lib_u8 width, memory, overflow;

    for (width = 0u; width < 2u; ++width)
    for (memory = 0u; memory < 2u; ++memory)
    for (overflow = 0u; overflow < 2u; ++overflow) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u8 code[6] = {0};
        lib_size bytes = 0u;
        const lib_i32 lhs = left[width * 2u + overflow];
        const lib_i32 rhs = right[width * 2u + overflow];
        const lib_i64 product = (lib_i64)lhs * (lib_i64)rhs;
        const lib_u32 expected = (lib_u32)product;
        const lib_i32 overflows = width ?
            (product > INT32_MAX || product < INT32_MIN) :
            (product > INT16_MAX || product < INT16_MIN);
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (memory && width) code[bytes++] = 0x67u;
        if (width) code[bytes++] = 0x66u;
        code[bytes++] = 0x0fu;
        code[bytes++] = 0xafu;
        if (memory) {
            code[bytes++] = 0x0eu;
            if (!width) {
                code[bytes++] = 0x00u;
                code[bytes++] = 0x40u;
            }
        } else code[bytes++] = 0xc8u;

        state.cpu.data.eax = (lib_u32)rhs;
        state.cpu.data.ecx = width ? (lib_u32)lhs :
            0xaabb0000u | ((lib_u32)lhs & 0xffffu);
        state.cpu.data.esi = 0x4000u;
        state.cpu.data.eflags = VCPU_EFLAGS_ZF;
        if (memory)
            failed |= cpu_instruction_write(&state, 0x4000u, &rhs,
                width ? 4u : 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
                LIB_STATUS_OK;
        failed |= cpu_instruction_run(&state, code, (lib_u8)bytes, &after) !=
            LIB_STATUS_OK || state.fault.valid ||
            (width ? after.data.ecx : (after.data.ecx & 0xffffu)) !=
                (width ? expected : (expected & 0xffffu)) ||
            !!X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_CF) != overflows ||
            !!X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_OF) != overflows;
        if (failed) return 0;
    }
    return 1;
}

/* T337_REAL_UD_TERMINAL_CPU_OWNER: unsupported IMUL forms are CPU-owned. */
static lib_i32 imul_profile(void)
{
    static const lib_u8 code[] = { 0x0fu, 0xafu, 0x0eu, 0x00u, 0x50u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 index;

    for (index = 0u; index < 2u; ++index) {
        cpu_operand_probe_fixture fixture;
        cpu_instruction_fixture *state = &fixture.instruction;

        cpu_operand_probe_prepare(&fixture, profiles[index]);
        state->cpu.data.ecx = 0xaabbccddu;
        state->cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        if (!cpu_instruction_expect_real_fault(state, code, sizeof(code), 6u) ||
            fixture.operand_reads != 0u || fixture.operand_writes != 0u) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!imul_forms() || !imul_profile()) return 1;
    lib_c_printf("M5:T539:S30:CPU-IMUL2:OK\n");
    return 0;
}
