#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* T337_REAL_UD_TERMINAL_CPU_OWNER: negative cases retain the CPU-owned IDTR limit. */
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
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
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
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            before = state.cpu;
            failed |= cpu_instruction_run(&state, memory_codes[form], 4u, &after) != LIB_STATUS_OK || state.fault.valid || after.data.eip != 4u ||
                !gpr_mov_nonparticipants(&before, &after,
                    form == 3u ? 1u :
                    (form == 0u || form == 1u) ? 8u : 0u);
            if (form == 0u) failed |= (after.data.eax & 0xffu) != 0x44u;
            if (form == 1u) failed |= cpu_instruction_read(&state, 0x1000u, &image, 2u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || image != 0x5566u;
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

        for (direction = 0u; direction != sizeof(codes) / sizeof(codes[0]);
             ++direction) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u8 destination = direction < 2u ? 3u : 1u;
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
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
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            image != (form == 0u ? 0x005au : 0x1234u);

        if (failed) return 0;
    }
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != 2u; ++form) {
        lib_u8 extension;

        for (extension = 1u; extension != 8u; ++extension) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u16 image = 0xbeefu;
            lib_u8 code[] = {(lib_u8)(form ? 0xc7u : 0xc6u),
                (lib_u8)(0x06u | (extension << 3u)),0,0x10u,0,0};
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            gpr_mov_seed(&state);
            failed |= cpu_instruction_write(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            state.cpu.data.idtr.limit = 0x17u;
            before = state.cpu;
            failed |= cpu_instruction_run(&state, code, form ? 6u : 5u, &after) != LIB_STATUS_INTERNAL_ERROR ||
                !state.fault.valid || !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
                after.data.eip != before.data.eip ||
                !gpr_mov_nonparticipants(&before, &after, 8u) ||
                cpu_instruction_read(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK || image != 0xbeefu;

            if (failed) return 0;
        }
    }
    return 1;
}

static lib_i32 gpr_mov_test_386_attributes(void)
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        gpr_mov_seed(&state);
        before = state.cpu;
        failed |= cpu_instruction_write(&state, 0x1000u, &image, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, codes[form], bytes[form], &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.eip != bytes[form] ||
            !gpr_mov_nonparticipants(&before, &after, destination);
        if (form == 0u)
            failed |= cpu_instruction_read(&state, 0x1000u, &image, 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
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

static lib_i32 gpr_mov_test_immediate_register_386_attributes(void)
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile) {
        for (form = 0u; form != sizeof(prefix_sizes); ++form) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u16 image = 0xbeefu;
            lib_i32 failed = 0;

            cpu_instruction_prepare(&state, profiles[profile]);
            gpr_mov_seed(&state);
            failed |= cpu_instruction_write(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            state.cpu.data.idtr.limit = 0x17u;
            before = state.cpu;
            failed |= cpu_instruction_run(&state, prefix_codes[form], prefix_sizes[form], &after) != LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
                !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) || after.data.eip != before.data.eip ||
                !gpr_mov_nonparticipants(&before, &after, 8u) ||
                cpu_instruction_read(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK || image != 0xbeefu;

            if (failed) return 0;
        }
    }
    for (form = 0u; form != sizeof(lock_sizes); ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u16 image = 0xbeefu;
        lib_i32 failed = 0;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        gpr_mov_seed(&state);
        failed |= cpu_instruction_write(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        state.cpu.data.idtr.limit = 0x17u;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, lock_codes[form], lock_sizes[form], &after) != LIB_STATUS_INTERNAL_ERROR ||
            !state.fault.valid || !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
            after.data.eip != before.data.eip ||
            !gpr_mov_nonparticipants(&before, &after, 8u) ||
            cpu_instruction_read(&state, 0x1000u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || image != 0xbeefu;

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 gpr_mov_test_segments(void)
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

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
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
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                image != 0x44u;
        }

        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!gpr_mov_test_defaults() ||
        !gpr_mov_test_immediate_and_reject() ||
        !gpr_mov_test_386_attributes() ||
        !gpr_mov_test_immediate_register_386_attributes() ||
        !gpr_mov_test_prefix_lock() ||
        !gpr_mov_test_segments()) return 1;
    lib_c_printf("M5:T539:S22:GPR_MOV:CPU:OK\n");
    return 0;
}
