#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct movs_machine {
    core_machine *machine;
} movs_machine;

static C_VOID movs_reset(C_VOID *opaque)
{
    movs_machine *state = (movs_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider movs_provider = {
    movs_reset, STD_NULL
};

static C_INT movs_prepare(core_machine_cpu_profile profile, movs_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &movs_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID movs_seed(movs_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xaabb3344u;
    cpu->data.ecx = 0x11225566u;
    cpu->data.edx = 0x778899aau;
    cpu->data.ebx = 0xbbccddeeU;
    cpu->data.esp = 0x00008000u;
    cpu->data.ebp = 0x00000120u;
    cpu->data.esi = 0x00000010u;
    cpu->data.edi = 0x00000020u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    cpu->data.ds.selector = 0x1000u;
    cpu->data.ds.base = 0x10000u;
    cpu->data.es.selector = 0x2000u;
    cpu->data.es.base = 0x20000u;
    cpu->data.ss.selector = 0x3000u;
    cpu->data.ss.base = 0x30000u;
    cpu->data.fs.selector = 0x4000u;
    cpu->data.fs.base = 0x40000u;
    cpu->data.gs.selector = 0x5000u;
    cpu->data.gs.base = 0x50000u;
}

static C_INT movs_run(movs_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes,
    t_cpu *after, core_machine_cpu_diagnostic *diagnostic, type_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine, (core_machine_run_budget){1u, 0u},
        &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT movs_run_repeated(movs_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, type_unsigned_32 repetitions, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, type_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){repetitions == 0u ? 1u : repetitions, 0u},
        &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT movs_nonindexes_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.eflags == after->data.eflags;
}

static C_INT movs_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return movs_nonindexes_same(before, after) &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT movs_test_single_defaults(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 codes[] = {0xa4u, 0xa5u};
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile) {
        for (form = 0u; form != sizeof(codes); ++form) {
            movs_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_32 source = 0x12345678u;
            type_unsigned_32 source_after = source;
            type_unsigned_32 destination = 0xa5a5a5a5u;
            type_unsigned_8 width = form == 0u ? 1u : 2u;
            C_INT failed = !movs_prepare(profiles[profile], &state);

            if (!failed) {
                movs_seed(&state);
                failed |= core_machine_memory_write(state.machine, 0x10010u,
                    &source, width) != TYPE_STATUS_OK ||
                    core_machine_memory_write(state.machine, 0x20020u,
                        &destination, width) != TYPE_STATUS_OK;
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !movs_run(&state, &codes[form], 1u, &after,
                    &diagnostic, &status) || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid || after.data.eip != 1u ||
                    !movs_nonindexes_same(&before, &after) ||
                    after.data.esi != before.data.esi + width ||
                    after.data.edi != before.data.edi + width ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                        0x10010u, TYPE_REFERENCE_OF(source_after), width) !=
                        TYPE_STATUS_OK || source_after != source ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                        0x20020u, TYPE_REFERENCE_OF(destination), width) !=
                        TYPE_STATUS_OK || destination != (width == 1u ?
                        0xa5a5a578u : 0xa5a55678u);
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT movs_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 codes[][3] = {
        {0x66u, 0xa5u, 0u},
        {0x67u, 0xa4u, 0u},
        {0x66u, 0x67u, 0xa5u}
    };
    static const type_unsigned_8 bytes[] = {2u, 2u, 3u};
    static const type_unsigned_8 widths[] = {4u, 1u, 4u};
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(bytes); ++form) {
        movs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_32 source = 0x12345678u;
        type_unsigned_32 source_after = source;
        type_unsigned_32 destination = 0xa5a5a5a5u;
        type_unsigned_32 source_address;
        type_unsigned_32 destination_address;
        C_INT failed = !movs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            movs_seed(&state);
            if (form != 0u) {
                state.machine->executor_cpu.data.esi = 0x00001010u;
                state.machine->executor_cpu.data.edi = 0x00001020u;
            }
            source_address = form == 0u ? 0x10010u : 0x11010u;
            destination_address = form == 0u ? 0x20020u : 0x21020u;
            failed |= core_machine_memory_write(state.machine, source_address,
                &source, widths[form]) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, destination_address,
                    &destination, widths[form]) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !movs_run(&state, codes[form], bytes[form], &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != bytes[form] ||
                !movs_nonindexes_same(&before, &after) ||
                after.data.esi != before.data.esi + widths[form] ||
                after.data.edi != before.data.edi + widths[form] ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    source_address, TYPE_REFERENCE_OF(source_after), widths[form]) !=
                    TYPE_STATUS_OK || source_after != source ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    destination_address, TYPE_REFERENCE_OF(destination), widths[form]) !=
                    TYPE_STATUS_OK || destination != (widths[form] == 1u ?
                    0xa5a5a578u : source);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT movs_test_source_segments_and_df(C_VOID)
{
    static const type_unsigned_8 codes[][2] = {
        {0x2eu, 0xa4u}, {0x36u, 0xa4u}, {0x64u, 0xa4u}, {0x65u, 0xa4u},
        {0x26u, 0xa4u}, {0xa5u, 0u}
    };
    static const type_unsigned_32 addresses[] = {
        0x0010u, 0x30010u, 0x40010u, 0x50010u, 0x20010u, 0x10010u
    };
    static const type_unsigned_8 sizes[] = {2u, 2u, 2u, 2u, 2u, 1u};
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(sizes); ++form) {
        movs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_16 source = (type_unsigned_16)(0x1100u + form);
        type_unsigned_16 source_after = source;
        type_unsigned_16 destination = 0xa5a5u;
        type_unsigned_8 width = form == 5u ? 2u : 1u;
        C_INT failed = !movs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            movs_seed(&state);
            if (form == 5u)
                state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
            failed |= core_machine_memory_write(state.machine, addresses[form],
                &source, width) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20020u,
                    &destination, width) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !movs_run(&state, codes[form], sizes[form], &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != sizes[form] ||
                !movs_nonindexes_same(&before, &after) ||
                after.data.esi != before.data.esi + (form == 5u ? -2 : 1) ||
                after.data.edi != before.data.edi + (form == 5u ? -2 : 1) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    addresses[form], TYPE_REFERENCE_OF(source_after), width) !=
                    TYPE_STATUS_OK || source_after != source ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x20020u, TYPE_REFERENCE_OF(destination), width) !=
                    TYPE_STATUS_OK || destination != (width == 1u ?
                    (type_unsigned_16)(0xa500u | (source & 0x00ffu)) : source);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static type_unsigned_32 movs_replace_low(type_unsigned_32 original, type_unsigned_32 value,
    type_unsigned_8 width)
{
    if (width == 1u)
        return (original & 0xffffff00u) | (value & 0x000000ffu);
    if (width == 2u)
        return (original & 0xffff0000u) | (value & 0x0000ffffu);
    return value;
}

static C_INT movs_nonindexes_except_count_same(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.eflags == after->data.eflags;
}

static C_INT movs_test_rep_case(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width, C_INT address32,
    type_unsigned_8 count, C_INT decrement)
{
    movs_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_status status;
    type_unsigned_32 source[3] = {0x11223344u, 0x55667788u, 0x99aabbccu};
    type_unsigned_32 source_after;
    type_unsigned_32 destination = 0xa5a5a5a5u;
    type_unsigned_32 destination_after;
    type_unsigned_32 source_index = address32 ? 0x00001010u : 0x00000010u;
    type_unsigned_32 destination_index = address32 ? 0x00001020u : 0x00000020u;
    type_unsigned_8 slots = count == 0u ? 1u : count;
    type_unsigned_8 index;
    C_INT failed = !movs_prepare(profile, &state);

    if (!failed) {
        movs_seed(&state);
        if (decrement) {
            source_index += (count - 1u) * width;
            destination_index += (count - 1u) * width;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
        }
        if (address32) {
            state.machine->executor_cpu.data.esi = source_index;
            state.machine->executor_cpu.data.edi = destination_index;
        } else {
            state.machine->executor_cpu.data.esi =
                (state.machine->executor_cpu.data.esi & 0xffff0000u) |
                source_index;
            state.machine->executor_cpu.data.edi =
                (state.machine->executor_cpu.data.edi & 0xffff0000u) |
                destination_index;
        }
        if (address32)
            state.machine->executor_cpu.data.ecx = count;
        else
            state.machine->executor_cpu.data.ecx =
                (state.machine->executor_cpu.data.ecx & 0xffff0000u) | count;
        for (index = 0u; index != slots; ++index) {
            type_unsigned_32 step = decrement ? (count - 1u - index) * width :
                index * width;

            failed |= core_machine_memory_write(state.machine,
                0x10000u + source_index - (decrement ?
                (count - 1u) * width : 0u) + step, &source[index], width) !=
                TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                0x20000u + destination_index - (decrement ?
                (count - 1u) * width : 0u) + step, &destination, width) !=
                TYPE_STATUS_OK;
        }
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !movs_run_repeated(&state, code, bytes, count, &after,
            &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != bytes ||
            !movs_nonindexes_except_count_same(&before, &after) ||
            after.data.ecx != (address32 ? 0u :
            (before.data.ecx & 0xffff0000u)) ||
            after.data.esi != (address32 ? source_index +
            (decrement ? -(type_signed_32)(count * width) : count * width) :
            ((before.data.esi & 0xffff0000u) |
            (type_unsigned_16)(source_index + (decrement ? -(type_signed_32)(count * width) :
            count * width)))) ||
            after.data.edi != (address32 ? destination_index +
            (decrement ? -(type_signed_32)(count * width) : count * width) :
            ((before.data.edi & 0xffff0000u) |
            (type_unsigned_16)(destination_index +
            (decrement ? -(type_signed_32)(count * width) : count * width))));
        for (index = 0u; !failed && index != slots; ++index) {
            type_unsigned_32 step = index * width;
            type_unsigned_32 source_address = 0x10000u + source_index -
                (decrement ? (count - 1u) * width : 0u) + step;
            type_unsigned_32 destination_address = 0x20000u + destination_index -
                (decrement ? (count - 1u) * width : 0u) + step;
            type_unsigned_8 element = decrement && count != 0u ? count - 1u - index :
                index;

            source_after = source[element];
            destination_after = destination;
            failed |= core_machine_memory_read_physical(
                &state.machine->executor_memory, source_address,
                TYPE_REFERENCE_OF(source_after), width) != TYPE_STATUS_OK ||
                source_after != source[element] ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    destination_address, TYPE_REFERENCE_OF(destination_after),
                    width) != TYPE_STATUS_OK || destination_after !=
                movs_replace_low(destination, count == 0u ? destination :
                source[element], width);
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT movs_test_rep(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 rep_movsb[] = {0xf3u, 0xa4u};
    static const type_unsigned_8 rep_movsw[] = {0xf3u, 0xa5u};
    static const type_unsigned_8 rep_movsd[] = {0x66u, 0xf3u, 0xa5u};
    static const type_unsigned_8 rep_addr32_movsb[] = {0xf3u, 0x67u, 0xa4u};
    static const type_unsigned_8 rep_addr32_movsd[] = {0xf3u, 0x66u, 0x67u, 0xa5u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile) {
        if (!movs_test_rep_case(profiles[profile], rep_movsb, 2u, 1u, 0,
                0u, 0) || !movs_test_rep_case(profiles[profile], rep_movsb,
                2u, 1u, 0, 1u, 0) ||
            !movs_test_rep_case(profiles[profile], rep_movsb, 2u, 1u, 0,
                3u, 0) || !movs_test_rep_case(profiles[profile], rep_movsw,
                2u, 2u, 0, 3u, 0))
            return 0;
    }
    return movs_test_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_movsd,
        3u, 4u, 0, 3u, 0) &&
        movs_test_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_addr32_movsb,
            3u, 1u, 1, 3u, 0) &&
        movs_test_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_addr32_movsd,
            4u, 4u, 1, 3u, 0) &&
        movs_test_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_movsw,
            2u, 2u, 0, 3u, 1);
}

static C_INT movs_test_rejections(C_VOID)
{
    static const type_unsigned_8 prefixes[][3] = {
        {0x66u, 0xa4u, 0u}, {0x67u, 0xa5u, 0u},
        {0x66u, 0x67u, 0xa5u}
    };
    static const type_unsigned_8 prefix_bytes[] = {2u, 2u, 3u};
    static const type_unsigned_8 locks[][3] = {
        {0xf0u, 0xa4u, 0u}, {0xf0u, 0xf3u, 0xa5u}
    };
    static const type_unsigned_8 lock_bytes[] = {2u, 3u};
    type_unsigned_8 form;
    type_unsigned_8 profile;

    for (profile = CORE_MACHINE_CPU_PROFILE_8086;
         profile < CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        for (form = 0u; form != sizeof(prefix_bytes); ++form) {
            movs_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_16 source = 0x3344u;
            type_unsigned_16 destination = 0xa5a5u;
            C_INT failed = !movs_prepare(profile, &state);

            if (!failed) {
                movs_seed(&state);
                failed |= core_machine_memory_write(state.machine, 0x10010u,
                    &source, sizeof(source)) != TYPE_STATUS_OK ||
                    core_machine_memory_write(state.machine, 0x20020u,
                    &destination, sizeof(destination)) != TYPE_STATUS_OK;
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !movs_run(&state, prefixes[form], prefix_bytes[form],
                    &after, &diagnostic, &status) || status != TYPE_STATUS_FAULT ||
                    !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    after.data.eip != 0u || !movs_gprs_same(&before, &after) ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x10010u, TYPE_REFERENCE_OF(source), sizeof(source)) !=
                    TYPE_STATUS_OK || source != 0x3344u ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x20020u, TYPE_REFERENCE_OF(destination), sizeof(destination)) !=
                    TYPE_STATUS_OK || destination != 0xa5a5u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    for (form = 0u; form != sizeof(lock_bytes); ++form) {
        movs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_16 source = 0x3344u;
        type_unsigned_16 destination = 0xa5a5u;
        C_INT failed = !movs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            movs_seed(&state);
            state.machine->executor_cpu.data.ecx = 3u;
            failed |= core_machine_memory_write(state.machine, 0x10010u,
                &source, sizeof(source)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20020u,
                &destination, sizeof(destination)) != TYPE_STATUS_OK;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !movs_run(&state, locks[form], lock_bytes[form], &after,
                &diagnostic, &status) || status != TYPE_STATUS_FAULT ||
                !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || !movs_gprs_same(&before, &after) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x10010u, TYPE_REFERENCE_OF(source), sizeof(source)) !=
                TYPE_STATUS_OK || source != 0x3344u ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x20020u, TYPE_REFERENCE_OF(destination), sizeof(destination)) !=
                TYPE_STATUS_OK || destination != 0xa5a5u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT movs_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 codes[][3] = {
        {0xa4u, 0x90u, 0u}, {0xf3u, 0xa4u, 0x90u}
    };
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        movs_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_16 offset = 0x0100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_8 image[] = {0x51u, 0x62u, 0x73u};
        type_unsigned_8 destination[] = {0xa5u, 0xa5u, 0xa5u};
        type_unsigned_8 count = form == 0u ? 1u : 3u;
        C_INT failed = !movs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u) || core_machine_memory_write(state.machine,
                0x10010u, image, count) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20020u, destination,
                count) != TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                0u, codes[form], form == 0u ? 2u : 3u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x80u, &offset,
                sizeof(offset)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x82u, &segment, sizeof(segment)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &hlt,
                sizeof(hlt)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            movs_seed(&state);
            state.machine->executor_cpu.data.ecx =
                (state.machine->executor_cpu.data.ecx & 0xffff0000u) | count;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){count + 1u, 0u}, &result) !=
                TYPE_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != (form == 0u ? 1u : 0u) ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                after.data.esi != 0x11u || after.data.edi != 0x21u ||
                after.data.ecx != (form == 0u ? 0x11220001u : 0x11220002u) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x10010u, (type_virtual_address)image, count) != TYPE_STATUS_OK ||
                image[0] != 0x51u ||
                image[1] != 0x62u || image[2] != 0x73u ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x20020u, (type_virtual_address)destination, count) != TYPE_STATUS_OK ||
                destination[0] != 0x51u || (count == 3u &&
                (destination[1] != 0xa5u || destination[2] != 0xa5u));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT movs_boot_protected(movs_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x18u,0,0x8eu,0xd8u, 0xb8u,0x10u,0,0x8eu,0xc0u,
        0xb8u,0x18u,0,0x8eu,0xd0u,0xbcu,0,0x80u,
        0xeau,0,0,8u,0
    };
    static const type_unsigned_8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, boot, sizeof(boot)) ==
        TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0x2000u,
        &halt, sizeof(halt)) == TYPE_STATUS_OK && core_machine_run(state->machine,
        (core_machine_run_budget){96u, 0u}, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT movs_test_protected_limits(C_VOID)
{
    static const type_unsigned_8 codes[][2] = {{0xa4u, 0u}, {0x66u, 0xa5u}};
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        movs_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_32 source = 0x11223344u;
        type_unsigned_32 destination = 0xa5a5a5a5u;
        type_unsigned_32 source_address = form == 0u ? 0x3010u : 0x4010u;
        type_unsigned_32 destination_address = form == 0u ? 0x3020u : 0x3010u;
        type_unsigned_8 bytes = form == 0u ? 1u : 2u;
        type_unsigned_8 width = form == 0u ? 1u : 4u;
        C_INT failed = !movs_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) failed |= !movs_boot_protected(&state);
        if (!failed) {
            movs_seed(&state);
            state.machine->executor_cpu.data.ds.selector = form == 0u ? 0x10u :
                0x18u;
            state.machine->executor_cpu.data.ds.base = form == 0u ? 0x3000u :
                0x4000u;
            state.machine->executor_cpu.data.ds.limit = form == 0u ? 0x0fu :
                0xffffu;
            state.machine->executor_cpu.data.es.selector = 0x10u;
            state.machine->executor_cpu.data.es.base = 0x3000u;
            state.machine->executor_cpu.data.es.limit = 0x0fu;
            failed |= core_machine_memory_write(state.machine, source_address,
                &source, width) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, destination_address, &destination, width) !=
                TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x2000u,
                codes[form], bytes) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u, 0u}, &result) != TYPE_STATUS_FAULT ||
                result.reason != CORE_MACHINE_STOP_FAULT || core_machine_get_cpu_diagnostic(
                state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || !movs_gprs_same(&before, &after) ||
                STD_MEMCMP(&before.data.ds, &after.data.ds,
                sizeof(before.data.ds)) != 0 || STD_MEMCMP(&before.data.es,
                &after.data.es, sizeof(before.data.es)) != 0 ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                source_address, TYPE_REFERENCE_OF(source), width) != TYPE_STATUS_OK ||
                source != 0x11223344u || core_machine_memory_read_physical(
                &state.machine->executor_memory, destination_address,
                TYPE_REFERENCE_OF(destination), width) != TYPE_STATUS_OK ||
                destination != 0xa5a5a5a5u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!movs_test_single_defaults()) {
        STD_PRINTF("MOVS stage=defaults\n");
        return 1;
    }
    if (!movs_test_386_attributes()) {
        STD_PRINTF("MOVS stage=attributes\n");
        return 1;
    }
    if (!movs_test_source_segments_and_df()) {
        STD_PRINTF("MOVS stage=segments-df\n");
        return 1;
    }
    if (!movs_test_rep()) {
        STD_PRINTF("MOVS stage=rep\n");
        return 1;
    }
    if (!movs_test_rejections()) {
        STD_PRINTF("MOVS stage=rejections\n");
        return 1;
    }
    if (!movs_test_irq_no_shadow()) {
        STD_PRINTF("MOVS stage=irq\n");
        return 1;
    }
    if (!movs_test_protected_limits()) {
        STD_PRINTF("MOVS stage=protected\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S33:MOVS:OK\n");
    STD_PRINTF("M5:T401:S15:MOVS-PROFILES:OK\n");
    return 0;
}
