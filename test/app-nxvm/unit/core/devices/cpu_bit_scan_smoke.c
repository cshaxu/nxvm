#include "support/cpu_operand_probe_fixture.h"
#include <stdio.h>

/* T337_REAL_UD_TERMINAL_CPU_OWNER: unsupported-profile #UD is CPU-owned. */
static lib_i32 scan_test_forms(void)
{
    static const lib_u8 opcodes[] = { 0xbcu, 0xbdu };
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
    lib_u8 opcode, width, memory, zero;

    for (opcode = 0u; opcode < 2u; ++opcode)
    for (width = 0u; width < 2u; ++width)
    for (memory = 0u; memory < 2u; ++memory)
    for (zero = 0u; zero < 2u; ++zero) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u8 code[6] = {0};
        lib_size bytes = 0u;
        lib_u32 read = 0u;
        const lib_u32 source = zero ? 0u : (width ? 0x80000120u : 0x00008120u);
        const lib_u32 expected = opcode ? (width ? 31u : 15u) : 5u;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (memory && width) code[bytes++] = 0x67u;
        if (width) code[bytes++] = 0x66u;
        code[bytes++] = 0x0fu;
        code[bytes++] = opcodes[opcode];
        if (memory) {
            code[bytes++] = 0x0eu;
            if (!width) {
                code[bytes++] = 0x00u;
                code[bytes++] = 0x40u;
            }
        } else code[bytes++] = 0xc8u;

        state.cpu.data.eax = source;
        state.cpu.data.ecx = 0xaabbccddu;
        state.cpu.data.esi = 0x4000u;
        state.cpu.data.eflags = flags;
        if (memory)
            failed |= cpu_instruction_write(&state, 0x4000u, &source,
                width ? 4u : 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
                LIB_STATUS_OK;
        failed |= cpu_instruction_run(&state, code, (lib_u8)bytes, &after) !=
            LIB_STATUS_OK || state.fault.valid;
        if (!zero)
            failed |= (width ? after.data.ecx : (after.data.ecx & 0xffffu)) !=
                expected || X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF);
        else failed |= !X86_CPU_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_ZF);
        if (memory)
            failed |= cpu_instruction_read(&state, 0x4000u, &read,
                width ? 4u : 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA,
                LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || read != source;
        if (failed) {
            fprintf(stderr, "bit scan form %u/%u/%u/%u status fault=%u eip=%u\n",
                (unsigned)opcode, (unsigned)width, (unsigned)memory,
                (unsigned)zero, (unsigned)state.fault.exception_mask,
                (unsigned)after.data.eip);
            return 0;
        }
    }
    return 1;
}

static lib_i32 scan_test_profile(void)
{
    static const lib_u8 code[] = { 0x0fu, 0xbcu, 0x0eu, 0x00u, 0x50u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 index;

    for (index = 0u; index < 2u; ++index) {
        cpu_operand_probe_fixture fixture;
        cpu_instruction_fixture *state = &fixture.instruction;
        t_cpu after;

        cpu_operand_probe_prepare(&fixture, profiles[index]);
        state->cpu.data.eax = 0xaabbccddu;
        state->cpu.data.eflags = VCPU_EFLAGS_CF;
        state->cpu.data.idtr.limit = 0x17u;
        if (cpu_instruction_run(state, code, sizeof(code), &after) !=
                LIB_STATUS_INTERNAL_ERROR || !state->fault.valid ||
            !X86_CPU_BIT_IS_SET(state->fault.exception_mask,
                VCPUINS_EXCEPT_UD) || fixture.operand_reads != 0u ||
            fixture.operand_writes != 0u ||
            after.data.eax != 0xaabbccddu ||
            after.data.eflags != VCPU_EFLAGS_CF || after.data.eip != 0u)
        {
            fprintf(stderr, "bit scan profile %u fault=%u eip=%u\n",
                (unsigned)index, (unsigned)state->fault.exception_mask,
                (unsigned)after.data.eip);
            return 0;
        }
    }
    return 1;
}

lib_i32 main(void)
{
    if (!scan_test_forms() || !scan_test_profile()) return 1;
    printf("M5:T539:S30:CPU-BIT-SCAN:OK\n");
    return 0;
}
