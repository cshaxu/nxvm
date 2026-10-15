#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_u32 setcc_flags(lib_u8 image)
{
    return VCPU_EFLAGS_AF | VCPU_EFLAGS_DF | VCPU_EFLAGS_IF |
        ((image & 1u) != 0u ? VCPU_EFLAGS_CF : 0u) |
        ((image & 2u) != 0u ? VCPU_EFLAGS_PF : 0u) |
        ((image & 4u) != 0u ? VCPU_EFLAGS_ZF : 0u) |
        ((image & 8u) != 0u ? VCPU_EFLAGS_SF : 0u) |
        ((image & 16u) != 0u ? VCPU_EFLAGS_OF : 0u);
}

static lib_bool setcc_expected(lib_u8 condition, lib_u8 image)
{
    const lib_bool cf = (image & 1u) != 0u;
    const lib_bool pf = (image & 2u) != 0u;
    const lib_bool zf = (image & 4u) != 0u;
    const lib_bool sf = (image & 8u) != 0u;
    const lib_bool of = (image & 16u) != 0u;
    const lib_bool values[] = {of,!of,cf,!cf,zf,!zf,cf || zf,!cf && !zf,
        sf,!sf,pf,!pf,sf != of,sf == of,zf || sf != of,!zf && sf == of};

    return values[condition];
}

static lib_i32 setcc_test_register_conditions(core_machine_cpu_profile profile)
{
    for (lib_u8 context = 0u; context < 12u; ++context) {
        const lib_u8 mode = (lib_u8)(context % 6u);
        const lib_bool code32 = context >= 6u;
        cpu_instruction_fixture state;
        t_cpu initial;

        if (code32 && (mode == 0u || mode == 5u)) continue;
        cpu_instruction_prepare(&state, profile);
        if (mode != 0u) cpu_instruction_prepare_protected_exception(
            &state, mode == 5u ? 4u : (lib_u8)(mode - 1u));
        state.cpu.data.cs.seg.exec.defsize = code32;
        state.cpu.data.eax = 0x123456a5u;
        state.cpu.data.ecx = 0x89abcdefu;
        state.cpu.data.edx = 0xfedcba98u;
        state.cpu.data.ebx = 0x76543210u;
        initial = state.cpu;
        for (lib_u32 index = 0u; index < 4096u; ++index) {
            const lib_u8 condition = (lib_u8)(index & 15u);
            const lib_u8 image = (lib_u8)((index >> 4u) & 31u);
            const lib_u8 reg = (lib_u8)(index >> 9u);
            const lib_u8 code[] = {0x0fu, (lib_u8)(0x90u | condition),
                (lib_u8)(0xc0u | reg)};
            const lib_u32 shift = reg < 4u ? 0u : 8u;
            t_cpu expected;
            t_cpu after;
            lib_u32 *destinations[] = {&expected.data.eax, &expected.data.ecx,
                &expected.data.edx, &expected.data.ebx};
            lib_u32 *destination = destinations[reg & 3u];

            state.cpu = initial;
            state.cpu.data.eflags |= setcc_flags(image);
            expected = state.cpu;
            *destination = (*destination & ~(0xffu << shift)) |
                ((lib_u32)setcc_expected(condition, image) << shift);
            expected.data.eip = sizeof(code);
            core_machine_cpu_execution_invalidate_prefetch(&state.execution);
            if (cpu_instruction_run(&state, code, sizeof(code), &after) !=
                    LIB_STATUS_OK || state.fault.valid ||
                state.delivered_exception.valid || state.writes != 0u ||
                core_machine_cpu_is_shutdown(&state.execution) ||
                lib_memory_compare(&expected, &after, sizeof(after)) != 0) {
                lib_c_fprintf(lib_c_stderr,
                    "SETCC:MATRIX profile=%u context=%u case=%u\n",
                    profile, context, index);
                return 0;
            }
        }
    }
    return 1;
}

static lib_i32 setcc_test_memory_conditions(core_machine_cpu_profile profile)
{
    const lib_u16 destination = 0x1200u;
    lib_u8 condition;
    lib_u8 image;

    for (condition = 0u; condition != 16u; ++condition)
    for (image = 0u; image != 32u; ++image) {
        const lib_u8 code[] = {0x0fu, (lib_u8)(0x90u + condition), 0x06u,
            (lib_u8)destination, (lib_u8)(destination >> 8u)};
        const lib_u32 flags = setcc_flags(image);
        const lib_bool truth = setcc_expected(condition, image);
        const lib_u8 initial = 0xa5u;
        lib_u8 value = 0u;
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, profile);
        state.cpu.data.eax = 0x87654321u;
        state.cpu.data.eflags = flags;
        if (cpu_instruction_write(&state, destination, &initial, sizeof(initial),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, code, sizeof(code), &after) !=
                LIB_STATUS_OK || state.fault.valid ||
            cpu_instruction_read(&state, destination, &value, sizeof(value),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) !=
                LIB_STATUS_OK || value != (lib_u8)truth ||
            after.data.eax != 0x87654321u || after.data.eflags != flags ||
            after.data.eip != sizeof(code)) return 0;
    }
    return 1;
}

static lib_i32 setcc_test_prefix_forms(core_machine_cpu_profile profile)
{
    static const lib_u8 operand_prefix[] = {0x66u, 0x0fu, 0x94u, 0xc0u};
    static const lib_u8 address_prefix[] = {0x67u, 0x0fu, 0x94u, 0x06u};
    const lib_u32 flags = VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF;
    const lib_u32 address = 0x00002345u;
    const lib_u8 initial = 0xa5u;
    lib_u8 value = 0u;
    cpu_instruction_fixture state;
    t_cpu after;

    cpu_instruction_prepare(&state, profile);
    state.cpu.data.eax = 0x112233a5u;
    state.cpu.data.eflags = flags;
    if (cpu_instruction_run(&state, operand_prefix, sizeof(operand_prefix),
            &after) != LIB_STATUS_OK || state.fault.valid ||
        after.data.eax != 0x11223301u || after.data.eflags != flags ||
        after.data.eip != sizeof(operand_prefix)) return 0;

    cpu_instruction_prepare(&state, profile);
    state.cpu.data.esi = address;
    state.cpu.data.eflags = flags;
    if (cpu_instruction_write(&state, address, &initial, sizeof(initial),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK ||
        cpu_instruction_run(&state, address_prefix, sizeof(address_prefix),
            &after) != LIB_STATUS_OK || state.fault.valid ||
        cpu_instruction_read(&state, address, &value, sizeof(value),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) !=
            LIB_STATUS_OK || value != 1u || after.data.esi != address ||
        after.data.eflags != flags ||
        after.data.eip != sizeof(address_prefix)) return 0;
    return 1;
}

/* REAL_UD_TERMINAL_CPU_OWNER: SETcc before 386 is CPU-owned #UD. */
static lib_i32 setcc_test_pre_fault_nonpublication(void)
{
    static const lib_u8 ud_code[] = {0x0fu, 0x94u, 0xc0u};
    const lib_u32 flags = VCPU_EFLAGS_ZF | VCPU_EFLAGS_OF;
    cpu_instruction_fixture state;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
    state.cpu.data.eax = 0x556677a5u;
    state.cpu.data.eflags = flags;
    return cpu_instruction_expect_real_fault(&state, ud_code, sizeof(ud_code), 6u);
}

lib_i32 main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_CPU_PROFILE_80486SX
    };

    if (!setcc_test_pre_fault_nonpublication()) return 1;
    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]);
        ++index) {
        if (!setcc_test_register_conditions(profiles[index]) ||
            !setcc_test_memory_conditions(profiles[index]) ||
            !setcc_test_prefix_forms(profiles[index])) return 1;
    }
    lib_c_printf("CPU-SETCC:OK\n");
    return 0;
}
