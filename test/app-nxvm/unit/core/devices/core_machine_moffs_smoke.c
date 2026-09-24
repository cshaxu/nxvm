#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct moffs_machine {
    core_machine *machine;
} moffs_machine;

static void moffs_reset(void *opaque)
{
    moffs_machine *state = (moffs_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider moffs_provider = {
    moffs_reset, LIB_NULL
};

static lib_i32 moffs_prepare(core_machine_cpu_profile profile, moffs_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    lib_memory_set(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &moffs_provider, state, &state->machine);
}

static lib_i32 moffs_run(moffs_machine *state, const lib_u8 *code, lib_u8 bytes,
    t_cpu *after, core_machine_cpu_diagnostic *diagnostic, lib_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) != LIB_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == LIB_STATUS_OK;
}

static void moffs_set_registers(moffs_machine *state)
{
    state->machine->executor_cpu.data.eax = 0xaabb3344u;
    state->machine->executor_cpu.data.ecx = 0x11223344u;
    state->machine->executor_cpu.data.edx = 0x55667788u;
    state->machine->executor_cpu.data.ebx = 0x99aabbccu;
    state->machine->executor_cpu.data.esi = 0xddeeff00u;
    state->machine->executor_cpu.data.edi = 0x10203040u;
    state->machine->executor_cpu.data.ebp = 0x50607080u;
    state->machine->executor_cpu.data.esp = 0x00007777u;
    state->machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
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
            moffs_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            lib_status status;
            lib_u8 code[] = { opcodes[opcode], 0x00u, 0x10u };
            lib_u32 image = opcodes[opcode] == 0xa0u || opcodes[opcode] == 0xa2u ?
                0x0000005au : 0x0000beefu;
            lib_u32 expected_eax;
            lib_i32 failed;

            lib_memory_set(&state, 0, sizeof(state));
            lib_memory_set(&before, 0, sizeof(before));
            lib_memory_set(&after, 0, sizeof(after));
            lib_memory_set(&diagnostic, 0, sizeof(diagnostic));
            status = LIB_STATUS_INVALID_ARGUMENT;
            failed = !moffs_prepare(profiles[profile], &state);
            if (!failed)
            {
                failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u);
                moffs_set_registers(&state);
                if (opcodes[opcode] == 0xa0u || opcodes[opcode] == 0xa1u)
                    failed |= core_machine_memory_write(state.machine, 0x1000u,
                        &image, opcodes[opcode] == 0xa0u ? 1u : 2u) != LIB_STATUS_OK;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                expected_eax = opcodes[opcode] == 0xa0u ? 0xaabb335au :
                    opcodes[opcode] == 0xa1u ? 0xaabbbeefu : before.data.eax;
                failed |= !moffs_run(&state, code, sizeof(code), &after,
                    &diagnostic, &status) || status != LIB_STATUS_OK ||
                    diagnostic.first_fault.valid || after.data.eip != 3u ||
                    after.data.eax != expected_eax ||
                    !moffs_nonparticipants(&before, &after, opcodes[opcode]);
                if (opcodes[opcode] == 0xa2u || opcodes[opcode] == 0xa3u)
                {
                    image = 0u;
                    failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                        0x1000u, (lib_uptr)&image,
                        opcodes[opcode] == 0xa2u ? 1u : 2u) != LIB_STATUS_OK ||
                        image != (opcodes[opcode] == 0xa2u ? 0x44u : 0x3344u);
                }
            }
            core_machine_destroy(state.machine);
            if (failed)
            {
                printf("MOFFS default profile=%u opcode=%02x\n",
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
        moffs_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        lib_status status = LIB_STATUS_INVALID_STATE;
        lib_u32 image = 0x1122335au;
        lib_i32 failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
        {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            moffs_set_registers(&state);
            if (!write[form])
                failed |= core_machine_memory_write(state.machine, 0x10000u, &image,
                    widths[form]) != LIB_STATUS_OK;
            failed |= !moffs_run(&state, codes[form], 7u, &after, &diagnostic,
                &status) || status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != 7u;
            if (form == 0u) failed |= after.data.eax != 0x1122335au;
            if (form == 2u) failed |= after.data.eax != 0xaabb335au;
            if (write[form])
            {
                image = 0u;
                failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x10000u, (lib_uptr)&image, widths[form]) != LIB_STATUS_OK ||
                    image != (form == 1u ? 0xaabb3344u : 0x44u);
            }
        }
        core_machine_destroy(state.machine);
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
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status;
        lib_u8 code[] = {0x66u,0x67u,opcode,0,0x80u,0,0};
        const lib_u8 bytes = attribute == 0u ? 4u :
            attribute == 1u ? 6u : 7u;
        const lib_u32 address = attribute == 0u ? 0x1000u : 0x8000u;
        const lib_u8 width = opcode == 0xa0u || opcode == 0xa2u ?
            1u : attribute == 1u ? 2u : 4u;
        lib_u32 image = 0x1122335au;
        lib_i32 failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            if (attribute == 0u) {
                code[1] = opcode; code[2] = 0; code[3] = 0x10u;
            } else if (attribute == 1u) {
                code[0] = 0x67u; code[1] = opcode; code[2] = 0; code[3] = 0x80u; code[4] = 0;
            }
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            moffs_set_registers(&state);
            if (opcode == 0xa0u || opcode == 0xa1u)
                failed |= core_machine_memory_write(state.machine, address,
                    &image, width) != LIB_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, code, bytes, &after, &diagnostic,
                &status) || status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != bytes || !moffs_nonparticipants(&before,
                &after, opcode);
            if (opcode == 0xa0u) failed |= after.data.eax != 0xaabb335au;
            if (opcode == 0xa1u) failed |= after.data.eax != (attribute == 1u ?
                0xaabb335au : 0x1122335au);
            if (opcode == 0xa2u || opcode == 0xa3u) {
                image = 0u;
                failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    address, CORE_MACHINE_REFERENCE_OF(image), width) != LIB_STATUS_OK ||
                    image != (width == 1u ? 0x44u : width == 2u ? 0x3344u :
                    0xaabb3344u);
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 moffs_state_equal(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.eflags == after->data.eflags &&
        before->data.eip == after->data.eip;
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
        moffs_machine state;
        t_cpu before = {0};
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        lib_status status = LIB_STATUS_INVALID_STATE;
        lib_u8 code[] = { prefixes[prefix], opcodes[opcode], 0u, 0x10u };
        lib_i32 failed = !moffs_prepare(profiles[profile], &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
            moffs_set_registers(&state);
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, code, sizeof(code), &after, &diagnostic,
                &status) || status != LIB_STATUS_INTERNAL_ERROR ||
                !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                !moffs_state_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            printf("MOFFS reject profile=%u prefix=%02x opcode=%02x status=%d fault=%08x\n",
                profiles[profile], prefixes[prefix], opcodes[opcode], status,
                diagnostic.first_fault.exception_mask);
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
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status;
        lib_u8 code[] = { 0xf0u, opcodes[opcode], 0u, 0x10u };
        lib_i32 failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
            moffs_set_registers(&state);
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, code, sizeof(code), &after, &diagnostic,
                &status) || status != LIB_STATUS_INTERNAL_ERROR ||
                !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                !moffs_state_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
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
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status;
        lib_u32 address = form == 0u ? 0x10u : (lib_u32)form * 0x100u + 0x10u;
        lib_i32 failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
            if (form == 1u) failed |= core_machine_cpu_execution_load_segment(
                &state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.es, 0x10u) != 0;
            if (form == 2u) failed |= core_machine_cpu_execution_load_segment(
                &state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.fs, 0x20u) != 0;
            if (form == 3u) failed |= core_machine_cpu_execution_load_segment(
                &state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.gs, 0x30u) != 0;
            moffs_set_registers(&state);
            failed |= core_machine_memory_write(state.machine, address, &values[form], 1u) != LIB_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, codes[form], bytes[form], &after, &diagnostic,
                &status) || status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != bytes[form] || after.data.eax != (0xaabb3300u | values[form]) ||
                !moffs_nonparticipants(&before, &after, 0xa0u);
        }
        core_machine_destroy(state.machine);
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
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status;
        lib_u32 image = 0u;
        lib_u32 address = form == 0u ? 0x10u : (lib_u32)form * 0x100u + 0x10u;
        lib_i32 failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
            if (form == 1u) failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.es, 0x10u) != 0;
            if (form == 2u) failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.fs, 0x20u) != 0;
            if (form == 3u) failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.gs, 0x30u) != 0;
            moffs_set_registers(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, codes[form], bytes[form], &after, &diagnostic, &status) || status != LIB_STATUS_OK || diagnostic.first_fault.valid || after.data.eip != bytes[form] || !moffs_nonparticipants(&before, &after, 0xa2u) || core_machine_memory_read_physical(&state.machine->executor_memory, address, (lib_uptr)&image, widths[form]) != LIB_STATUS_OK || image != (widths[form] == 1u ? 0x44u : 0x3344u);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 moffs_test_protected_read_limit(void)
{
    static const lib_u8 gdt_pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const lib_u8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0x0fu, 0, 0, 0x30u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x40u, 0, 0x92u, 0, 0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0xb8u, 0x18u, 0x00u, 0x8eu, 0xd0u,
        0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const lib_u8 halt[] = { 0xf4u };
    static const lib_u8 read_code[] = { 0xa0u, 0x10u, 0x00u };
    static const lib_u8 write_code[] = { 0x66u, 0xa3u, 0x10u, 0x00u };
    const lib_u8 *codes[] = { read_code, write_code };
    const lib_u8 bytes[] = { sizeof(read_code), sizeof(write_code) };
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        lib_u32 image = 0x11223344u;
        lib_i32 failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
        {
            failed |= core_machine_memory_write(state.machine, 0x0100u,
                gdt_pointer, sizeof(gdt_pointer)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0300u, gdt,
                    sizeof(gdt)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, bootstrap,
                    sizeof(bootstrap)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, halt,
                    sizeof(halt)) != LIB_STATUS_OK ||
                core_machine_run(state.machine, (core_machine_run_budget){ 96u, 0u },
                    &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        }
        if (!failed)
        {
            moffs_set_registers(&state);
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &image,
                sizeof(image)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, codes[form],
                    bytes[form]) != LIB_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi || after.data.ebp != before.data.ebp ||
                after.data.esp != before.data.esp ||
                after.data.eflags != before.data.eflags ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x3010u, (lib_uptr)&image, sizeof(image)) !=
                    LIB_STATUS_OK || image != 0x11223344u;
        }
        core_machine_destroy(state.machine);
        if (failed)
        {
            printf("MOFFS protected-limit form=%u\n", form);
            return 0;
        }
    }
    return 1;
}

static lib_i32 moffs_test_irq_no_shadow(void)
{
    static const lib_u8 codes[][4] = {
        { 0xa0u, 0x00u, 0x10u, 0x90u },
        { 0xa2u, 0x00u, 0x10u, 0x90u }
    };
    static const lib_u8 hlt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        moffs_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        lib_u16 vector_offset = 0x0100u;
        lib_u16 vector_segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u8 image = form == 0u ? 0x5au : 0u;
        lib_i32 failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
        {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u) ||
                core_machine_memory_write(state.machine, 0x1000u, &image,
                    sizeof(image)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, codes[form],
                    sizeof(codes[form])) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u,
                    &vector_offset, sizeof(vector_offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u + 2u,
                    &vector_segment, sizeof(vector_segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0100u, &hlt,
                    sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed)
        {
            moffs_set_registers(&state);
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            lib_memory_set(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    after.data.ss.base + (lib_u16)after.data.esp,
                    (lib_uptr)&frame_ip, sizeof(frame_ip)) !=
                    LIB_STATUS_OK || after.data.eip != 0x0101u || frame_ip != 3u ||
                !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(
                    state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                (form == 0u && after.data.eax != 0xaabb335au) ||
                (form == 1u && (core_machine_memory_read_physical(
                    &state.machine->executor_memory, 0x1000u,
                    (lib_uptr)&image, sizeof(image)) != LIB_STATUS_OK ||
                    image != 0x44u));
        }
        core_machine_destroy(state.machine);
        if (failed)
        {
            printf("MOFFS irq form=%u\n", form);
            return 0;
        }
    }
    return 1;
}

lib_i32 main(void)
{
    if (!moffs_test_default())
    {
        printf("MOFFS stage=default\n");
        return 1;
    }
    if (!moffs_test_386_attributes() ||
        !moffs_test_386_single_attributes())
    {
        printf("MOFFS stage=attributes\n");
        return 1;
    }
    if (!moffs_test_reject())
    {
        printf("MOFFS stage=reject\n");
        return 1;
    }
    if (!moffs_test_lock())
    {
        printf("MOFFS stage=lock\n");
        return 1;
    }
    if (!moffs_test_segment_overrides())
    {
        printf("MOFFS stage=segment\n");
        return 1;
    }
    if (!moffs_test_segment_writes())
    {
        printf("MOFFS stage=segment-write\n");
        return 1;
    }
    if (!moffs_test_protected_read_limit())
    {
        printf("MOFFS stage=protected-limit\n");
        return 1;
    }
    if (!moffs_test_irq_no_shadow())
    {
        printf("MOFFS stage=irq\n");
        return 1;
    }
    printf("M5:T316:S30:MOFFS:OK\n");
    printf("M5:T401:S14:MOFFS-MOV-PROFILES:OK\n");
    return 0;
}
