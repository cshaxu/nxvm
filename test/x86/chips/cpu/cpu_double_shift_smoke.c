#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_u32 shift_parity(lib_u32 value)
{
    lib_u8 bits = 0u;
    value &= 0xffu;
    while (value) { bits ^= (lib_u8)(value & 1u); value >>= 1u; }
    return bits ? 0u : VCPU_EFLAGS_PF;
}

static lib_u32 shift_flags(lib_u32 result, lib_u32 destination, lib_u8 count,
    lib_u8 width, lib_i32 right)
{
    const lib_u32 sign = width == 16u ? 0x8000u : 0x80000000u;
    const lib_u32 mask = width == 16u ? 0xffffu : 0xffffffffu;
    lib_u32 flags = shift_parity(result);
    lib_u32 cf = right ? (destination >> (count - 1u)) :
        (destination >> (width - count));
    if (cf & 1u) flags |= VCPU_EFLAGS_CF;
    if ((result & mask) == 0u) flags |= VCPU_EFLAGS_ZF;
    if (result & sign) flags |= VCPU_EFLAGS_SF;
    if (count == 1u && ((destination ^ result) & sign)) flags |= VCPU_EFLAGS_OF;
    return flags;
}

static lib_u32 shift_result(lib_u32 destination, lib_u32 source, lib_u8 count,
    lib_u8 width, lib_i32 right)
{
    const lib_u32 mask = width == 16u ? 0xffffu : 0xffffffffu;
    destination &= mask; source &= mask;
    return right ? ((destination >> count) | (source << (width - count))) & mask :
        ((destination << count) | (source >> (width - count))) & mask;
}

static lib_i32 shift_test_forms(void)
{
    static const lib_u8 immediate_opcodes[] = {0xa4u, 0xacu};
    static const lib_u8 cl_opcodes[] = {0xa5u, 0xadu};
    const lib_u32 initial_flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF;
    lib_u8 direction, width, memory, count_index, cl;

    for (direction = 0u; direction < 2u; ++direction)
    for (width = 0u; width < 2u; ++width)
    for (memory = 0u; memory < 2u; ++memory)
    for (cl = 0u; cl < 2u; ++cl)
    for (count_index = 1u; count_index <= (width ? 31u : 16u); ++count_index) {
        const lib_u8 count = count_index;
        const lib_u8 opcode = cl ? cl_opcodes[direction] : immediate_opcodes[direction];
        const lib_u32 destination = width ? 0x81234567u : 0xaabb8123u;
        const lib_u32 source = cl ? ((width ? 0x76543200u : 0xccdd7600u) | count) :
            (width ? 0x76543210u : 0xccdd7654u);
        const lib_u32 expected = shift_result(destination, source, count,
            width ? 32u : 16u, direction);
        const lib_u32 expected_flags = shift_flags(expected, destination, count,
            width ? 32u : 16u, direction);
        const lib_u32 flag_mask = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
            (count == 1u ? VCPU_EFLAGS_OF : 0u);
        lib_u8 code[8] = {0};
        lib_u8 bytes = 0u;
        cpu_instruction_fixture state;
        t_cpu after = {0};
        lib_u32 observed = 0u;
        lib_i32 failed = 0;

        if (memory && width) code[bytes++] = 0x67u;
        if (width) code[bytes++] = 0x66u;
        code[bytes++] = 0x0fu;
        code[bytes++] = opcode;
        if (memory) {
            code[bytes++] = 0x0eu;
            if (!width) {
                code[bytes++] = 0x00u;
                code[bytes++] = 0x40u;
            }
        } else code[bytes++] = 0xc8u;
        if (!cl) code[bytes++] = count;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = destination;
        state.cpu.data.ecx = source;
        state.cpu.data.esi = 0x4000u;
        state.cpu.data.eflags = initial_flags;
        if (memory)
            failed |= cpu_instruction_write(&state, 0x4000u, &destination,
                width ? 4u : 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
                LIB_STATUS_OK;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
            state.fault.valid;
        if (memory)
            failed |= cpu_instruction_read(&state, 0x4000u, &observed,
                width ? 4u : 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA,
                LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK;
        else observed = after.data.eax;
        /* Intel 386 pseudocode leaves count >= operand width undefined.
         * Retain that context without turning a deterministic choice into L3. */
        if (count < (width ? 32u : 16u))
            failed |= (width ? observed : (observed & 0xffffu)) != expected ||
                (after.data.eflags & flag_mask) != (expected_flags & flag_mask);
        else
            failed |= (state.instructions.data.udf &
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
                    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
                    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF);
        failed |= after.data.eip != bytes || after.data.ecx != source ||
            (memory && after.data.eax != destination);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 shift_test_count_zero(void)
{
    static const lib_u8 immediate[] = {0x66u, 0x0fu, 0xa4u, 0xc8u, 0u};
    static const lib_u8 cl[] = {0x0fu, 0xadu, 0xc8u};
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_OF;
    lib_u8 index;

    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        t_cpu after;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = 0xaabbccddu;
        state.cpu.data.ecx = 0u;
        state.cpu.data.eflags = flags;
        if (cpu_instruction_run(&state, index ? cl : immediate,
                index ? sizeof(cl) : sizeof(immediate), &after) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eax != 0xaabbccddu ||
            after.data.eflags != flags) return 0;
    }
    return 1;
}

/* T337_REAL_UD_TERMINAL_CPU_OWNER: unsupported-profile #UD is CPU-owned. */
static lib_i32 shift_test_profile(void)
{
    static const lib_u8 code[] = {0x0fu, 0xa4u, 0xc8u, 1u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 index;

    for (index = 0u; index < 2u; ++index) {
        cpu_instruction_fixture state;
        cpu_instruction_prepare(&state, profiles[index]);
        state.cpu.data.eax = 0xaabbccddu;
        state.cpu.data.ecx = 0x11223344u;
        state.cpu.data.eflags = VCPU_EFLAGS_ZF;
        if (!cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u)) return 0;
    }
    return 1;
}

static lib_bool shift_test_undefined_capture(void)
{
    static const lib_u8 counts[] = {17u, 18u, 31u, 255u};
    lib_u8 direction, count_index, memory, cl;

    for (direction = 0u; direction < 2u; ++direction)
    for (count_index = 0u; count_index < sizeof(counts); ++count_index)
    for (memory = 0u; memory < 2u; ++memory)
    for (cl = 0u; cl < 2u; ++cl) {
        lib_u8 code[7] = {0x0fu};
        lib_u8 bytes = 1u;
        const lib_u32 destination = 0xaabb8123u;
        const lib_u32 source = 0xccdd7600u | counts[count_index];
        const lib_u32 arithmetic = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
        cpu_instruction_fixture state;
        t_cpu after;

        code[bytes++] = direction ? (cl ? 0xadu : 0xacu) : (cl ? 0xa5u : 0xa4u);
        code[bytes++] = memory ? 0x0eu : 0xc8u;
        if (memory) { code[bytes++] = 0u; code[bytes++] = 0x40u; }
        if (!cl) code[bytes++] = counts[count_index];
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = destination;
        state.cpu.data.ecx = source;
        state.cpu.data.eflags = VCPU_EFLAGS_IF;
        if (memory && cpu_instruction_write(&state, 0x4000u, &destination,
                2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK)
            return LIB_FALSE;
        if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
                state.fault.valid || after.data.eip != bytes ||
                after.data.ecx != source ||
                (memory && after.data.eax != destination) ||
                state.instructions.data.bit != 16u ||
                state.instructions.data.opr1 != (destination & 0xffffu) ||
                state.instructions.data.opr2 != (source & 0xffffu) ||
                (state.instructions.data.udf & arithmetic) != arithmetic ||
                !(after.data.eflags & VCPU_EFLAGS_IF)) return LIB_FALSE;
        /* No exact destination or arithmetic FLAGS assertion is permitted. */
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    if (!shift_test_forms() || !shift_test_count_zero() ||
        !shift_test_profile() || !shift_test_undefined_capture()) return 1;
    lib_c_printf("M5:T539:S30:CPU-DOUBLE-SHIFT:OK\n");
    return 0;
}
