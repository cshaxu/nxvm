#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct moffs_machine {
    core_machine *machine;
} moffs_machine;

static C_VOID moffs_reset(C_VOID *opaque)
{
    moffs_machine *state = (moffs_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider moffs_provider = {
    moffs_reset, STD_NULL
};

static C_INT moffs_prepare(core_machine_cpu_profile profile, moffs_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &moffs_provider, state, &state->machine);
}

static C_INT moffs_run(moffs_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes,
    t_cpu *after, core_machine_cpu_diagnostic *diagnostic, type_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK;
}

static C_VOID moffs_set_registers(moffs_machine *state)
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

static C_INT moffs_nonparticipants(const t_cpu *before, const t_cpu *after,
    type_unsigned_8 opcode)
{
    return before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esi == after->data.esi &&
        before->data.eflags == after->data.eflags &&
        (opcode == 0xa0u || opcode == 0xa1u ||
            before->data.eax == after->data.eax);
}

static C_INT moffs_test_default(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 opcodes[] = { 0xa0u, 0xa1u, 0xa2u, 0xa3u };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
        {
            moffs_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_8 code[] = { opcodes[opcode], 0x00u, 0x10u };
            type_unsigned_32 image = opcodes[opcode] == 0xa0u || opcodes[opcode] == 0xa2u ?
                0x0000005au : 0x0000beefu;
            type_unsigned_32 expected_eax;
            C_INT failed;

            STD_MEMSET(&state, 0, sizeof(state));
            STD_MEMSET(&before, 0, sizeof(before));
            STD_MEMSET(&after, 0, sizeof(after));
            STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
            status = TYPE_STATUS_INVALID_ARGUMENT;
            failed = !moffs_prepare(profiles[profile], &state);
            if (!failed)
            {
                failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u);
                moffs_set_registers(&state);
                if (opcodes[opcode] == 0xa0u || opcodes[opcode] == 0xa1u)
                    failed |= core_machine_memory_write(state.machine, 0x1000u,
                        &image, opcodes[opcode] == 0xa0u ? 1u : 2u) != TYPE_STATUS_OK;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                expected_eax = opcodes[opcode] == 0xa0u ? 0xaabb335au :
                    opcodes[opcode] == 0xa1u ? 0xaabbbeefu : before.data.eax;
                failed |= !moffs_run(&state, code, sizeof(code), &after,
                    &diagnostic, &status) || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid || after.data.eip != 3u ||
                    after.data.eax != expected_eax ||
                    !moffs_nonparticipants(&before, &after, opcodes[opcode]);
                if (opcodes[opcode] == 0xa2u || opcodes[opcode] == 0xa3u)
                {
                    image = 0u;
                    failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                        0x1000u, (type_virtual_address)&image,
                        opcodes[opcode] == 0xa2u ? 1u : 2u) != TYPE_STATUS_OK ||
                        image != (opcodes[opcode] == 0xa2u ? 0x44u : 0x3344u);
                }
            }
            core_machine_destroy(state.machine);
            if (failed)
            {
                STD_PRINTF("MOFFS default profile=%u opcode=%02x\n",
                    profiles[profile], opcodes[opcode]);
                return 0;
            }
        }
    }
    return 1;
}

static C_INT moffs_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 read32[] = { 0x66u,0x67u,0xa1u,0x00u,0x00u,0x01u,0x00u };
    static const type_unsigned_8 write32[] = { 0x66u,0x67u,0xa3u,0x00u,0x00u,0x01u,0x00u };
    static const type_unsigned_8 read8[] = { 0x66u,0x67u,0xa0u,0x00u,0x00u,0x01u,0x00u };
    static const type_unsigned_8 write8[] = { 0x66u,0x67u,0xa2u,0x00u,0x00u,0x01u,0x00u };
    const type_unsigned_8 *codes[] = { read32, write32, read8, write8 };
    const type_unsigned_8 write[] = { 0u, 1u, 0u, 1u };
    const type_unsigned_8 widths[] = { 4u, 4u, 1u, 1u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        moffs_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        type_status status = TYPE_STATUS_INVALID_STATE;
        type_unsigned_32 image = 0x1122335au;
        C_INT failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
        {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            moffs_set_registers(&state);
            if (!write[form])
                failed |= core_machine_memory_write(state.machine, 0x10000u, &image,
                    widths[form]) != TYPE_STATUS_OK;
            failed |= !moffs_run(&state, codes[form], 7u, &after, &diagnostic,
                &status) || status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != 7u;
            if (form == 0u) failed |= after.data.eax != 0x1122335au;
            if (form == 2u) failed |= after.data.eax != 0xaabb335au;
            if (write[form])
            {
                image = 0u;
                failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x10000u, (type_virtual_address)&image, widths[form]) != TYPE_STATUS_OK ||
                    image != (form == 1u ? 0xaabb3344u : 0x44u);
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT moffs_test_386_single_attributes(C_VOID)
{
    type_unsigned_8 attribute;
    type_unsigned_8 opcode;

    for (attribute = 0u; attribute != 3u; ++attribute)
    for (opcode = 0xa0u; opcode != 0xa4u; ++opcode) {
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 code[] = {0x66u,0x67u,opcode,0,0x80u,0,0};
        const type_unsigned_8 bytes = attribute == 0u ? 4u :
            attribute == 1u ? 6u : 7u;
        const type_unsigned_32 address = attribute == 0u ? 0x1000u : 0x8000u;
        const type_unsigned_8 width = opcode == 0xa0u || opcode == 0xa2u ?
            1u : attribute == 1u ? 2u : 4u;
        type_unsigned_32 image = 0x1122335au;
        C_INT failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

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
                    &image, width) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, code, bytes, &after, &diagnostic,
                &status) || status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != bytes || !moffs_nonparticipants(&before,
                &after, opcode);
            if (opcode == 0xa0u) failed |= after.data.eax != 0xaabb335au;
            if (opcode == 0xa1u) failed |= after.data.eax != (attribute == 1u ?
                0xaabb335au : 0x1122335au);
            if (opcode == 0xa2u || opcode == 0xa3u) {
                image = 0u;
                failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    address, TYPE_REFERENCE_OF(image), width) != TYPE_STATUS_OK ||
                    image != (width == 1u ? 0x44u : width == 2u ? 0x3344u :
                    0xaabb3344u);
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT moffs_state_equal(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.eflags == after->data.eflags &&
        before->data.eip == after->data.eip;
}

static C_INT moffs_test_reject(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 prefixes[] = { 0x66u, 0x67u };
    static const type_unsigned_8 opcodes[] = { 0xa0u, 0xa1u, 0xa2u, 0xa3u };
    type_unsigned_8 profile;
    type_unsigned_8 prefix;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (prefix = 0u; prefix != sizeof(prefixes); ++prefix)
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
    {
        moffs_machine state;
        t_cpu before = {0};
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic = {0};
        type_status status = TYPE_STATUS_INVALID_STATE;
        type_unsigned_8 code[] = { prefixes[prefix], opcodes[opcode], 0u, 0x10u };
        C_INT failed = !moffs_prepare(profiles[profile], &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
            moffs_set_registers(&state);
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, code, sizeof(code), &after, &diagnostic,
                &status) || status != TYPE_STATUS_FAULT ||
                !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                !moffs_state_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            STD_PRINTF("MOFFS reject profile=%u prefix=%02x opcode=%02x status=%d fault=%08x\n",
                profiles[profile], prefixes[prefix], opcodes[opcode], status,
                diagnostic.first_fault.exception_mask);
            return 0;
        }
    }
    return 1;
}

static C_INT moffs_test_lock(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xa0u, 0xa1u, 0xa2u, 0xa3u };
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
    {
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 code[] = { 0xf0u, opcodes[opcode], 0u, 0x10u };
        C_INT failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
            moffs_set_registers(&state);
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, code, sizeof(code), &after, &diagnostic,
                &status) || status != TYPE_STATUS_FAULT ||
                !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                !moffs_state_equal(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT moffs_test_segment_overrides(C_VOID)
{
    static const type_unsigned_8 codes[][4] = {
        { 0xa0u, 0x10u, 0x00u, 0u },
        { 0x26u, 0xa0u, 0x10u, 0x00u },
        { 0x64u, 0xa0u, 0x10u, 0x00u },
        { 0x65u, 0xa0u, 0x10u, 0x00u }
    };
    static const type_unsigned_8 values[] = { 0x11u, 0x22u, 0x33u, 0x44u };
    static const type_unsigned_8 bytes[] = { 3u, 4u, 4u, 4u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(values); ++form)
    {
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_32 address = form == 0u ? 0x10u : (type_unsigned_32)form * 0x100u + 0x10u;
        C_INT failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
            if (form == 1u) failed |= core_machine_cpu_execution_load_segment(
                &state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.es, 0x10u) != 0;
            if (form == 2u) failed |= core_machine_cpu_execution_load_segment(
                &state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.fs, 0x20u) != 0;
            if (form == 3u) failed |= core_machine_cpu_execution_load_segment(
                &state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.gs, 0x30u) != 0;
            moffs_set_registers(&state);
            failed |= core_machine_memory_write(state.machine, address, &values[form], 1u) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, codes[form], bytes[form], &after, &diagnostic,
                &status) || status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != bytes[form] || after.data.eax != (0xaabb3300u | values[form]) ||
                !moffs_nonparticipants(&before, &after, 0xa0u);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT moffs_test_segment_writes(C_VOID)
{
    static const type_unsigned_8 codes[][4] = {
        { 0xa2u, 0x10u, 0x00u, 0u },
        { 0x26u, 0xa3u, 0x10u, 0x00u },
        { 0x64u, 0xa2u, 0x10u, 0x00u },
        { 0x65u, 0xa3u, 0x10u, 0x00u }
    };
    static const type_unsigned_8 bytes[] = { 3u, 4u, 4u, 4u };
    static const type_unsigned_8 widths[] = { 1u, 2u, 1u, 2u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(bytes); ++form)
    {
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_32 image = 0u;
        type_unsigned_32 address = form == 0u ? 0x10u : (type_unsigned_32)form * 0x100u + 0x10u;
        C_INT failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
            if (form == 1u) failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.es, 0x10u) != 0;
            if (form == 2u) failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.fs, 0x20u) != 0;
            if (form == 3u) failed |= core_machine_cpu_execution_load_segment(&state.machine->executor_cpu_execution, &state.machine->executor_cpu.data.gs, 0x30u) != 0;
            moffs_set_registers(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !moffs_run(&state, codes[form], bytes[form], &after, &diagnostic, &status) || status != TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eip != bytes[form] || !moffs_nonparticipants(&before, &after, 0xa2u) || core_machine_memory_read_physical(&state.machine->executor_memory, address, (type_virtual_address)&image, widths[form]) != TYPE_STATUS_OK || image != (widths[form] == 1u ? 0x44u : 0x3344u);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT moffs_test_protected_read_limit(C_VOID)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const type_unsigned_8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0x0fu, 0, 0, 0x30u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x40u, 0, 0x92u, 0, 0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0xb8u, 0x18u, 0x00u, 0x8eu, 0xd0u,
        0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    static const type_unsigned_8 read_code[] = { 0xa0u, 0x10u, 0x00u };
    static const type_unsigned_8 write_code[] = { 0x66u, 0xa3u, 0x10u, 0x00u };
    const type_unsigned_8 *codes[] = { read_code, write_code };
    const type_unsigned_8 bytes[] = { sizeof(read_code), sizeof(write_code) };
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        moffs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_32 image = 0x11223344u;
        C_INT failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
        {
            failed |= core_machine_memory_write(state.machine, 0x0100u,
                gdt_pointer, sizeof(gdt_pointer)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0300u, gdt,
                    sizeof(gdt)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, bootstrap,
                    sizeof(bootstrap)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, halt,
                    sizeof(halt)) != TYPE_STATUS_OK ||
                core_machine_run(state.machine, (core_machine_run_budget){ 96u, 0u },
                    &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        }
        if (!failed)
        {
            moffs_set_registers(&state);
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &image,
                sizeof(image)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, codes[form],
                    bytes[form]) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_FAULT ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi || after.data.ebp != before.data.ebp ||
                after.data.esp != before.data.esp ||
                after.data.eflags != before.data.eflags ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x3010u, (type_virtual_address)&image, sizeof(image)) !=
                    TYPE_STATUS_OK || image != 0x11223344u;
        }
        core_machine_destroy(state.machine);
        if (failed)
        {
            STD_PRINTF("MOFFS protected-limit form=%u\n", form);
            return 0;
        }
    }
    return 1;
}

static C_INT moffs_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 codes[][4] = {
        { 0xa0u, 0x00u, 0x10u, 0x90u },
        { 0xa2u, 0x00u, 0x10u, 0x90u }
    };
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        moffs_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_16 vector_offset = 0x0100u;
        type_unsigned_16 vector_segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_8 image = form == 0u ? 0x5au : 0u;
        C_INT failed = !moffs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
        {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u) ||
                core_machine_memory_write(state.machine, 0x1000u, &image,
                    sizeof(image)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, codes[form],
                    sizeof(codes[form])) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u,
                    &vector_offset, sizeof(vector_offset)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u + 2u,
                    &vector_segment, sizeof(vector_segment)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0100u, &hlt,
                    sizeof(hlt)) != TYPE_STATUS_OK;
        }
        if (!failed)
        {
            moffs_set_registers(&state);
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    after.data.ss.base + (type_unsigned_16)after.data.esp,
                    (type_virtual_address)&frame_ip, sizeof(frame_ip)) !=
                    TYPE_STATUS_OK || after.data.eip != 0x0101u || frame_ip != 3u ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                    state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                (form == 0u && after.data.eax != 0xaabb335au) ||
                (form == 1u && (core_machine_memory_read_physical(
                    &state.machine->executor_memory, 0x1000u,
                    (type_virtual_address)&image, sizeof(image)) != TYPE_STATUS_OK ||
                    image != 0x44u));
        }
        core_machine_destroy(state.machine);
        if (failed)
        {
            STD_PRINTF("MOFFS irq form=%u\n", form);
            return 0;
        }
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!moffs_test_default())
    {
        STD_PRINTF("MOFFS stage=default\n");
        return 1;
    }
    if (!moffs_test_386_attributes() ||
        !moffs_test_386_single_attributes())
    {
        STD_PRINTF("MOFFS stage=attributes\n");
        return 1;
    }
    if (!moffs_test_reject())
    {
        STD_PRINTF("MOFFS stage=reject\n");
        return 1;
    }
    if (!moffs_test_lock())
    {
        STD_PRINTF("MOFFS stage=lock\n");
        return 1;
    }
    if (!moffs_test_segment_overrides())
    {
        STD_PRINTF("MOFFS stage=segment\n");
        return 1;
    }
    if (!moffs_test_segment_writes())
    {
        STD_PRINTF("MOFFS stage=segment-write\n");
        return 1;
    }
    if (!moffs_test_protected_read_limit())
    {
        STD_PRINTF("MOFFS stage=protected-limit\n");
        return 1;
    }
    if (!moffs_test_irq_no_shadow())
    {
        STD_PRINTF("MOFFS stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S30:MOFFS:OK\n");
    STD_PRINTF("M5:T401:S14:MOFFS-MOV-PROFILES:OK\n");
    return 0;
}
