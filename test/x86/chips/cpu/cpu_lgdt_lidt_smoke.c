#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* REAL_UD_TERMINAL_CPU_OWNER: invalid LGDT/LIDT stops at the CPU. */
static void lgdt_lidt_image(lib_u8 image[6], lib_u16 limit, lib_u32 base)
{
    image[0] = (lib_u8)limit;
    image[1] = (lib_u8)(limit >> 8u);
    image[2] = (lib_u8)base;
    image[3] = (lib_u8)(base >> 8u);
    image[4] = (lib_u8)(base >> 16u);
    image[5] = (lib_u8)(base >> 24u);
}

static lib_i32 lgdt_lidt_run(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    lib_memory_copy(state->memory, code, bytes);
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return !state->execution.stop_requested && !state->fault.valid &&
        after->data.eip == bytes;
}

static lib_i32 lgdt_lidt_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_bool idt, lib_bool operand32)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    lib_u8 image[6];
    const lib_u16 limit = idt ? 0x1357u : 0x2468u;
    const lib_u32 base = idt ? 0x89abcdefu : 0x12345678u;

    cpu_instruction_prepare(&state, profile);
    lgdt_lidt_image(image, limit, base);
    lib_memory_copy(state.memory + 0x0200u, image, sizeof(image));
    state.cpu.data.eax = 0x11223344u;
    state.cpu.data.ecx = 0x55667788u;
    state.cpu.data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF;
    before = state.cpu;
    if (!lgdt_lidt_run(&state, code, bytes, &after)) return 0;
    if (idt) return after.data.idtr.limit == limit &&
        after.data.idtr.base == (operand32 ? base : (base & 0x00ffffffu));
    return after.data.gdtr.limit == limit &&
        after.data.gdtr.base == (operand32 ? base : (base & 0x00ffffffu)) &&
        after.data.eax == before.data.eax && after.data.ecx == before.data.ecx &&
        after.data.eflags == before.data.eflags;
}

static lib_i32 lgdt_lidt_test_values(void)
{
    static const lib_u8 lgdt[] = {0x0fu,0x01u,0x16u,0x00u,0x02u};
    static const lib_u8 lidt[] = {0x0fu,0x01u,0x1eu,0x00u,0x02u};
    static const lib_u8 lgdt32[] = {0x66u,0x0fu,0x01u,0x16u,0x00u,0x02u};
    static const lib_u8 lidt32[] = {0x66u,0x0fu,0x01u,0x1eu,0x00u,0x02u};
    static const lib_u8 lgdt_address32[] = {
        0x67u,0x0fu,0x01u,0x15u,0x00u,0x02u,0x00u,0x00u
    };
    static const lib_u8 lidt_address32[] = {
        0x67u,0x0fu,0x01u,0x1du,0x00u,0x02u,0x00u,0x00u
    };
    static const lib_u8 lgdt_both32[] = {
        0x66u,0x67u,0x0fu,0x01u,0x15u,0x00u,0x02u,0x00u,0x00u
    };
    static const lib_u8 lidt_both32[] = {
        0x66u,0x67u,0x0fu,0x01u,0x1du,0x00u,0x02u,0x00u,0x00u
    };

    return lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80286, lgdt,
            sizeof(lgdt), LIB_FALSE, LIB_FALSE) &&
        lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80286, lidt,
            sizeof(lidt), LIB_TRUE, LIB_FALSE) &&
        lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80386, lgdt,
            sizeof(lgdt), LIB_FALSE, LIB_FALSE) &&
        lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80386, lidt,
            sizeof(lidt), LIB_TRUE, LIB_FALSE) &&
        lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80386, lgdt32,
            sizeof(lgdt32), LIB_FALSE, LIB_TRUE) &&
        lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80386, lidt32,
            sizeof(lidt32), LIB_TRUE, LIB_TRUE) &&
        lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80386, lgdt_address32,
            sizeof(lgdt_address32), LIB_FALSE, LIB_FALSE) &&
        lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80386, lidt_address32,
            sizeof(lidt_address32), LIB_TRUE, LIB_FALSE) &&
        lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80386, lgdt_both32,
            sizeof(lgdt_both32), LIB_FALSE, LIB_TRUE) &&
        lgdt_lidt_case(CORE_MACHINE_CPU_PROFILE_80386, lidt_both32,
            sizeof(lidt_both32), LIB_TRUE, LIB_TRUE);
}

static lib_i32 lgdt_lidt_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    cpu_instruction_prepare(&state, profile);
    return cpu_instruction_expect_real_fault(&state, code, bytes, 6u);
}

static lib_i32 lgdt_lidt_test_rejections(void)
{
    static const lib_u8 low[] = {0x0fu,0x01u,0x16u,0x00u,0x02u};
    static const lib_u8 register_form[] = {0x0fu,0x01u,0xd0u};
    static const lib_u8 lock[] = {0xf0u,0x0fu,0x01u,0x16u,0x00u,0x02u};
    static const lib_u8 prefix66[] = {0x66u,0x0fu,0x01u,0x16u,0x00u,0x02u};
    static const lib_u8 prefix67[] = {0x67u,0x0fu,0x01u,0x16u,0x00u,0x02u};
    lib_u8 table;

    for (table = 2u; table != 4u; ++table) {
        lib_u8 low_code[sizeof(low)], direct[sizeof(register_form)],
            locked[sizeof(lock)], operand[sizeof(prefix66)],
            address[sizeof(prefix67)];
        lib_memory_copy(low_code, low, sizeof(low_code));
        lib_memory_copy(direct, register_form, sizeof(direct));
        lib_memory_copy(locked, lock, sizeof(locked));
        lib_memory_copy(operand, prefix66, sizeof(operand));
        lib_memory_copy(address, prefix67, sizeof(address));
        low_code[2] |= table << 3u;
        direct[2] |= table << 3u;
        locked[3] |= table << 3u;
        operand[3] |= table << 3u;
        address[3] |= table << 3u;
        if (!lgdt_lidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80186, low_code,
                sizeof(low_code)) || !lgdt_lidt_expect_ud(
                CORE_MACHINE_CPU_PROFILE_80386, direct, sizeof(direct)) ||
            !lgdt_lidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, locked,
                sizeof(locked)) || !lgdt_lidt_expect_ud(
                CORE_MACHINE_CPU_PROFILE_80286, operand, sizeof(operand)) ||
            !lgdt_lidt_expect_ud(CORE_MACHINE_CPU_PROFILE_80286, address,
                sizeof(address))) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!lgdt_lidt_test_values() || !lgdt_lidt_test_rejections()) {
        lib_c_fprintf(lib_c_stderr, "LGDT-LIDT CPU failed\n");
        return 1;
    }
    lib_c_printf("%s\n", "LGDT-LIDT-CPU:OK");
    return 0;
}
