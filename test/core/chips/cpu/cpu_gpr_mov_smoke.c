#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* REAL_UD_TERMINAL_CPU_OWNER: negative cases retain the CPU-owned IDTR limit. */
static void gpr_mov_seed(cpu_instruction_fixture *state)
{
    state->cpu.data.eax = 0xaabb3344u;
    state->cpu.data.ecx = 0x11225566u;
    state->cpu.data.edx = 0x778899aau;
    state->cpu.data.ebx = 0xbbccddeeU;
    state->cpu.data.esp = 0x00008000u;
    state->cpu.data.ebp = 0x00000120u;
    state->cpu.data.esi = 0x00000010u;
    state->cpu.data.edi = 0x00000020u;
    state->cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
}

static lib_u32 gpr_mov_gpr(const t_cpu *cpu, lib_u8 index)
{
    switch (index) {
    case 0: return cpu->data.eax;
    case 1: return cpu->data.ecx;
    case 2: return cpu->data.edx;
    case 3: return cpu->data.ebx;
    case 4: return cpu->data.esp;
    case 5: return cpu->data.ebp;
    case 6: return cpu->data.esi;
    default: return cpu->data.edi;
    }
}

static lib_i32 gpr_mov_nonparticipants(const t_cpu *before, const t_cpu *after,
    lib_u8 destination)
{
    return before->data.eflags == after->data.eflags &&
        (destination == 0u || before->data.eax == after->data.eax) &&
        (destination == 1u || before->data.ecx == after->data.ecx) &&
        (destination == 2u || before->data.edx == after->data.edx) &&
        (destination == 3u || before->data.ebx == after->data.ebx) &&
        (destination == 4u || before->data.esp == after->data.esp) &&
        (destination == 5u || before->data.ebp == after->data.ebp) &&
        (destination == 6u || before->data.esi == after->data.esi) &&
        (destination == 7u || before->data.edi == after->data.edi);
}

static lib_i32 gpr_mov_test_defaults(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_CPU_PROFILE_80486SX
    };
    static const lib_u8 memory_codes[][4] = {
        {0x88u,0x06u,0x00u,0x10u}, {0x89u,0x0eu,0x00u,0x10u},
        {0x8au,0x06u,0x00u,0x10u}, {0x8bu,0x0eu,0x00u,0x10u}
    };
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile) {
        for (form = 0u; form != sizeof(memory_codes) / sizeof(memory_codes[0]);
             ++form) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u16 image = 0xbe5au;
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            gpr_mov_seed(&state);
            failed |= cpu_instruction_write(&state, 0x1000u, &image, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK;
            before = state.cpu;
            failed |= cpu_instruction_run(&state, memory_codes[form], 4u, &after) != LIB_STATUS_OK || state.fault.valid || after.data.eip != 4u ||
                !gpr_mov_nonparticipants(&before, &after,
                    form == 3u ? 1u :
                    (form == 0u || form == 1u) ? 8u : 0u);
            if (form == 0u) failed |= (after.data.eax & 0xffu) != 0x44u;
            if (form == 1u) failed |= cpu_instruction_read(&state, 0x1000u, &image, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK || image != 0x5566u;
            if (form == 2u)
                failed |= after.data.eax !=
                    ((before.data.eax & 0xffffff00u) | 0x5au);
            if (form == 3u)
                failed |= after.data.ecx !=
                    ((before.data.ecx & 0xffff0000u) | 0xbe5au);

            if (failed) return 0;
        }
        for (form = 0u; form != 8u; ++form) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u8 code[] = {(lib_u8)(0xb0u + form),
                (lib_u8)(0x80u + form)};
            lib_u8 target = form & 3u;
            lib_u32 expected;
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            gpr_mov_seed(&state);
            before = state.cpu;
            expected = gpr_mov_gpr(&before, target);
            expected = form < 4u ?
                (expected & 0xffffff00u) | (0x80u + form) :
                (expected & 0xffff00ffu) |
                    ((lib_u32)(0x80u + form) << 8u);
            failed |= cpu_instruction_run(&state, code, sizeof(code), &after) != LIB_STATUS_OK ||
                state.fault.valid || after.data.eip != sizeof(code) ||
                !gpr_mov_nonparticipants(&before, &after, target) ||
                gpr_mov_gpr(&after, target) != expected;

            if (failed) return 0;
        }
        for (form = 0u; form != 8u; ++form) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u8 code[] = {(lib_u8)(0xb8u + form), 0x34u,
                (lib_u8)(0x12u + form)};
            lib_u32 expected;
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            gpr_mov_seed(&state);
            before = state.cpu;
            expected = (gpr_mov_gpr(&before, form) & 0xffff0000u) |
                (lib_u16)(0x1234u + (form << 8u));
            failed |= cpu_instruction_run(&state, code, sizeof(code), &after) != LIB_STATUS_OK ||
                state.fault.valid || after.data.eip != sizeof(code) ||
                !gpr_mov_nonparticipants(&before, &after, form) ||
                gpr_mov_gpr(&after, form) != expected;

            if (failed) return 0;
        }
    }
    {
        static const lib_u8 codes[][2] = {
            {0x88u,0xcbu}, {0x89u,0xcbu}, {0x8au,0xcbu}, {0x8bu,0xcbu}
        };
        lib_u8 direction;

        for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
             ++profile)
        for (direction = 0u; direction != sizeof(codes) / sizeof(codes[0]);
             ++direction) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u8 destination = direction < 2u ? 3u : 1u;
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            gpr_mov_seed(&state);
            before = state.cpu;
            failed |= cpu_instruction_run(&state, codes[direction], sizeof(codes[direction]), &after) != LIB_STATUS_OK || state.fault.valid ||
                after.data.eip != sizeof(codes[direction]) ||
                !gpr_mov_nonparticipants(&before, &after, destination);
            if (direction == 0u)
                failed |= after.data.ebx !=
                    ((before.data.ebx & 0xffffff00u) | before.data.cl);
            if (direction == 1u)
                failed |= after.data.ebx !=
                    ((before.data.ebx & 0xffff0000u) | before.data.cx);
            if (direction == 2u)
                failed |= after.data.ecx !=
                    ((before.data.ecx & 0xffffff00u) | before.data.bl);
            if (direction == 3u)
                failed |= after.data.ecx !=
                    ((before.data.ecx & 0xffff0000u) | before.data.bx);

            if (failed) return 0;
        }
    }
    return 1;
}

static lib_i32 gpr_mov_test_immediate_and_reject(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_CPU_PROFILE_80486SX
    };
    static const lib_u8 valid[][6] = {
        {0xc6u,0x06u,0,0x10u,0x5au,0},
        {0xc7u,0x06u,0,0x10u,0x34u,0x12u}
    };
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != 2u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u16 image = 0u;
        lib_u8 bytes = form == 0u ? 5u : 6u;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, profiles[profile]);
        gpr_mov_seed(&state);
        before = state.cpu;
        failed |= cpu_instruction_run(&state, valid[form], bytes, &after) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != bytes ||
            !gpr_mov_nonparticipants(&before, &after, 8u) ||
            cpu_instruction_read(&state, 0x1000u, &image, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK ||
            image != (form == 0u ? 0x005au : 0x1234u);

        if (failed) return 0;
    }
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != 2u; ++form) {
        lib_u8 extension;

        for (extension = 1u; extension != 8u; ++extension) {
            cpu_instruction_fixture state;
            lib_u16 image = 0xbeefu;
            lib_u8 code[] = {(lib_u8)(form ? 0xc7u : 0xc6u),
                (lib_u8)(0x06u | (extension << 3u)),0,0x10u,0,0};
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            gpr_mov_seed(&state);
            failed |= cpu_instruction_write(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK;
            failed |= !cpu_instruction_expect_real_fault(&state, code,
                form ? 6u : 5u, 6u) ||
                cpu_instruction_read(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) !=
                    LIB_STATUS_OK || image != 0xbeefu;

            if (failed) return 0;
        }
    }
    return 1;
}

static lib_i32 gpr_mov_test_386_attributes(core_machine_cpu_profile profile)
{
    static const lib_u8 codes[][12] = {
        {0x66u, 0x89u, 0x0eu, 0, 0x10u},
        {0x67u, 0x8bu, 0x05u, 0, 0x10u, 0, 0},
        {0x66u, 0x67u, 0x8bu, 0x05u, 0, 0x10u, 0, 0},
        {0x66u, 0xb8u, 0x44u, 0x33u, 0x22u, 0x11u},
        {0x67u, 0xb0u, 0x5au}
    };
    static const lib_u8 bytes[] = {5u, 7u, 8u, 6u, 3u};
    lib_u8 form;

    for (form = 0u; form != sizeof(bytes); ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u32 image = 0xaabbccddU;
        lib_u8 destination = form == 0u ? 8u : 0u;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, profile);
        gpr_mov_seed(&state);
        before = state.cpu;
        failed |= cpu_instruction_write(&state, 0x1000u, &image, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, codes[form], bytes[form], &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.eip != bytes[form] ||
            !gpr_mov_nonparticipants(&before, &after, destination);
        if (form == 0u)
            failed |= cpu_instruction_read(&state, 0x1000u, &image, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK ||
                image != 0x11225566u;
        if (form == 1u)
            failed |= after.data.eax != 0xaabbccddu;
        if (form == 2u)
            failed |= after.data.eax != 0xaabbccddu;
        if (form == 3u)
            failed |= after.data.eax != 0x11223344u;
        if (form == 4u)
            failed |= after.data.eax != 0xaabb335au;

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 gpr_mov_test_immediate_register_386_attributes(
    core_machine_cpu_profile profile)
{
    lib_u8 form;

    for (form = 0u; form != 8u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u8 code[] = {0x66u, (lib_u8)(0xb0u + form),
            (lib_u8)(0x80u + form)};
        lib_u8 target = form & 3u;
        lib_u32 expected;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, profile);
        gpr_mov_seed(&state);
        before = state.cpu;
        expected = gpr_mov_gpr(&before, target);
        expected = form < 4u ?
            (expected & 0xffffff00u) | (0x80u + form) :
            (expected & 0xffff00ffu) |
                ((lib_u32)(0x80u + form) << 8u);
        failed |= cpu_instruction_run(&state, code, sizeof(code), &after) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != sizeof(code) ||
            !gpr_mov_nonparticipants(&before, &after, target) ||
            gpr_mov_gpr(&after, target) != expected;

        if (failed) return 0;
    }
    for (form = 0u; form != 8u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u8 code[] = {0x66u, (lib_u8)(0xb8u + form),
            0x44u, 0x33u, 0x22u, (lib_u8)(0x11u + form)};
        lib_u32 expected = 0x11223344u + ((lib_u32)form << 24u);
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, profile);
        gpr_mov_seed(&state);
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, sizeof(code), &after) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != sizeof(code) ||
            !gpr_mov_nonparticipants(&before, &after, form) ||
            gpr_mov_gpr(&after, form) != expected;

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 gpr_mov_test_prefix_lock(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 prefix_codes[][7] = {
        {0x66u,0x8bu,0x06u,0,0x10u},
        {0x67u,0x89u,0x06u,0,0x10u},
        {0x66u,0xc7u,0x06u,0,0x10u,0},
        {0x67u,0xb8u,0,0,0},
        {0x66u,0xb0u,0},
        {0x66u,0xb8u,0,0,0}
    };
    static const lib_u8 prefix_sizes[] = {5u,5u,6u,5u,3u,5u};
    static const lib_u8 lock_codes[][7] = {
        {0xf0u,0x88u,0x06u,0,0x10u},
        {0xf0u,0x89u,0x0eu,0,0x10u},
        {0xf0u,0x8au,0x06u,0,0x10u},
        {0xf0u,0x8bu,0x0eu,0,0x10u},
        {0xf0u,0xc6u,0x06u,0,0x10u,0x5au},
        {0xf0u,0xc7u,0x06u,0,0x10u,0x34u,0x12u},
        {0xf0u,0xb0u,0x5au}, {0xf0u,0xb8u,0x34u,0x12u}
    };
    static const lib_u8 lock_sizes[] = {5u,5u,5u,5u,6u,7u,3u,4u};
    static const core_machine_cpu_profile modern[] = {
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_CPU_PROFILE_80486SX
    };
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile) {
        for (form = 0u; form != sizeof(prefix_sizes); ++form) {
            cpu_instruction_fixture state;
            lib_u16 image = 0xbeefu;
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            gpr_mov_seed(&state);
            failed |= cpu_instruction_write(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK;
            failed |= !cpu_instruction_expect_real_fault(&state,
                prefix_codes[form], prefix_sizes[form], 6u) ||
                cpu_instruction_read(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) !=
                    LIB_STATUS_OK || image != 0xbeefu;

            if (failed) return 0;
        }
    }
    for (profile = 0u; profile != sizeof(modern) / sizeof(modern[0]); ++profile)
    for (form = 0u; form != sizeof(lock_sizes); ++form) {
        cpu_instruction_fixture state;
        lib_u16 image = 0xbeefu;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, modern[profile]);
        gpr_mov_seed(&state);
        failed |= cpu_instruction_write(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK;
        failed |= !cpu_instruction_expect_real_fault(&state, lock_codes[form],
            lock_sizes[form], 6u) ||
            cpu_instruction_read(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) !=
                LIB_STATUS_OK || image != 0xbeefu;

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 gpr_mov_test_segments(core_machine_cpu_profile profile)
{
    static const lib_u8 codes[][5] = {
        {0x8au, 0x06u, 0x10u, 0, 0},
        {0x8au, 0x46u, 0, 0, 0},
        {0x26u, 0x88u, 0x06u, 0x10u, 0},
        {0x64u, 0x8au, 0x06u, 0x10u, 0},
        {0x65u, 0x88u, 0x06u, 0x10u, 0},
        {0x67u, 0x8au, 0x45u, 0, 0}
    };
    static const lib_u8 bytes[] = {4u, 3u, 5u, 5u, 5u, 4u};
    static const lib_u8 values[] = {0x11u, 0x22u, 0, 0x44u, 0, 0x66u};
    lib_u8 form;

    for (form = 0u; form != sizeof(bytes); ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u8 image = values[form];
        lib_u32 address = form == 0u ? 0x10u : form == 1u ? 0x110u :
            form == 2u ? 0x110u : form == 3u ? 0x210u :
            form == 4u ? 0x310u : 0x110u;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, profile);
        if (form == 1u || form == 5u) {
            failed |= core_machine_cpu_execution_load_segment(
                &state.execution,
                &state.cpu.data.ss, 0x10u) != 0;
        }
        if (form == 2u) {
            failed |= core_machine_cpu_execution_load_segment(
                &state.execution,
                &state.cpu.data.es, 0x10u) != 0;
        }
        if (form == 3u) {
            failed |= core_machine_cpu_execution_load_segment(
                &state.execution,
                &state.cpu.data.fs, 0x20u) != 0;
        }
        if (form == 4u) {
            failed |= core_machine_cpu_execution_load_segment(
                &state.execution,
                &state.cpu.data.gs, 0x30u) != 0;
        }
        gpr_mov_seed(&state);
        state.cpu.data.ebp = 0x10u;
        before = state.cpu;
        failed |= cpu_instruction_write(&state, address, &image, 1u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, codes[form], bytes[form], &after) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != bytes[form] ||
            !gpr_mov_nonparticipants(&before, &after,
                (form == 2u || form == 4u) ? 8u : 0u);
        if (form == 0u || form == 1u || form == 3u || form == 5u) {
            failed |= after.data.eax !=
                ((before.data.eax & 0xffffff00u) | values[form]);
        }
        if (form == 2u || form == 4u) {
            image = 0u;
            failed |= cpu_instruction_read(&state, address, &image, 1u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK ||
                image != 0x44u;
        }

        if (failed) return 0;
    }
    return 1;
}

static lib_bool gpr_mov_test_register_aliases(core_machine_cpu_profile profile)
{
    static const struct {
        lib_u8 mode;
        lib_bool code32;
    } contexts[] = {
        {0u, LIB_FALSE},
        {1u, LIB_FALSE}, {1u, LIB_TRUE},
        {2u, LIB_FALSE}, {2u, LIB_TRUE},
        {3u, LIB_FALSE}, {3u, LIB_TRUE},
        {4u, LIB_FALSE}, {4u, LIB_TRUE},
        {5u, LIB_FALSE}
    };
    static const lib_u8 widths[] = {1u, 2u, 4u};

    for (lib_size mode = 0u; mode < sizeof(contexts) / sizeof(contexts[0]); ++mode)
    for (lib_size width = 0u; width < sizeof(widths); ++width)
    for (lib_u8 direction = 0u; direction < 2u; ++direction)
    for (lib_u8 extended = 0u; extended <
         (profile == CORE_MACHINE_CPU_PROFILE_80486SX ? 2u : 1u); ++extended)
    for (lib_u8 alias = 0u; alias < 64u; ++alias) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        const lib_u8 source = alias >> 3u;
        const lib_u8 destination = alias & 7u;
        const lib_u8 source_gpr = widths[width] == 1u ? source & 3u : source;
        const lib_u8 destination_gpr = widths[width] == 1u ?
            destination & 3u : destination;
        const lib_u8 source_shift = widths[width] == 1u && source >= 4u ? 8u : 0u;
        const lib_u8 destination_shift = widths[width] == 1u &&
            destination >= 4u ? 8u : 0u;
        const lib_u32 mask = widths[width] == 1u ? 0xffu :
            widths[width] == 2u ? 0xffffu : 0xffffffffu;
        lib_u8 code[4] = {0u};
        lib_u8 bytes = 0u;
        lib_u32 expected;

        cpu_instruction_prepare(&state, profile);
        gpr_mov_seed(&state);
        if (contexts[mode].mode != 0u)
            cpu_instruction_prepare_protected_exception(&state,
                contexts[mode].mode - 1u);
        state.cpu.data.cs.seg.exec.defsize = contexts[mode].code32;
        state.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
        if (((state.cpu.data.cr0 & VCPU_CR0_PE) != 0u) !=
                (contexts[mode].mode != 0u) ||
            ((state.cpu.data.eflags & VCPU_EFLAGS_VM) != 0u) !=
                (contexts[mode].mode == 5u)) return LIB_FALSE;
        before = state.cpu;
        if (widths[width] != 1u &&
            contexts[mode].code32 != (widths[width] == 4u)) code[bytes++] = 0x66u;
        if (extended) code[bytes++] = 0x0fu;
        code[bytes++] = (lib_u8)((extended ? 0x10u : 0x88u) + (direction << 1u) +
            (widths[width] != 1u));
        code[bytes++] = (lib_u8)(0xc0u | (direction ?
            (destination << 3u) | source : (source << 3u) | destination));
        expected = (gpr_mov_gpr(&before, destination_gpr) &
            ~(mask << destination_shift)) |
            (((gpr_mov_gpr(&before, source_gpr) >> source_shift) & mask) <<
                destination_shift);
        if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
            state.fault.valid || state.delivered_exception.valid ||
            after.data.eip != bytes ||
            !gpr_mov_nonparticipants(&before, &after, destination_gpr) ||
            gpr_mov_gpr(&after, destination_gpr) != expected) {
            lib_c_fprintf(lib_c_stderr,
                "MOV alias profile=%u context=%u width=%u direction=%u src=%u dst=%u extended=%u\n",
                (unsigned)profile, (unsigned)mode, widths[width], direction,
                source, destination, (unsigned)extended);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool gpr_mov_test_umov_memory(void)
{
    static const lib_u8 widths[] = {1u, 2u, 4u};

    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_size width = 0u; width < sizeof(widths); ++width)
    for (lib_u8 address32 = 0u; address32 < 2u; ++address32)
    for (lib_u8 direction = 0u; direction < 2u; ++direction)
    for (lib_u8 reg = 0u; reg < 8u; ++reg)
    for (lib_u8 segment = 0u; segment < 2u; ++segment)
    for (lib_u8 failure = 0u; failure < (reg ? 1u : 3u); ++failure) {
        t_cpu expected;
        lib_u32 expected_image = 0u, expected_writes = 0u;
        const lib_u32 address = (segment ? 0x20000u : 0x10000u) + 0x1000u;
        const lib_u32 mask = widths[width] == 1u ? 0xffu :
            widths[width] == 2u ? 0xffffu : LIB_UINT32_MAX;
        const lib_u8 gpr = widths[width] == 1u ? reg & 3u : reg;
        const lib_u8 shift = widths[width] == 1u && reg >= 4u ? 8u : 0u;

        if (mode == 5u && code32) continue;
        for (lib_u8 extended = 0u; extended < 2u; ++extended) {
            cpu_instruction_fixture fixture;
            core_machine_cpu_instruction_lexeme lexeme;
            t_cpu before, after;
            lib_u8 code[10] = {0}, bytes = 0u;
            lib_u32 image = 0x89abcdefu;
            const lib_u32 destination = direction ? gpr : 8u;

            cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
            gpr_mov_seed(&fixture);
            if (mode) cpu_instruction_prepare_protected_exception(&fixture, mode - 1u);
            else {
                const lib_u16 handler[] = {0x300u, 0u};

                lib_memory_copy(fixture.memory + 13u * 4u, handler, sizeof(handler));
            }
            fixture.cpu.data.cs.seg.exec.defsize = code32;
            fixture.cpu.data.ds = fixture.cpu.data.ss;
            fixture.cpu.data.ds.sregtype = SREG_DATA;
            fixture.cpu.data.ds.base = 0x10000u;
            fixture.cpu.data.es = fixture.cpu.data.ds;
            fixture.cpu.data.es.base = 0x20000u;
            t_cpu_data_sreg *const cache = segment ? &fixture.cpu.data.es : &fixture.cpu.data.ds;
            if (failure == 1u) cache->limit = 0xfffu;
            if (failure == 2u) cache->base = sizeof(fixture.memory);
            lib_memory_copy(fixture.memory + address, &image, sizeof(image));
            before = fixture.cpu;
            if (widths[width] != 1u && code32 != (widths[width] == 4u))
                code[bytes++] = 0x66u;
            if (address32 != code32) code[bytes++] = 0x67u;
            if (segment) code[bytes++] = 0x26u;
            if (extended) code[bytes++] = 0x0fu;
            code[bytes++] = (lib_u8)((extended ? 0x10u : 0x88u) +
                (direction << 1u) + (widths[width] != 1u));
            code[bytes++] = (lib_u8)((reg << 3u) | (address32 ? 5u : 6u));
            code[bytes++] = 0u;
            code[bytes++] = 0x10u;
            if (address32) bytes += 2u;
            if (!core_machine_cpu_instruction_lexeme_scan(code, bytes,
                    CORE_MACHINE_CPU_PROFILE_80486SX, code32, &lexeme) ||
                !lexeme.available || lexeme.byte_count != bytes) return LIB_FALSE;
            const lib_status status = cpu_instruction_run(&fixture, code, bytes, &after);
            if (status != (failure == 2u ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
                fixture.fault.valid != (failure == 2u) ||
                fixture.delivered_exception.valid != (failure == 1u) ||
                (failure == 1u && (fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
                    fixture.delivered_exception.exception_code != 0u ||
                    fixture.delivered_exception.point.eip != 0u)) ||
                (failure == 2u && (fixture.fault.exception_mask != VCPUINS_EXCEPT_CE ||
                    lib_memory_compare(&before, &after, sizeof(before)) != 0)))
                return LIB_FALSE;
            lib_memory_copy(&image, fixture.memory + address, sizeof(image));
            if (!failure) {
                const lib_u32 result = direction ?
                    (gpr_mov_gpr(&before, gpr) & ~(mask << shift)) |
                        ((0x89abcdefu & mask) << shift) :
                    (0x89abcdefu & ~mask) | ((gpr_mov_gpr(&before, gpr) >> shift) & mask);

                if (after.data.eip != bytes ||
                    !gpr_mov_nonparticipants(&before, &after, (lib_u8)destination) ||
                    (direction ? gpr_mov_gpr(&after, gpr) != result : image != result))
                    return LIB_FALSE;
            } else if (image != 0x89abcdefu) return LIB_FALSE;
            if (!extended) {
                expected = after;
                expected_image = image;
                expected_writes = fixture.writes;
                if (!failure) ++expected.data.eip;
            } else if (lib_memory_compare(&expected, &after, sizeof(expected)) != 0 ||
                expected_image != image || expected_writes != fixture.writes)
                return LIB_FALSE;
        }
    }
    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80286;
         profile <= CORE_MACHINE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 opcode = 0x10u; opcode <= 0x13u; ++opcode) {
        cpu_instruction_fixture fixture;
        core_machine_cpu_instruction_lexeme lexeme;
        const lib_u8 code[] = {0xf0u, 0x0fu, opcode, 0xc0u};
        const lib_u8 start = profile == CORE_MACHINE_CPU_PROFILE_80486SX ? 0u : 1u;

        cpu_instruction_prepare(&fixture, profile);
        if (core_machine_cpu_instruction_lexeme_scan(code + start, sizeof(code) - start,
                profile, LIB_FALSE, &lexeme) || lexeme.available ||
            !cpu_instruction_expect_real_fault(&fixture, code + start, sizeof(code) - start, 6u))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_CPU_PROFILE_80486SX
    };

    if (!gpr_mov_test_defaults() ||
        !gpr_mov_test_immediate_and_reject() ||
        !gpr_mov_test_prefix_lock()) return 1;
    if (!gpr_mov_test_umov_memory()) {
        lib_c_printf("UMOV memory/failure: FAILED\n");
        return 1;
    }
    for (lib_size index = 0u; index < sizeof(profiles) / sizeof(profiles[0]);
        ++index) {
        if (!gpr_mov_test_register_aliases(profiles[index]) ||
            !gpr_mov_test_386_attributes(profiles[index]) ||
            !gpr_mov_test_immediate_register_386_attributes(profiles[index]) ||
            !gpr_mov_test_segments(profiles[index])) return 1;
    }
    lib_c_printf("GPR_MOV:CPU:OK\n");
    return 0;
}
