#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* T337_REAL_UD_TERMINAL_CPU_OWNER: invalid prefixes stop at the CPU owner. */
#define PREFIX_ATTRIBUTES_S64_CMP_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | \
    VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

typedef struct prefix_attributes_s64_machine {
    cpu_instruction_fixture chip;
} prefix_attributes_s64_machine;

static lib_i32 prefix_attributes_s64_prepare(core_machine_cpu_profile profile,
    prefix_attributes_s64_machine *state)
{
    if (state == LIB_NULL) return 0;
    cpu_instruction_prepare(&state->chip, profile);
    return 1;
}

static lib_status prefix_attributes_s64_write(prefix_attributes_s64_machine *state,
    lib_u32 address, const void *bytes, lib_size byte_count)
{
    if (byte_count > 255u) return LIB_STATUS_INVALID_ARGUMENT;
    return cpu_instruction_write(&state->chip, address, bytes, (lib_u8)byte_count,
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
}

static lib_status prefix_attributes_s64_read(prefix_attributes_s64_machine *state,
    lib_u32 address, void *bytes, lib_size byte_count)
{
    if (byte_count > 255u) return LIB_STATUS_INVALID_ARGUMENT;
    return cpu_instruction_read(&state->chip, address, bytes, (lib_u8)byte_count,
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE);
}

static lib_i32 prefix_attributes_s64_preflight_ud(prefix_attributes_s64_machine *state)
{
    state->chip.cpu.data.idtr.limit = 0x17u;
    return 1;
}

static lib_i32 prefix_attributes_s64_run(prefix_attributes_s64_machine *state,
    const lib_u8 *code, lib_size code_size, lib_u32 instructions,
    t_cpu *out_cpu, core_machine_cpu_fault_snapshot *out_diagnostic,
    lib_status *out_status)
{
    lib_u32 step;
    lib_u32 address;

    if (state == LIB_NULL || code == LIB_NULL || out_cpu == LIB_NULL ||
        out_diagnostic == LIB_NULL || out_status == LIB_NULL ||
        code_size > 255u) return 0;
    address = state->chip.cpu.data.cs.base + state->chip.cpu.data.eip;
    if (prefix_attributes_s64_write(state, address, code, code_size) !=
        LIB_STATUS_OK) return 0;
    for (step = 0u; step != instructions; ++step) {
        if (state->chip.cpu.data.flagHalt || state->chip.execution.stop_requested)
            break;
        core_machine_cpu_execution_refresh(&state->chip.execution);
    }
    *out_cpu = state->chip.cpu;
    *out_diagnostic = state->chip.fault;
    *out_status = state->chip.execution.stop_requested ?
        LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
    return 1;
}

static lib_i32 prefix_attributes_s64_sregs_same(const t_cpu *before,
    const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs,
        sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
        sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
        sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
        sizeof(before->data.gs)) == 0;
}

static lib_i32 prefix_attributes_s64_cpu_same(const t_cpu *before,
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

static lib_i32 prefix_attributes_s64_gprs_same_except_eax(const t_cpu *before,
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

static lib_i32 prefix_attributes_s64_gprs_same(const t_cpu *before,
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

static lib_i32 prefix_attributes_s64_gprs_same_except_ecx_edi(
    const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ebx == after->data.ebx &&
        before->data.edx == after->data.edx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi;
}

static lib_i32 prefix_attributes_s64_test_segments(void)
{
    static const lib_u8 prefixes[] = {
        0x26u, 0x2eu, 0x36u, 0x3eu, 0x64u, 0x65u
    };
    const lib_u32 bases[] = {
        0x10000u, 0x11000u, 0x12000u, 0x13000u, 0x14000u, 0x15000u
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(prefixes); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_fault_snapshot diagnostic;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u8 source = (lib_u8)(0x40u + form);
        const lib_u8 code[] = {
            prefixes[form], 0x8au, 0x06u, 0x00u, 0x01u
        };
        lib_i32 failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.chip.cpu.data.es.base = bases[0u];
            state.chip.cpu.data.cs.base = bases[1u];
            state.chip.cpu.data.ss.base = bases[2u];
            state.chip.cpu.data.ds.base = bases[3u];
            state.chip.cpu.data.fs.base = bases[4u];
            state.chip.cpu.data.gs.base = bases[5u];
            state.chip.cpu.data.fs.flagValid = LIB_TRUE;
            state.chip.cpu.data.gs.flagValid = LIB_TRUE;
            failed |= prefix_attributes_s64_write(&state, bases[form] + 0x100u, &source, sizeof(source)) !=
                LIB_STATUS_OK;
            state.chip.cpu.data.eax = 0xaabbcc00u;
            before = state.chip.cpu;
            failed |= !prefix_attributes_s64_run(&state, code, sizeof(code), 1u,
                &after, &diagnostic, &status) || status != LIB_STATUS_OK ||
                diagnostic.valid || after.data.eip != sizeof(code) ||
                after.data.eax != (0xaabbcc00u | source) ||
                after.data.eflags != before.data.eflags ||
                !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
                !prefix_attributes_s64_sregs_same(&before, &after);
        }

        if (failed) {
            return 0;
        }
    }
    return 1;
}

static lib_i32 prefix_attributes_s64_test_last_wins(void)
{
    static const lib_u8 code[] = { 0x2eu, 0x36u, 0x8au, 0x06u, 0x00u, 0x01u };
    prefix_attributes_s64_machine state;
    core_machine_cpu_fault_snapshot diagnostic;
    t_cpu after;
    lib_status status;
    lib_u8 cs_source = 0x11u;
    lib_u8 ss_source = 0x22u;
    lib_i32 failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.chip.cpu.data.cs.base = 0x11000u;
        state.chip.cpu.data.ss.base = 0x12000u;
        failed |= prefix_attributes_s64_write(&state, 0x11100u,
            &cs_source, sizeof(cs_source)) != LIB_STATUS_OK ||
            prefix_attributes_s64_write(&state, 0x12100u, &ss_source,
                sizeof(ss_source)) != LIB_STATUS_OK ||
            !prefix_attributes_s64_run(&state, code, sizeof(code), 1u, &after,
                &diagnostic, &status) || status != LIB_STATUS_OK ||
            diagnostic.valid || after.data.eip != sizeof(code) ||
            after.data.al != ss_source;
    }

    return !failed;
}

static lib_i32 prefix_attributes_s64_test_attributes_and_lock(void)
{
    static const lib_u8 read_operand32[] = {
        0x66u, 0x8bu, 0x06u, 0x00u, 0x01u
    };
    static const lib_u8 write_operand32[] = {
        0x66u, 0x89u, 0x06u, 0x00u, 0x01u
    };
    static const lib_u8 read_address32[] = { 0x67u, 0x8au, 0x06u };
    static const lib_u8 write_address32[] = { 0x67u, 0x88u, 0x06u };
    static const lib_u8 read32[] = { 0x66u, 0x67u, 0x8bu, 0x06u };
    static const lib_u8 write32[] = { 0x66u, 0x67u, 0x89u, 0x06u };
    static const lib_u8 lock_add[] = { 0xf0u, 0x01u, 0x06u, 0x00u, 0x01u };
    static const lib_u8 lock_read[] = { 0xf0u, 0x8bu, 0x06u, 0x00u, 0x01u };
    prefix_attributes_s64_machine state;
    core_machine_cpu_fault_snapshot diagnostic;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_u32 image = 0x11223344u;
    lib_u32 other_image = 0u;
    lib_u8 byte_image = 0u;
    lib_u8 other_byte = 0u;
    lib_u8 observed_byte = 0u;
    lib_i32 failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.chip.cpu.data.esi = 0x0100u;
        before = state.chip.cpu;
        failed |= prefix_attributes_s64_write(&state, 0x0100u, &image,
            sizeof(image)) != LIB_STATUS_OK || !prefix_attributes_s64_run(&state,
            read32, sizeof(read32), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(read32) || after.data.eax != image ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            !prefix_attributes_s64_sregs_same(&before, &after);
    }

    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        image = 0x55667788u;
        state.chip.cpu.data.eax = 0xaabbccdd;
        before = state.chip.cpu;
        failed |= prefix_attributes_s64_write(&state, 0x0100u, &image,
            sizeof(image)) != LIB_STATUS_OK ||
            !prefix_attributes_s64_run(&state, read_operand32,
                sizeof(read_operand32), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(read_operand32) ||
            after.data.eax != image ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            !prefix_attributes_s64_sregs_same(&before, &after);
    }

    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        image = 0x11223344u;
        state.chip.cpu.data.eax = image;
        before = state.chip.cpu;
        failed |= !prefix_attributes_s64_run(&state, write_operand32,
            sizeof(write_operand32), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(write_operand32) ||
            after.data.eax != before.data.eax ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after) ||
            prefix_attributes_s64_read(&state, 0x0100u, &other_image,
                sizeof(other_image)) != LIB_STATUS_OK || other_image != image;
    }

    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        byte_image = 0x6du;
        other_byte = 0x2bu;
        state.chip.cpu.data.eax = 0xaabbcc00u;
        state.chip.cpu.data.esi = 0x00010100u;
        before = state.chip.cpu;
        failed |= prefix_attributes_s64_write(&state, 0x00010100u,
            &byte_image, sizeof(byte_image)) != LIB_STATUS_OK ||
            prefix_attributes_s64_write(&state, 0x0100u, &other_byte,
                sizeof(other_byte)) != LIB_STATUS_OK ||
            !prefix_attributes_s64_run(&state, read_address32,
                sizeof(read_address32), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(read_address32) ||
            after.data.eax != 0xaabbcc6du ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            !prefix_attributes_s64_sregs_same(&before, &after) ||
            prefix_attributes_s64_read(&state, 0x00010100u,
                &observed_byte,
                sizeof(byte_image)) != LIB_STATUS_OK ||
            observed_byte != byte_image ||
            prefix_attributes_s64_read(&state, 0x0100u, &observed_byte,
                sizeof(other_byte)) != LIB_STATUS_OK ||
            observed_byte != other_byte;
    }

    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        byte_image = 0x22u;
        other_byte = 0x33u;
        state.chip.cpu.data.eax = 0xaabbcc6du;
        state.chip.cpu.data.esi = 0x00010100u;
        before = state.chip.cpu;
        failed |= prefix_attributes_s64_write(&state, 0x00010100u,
            &byte_image, sizeof(byte_image)) != LIB_STATUS_OK ||
            prefix_attributes_s64_write(&state, 0x0100u, &other_byte,
                sizeof(other_byte)) != LIB_STATUS_OK ||
            !prefix_attributes_s64_run(&state, write_address32,
                sizeof(write_address32), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(write_address32) ||
            after.data.eax != before.data.eax ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after) ||
            prefix_attributes_s64_read(&state, 0x00010100u, &byte_image,
                sizeof(byte_image)) != LIB_STATUS_OK || byte_image != 0x6du ||
            prefix_attributes_s64_read(&state, 0x0100u, &other_byte,
                sizeof(other_byte)) != LIB_STATUS_OK || other_byte != 0x33u;
    }

    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        state.chip.cpu.data.esi = 0x0100u;
        image = 0x11223344u;
        state.chip.cpu.data.eax = image;
        before = state.chip.cpu;
        failed |= !prefix_attributes_s64_run(&state, write32, sizeof(write32),
            1u, &after, &diagnostic, &status) || status != LIB_STATUS_OK ||
            diagnostic.valid || after.data.eip != sizeof(write32) ||
            after.data.eax != before.data.eax ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after) ||
            prefix_attributes_s64_read(&state, 0x0100u, &image,
                sizeof(image)) != LIB_STATUS_OK || image != 0x11223344u;
    }

    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        image = 1u;
        state.chip.cpu.data.eax = 2u;
        failed |= prefix_attributes_s64_write(&state, 0x0100u, &image,
            sizeof(image)) != LIB_STATUS_OK || !prefix_attributes_s64_run(&state,
            lock_add, sizeof(lock_add), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(lock_add) ||
            prefix_attributes_s64_read(&state, 0x0100u, &image,
                sizeof(image)) != LIB_STATUS_OK || image != 3u;
    }

    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        image = 0x55667788u;
        failed |= prefix_attributes_s64_write(&state, 0x0100u, &image,
            sizeof(image)) != LIB_STATUS_OK;
        before = state.chip.cpu;
        failed |= !prefix_attributes_s64_preflight_ud(&state) || !prefix_attributes_s64_run(&state, lock_read,
            sizeof(lock_read), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_INTERNAL_ERROR || !diagnostic.valid ||
            !X86_CPU_BIT_IS_SET(diagnostic.exception_mask,
                VCPUINS_EXCEPT_UD) || !prefix_attributes_s64_cpu_same(&before,
                    &after) || prefix_attributes_s64_read(&state, 0x0100u,
                        &image, sizeof(image)) !=
                LIB_STATUS_OK || image != 0x55667788u;
    }

    if (failed) {
        return 0;
    }

    {
        static const core_machine_cpu_profile profiles[] = {
            CORE_MACHINE_CPU_PROFILE_8086,
            CORE_MACHINE_CPU_PROFILE_80186,
            CORE_MACHINE_CPU_PROFILE_80286
        };
        static const lib_u8 forms[][6] = {
            { 0x66u, 0x8bu, 0x06u, 0x00u, 0x01u, 0u },
            { 0x67u, 0x8bu, 0x06u, 0x00u, 0x01u, 0u },
            { 0x66u, 0x67u, 0x8bu, 0x06u, 0u, 0u }
        };
        static const lib_size lengths[] = { 5u, 5u, 4u };
        lib_u8 profile;
        lib_u8 form;

        for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
            for (form = 0u; form != sizeof(forms) / sizeof(forms[0]);
                ++form) {
                failed = !prefix_attributes_s64_prepare(profiles[profile],
                    &state);
                if (!failed) {
                    before = state.chip.cpu;
                    failed |= !prefix_attributes_s64_preflight_ud(&state) || !prefix_attributes_s64_run(&state, forms[form],
                        lengths[form], 1u, &after, &diagnostic, &status) ||
                        status != LIB_STATUS_INTERNAL_ERROR ||
                        !diagnostic.valid ||
                        !X86_CPU_BIT_IS_SET(diagnostic.exception_mask,
                            VCPUINS_EXCEPT_UD) ||
                        !prefix_attributes_s64_cpu_same(&before, &after);
                }

                if (failed) {
                    return 0;
                }
            }
        }
    }
    return !failed;
}

static lib_i32 prefix_attributes_s64_test_lock_group_legality(void)
{
    static const lib_u8 forms[][7] = {
        { 0xf0u, 0x0fu, 0xa3u, 0x06u, 0x00u, 0x01u, 0u },
        { 0xf0u, 0x0fu, 0xbau, 0x26u, 0x00u, 0x01u, 0u },
        { 0xf0u, 0xf6u, 0x06u, 0x00u, 0x01u, 0x01u, 0u },
        { 0xf0u, 0xf7u, 0x06u, 0x00u, 0x01u, 0x01u, 0u },
        { 0xf0u, 0xfeu, 0x16u, 0x00u, 0x01u, 0u, 0u },
        { 0xf0u, 0xffu, 0x16u, 0x00u, 0x01u, 0u, 0u }
    };
    static const lib_size lengths[] = { 6u, 7u, 6u, 7u, 5u, 5u };
    lib_u8 form;

    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_fault_snapshot diagnostic;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u16 image = 0x1234u;
        lib_i32 failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            before = state.chip.cpu;
            failed |= prefix_attributes_s64_write(&state, 0x0100u,
                &image, sizeof(image)) != LIB_STATUS_OK ||
                !prefix_attributes_s64_preflight_ud(&state) || !prefix_attributes_s64_run(&state,
                    forms[form], lengths[form], 1u, &after, &diagnostic,
                    &status) || status != LIB_STATUS_INTERNAL_ERROR ||
                !diagnostic.valid || !X86_CPU_BIT_IS_SET(
                    diagnostic.exception_mask, VCPUINS_EXCEPT_UD) ||
                !prefix_attributes_s64_cpu_same(&before, &after) ||
                prefix_attributes_s64_read(&state, 0x0100u, &image, sizeof(image)) !=
                    LIB_STATUS_OK || image != 0x1234u;
        }

        if (failed) {
            return 0;
        }
    }
    return 1;
}

static lib_i32 prefix_attributes_s64_test_lock_group_writes(void)
{
    static const lib_u8 forms[][5] = {
        { 0xf0u, 0xf6u, 0x16u, 0x00u, 0x01u },
        { 0xf0u, 0xf7u, 0x1eu, 0x00u, 0x01u },
        { 0xf0u, 0xffu, 0x06u, 0x00u, 0x01u }
    };
    static const lib_u16 expected[] = {
        0x12cbu, 0xedccu, 0x1235u
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_fault_snapshot diagnostic;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u16 image = 0x1234u;
        lib_i32 failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            before = state.chip.cpu;
            failed |= prefix_attributes_s64_write(&state, 0x0100u,
                &image, sizeof(image)) != LIB_STATUS_OK ||
                !prefix_attributes_s64_run(&state, forms[form],
                    sizeof(forms[form]), 1u, &after, &diagnostic, &status) ||
                status != LIB_STATUS_OK || diagnostic.valid ||
                after.data.eip != sizeof(forms[form]) ||
                !prefix_attributes_s64_gprs_same(&before, &after) ||
                !prefix_attributes_s64_sregs_same(&before, &after) ||
                prefix_attributes_s64_read(&state, 0x0100u, &image, sizeof(image)) !=
                    LIB_STATUS_OK || image != expected[form];
        }

        if (failed) {
            return 0;
        }
    }
    return 1;
}

static lib_i32 prefix_attributes_s64_test_repeated_width_prefixes(void)
{
    static const lib_u8 operand_code[] = {
        0x66u, 0x66u, 0xb8u, 0x78u, 0x56u, 0x34u, 0x12u
    };
    static const lib_u8 address_code[] = {
        0x67u, 0x67u, 0x8au, 0x06u
    };
    prefix_attributes_s64_machine state;
    core_machine_cpu_fault_snapshot diagnostic;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_u8 selected = 0x5au;
    lib_u8 unselected = 0x3cu;
    lib_i32 failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        before = state.chip.cpu;
        failed |= !prefix_attributes_s64_run(&state, operand_code,
            sizeof(operand_code), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(operand_code) ||
            after.data.eax != 0x12345678u ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after);
    }

    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        state.chip.cpu.data.eax = 0xaabbcc00u;
        state.chip.cpu.data.esi = 0x00010100u;
        before = state.chip.cpu;
        failed |= prefix_attributes_s64_write(&state, 0x00010100u,
            &selected, sizeof(selected)) != LIB_STATUS_OK ||
            prefix_attributes_s64_write(&state, 0x0100u, &unselected,
                sizeof(unselected)) != LIB_STATUS_OK ||
            !prefix_attributes_s64_run(&state, address_code,
                sizeof(address_code), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(address_code) ||
            after.data.eax != 0xaabbcc5au ||
            !prefix_attributes_s64_gprs_same_except_eax(&before, &after) ||
            after.data.eflags != before.data.eflags ||
            !prefix_attributes_s64_sregs_same(&before, &after);
    }

    return !failed;
}

static lib_i32 prefix_attributes_s64_test_fixed_segment_and_register(void)
{
    static const lib_u8 fixed_segment[] = { 0x26u, 0xa4u };
    static const lib_u8 register_only[] = {
        0x66u, 0x67u, 0xb8u, 0x44u, 0x33u, 0x22u, 0x11u
    };
    prefix_attributes_s64_machine state;
    core_machine_cpu_fault_snapshot diagnostic;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_u8 source = 0x4du;
    lib_u8 target = 0u;
    lib_i32 failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.chip.cpu.data.ds.base = 0x10000u;
        state.chip.cpu.data.es.base = 0x11000u;
        state.chip.cpu.data.esi = 0x0100u;
        state.chip.cpu.data.edi = 0x0200u;
        failed |= prefix_attributes_s64_write(&state, 0x11100u, &source,
            sizeof(source)) != LIB_STATUS_OK || prefix_attributes_s64_write(&state, 0x11200u, &target, sizeof(target)) !=
            LIB_STATUS_OK || !prefix_attributes_s64_run(&state, fixed_segment,
                sizeof(fixed_segment), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(fixed_segment) || after.data.esi != 0x0101u ||
            after.data.edi != 0x0201u || prefix_attributes_s64_read(&state, 0x11200u, &target, sizeof(target)) != LIB_STATUS_OK ||
            target != source;
    }

    if (failed) {
        return 0;
    }

    failed = !prefix_attributes_s64_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);
    if (!failed) {
        before = state.chip.cpu;
        failed |= !prefix_attributes_s64_run(&state, register_only,
            sizeof(register_only), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
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

    return !failed;
}

static lib_i32 prefix_attributes_s64_test_rep_movs(void)
{
    static const lib_u8 code[] = { 0xf3u, 0xa4u, 0xf4u };
    prefix_attributes_s64_machine state;
    core_machine_cpu_fault_snapshot diagnostic;
    t_cpu after;
    lib_status status;
    lib_u8 source[] = { 0x31u, 0x42u, 0x53u };
    lib_u8 target[] = { 0u, 0u, 0u };
    lib_i32 failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.chip.cpu.data.ds.base = 0x10000u;
        state.chip.cpu.data.es.base = 0x11000u;
        state.chip.cpu.data.esi = 0x0100u;
        state.chip.cpu.data.edi = 0x0200u;
        state.chip.cpu.data.ecx = 0x11220003u;
        failed |= prefix_attributes_s64_write(&state, 0x10100u, source,
            sizeof(source)) != LIB_STATUS_OK || prefix_attributes_s64_write(&state, 0x11200u, target, sizeof(target)) !=
                LIB_STATUS_OK || !prefix_attributes_s64_run(&state, code,
                    sizeof(code), 4u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(code) || after.data.esi != 0x0103u ||
            after.data.edi != 0x0203u || after.data.ecx != 0x11220000u ||
            prefix_attributes_s64_read(&state, 0x11200u, target,
                sizeof(target)) != LIB_STATUS_OK ||
            lib_memory_compare(source, target, sizeof(source)) != 0;
    }

    return !failed;
}

static lib_i32 prefix_attributes_s64_test_rep_edges(void)
{
    static const lib_u8 code[] = { 0xf3u, 0xa4u, 0xf4u };
    const lib_u32 counts[] = { 0u, 1u };
    lib_u8 form;

    for (form = 0u; form != sizeof(counts) / sizeof(counts[0]); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_fault_snapshot diagnostic;
        t_cpu after;
        lib_status status;
        lib_u8 source = 0x5au;
        lib_u8 target = 0xc3u;
        lib_i32 failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.chip.cpu.data.esi = 0x0100u;
            state.chip.cpu.data.edi = 0x0200u;
            state.chip.cpu.data.ecx = 0x33440000u | counts[form];
            failed |= prefix_attributes_s64_write(&state, 0x0100u,
                &source, sizeof(source)) != LIB_STATUS_OK ||
                prefix_attributes_s64_write(&state, 0x0200u, &target,
                    sizeof(target)) != LIB_STATUS_OK ||
                !prefix_attributes_s64_run(&state, code, sizeof(code),
                    counts[form] + 2u, &after, &diagnostic, &status) ||
                status != LIB_STATUS_OK || diagnostic.valid ||
                after.data.eip != sizeof(code) ||
                after.data.ecx != 0x33440000u ||
                after.data.esi != 0x0100u + counts[form] ||
                after.data.edi != 0x0200u + counts[form] ||
                prefix_attributes_s64_read(&state, 0x0200u, &target,
                    sizeof(target)) != LIB_STATUS_OK ||
                target != (counts[form] ? source : 0xc3u);
        }

        if (failed) {
            return 0;
        }
    }
    return 1;
}

static lib_i32 prefix_attributes_s64_test_repne_movs(void)
{
    static const lib_u8 code[] = { 0xf2u, 0xa4u };
    prefix_attributes_s64_machine state;
    core_machine_cpu_fault_snapshot diagnostic;
    t_cpu after;
    lib_status status;
    lib_u8 source = 0x7eu;
    lib_u8 target = 0u;
    lib_i32 failed = !prefix_attributes_s64_prepare(
        CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.chip.cpu.data.esi = 0x0100u;
        state.chip.cpu.data.edi = 0x0200u;
        state.chip.cpu.data.ecx = 0x55660001u;
        failed |= prefix_attributes_s64_write(&state, 0x0100u, &source,
            sizeof(source)) != LIB_STATUS_OK || prefix_attributes_s64_write(&state, 0x0200u, &target, sizeof(target)) !=
            LIB_STATUS_OK || !prefix_attributes_s64_run(&state, code,
                sizeof(code), 1u, &after, &diagnostic, &status) ||
            status != LIB_STATUS_OK || diagnostic.valid ||
            after.data.eip != sizeof(code) || after.data.esi != 0x0101u ||
            after.data.edi != 0x0201u || after.data.ecx != 0x55660000u ||
            prefix_attributes_s64_read(&state, 0x0200u, &target,
                sizeof(target)) != LIB_STATUS_OK || target != source;
    }

    return !failed;
}

static lib_i32 prefix_attributes_s64_test_mixed_repeat_last_wins(void)
{
    static const lib_u8 forms[][3] = {
        { 0xf2u, 0xf3u, 0xaeu },
        { 0xf3u, 0xf2u, 0xaeu }
    };
    static const lib_u32 budgets[] = { 2u, 1u };
    static const lib_u16 final_cx[] = { 0u, 1u };
    static const lib_u16 final_di[] = { 0x0202u, 0x0201u };
    lib_u8 form;

    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        prefix_attributes_s64_machine state;
        core_machine_cpu_fault_snapshot diagnostic;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u8 image[] = { 0x3cu, 0x3cu };
        lib_u8 observed[] = { 0u, 0u };
        lib_i32 failed = !prefix_attributes_s64_prepare(
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.chip.cpu.data.eax = 0xaabbcc3cu;
            state.chip.cpu.data.edi = 0x77880200u;
            state.chip.cpu.data.ecx = 0x55660002u;
            before = state.chip.cpu;
            failed |= prefix_attributes_s64_write(&state, 0x0200u,
                image, sizeof(image)) != LIB_STATUS_OK ||
                !prefix_attributes_s64_run(&state, forms[form],
                    sizeof(forms[form]), budgets[form], &after, &diagnostic,
                    &status) || status != LIB_STATUS_OK ||
                diagnostic.valid ||
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
                prefix_attributes_s64_read(&state, 0x0200u, observed,
                    sizeof(observed)) != LIB_STATUS_OK ||
                lib_memory_compare(image, observed, sizeof(image)) != 0;
        }

        if (failed) {
            return 0;
        }
    }
    return 1;
}

lib_i32 main(void)
{
    if (!prefix_attributes_s64_test_segments()) {
        lib_c_fprintf(lib_c_stderr, "S64 prefix segment grid failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_last_wins()) {
        lib_c_fprintf(lib_c_stderr, "S64 prefix last-wins failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_attributes_and_lock()) {
        lib_c_fprintf(lib_c_stderr, "S64 attribute/LOCK failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_lock_group_legality()) {
        lib_c_fprintf(lib_c_stderr, "S64 LOCK group legality failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_lock_group_writes()) {
        lib_c_fprintf(lib_c_stderr, "S64 LOCK group writes failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_repeated_width_prefixes()) {
        lib_c_fprintf(lib_c_stderr, "S64 repeated width-prefix failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_fixed_segment_and_register()) {
        lib_c_fprintf(lib_c_stderr, "S64 fixed-segment/register prefix failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_rep_movs()) {
        lib_c_fprintf(lib_c_stderr, "S64 REP MOVS failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_rep_edges()) {
        lib_c_fprintf(lib_c_stderr, "S64 REP edges failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_repne_movs()) {
        lib_c_fprintf(lib_c_stderr, "S64 REPNE MOVS failed\n");
        return 1;
    }
    if (!prefix_attributes_s64_test_mixed_repeat_last_wins()) {
        lib_c_fprintf(lib_c_stderr, "S64 mixed repeat-prefix failed\n");
        return 1;
    }
    lib_c_printf("M5:T539:S29:CPU-PREFIX-ATTRIBUTES:OK\n");
    return 0;
}
