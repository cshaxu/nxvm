#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* REAL_UD_TERMINAL_CPU_OWNER: negative cases retain the CPU-owned IDTR limit. */
static void moffs_set_registers(cpu_instruction_fixture *state)
{
    state->cpu.data.eax = 0xaabb3344u;
    state->cpu.data.ecx = 0x11223344u;
    state->cpu.data.edx = 0x55667788u;
    state->cpu.data.ebx = 0x99aabbccu;
    state->cpu.data.esi = 0xddeeff00u;
    state->cpu.data.edi = 0x10203040u;
    state->cpu.data.ebp = 0x50607080u;
    state->cpu.data.esp = 0x00007777u;
    state->cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
}

static lib_i32 moffs_nonparticipants(const t_cpu *before, const t_cpu *after,
    lib_u8 opcode)
{
    return before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esi == after->data.esi &&
        before->data.eflags == after->data.eflags &&
        (opcode == 0xa0u || opcode == 0xa1u ||
            before->data.eax == after->data.eax);
}

static lib_i32 moffs_test_default(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 opcodes[] = { 0xa0u, 0xa1u, 0xa2u, 0xa3u };
    lib_u8 profile;
    lib_u8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
        {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u8 code[] = { opcodes[opcode], 0x00u, 0x10u };
            lib_u32 image = opcodes[opcode] == 0xa0u || opcodes[opcode] == 0xa2u ?
                0x0000005au : 0x0000beefu;
            lib_u32 expected_eax;
            lib_i32 failed;

            lib_memory_set(&state, 0, sizeof(state));
            lib_memory_set(&before, 0, sizeof(before));
            lib_memory_set(&after, 0, sizeof(after));
            failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            moffs_set_registers(&state);
            if (opcodes[opcode] == 0xa0u || opcodes[opcode] == 0xa1u)
                failed |= cpu_instruction_write(&state, 0x1000u, &image, opcodes[opcode] == 0xa0u ? 1u : 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            before = state.cpu;
            expected_eax = opcodes[opcode] == 0xa0u ? 0xaabb335au :
                opcodes[opcode] == 0xa1u ? 0xaabbbeefu : before.data.eax;
            failed |= cpu_instruction_run(&state, code, sizeof(code), &after) != LIB_STATUS_OK ||
                state.fault.valid || after.data.eip != 3u ||
                after.data.eax != expected_eax ||
                !moffs_nonparticipants(&before, &after, opcodes[opcode]);
            if (opcodes[opcode] == 0xa2u || opcodes[opcode] == 0xa3u)
            {
                image = 0u;
                failed |= cpu_instruction_read(&state, 0x1000u, &image, opcodes[opcode] == 0xa2u ? 1u : 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                    image != (opcodes[opcode] == 0xa2u ? 0x44u : 0x3344u);
            }

            if (failed)
            {
                lib_c_printf("MOFFS default profile=%u opcode=%02x\n",
                    profiles[profile], opcodes[opcode]);
                return 0;
            }
        }
    }
    return 1;
}

static lib_i32 moffs_test_386_attributes(void)
{
    static const lib_u8 read32[] = { 0x66u,0x67u,0xa1u,0x00u,0x00u,0x01u,0x00u };
    static const lib_u8 write32[] = { 0x66u,0x67u,0xa3u,0x00u,0x00u,0x01u,0x00u };
    static const lib_u8 read8[] = { 0x66u,0x67u,0xa0u,0x00u,0x00u,0x01u,0x00u };
    static const lib_u8 write8[] = { 0x66u,0x67u,0xa2u,0x00u,0x00u,0x01u,0x00u };
    const lib_u8 *codes[] = { read32, write32, read8, write8 };
    const lib_u8 write[] = { 0u, 1u, 0u, 1u };
    const lib_u8 widths[] = { 4u, 4u, 1u, 1u };
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        cpu_instruction_fixture state;
        t_cpu after = {0};
        lib_u32 image = 0x1122335au;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        /* 386 manual 14.5: PE-clear execution can retain a wide cached DS.
         * Address-size alone does not extend an ordinary real-mode segment. */
        state.cpu.data.ds.limit = 0xffffffffu;
        moffs_set_registers(&state);
        if (!write[form])
            failed |= cpu_instruction_write(&state, 0x10000u, &image, widths[form],
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        failed |= cpu_instruction_run(&state, codes[form], 7u, &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.eip != 7u;
        if (form == 0u) failed |= after.data.eax != 0x1122335au;
        if (form == 2u) failed |= after.data.eax != 0xaabb335au;
        if (write[form])
        {
            image = 0u;
            failed |= cpu_instruction_read(&state, 0x10000u, &image, widths[form],
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                image != (form == 1u ? 0xaabb3344u : 0x44u);
        }

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 moffs_test_386_single_attributes(void)
{
    lib_u8 attribute;
    lib_u8 opcode;

    for (attribute = 0u; attribute != 3u; ++attribute)
    for (opcode = 0xa0u; opcode != 0xa4u; ++opcode) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u8 code[] = {0x66u,0x67u,opcode,0,0x80u,0,0};
        const lib_u8 bytes = attribute == 0u ? 4u :
            attribute == 1u ? 6u : 7u;
        const lib_u32 address = attribute == 0u ? 0x1000u : 0x8000u;
        const lib_u8 width = opcode == 0xa0u || opcode == 0xa2u ?
            1u : attribute == 1u ? 2u : 4u;
        lib_u32 image = 0x1122335au;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (attribute == 0u) {
            code[1] = opcode; code[2] = 0; code[3] = 0x10u;
        } else if (attribute == 1u) {
            code[0] = 0x67u; code[1] = opcode; code[2] = 0; code[3] = 0x80u; code[4] = 0;
        }
        moffs_set_registers(&state);
        if (opcode == 0xa0u || opcode == 0xa1u)
            failed |= cpu_instruction_write(&state, address, &image, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.eip != bytes || !moffs_nonparticipants(&before,
            &after, opcode);
        if (opcode == 0xa0u) failed |= after.data.eax != 0xaabb335au;
        if (opcode == 0xa1u) failed |= after.data.eax != (attribute == 1u ?
            0xaabb335au : 0x1122335au);
        if (opcode == 0xa2u || opcode == 0xa3u) {
            image = 0u;
            failed |= cpu_instruction_read(&state, address, &image, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                image != (width == 1u ? 0x44u : width == 2u ? 0x3344u :
                0xaabb3344u);
        }

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 moffs_test_reject(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 prefixes[] = { 0x66u, 0x67u };
    static const lib_u8 opcodes[] = { 0xa0u, 0xa1u, 0xa2u, 0xa3u };
    lib_u8 profile;
    lib_u8 prefix;
    lib_u8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (prefix = 0u; prefix != sizeof(prefixes); ++prefix)
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
    {
        cpu_instruction_fixture state;
        lib_u8 code[] = { prefixes[prefix], opcodes[opcode], 0u, 0x10u };

        cpu_instruction_prepare(&state, profiles[profile]);
        moffs_set_registers(&state);
        if (!cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u)) {
            lib_c_printf("MOFFS reject profile=%u prefix=%02x opcode=%02x\n",
                profiles[profile], prefixes[prefix], opcodes[opcode]);
            return 0;
        }
    }
    return 1;
}

static lib_i32 moffs_test_lock(void)
{
    static const lib_u8 opcodes[] = { 0xa0u, 0xa1u, 0xa2u, 0xa3u };
    lib_u8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
    {
        cpu_instruction_fixture state;
        lib_u8 code[] = { 0xf0u, opcodes[opcode], 0u, 0x10u };

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        moffs_set_registers(&state);
        if (!cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u))
            return 0;
    }
    return 1;
}

static lib_i32 moffs_test_segment_overrides(void)
{
    static const lib_u8 codes[][4] = {
        { 0xa0u, 0x10u, 0x00u, 0u },
        { 0x26u, 0xa0u, 0x10u, 0x00u },
        { 0x64u, 0xa0u, 0x10u, 0x00u },
        { 0x65u, 0xa0u, 0x10u, 0x00u }
    };
    static const lib_u8 values[] = { 0x11u, 0x22u, 0x33u, 0x44u };
    static const lib_u8 bytes[] = { 3u, 4u, 4u, 4u };
    lib_u8 form;

    for (form = 0u; form != sizeof(values); ++form)
    {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u32 address = form == 0u ? 0x10u : (lib_u32)form * 0x100u + 0x10u;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (form == 1u) failed |= core_machine_cpu_execution_load_segment(
            &state.execution, &state.cpu.data.es, 0x10u) != 0;
        if (form == 2u) failed |= core_machine_cpu_execution_load_segment(
            &state.execution, &state.cpu.data.fs, 0x20u) != 0;
        if (form == 3u) failed |= core_machine_cpu_execution_load_segment(
            &state.execution, &state.cpu.data.gs, 0x30u) != 0;
        moffs_set_registers(&state);
        failed |= cpu_instruction_write(&state, address, &values[form], 1u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, codes[form], bytes[form], &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.eip != bytes[form] || after.data.eax != (0xaabb3300u | values[form]) ||
            !moffs_nonparticipants(&before, &after, 0xa0u);

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 moffs_test_segment_writes(void)
{
    static const lib_u8 codes[][4] = {
        { 0xa2u, 0x10u, 0x00u, 0u },
        { 0x26u, 0xa3u, 0x10u, 0x00u },
        { 0x64u, 0xa2u, 0x10u, 0x00u },
        { 0x65u, 0xa3u, 0x10u, 0x00u }
    };
    static const lib_u8 bytes[] = { 3u, 4u, 4u, 4u };
    static const lib_u8 widths[] = { 1u, 2u, 1u, 2u };
    lib_u8 form;

    for (form = 0u; form != sizeof(bytes); ++form)
    {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u32 image = 0u;
        lib_u32 address = form == 0u ? 0x10u : (lib_u32)form * 0x100u + 0x10u;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (form == 1u) failed |= core_machine_cpu_execution_load_segment(&state.execution, &state.cpu.data.es, 0x10u) != 0;
        if (form == 2u) failed |= core_machine_cpu_execution_load_segment(&state.execution, &state.cpu.data.fs, 0x20u) != 0;
        if (form == 3u) failed |= core_machine_cpu_execution_load_segment(&state.execution, &state.cpu.data.gs, 0x30u) != 0;
        moffs_set_registers(&state);
        before = state.cpu;
        failed |= cpu_instruction_run(&state, codes[form], bytes[form], &after) != LIB_STATUS_OK || state.fault.valid || after.data.eip != bytes[form] || !moffs_nonparticipants(&before, &after, 0xa2u) || cpu_instruction_read(&state, address, &image, widths[form],
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || image != (widths[form] == 1u ? 0x44u : 0x3344u);

        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!moffs_test_default() ||
        !moffs_test_386_attributes() ||
        !moffs_test_386_single_attributes() ||
        !moffs_test_reject() ||
        !moffs_test_lock() ||
        !moffs_test_segment_overrides() ||
        !moffs_test_segment_writes()) return 1;
    lib_c_printf("MOFFS:CPU:OK\n");
    return 0;
}
