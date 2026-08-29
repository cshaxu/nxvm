#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define PREFIX_ATTRIBUTES_S64_CMP_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | \
    VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

typedef struct prefix_attributes_s64_machine {
    core_machine *machine;
} prefix_attributes_s64_machine;

static C_VOID prefix_attributes_s64_reset(C_VOID *opaque)
{
    prefix_attributes_s64_machine *state =
        (prefix_attributes_s64_machine *)opaque;

    if (state != STD_NULL) {
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider prefix_attributes_s64_provider = {
    prefix_attributes_s64_reset,
    STD_NULL
};

static C_INT prefix_attributes_s64_prepare(core_machine_cpu_profile profile,
    prefix_attributes_s64_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) {
        return 0;
    }
    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &prefix_attributes_s64_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(
                state->machine, 0u);
}

static C_INT prefix_attributes_s64_run(prefix_attributes_s64_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T code_size, type_unsigned_32 instructions,
    t_cpu *out_cpu, core_machine_cpu_diagnostic *out_diagnostic,
    type_status *out_status)
{
    core_machine_run_result result;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        out_cpu == STD_NULL || out_diagnostic == STD_NULL ||
        out_status == STD_NULL || core_machine_memory_write(state->machine,
            state->machine->executor_cpu.data.cs.base +
            state->machine->executor_cpu.data.eip, code, code_size) !=
            TYPE_STATUS_OK) {
        return 0;
    }
    *out_status = core_machine_run(state->machine,
        (core_machine_run_budget){ instructions, 0u }, &result);
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT prefix_attributes_s64_sregs_same(const t_cpu *before,
    const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
        sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
        sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
        sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
        sizeof(before->data.gs)) == 0;
}

static C_INT prefix_attributes_s64_cpu_same(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ebx == after->data.ebx &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eip == after->data.eip &&
        before->data.eflags == after->data.eflags &&
        prefix_attributes_s64_sregs_same(before, after);
}

static C_INT prefix_attributes_s64_gprs_same_except_eax(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.ebx == after->data.ebx &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT prefix_attributes_s64_gprs_same(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ebx == after->data.ebx &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT prefix_attributes_s64_gprs_same_except_ecx_edi(
    const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ebx == after->data.ebx &&
        before->data.edx == after->data.edx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi;
}

static C_INT prefix_attributes_s64_test_segments(C_VOID)
{
    static const type_unsigned_8 prefixes[] = {
        0x26u, 0x2eu, 0x36u, 0x3eu, 0x64u, 0x65u
    };
    const type_unsigned_32 bases[] = {
        0x10000u, 0x11000u, 0x12000u, 0x13000u, 0x14000u, 0x15000u
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(prefixes); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_8 source = (type_unsigned_8)(0x40u + form);
        const type_unsigned_8 code[] = {
            prefixes[form], 0x8au, 0x06u, 0x00u, 0x01u
        };
        C_INT failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.es.base = bases[0u];
            state.machine->executor_cpu.data.cs.base = bases[1u];
            state.machine->executor_cpu.data.ss.base = bases[2u];
            state.machine->executor_cpu.data.ds.base = bases[3u];
            state.machine->executor_cpu.data.fs.base = bases[4u];
            state.machine->executor_cpu.data.gs.base = bases[5u];
            state.machine->executor_cpu.data.fs.flagValid = TYPE_TRUE;
            state.machine->executor_cpu.data.gs.flagValid = TYPE_TRUE;
            failed |= core_machine_memory_write(state.machine,
                bases[form] + 0x100u, &source, sizeof(source)) !=
                TYPE_STATUS_OK;
            state.machine->executor_cpu.data.eax = 0xaabbcc00u;
            before = state.machine->executor_cpu;
            failed |= !prefix_attributes_s64_run(&state, code, sizeof(code), 1u,
                &after, &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
                after.data.eax != (0xaabbcc00u | source) ||
                after.data.eflags != before.data.eflags ||
                !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
                !prefix_attributes_s64_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT prefix_attributes_s64_test_last_wins(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x2eu, 0x36u, 0x8au, 0x06u, 0x00u, 0x01u };
    prefix_attributes_s64_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_status status;
    type_unsigned_8 cs_source = 0x11u;
    type_unsigned_8 ss_source = 0x22u;
    C_INT failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.cs.base = 0x11000u;
        state.machine->executor_cpu.data.ss.base = 0x12000u;
        failed |= core_machine_memory_write(state.machine, 0x11100u,
            &cs_source, sizeof(cs_source)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x12100u, &ss_source,
                sizeof(ss_source)) != TYPE_STATUS_OK ||
            !prefix_attributes_s64_run(&state, code, sizeof(code), 1u, &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
            after.data.al != ss_source;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT prefix_attributes_s64_test_attributes_and_lock(C_VOID)
{
    static const type_unsigned_8 read_operand32[] = {
        0x66u, 0x8bu, 0x06u, 0x00u, 0x01u
    };
    static const type_unsigned_8 write_operand32[] = {
        0x66u, 0x89u, 0x06u, 0x00u, 0x01u
    };
    static const type_unsigned_8 read_address32[] = { 0x67u, 0x8au, 0x06u };
    static const type_unsigned_8 write_address32[] = { 0x67u, 0x88u, 0x06u };
    static const type_unsigned_8 read32[] = { 0x66u, 0x67u, 0x8bu, 0x06u };
    static const type_unsigned_8 write32[] = { 0x66u, 0x67u, 0x89u, 0x06u };
    static const type_unsigned_8 lock_add[] = { 0xf0u, 0x01u, 0x06u, 0x00u, 0x01u };
    static const type_unsigned_8 lock_read[] = { 0xf0u, 0x8bu, 0x06u, 0x00u, 0x01u };
    prefix_attributes_s64_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    type_unsigned_32 image = 0x11223344u;
    type_unsigned_32 other_image = 0u;
    type_unsigned_8 byte_image = 0u;
    type_unsigned_8 other_byte = 0u;
    type_unsigned_8 observed_byte = 0u;
    C_INT failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = 0x0100u;
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, 0x0100u, &image,
            sizeof(image)) != TYPE_STATUS_OK || !prefix_attributes_s64_run(&state,
            read32, sizeof(read32), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(read32) || after.data.eax != image ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            !prefix_attributes_s64_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        image = 0x55667788u;
        state.machine->executor_cpu.data.eax = 0xaabbccdd;
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, 0x0100u, &image,
            sizeof(image)) != TYPE_STATUS_OK ||
            !prefix_attributes_s64_run(&state, read_operand32,
                sizeof(read_operand32), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(read_operand32) ||
            after.data.eax != image ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            !prefix_attributes_s64_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        image = 0x11223344u;
        state.machine->executor_cpu.data.eax = image;
        before = state.machine->executor_cpu;
        failed |= !prefix_attributes_s64_run(&state, write_operand32,
            sizeof(write_operand32), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(write_operand32) ||
            after.data.eax != before.data.eax ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after) ||
            core_machine_memory_read(state.machine, 0x0100u, &other_image,
                sizeof(other_image)) != TYPE_STATUS_OK || other_image != image;
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        byte_image = 0x6du;
        other_byte = 0x2bu;
        state.machine->executor_cpu.data.eax = 0xaabbcc00u;
        state.machine->executor_cpu.data.esi = 0x00010100u;
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, 0x00010100u,
            &byte_image, sizeof(byte_image)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0100u, &other_byte,
                sizeof(other_byte)) != TYPE_STATUS_OK ||
            !prefix_attributes_s64_run(&state, read_address32,
                sizeof(read_address32), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(read_address32) ||
            after.data.eax != 0xaabbcc6du ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            !prefix_attributes_s64_sregs_same(&before, &after) ||
            core_machine_memory_read(state.machine, 0x00010100u,
                &observed_byte,
                sizeof(byte_image)) != TYPE_STATUS_OK ||
            observed_byte != byte_image ||
            core_machine_memory_read(state.machine, 0x0100u, &observed_byte,
                sizeof(other_byte)) != TYPE_STATUS_OK ||
            observed_byte != other_byte;
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        byte_image = 0x22u;
        other_byte = 0x33u;
        state.machine->executor_cpu.data.eax = 0xaabbcc6du;
        state.machine->executor_cpu.data.esi = 0x00010100u;
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, 0x00010100u,
            &byte_image, sizeof(byte_image)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0100u, &other_byte,
                sizeof(other_byte)) != TYPE_STATUS_OK ||
            !prefix_attributes_s64_run(&state, write_address32,
                sizeof(write_address32), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(write_address32) ||
            after.data.eax != before.data.eax ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after) ||
            core_machine_memory_read(state.machine, 0x00010100u, &byte_image,
                sizeof(byte_image)) != TYPE_STATUS_OK || byte_image != 0x6du ||
            core_machine_memory_read(state.machine, 0x0100u, &other_byte,
                sizeof(other_byte)) != TYPE_STATUS_OK || other_byte != 0x33u;
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        state.machine->executor_cpu.data.esi = 0x0100u;
        image = 0x11223344u;
        state.machine->executor_cpu.data.eax = image;
        before = state.machine->executor_cpu;
        failed |= !prefix_attributes_s64_run(&state, write32, sizeof(write32),
            1u, &after, &diagnostic, &status) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(write32) ||
            after.data.eax != before.data.eax ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after) ||
            core_machine_memory_read(state.machine, 0x0100u, &image,
                sizeof(image)) != TYPE_STATUS_OK || image != 0x11223344u;
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        image = 1u;
        state.machine->executor_cpu.data.eax = 2u;
        failed |= core_machine_memory_write(state.machine, 0x0100u, &image,
            sizeof(image)) != TYPE_STATUS_OK || !prefix_attributes_s64_run(&state,
            lock_add, sizeof(lock_add), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(lock_add) ||
            core_machine_memory_read(state.machine, 0x0100u, &image,
                sizeof(image)) != TYPE_STATUS_OK || image != 3u;
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        image = 0x55667788u;
        failed |= core_machine_memory_write(state.machine, 0x0100u, &image,
            sizeof(image)) != TYPE_STATUS_OK;
        before = state.machine->executor_cpu;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine) || !prefix_attributes_s64_run(&state, lock_read,
            sizeof(lock_read), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD) || !prefix_attributes_s64_cpu_same(&before,
                    &after) || core_machine_memory_read_physical(
                        &state.machine->executor_memory, 0x0100u,
                        (type_virtual_address)&image, sizeof(image)) !=
                TYPE_STATUS_OK || image != 0x55667788u;
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    {
        static const core_machine_cpu_profile profiles[] = {
            CORE_MACHINE_CPU_PROFILE_8086,
            CORE_MACHINE_CPU_PROFILE_80186,
            CORE_MACHINE_CPU_PROFILE_80286
        };
        static const type_unsigned_8 forms[][6] = {
            { 0x66u, 0x8bu, 0x06u, 0x00u, 0x01u, 0u },
            { 0x67u, 0x8bu, 0x06u, 0x00u, 0x01u, 0u },
            { 0x66u, 0x67u, 0x8bu, 0x06u, 0u, 0u }
        };
        static const STD_SIZE_T lengths[] = { 5u, 5u, 4u };
        type_unsigned_8 profile;
        type_unsigned_8 form;

        for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
            for (form = 0u; form != sizeof(forms) / sizeof(forms[0]);
                ++form) {
                failed = !prefix_attributes_s64_prepare(profiles[profile],
                    &state);
                if (!failed) {
                    before = state.machine->executor_cpu;
                    failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                        state.machine) || !prefix_attributes_s64_run(&state, forms[form],
                        lengths[form], 1u, &after, &diagnostic, &status) ||
                        status != TYPE_STATUS_FAULT ||
                        !diagnostic.first_fault.valid ||
                        !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                            VCPUINS_EXCEPT_UD) ||
                        !prefix_attributes_s64_cpu_same(&before, &after);
                }
                core_machine_destroy(state.machine);
                if (failed) {
                    return 0;
                }
            }
        }
    }
    return !failed;
}

static C_INT prefix_attributes_s64_test_lock_group_legality(C_VOID)
{
    static const type_unsigned_8 forms[][7] = {
        { 0xf0u, 0x0fu, 0xa3u, 0x06u, 0x00u, 0x01u, 0u },
        { 0xf0u, 0x0fu, 0xbau, 0x26u, 0x00u, 0x01u, 0u },
        { 0xf0u, 0xf6u, 0x06u, 0x00u, 0x01u, 0x01u, 0u },
        { 0xf0u, 0xf7u, 0x06u, 0x00u, 0x01u, 0x01u, 0u },
        { 0xf0u, 0xfeu, 0x16u, 0x00u, 0x01u, 0u, 0u },
        { 0xf0u, 0xffu, 0x16u, 0x00u, 0x01u, 0u, 0u }
    };
    static const STD_SIZE_T lengths[] = { 6u, 7u, 6u, 7u, 5u, 5u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_16 image = 0x1234u;
        C_INT failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            before = state.machine->executor_cpu;
            failed |= core_machine_memory_write(state.machine, 0x0100u,
                &image, sizeof(image)) != TYPE_STATUS_OK ||
                !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine) || !prefix_attributes_s64_run(&state,
                    forms[form], lengths[form], 1u, &after, &diagnostic,
                    &status) || status != TYPE_STATUS_FAULT ||
                !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                !prefix_attributes_s64_cpu_same(&before, &after) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x0100u, (type_virtual_address)&image, sizeof(image)) !=
                    TYPE_STATUS_OK || image != 0x1234u;
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT prefix_attributes_s64_test_lock_group_writes(C_VOID)
{
    static const type_unsigned_8 forms[][5] = {
        { 0xf0u, 0xf6u, 0x16u, 0x00u, 0x01u },
        { 0xf0u, 0xf7u, 0x1eu, 0x00u, 0x01u },
        { 0xf0u, 0xffu, 0x06u, 0x00u, 0x01u }
    };
    static const type_unsigned_16 expected[] = {
        0x12cbu, 0xedccu, 0x1235u
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_16 image = 0x1234u;
        C_INT failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            before = state.machine->executor_cpu;
            failed |= core_machine_memory_write(state.machine, 0x0100u,
                &image, sizeof(image)) != TYPE_STATUS_OK ||
                !prefix_attributes_s64_run(&state, forms[form],
                    sizeof(forms[form]), 1u, &after, &diagnostic, &status) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(forms[form]) ||
                !prefix_attributes_s64_gprs_same(&before, &after) ||
                !prefix_attributes_s64_sregs_same(&before, &after) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x0100u, (type_virtual_address)&image, sizeof(image)) !=
                    TYPE_STATUS_OK || image != expected[form];
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT prefix_attributes_s64_test_repeated_width_prefixes(C_VOID)
{
    static const type_unsigned_8 operand_code[] = {
        0x66u, 0x66u, 0xb8u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const type_unsigned_8 address_code[] = {
        0x67u, 0x67u, 0x8au, 0x06u
    };
    prefix_attributes_s64_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    type_unsigned_8 selected = 0x5au;
    type_unsigned_8 unselected = 0x3cu;
    C_INT failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        before = state.machine->executor_cpu;
        failed |= !prefix_attributes_s64_run(&state, operand_code,
            sizeof(operand_code), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(operand_code) ||
            after.data.eax != 0x12345678u ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabbcc00u;
        state.machine->executor_cpu.data.esi = 0x00010100u;
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, 0x00010100u,
            &selected, sizeof(selected)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0100u, &unselected,
                sizeof(unselected)) != TYPE_STATUS_OK ||
            !prefix_attributes_s64_run(&state, address_code,
                sizeof(address_code), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(address_code) ||
            after.data.eax != 0xaabbcc5au ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT prefix_attributes_s64_test_fixed_segment_and_register(C_VOID)
{
    static const type_unsigned_8 fixed_segment[] = { 0x26u, 0xa4u };
    static const type_unsigned_8 register_only[] = {
        0x66u, 0x67u, 0xb8u, 0x44u, 0x33u, 0x22u, 0x11u
    };
    prefix_attributes_s64_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_status status;
    type_unsigned_8 source = 0x4du;
    type_unsigned_8 target = 0u;
    C_INT failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.ds.base = 0x10000u;
        state.machine->executor_cpu.data.es.base = 0x11000u;
        state.machine->executor_cpu.data.esi = 0x0100u;
        state.machine->executor_cpu.data.edi = 0x0200u;
        failed |= core_machine_memory_write(state.machine, 0x11100u, &source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x11200u, &target, sizeof(target)) !=
            TYPE_STATUS_OK || !prefix_attributes_s64_run(&state, fixed_segment,
                sizeof(fixed_segment), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(fixed_segment) || after.data.esi != 0x0101u ||
            after.data.edi != 0x0201u || core_machine_memory_read(state.machine,
                0x11200u, &target, sizeof(target)) != TYPE_STATUS_OK ||
            target != source;
    }
    core_machine_destroy(state.machine);
    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        before = state.machine->executor_cpu;
        failed |= !prefix_attributes_s64_run(&state, register_only,
            sizeof(register_only), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(register_only) ||
            after.data.eax != 0x11223344u ||
            after.data.ebx != before.data.ebx ||
            after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx ||
            after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT prefix_attributes_s64_test_rep_movs(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xf3u, 0xa4u, 0xf4u };
    prefix_attributes_s64_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_status status;
    type_unsigned_8 source[] = { 0x31u, 0x42u, 0x53u };
    type_unsigned_8 target[] = { 0u, 0u, 0u };
    C_INT failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.ds.base = 0x10000u;
        state.machine->executor_cpu.data.es.base = 0x11000u;
        state.machine->executor_cpu.data.esi = 0x0100u;
        state.machine->executor_cpu.data.edi = 0x0200u;
        state.machine->executor_cpu.data.ecx = 0x11220003u;
        failed |= core_machine_memory_write(state.machine, 0x10100u, source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x11200u, target, sizeof(target)) !=
                TYPE_STATUS_OK || !prefix_attributes_s64_run(&state, code,
                    sizeof(code), 4u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(code) || after.data.esi != 0x0103u ||
            after.data.edi != 0x0203u || after.data.ecx != 0x11220000u ||
            core_machine_memory_read(state.machine, 0x11200u, target,
                sizeof(target)) != TYPE_STATUS_OK ||
            STD_MEMCMP(source, target, sizeof(source)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT prefix_attributes_s64_test_rep_edges(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xf3u, 0xa4u, 0xf4u };
    const type_unsigned_32 counts[] = { 0u, 1u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(counts) / sizeof(counts[0]); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu after;
        type_status status;
        type_unsigned_8 source = 0x5au;
        type_unsigned_8 target = 0xc3u;
        C_INT failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.esi = 0x0100u;
            state.machine->executor_cpu.data.edi = 0x0200u;
            state.machine->executor_cpu.data.ecx = 0x33440000u | counts[form];
            failed |= core_machine_memory_write(state.machine, 0x0100u,
                &source, sizeof(source)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0200u, &target,
                    sizeof(target)) != TYPE_STATUS_OK ||
                !prefix_attributes_s64_run(&state, code, sizeof(code),
                    counts[form] + 2u, &after, &diagnostic, &status) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(code) ||
                after.data.ecx != 0x33440000u ||
                after.data.esi != 0x0100u + counts[form] ||
                after.data.edi != 0x0200u + counts[form] ||
                core_machine_memory_read(state.machine, 0x0200u, &target,
                    sizeof(target)) != TYPE_STATUS_OK ||
                target != (counts[form] ? source : 0xc3u);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT prefix_attributes_s64_test_repne_movs(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xf2u, 0xa4u };
    prefix_attributes_s64_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_status status;
    type_unsigned_8 source = 0x7eu;
    type_unsigned_8 target = 0u;
    C_INT failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = 0x0100u;
        state.machine->executor_cpu.data.edi = 0x0200u;
        state.machine->executor_cpu.data.ecx = 0x55660001u;
        failed |= core_machine_memory_write(state.machine, 0x0100u, &source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x0200u, &target, sizeof(target)) !=
            TYPE_STATUS_OK || !prefix_attributes_s64_run(&state, code,
                sizeof(code), 1u, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(code) || after.data.esi != 0x0101u ||
            after.data.edi != 0x0201u || after.data.ecx != 0x55660000u ||
            core_machine_memory_read(state.machine, 0x0200u, &target,
                sizeof(target)) != TYPE_STATUS_OK || target != source;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT prefix_attributes_s64_test_mixed_repeat_last_wins(C_VOID)
{
    static const type_unsigned_8 forms[][3] = {
        { 0xf2u, 0xf3u, 0xaeu },
        { 0xf3u, 0xf2u, 0xaeu }
    };
    static const type_unsigned_32 budgets[] = { 2u, 1u };
    static const type_unsigned_16 final_cx[] = { 0u, 1u };
    static const type_unsigned_16 final_di[] = { 0x0202u, 0x0201u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_8 image[] = { 0x3cu, 0x3cu };
        type_unsigned_8 observed[] = { 0u, 0u };
        C_INT failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabbcc3cu;
            state.machine->executor_cpu.data.edi = 0x77880200u;
            state.machine->executor_cpu.data.ecx = 0x55660002u;
            before = state.machine->executor_cpu;
            failed |= core_machine_memory_write(state.machine, 0x0200u,
                image, sizeof(image)) != TYPE_STATUS_OK ||
                !prefix_attributes_s64_run(&state, forms[form],
                    sizeof(forms[form]), budgets[form], &after, &diagnostic,
                    &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid ||
                after.data.eip != sizeof(forms[form]) ||
                after.data.ecx != (0x55660000u | final_cx[form]) ||
                after.data.edi != (0x77880000u | final_di[form]) ||
                !prefix_attributes_s64_gprs_same_except_ecx_edi(&before,
                    &after) ||
                (after.data.eflags & ~PREFIX_ATTRIBUTES_S64_CMP_FLAGS) !=
                    (before.data.eflags & ~PREFIX_ATTRIBUTES_S64_CMP_FLAGS) ||
                (after.data.eflags & PREFIX_ATTRIBUTES_S64_CMP_FLAGS) !=
                    (VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
                !prefix_attributes_s64_sregs_same(&before, &after) ||
                core_machine_memory_read(state.machine, 0x0200u, observed,
                    sizeof(observed)) != TYPE_STATUS_OK ||
                STD_MEMCMP(image, observed, sizeof(image)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT prefix_attributes_s64_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x3eu, 0x8au, 0x06u, 0x00u, 0x10u, 0x90u
    };
    static const type_unsigned_8 hlt[] = { 0xf4u };
    prefix_attributes_s64_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_16 vector_offset = 0x0100u;
    type_unsigned_16 vector_segment = 0u;
    type_unsigned_16 frame_ip = 0u;
    type_unsigned_8 image = 0x6du;
    C_INT failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x1000u, &image,
            sizeof(image)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x0080u, &vector_offset, sizeof(vector_offset)) !=
                TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                    0x0082u, &vector_segment, sizeof(vector_segment)) !=
                TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                    0x0100u, hlt, sizeof(hlt)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, code,
                    sizeof(code)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabbcc00u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            (type_virtual_address)&frame_ip, sizeof(frame_ip)) != TYPE_STATUS_OK ||
            after.data.eip != 0x0101u || frame_ip != 5u ||
            after.data.al != image || !TYPE_GET_BIT(
                state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!prefix_attributes_s64_test_segments()) {
        STD_FPRINTF(STD_STDERR, "S64 prefix segment grid failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_last_wins()) {
        STD_FPRINTF(STD_STDERR, "S64 prefix last-wins failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_attributes_and_lock()) {
        STD_FPRINTF(STD_STDERR, "S64 attribute/LOCK failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_lock_group_legality()) {
        STD_FPRINTF(STD_STDERR, "S64 LOCK group legality failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_lock_group_writes()) {
        STD_FPRINTF(STD_STDERR, "S64 LOCK group writes failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_repeated_width_prefixes()) {
        STD_FPRINTF(STD_STDERR, "S64 repeated width-prefix failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_fixed_segment_and_register()) {
        STD_FPRINTF(STD_STDERR, "S64 fixed-segment/register prefix failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_rep_movs()) {
        STD_FPRINTF(STD_STDERR, "S64 REP MOVS failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_rep_edges()) {
        STD_FPRINTF(STD_STDERR, "S64 REP edges failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_repne_movs()) {
        STD_FPRINTF(STD_STDERR, "S64 REPNE MOVS failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_mixed_repeat_last_wins()) {
        STD_FPRINTF(STD_STDERR, "S64 mixed repeat-prefix failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_irq_no_shadow()) {
        STD_FPRINTF(STD_STDERR, "S64 prefix IRQ failed\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S64:PREFIX-ATTRIBUTES:OK\n");
    STD_PRINTF("M5:T401:S57:SHARED-PREFIX-PROFILES:OK\n");
    return 0;
}
