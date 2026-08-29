#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define CMPS_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

typedef struct cmps_machine {
    core_machine *machine;
} cmps_machine;

static C_VOID cmps_reset(C_VOID *opaque)
{
    cmps_machine *state = (cmps_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider cmps_provider = {
    cmps_reset, STD_NULL
};

static C_INT cmps_prepare(core_machine_cpu_profile profile, cmps_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &cmps_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID cmps_seed(cmps_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xaabbcc10u;
    cpu->data.ecx = 0x11220003u;
    cpu->data.edx = 0x778899aau;
    cpu->data.ebx = 0xbbccddeeU;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF;
    cpu->data.cs.base = 0u;
    cpu->data.ds.base = 0x20000u;
    cpu->data.es.base = 0x30000u;
    cpu->data.fs.base = 0x40000u;
}

static C_INT cmps_nonindexes_same(const t_cpu *before, const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp &&
        after->data.ebp == before->data.ebp;
}

static C_INT cmps_all_gpr_same(const t_cpu *before, const t_cpu *after)
{
    return cmps_nonindexes_same(before, after) &&
        after->data.ecx == before->data.ecx &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi;
}

static C_INT cmps_run(cmps_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes,
    type_unsigned_32 budget, t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    type_status *status, core_machine_run_result *result)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){budget, 0u}, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT cmps_memory_same(cmps_machine *state, type_unsigned_32 source,
    type_unsigned_32 destination, const void *source_image, const void *destination_image,
    type_unsigned_8 width)
{
    type_unsigned_32 source_observed = 0u;
    type_unsigned_32 destination_observed = 0u;

    return core_machine_memory_read_physical(&state->machine->executor_memory,
        source, TYPE_REFERENCE_OF(source_observed), width) == TYPE_STATUS_OK &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
        destination, TYPE_REFERENCE_OF(destination_observed), width) ==
        TYPE_STATUS_OK && STD_MEMCMP(&source_observed, source_image, width) == 0 &&
        STD_MEMCMP(&destination_observed, destination_image, width) == 0;
}

static C_INT cmps_single_case(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width, C_INT address32,
    C_INT decrement, type_unsigned_32 source, type_unsigned_32 destination)
{
    cmps_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 left = width == 4u ? 0xaabb0010u : 0x10u;
    type_unsigned_32 right = 1u;
    type_unsigned_32 initial_index = address32 ? 0x1020u : 0x20u;
    type_unsigned_32 expected_index = initial_index + (decrement ? -(type_signed_32)width : width);
    C_INT failed = !cmps_prepare(profile, &state);

    if (!failed) {
        cmps_seed(&state);
        if (address32) {
            state.machine->executor_cpu.data.esi = 0x1010u;
            state.machine->executor_cpu.data.edi = initial_index;
        } else {
            state.machine->executor_cpu.data.esi =
                (state.machine->executor_cpu.data.esi & 0xffff0000u) | 0x10u;
            state.machine->executor_cpu.data.edi =
                (state.machine->executor_cpu.data.edi & 0xffff0000u) | initial_index;
        }
        if (decrement)
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
        failed |= core_machine_memory_write(state.machine, source, &left, width) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, destination,
            &right, width) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cmps_run(&state, code, bytes, 1u, &after, &diagnostic,
            &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            !cmps_nonindexes_same(&before, &after) ||
            after.data.ecx != before.data.ecx ||
            after.data.esi != (address32 ? 0x1010u +
            (decrement ? -(type_signed_32)width : width) :
            (before.data.esi & 0xffff0000u) | (type_unsigned_16)(expected_index - 0x10u)) ||
            after.data.edi != (address32 ? expected_index :
            (before.data.edi & 0xffff0000u) | (type_unsigned_16)expected_index) ||
            (after.data.eflags & CMPS_FLAGS) !=
            (VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
            (width == 4u ? VCPU_EFLAGS_SF : 0u)) ||
            (after.data.eflags & ~CMPS_FLAGS) !=
            (before.data.eflags & ~CMPS_FLAGS) || !cmps_memory_same(&state, source,
            destination, &left, &right, width);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cmps_test_single(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 cmpsb = 0xa6u;
    static const type_unsigned_8 cmpsw = 0xa7u;
    static const type_unsigned_8 cmpsd[] = {0x66u, 0xa7u};
    static const type_unsigned_8 address32[] = {0x67u, 0xa6u};
    static const type_unsigned_8 combined[] = {0x66u, 0x67u, 0xa7u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        if (!cmps_single_case(profiles[profile], &cmpsb, 1u, 1u, 0, 0,
            0x20010u, 0x30020u) || !cmps_single_case(profiles[profile],
            &cmpsw, 1u, 2u, 0, 0, 0x20010u, 0x30020u))
            return 0;
    }
    return cmps_single_case(CORE_MACHINE_CPU_PROFILE_80386, cmpsd,
        sizeof(cmpsd), 4u, 0, 0, 0x20010u, 0x30020u) &&
        cmps_single_case(CORE_MACHINE_CPU_PROFILE_80386, address32,
        sizeof(address32), 1u, 1, 0, 0x21010u, 0x31020u) &&
        cmps_single_case(CORE_MACHINE_CPU_PROFILE_80386, combined,
        sizeof(combined), 4u, 1, 0, 0x21010u, 0x31020u) &&
        cmps_single_case(CORE_MACHINE_CPU_PROFILE_80386, &cmpsw, 1u, 2u,
        0, 1, 0x20010u, 0x30020u);
}

static C_INT cmps_flag_case(type_unsigned_8 left, type_unsigned_8 right, type_unsigned_32 flags)
{
    static const type_unsigned_8 code = 0xa6u;
    cmps_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    C_INT failed = !cmps_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        cmps_seed(&state);
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        failed |= core_machine_memory_write(state.machine, 0x20010u, &left,
            sizeof(left)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x30020u, &right, sizeof(right)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cmps_run(&state, &code, sizeof(code), 1u, &after,
            &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != 1u ||
            !cmps_nonindexes_same(&before, &after) ||
            after.data.ecx != before.data.ecx || after.data.esi != 0x11u ||
            after.data.edi != 0x21u || (after.data.eflags & CMPS_FLAGS) != flags ||
            (after.data.eflags & ~CMPS_FLAGS) !=
            (before.data.eflags & ~CMPS_FLAGS) || !cmps_memory_same(&state,
            0x20010u, 0x30020u, &left, &right, sizeof(left));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cmps_test_flags(C_VOID)
{
    return cmps_flag_case(0x10u, 0x01u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) &&
        cmps_flag_case(0x10u, 0x10u, VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) &&
        cmps_flag_case(0x00u, 0x01u, VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_SF) && cmps_flag_case(0x80u, 0x01u,
        VCPU_EFLAGS_AF | VCPU_EFLAGS_OF);
}

static C_INT cmps_override_case(const type_unsigned_8 *code, type_unsigned_8 bytes,
    type_unsigned_32 source)
{
    cmps_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_8 left = 0x10u;
    type_unsigned_8 right = 1u;
    C_INT failed = !cmps_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        cmps_seed(&state);
        failed |= core_machine_memory_write(state.machine, source, &left,
            sizeof(left)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x30020u, &right, sizeof(right)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cmps_run(&state, code, bytes, 1u, &after, &diagnostic,
            &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            !cmps_nonindexes_same(&before, &after) ||
            after.data.ecx != before.data.ecx || after.data.esi != 0x11u ||
            after.data.edi != 0x21u || (after.data.eflags & CMPS_FLAGS) !=
            (VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) || !cmps_memory_same(&state,
            source, 0x30020u, &left, &right, sizeof(left));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cmps_test_segments(C_VOID)
{
    static const type_unsigned_8 cs[] = {0x2eu, 0xa6u};
    static const type_unsigned_8 fs[] = {0x64u, 0xa6u};

    return cmps_override_case(cs, sizeof(cs), 0x10u) &&
        cmps_override_case(fs, sizeof(fs), 0x40010u);
}

static C_INT cmps_rep_case(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_16 count, const type_unsigned_8 *left,
    const type_unsigned_8 *right, type_unsigned_16 expected_count, type_unsigned_16 expected_si,
    type_unsigned_16 expected_di, type_unsigned_32 flags)
{
    cmps_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    C_INT failed = !cmps_prepare(profile, &state);

    if (!failed) {
        cmps_seed(&state);
        state.machine->executor_cpu.data.ecx = 0x11220000u | count;
        failed |= core_machine_memory_write(state.machine, 0x20010u, left, 3u) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x30020u,
            right, 3u) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cmps_run(&state, code, bytes, count == 0u ? 1u :
            (type_unsigned_8)(count - expected_count), &after, &diagnostic, &status, &result) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != bytes || !cmps_nonindexes_same(&before, &after) ||
            after.data.ecx != ((before.data.ecx & 0xffff0000u) | expected_count) ||
            after.data.esi != ((before.data.esi & 0xffff0000u) | expected_si) ||
            after.data.edi != ((before.data.edi & 0xffff0000u) | expected_di) ||
            (after.data.eflags & CMPS_FLAGS) != flags ||
            (after.data.eflags & ~CMPS_FLAGS) !=
            (before.data.eflags & ~CMPS_FLAGS) || !cmps_memory_same(&state,
            0x20010u, 0x30020u, left, right, 3u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cmps_test_rep(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 repe[] = {0xf3u, 0xa6u};
    static const type_unsigned_8 repne[] = {0xf2u, 0xa6u};
    static const type_unsigned_8 zero[] = {1u, 1u, 1u};
    static const type_unsigned_8 equal[] = {0x10u, 0x10u, 0x10u};
    static const type_unsigned_8 different[] = {0x10u, 1u, 0x10u};

    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        if (!cmps_rep_case(profiles[profile], repe, sizeof(repe), 0u, zero,
            zero, 0u, 0x10u, 0x20u, 0u) || !cmps_rep_case(profiles[profile],
            repe, sizeof(repe), 1u, equal, equal, 0u, 0x11u, 0x21u,
            VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) || !cmps_rep_case(profiles[profile],
            repe, sizeof(repe), 3u, equal, different, 1u, 0x12u, 0x22u,
            VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) || !cmps_rep_case(profiles[profile],
            repne, sizeof(repne), 0u, zero, zero, 0u, 0x10u, 0x20u, 0u) ||
            !cmps_rep_case(profiles[profile], repne, sizeof(repne), 1u,
            different, zero, 0u, 0x11u, 0x21u, VCPU_EFLAGS_PF |
            VCPU_EFLAGS_AF) || !cmps_rep_case(profiles[profile], repne,
            sizeof(repne), 3u, different, equal, 2u, 0x11u, 0x21u,
            VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF))
            return 0;
    }
    return 1;
}

static C_INT cmps_rep_attribute_case(const type_unsigned_8 *code, type_unsigned_8 bytes,
    type_unsigned_8 width, C_INT address32, C_INT decrement)
{
    cmps_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 image = width == 4u ? 0x11223344u : width == 2u ? 0x3344u : 0x44u;
    type_unsigned_32 source_base = address32 ? 0x21010u : 0x20010u;
    type_unsigned_32 destination_base = address32 ? 0x31020u : 0x30020u;
    type_unsigned_32 source_index = address32 ? 0x1010u : 0x10u;
    type_unsigned_32 destination_index = address32 ? 0x1020u : 0x20u;
    type_unsigned_32 expected_source;
    type_unsigned_32 expected_destination;
    type_unsigned_8 item;
    C_INT failed = !cmps_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        cmps_seed(&state);
        if (decrement) {
            source_index += width * 2u;
            destination_index += width * 2u;
        }
        if (address32) {
            state.machine->executor_cpu.data.esi = source_index;
            state.machine->executor_cpu.data.edi = destination_index;
        } else {
            state.machine->executor_cpu.data.esi =
                (state.machine->executor_cpu.data.esi & 0xffff0000u) | source_index;
            state.machine->executor_cpu.data.edi =
                (state.machine->executor_cpu.data.edi & 0xffff0000u) | destination_index;
        }
        state.machine->executor_cpu.data.ecx = address32 ? 3u : 0x11220003u;
        if (decrement)
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
        for (item = 0u; item != 3u; ++item) {
            failed |= core_machine_memory_write(state.machine, source_base +
                item * width, &image, width) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, destination_base +
                item * width, &image, width) != TYPE_STATUS_OK;
        }
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cmps_run(&state, code, bytes, 3u, &after, &diagnostic,
            &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            !cmps_nonindexes_same(&before, &after) ||
            after.data.ecx != (address32 ? 0u : 0x11220000u) ||
            (after.data.eflags & CMPS_FLAGS) !=
            (VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
            (after.data.eflags & ~CMPS_FLAGS) !=
            (before.data.eflags & ~CMPS_FLAGS);
        expected_source = decrement ? source_index - width * 3u :
            source_index + width * 3u;
        expected_destination = decrement ? destination_index - width * 3u :
            destination_index + width * 3u;
        failed |= after.data.esi != (address32 ? expected_source :
            (before.data.esi & 0xffff0000u) | (type_unsigned_16)expected_source) ||
            after.data.edi != (address32 ? expected_destination :
            (before.data.edi & 0xffff0000u) | (type_unsigned_16)expected_destination);
        for (item = 0u; item != 3u; ++item)
            failed |= !cmps_memory_same(&state, source_base + item * width,
                destination_base + item * width, &image, &image, width);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cmps_test_rep_attributes(C_VOID)
{
    static const type_unsigned_8 operand32[] = {0xf3u, 0x66u, 0xa7u};
    static const type_unsigned_8 address32[] = {0xf3u, 0x67u, 0xa6u};
    static const type_unsigned_8 combined[] = {0xf3u, 0x66u, 0x67u, 0xa7u};
    static const type_unsigned_8 reverse[] = {0xf3u, 0xa6u};

    return cmps_rep_attribute_case(operand32, sizeof(operand32), 4u, 0, 0) &&
        cmps_rep_attribute_case(address32, sizeof(address32), 1u, 1, 0) &&
        cmps_rep_attribute_case(combined, sizeof(combined), 4u, 1, 0) &&
        cmps_rep_attribute_case(reverse, sizeof(reverse), 1u, 0, 1);
}

static C_INT cmps_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    cmps_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_8 left = 0x10u;
    type_unsigned_8 right = 1u;
    C_INT failed = !cmps_prepare(profile, &state);

    if (!failed) {
        cmps_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x20010u, &left,
            sizeof(left)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x30020u, &right, sizeof(right)) != TYPE_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cmps_run(&state, code, bytes, 1u, &after, &diagnostic,
            &status, &result) || status != TYPE_STATUS_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || !cmps_all_gpr_same(&before, &after) ||
            after.data.eflags != before.data.eflags || !cmps_memory_same(&state,
            0x20010u, 0x30020u, &left, &right, sizeof(left));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cmps_test_rejections(C_VOID)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 attr[][4] = {
        {0x66u, 0xa6u, 0u, 0u}, {0x67u, 0xa7u, 0u, 0u},
        {0x66u, 0x67u, 0xa7u, 0u}, {0xf3u, 0x66u, 0xa6u, 0u},
        {0xf2u, 0x67u, 0xa7u, 0u}, {0xf3u, 0x66u, 0x67u, 0xa7u}
    };
    static const type_unsigned_8 attr_bytes[] = {2u, 2u, 3u, 3u, 3u, 4u};
    static const type_unsigned_8 lock[][5] = {
        {0xf0u, 0xa6u, 0u, 0u, 0u}, {0xf0u, 0xa7u, 0u, 0u, 0u},
        {0xf0u, 0xf3u, 0xa6u, 0u, 0u}, {0xf0u, 0xf2u, 0xa7u, 0u, 0u},
        {0xf0u, 0x66u, 0xa7u, 0u, 0u}, {0xf0u, 0x67u, 0xa6u, 0u, 0u},
        {0xf0u, 0x66u, 0x67u, 0xa7u, 0u},
        {0xf0u, 0xf3u, 0x66u, 0x67u, 0xa7u}
    };
    static const type_unsigned_8 lock_bytes[] = {2u, 2u, 3u, 3u, 3u, 3u, 4u, 5u};
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
        for (form = 0u; form != sizeof(attr_bytes); ++form)
            if (!cmps_expect_ud(legacy[profile], attr[form], attr_bytes[form]))
                return 0;
    for (form = 0u; form != sizeof(lock_bytes); ++form)
        if (!cmps_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock[form],
            lock_bytes[form]))
            return 0;
    return 1;
}

static C_INT cmps_boot_protected(cmps_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x27u, 0u, 0u, 0x03u, 0u, 0u};
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0, 0x0fu,0,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u, 0xb8u,0x18u,0,0x8eu,0xc0u,
        0xb8u,0x20u,0,0x8eu,0xd0u,0xbcu,0,0x80u, 0xeau,0,0,8u,0
    };
    static const type_unsigned_8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, boot, sizeof(boot)) ==
        TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0x2000u,
        &halt, sizeof(halt)) == TYPE_STATUS_OK && core_machine_run(
        state->machine, (core_machine_run_budget){96u, 0u}, &result) ==
        TYPE_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT cmps_protected_case(C_INT source_fault)
{
    static const type_unsigned_8 code = 0xa6u;
    cmps_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_8 left = 0x10u;
    type_unsigned_8 right = 1u;
    C_INT failed = !cmps_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !cmps_boot_protected(&state);
    if (!failed) {
        t_cpu_data_sreg cs = state.machine->executor_cpu.data.cs;
        t_cpu_data_sreg ds = state.machine->executor_cpu.data.ds;
        t_cpu_data_sreg es = state.machine->executor_cpu.data.es;
        t_cpu_data_sreg ss = state.machine->executor_cpu.data.ss;

        cmps_seed(&state);
        state.machine->executor_cpu.data.cs = cs;
        state.machine->executor_cpu.data.ds = ds;
        state.machine->executor_cpu.data.es = es;
        state.machine->executor_cpu.data.ss = ss;
        state.machine->executor_cpu.data.esi = source_fault ? 0x10u : 0u;
        state.machine->executor_cpu.data.edi = source_fault ? 0u : 0x10u;
        failed |= core_machine_memory_write(state.machine, 0x3010u, &left,
            sizeof(left)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x4010u, &right, sizeof(right)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x2000u, &code,
            sizeof(code)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u, 0u}, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || core_machine_get_cpu_diagnostic(
            state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || !cmps_all_gpr_same(&before, &after) ||
            after.data.eflags != before.data.eflags || STD_MEMCMP(&before.data.ds,
            &after.data.ds, sizeof(before.data.ds)) != 0 || STD_MEMCMP(
            &before.data.es, &after.data.es, sizeof(before.data.es)) != 0 ||
            !cmps_memory_same(&state, 0x3010u, 0x4010u, &left, &right,
            sizeof(left));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cmps_protected_rep_case(C_INT source_fault)
{
    static const type_unsigned_8 code[] = {0xf3u, 0xa6u};
    cmps_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_8 left[] = {0x10u, 1u};
    type_unsigned_8 right[] = {0x10u, 1u};
    C_INT failed = !cmps_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !cmps_boot_protected(&state);
    if (!failed) {
        t_cpu_data_sreg cs = state.machine->executor_cpu.data.cs;
        t_cpu_data_sreg ds = state.machine->executor_cpu.data.ds;
        t_cpu_data_sreg es = state.machine->executor_cpu.data.es;
        t_cpu_data_sreg ss = state.machine->executor_cpu.data.ss;

        cmps_seed(&state);
        state.machine->executor_cpu.data.cs = cs;
        state.machine->executor_cpu.data.ds = ds;
        state.machine->executor_cpu.data.es = es;
        state.machine->executor_cpu.data.ss = ss;
        if (source_fault) {
            state.machine->executor_cpu.data.ds.limit = 0x10u;
            state.machine->executor_cpu.data.es.limit = 0xffffu;
        } else {
            state.machine->executor_cpu.data.ds.limit = 0xffffu;
            state.machine->executor_cpu.data.es.limit = 0x10u;
        }
        state.machine->executor_cpu.data.esi = 0x10u;
        state.machine->executor_cpu.data.edi = 0x10u;
        state.machine->executor_cpu.data.ecx = 0x11220003u;
        failed |= core_machine_memory_write(state.machine, 0x3010u, left,
            sizeof(left)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x4010u, right, sizeof(right)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x2000u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){2u, 0u}, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || core_machine_get_cpu_diagnostic(
            state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || !cmps_nonindexes_same(&before, &after) ||
            after.data.ecx != 0x11220002u || after.data.esi != 0x11u ||
            after.data.edi != 0x11u || after.data.eflags !=
            (VCPU_EFLAGS_IF | VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
            STD_MEMCMP(&before.data.ds, &after.data.ds, sizeof(before.data.ds)) != 0 ||
            STD_MEMCMP(&before.data.es, &after.data.es, sizeof(before.data.es)) != 0 ||
            !cmps_memory_same(&state, 0x3010u, 0x4010u, left, right,
            sizeof(left));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cmps_test_protected(C_VOID)
{
    return cmps_protected_case(1) && cmps_protected_case(0) &&
        cmps_protected_rep_case(1) && cmps_protected_rep_case(0);
}

static C_INT cmps_irq_case(C_INT repeat)
{
    static const type_unsigned_8 single[] = {0xa6u, 0x90u};
    static const type_unsigned_8 repeated[] = {0xf3u, 0xa6u, 0x90u};
    static const type_unsigned_8 hlt = 0xf4u;
    cmps_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_16 offset = 0x100u;
    type_unsigned_16 segment = 0u;
    type_unsigned_16 frame_ip = 0u;
    type_unsigned_8 left[] = {0x10u, 1u, 1u};
    type_unsigned_8 right[] = {0x10u, 1u, 1u};
    const type_unsigned_8 *code = repeat ? repeated : single;
    type_unsigned_8 bytes = repeat ? sizeof(repeated) : sizeof(single);
    C_INT failed = !cmps_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x20010u, left,
            sizeof(left)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x30020u, right, sizeof(right)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0u, code, bytes) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x80u,
            &offset, sizeof(offset)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x82u, &segment, sizeof(segment)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x100u, &hlt, sizeof(hlt)) !=
            TYPE_STATUS_OK;
    }
    if (!failed) {
        cmps_seed(&state);
        if (repeat)
            state.machine->executor_cpu.data.ecx = 0x11220003u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        STD_MEMSET(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){repeat ? 4u : 2u, 0u}, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
            after.data.eip != 0x101u || frame_ip != (repeat ? 0u : 1u) ||
            after.data.esi != 0x11u || after.data.edi != 0x21u ||
            after.data.ecx != (repeat ? 0x11220002u : 0x11220003u) ||
            (after.data.eflags & CMPS_FLAGS) !=
            (VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u)) || !cmps_memory_same(&state, 0x20010u, 0x30020u,
            left, right, sizeof(left));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cmps_test_irq(C_VOID)
{
    return cmps_irq_case(0) && cmps_irq_case(1);
}

C_INT main(C_VOID)
{
    if (!cmps_test_single()) {
        STD_PRINTF("CMPS stage=single\n");
        return 1;
    }
    if (!cmps_test_flags()) {
        STD_PRINTF("CMPS stage=flags\n");
        return 1;
    }
    if (!cmps_test_segments()) {
        STD_PRINTF("CMPS stage=segments\n");
        return 1;
    }
    if (!cmps_test_rep()) {
        STD_PRINTF("CMPS stage=rep\n");
        return 1;
    }
    if (!cmps_test_rep_attributes()) {
        STD_PRINTF("CMPS stage=rep-attributes\n");
        return 1;
    }
    if (!cmps_test_rejections()) {
        STD_PRINTF("CMPS stage=rejections\n");
        return 1;
    }
    if (!cmps_test_protected()) {
        STD_PRINTF("CMPS stage=protected\n");
        return 1;
    }
    if (!cmps_test_irq()) {
        STD_PRINTF("CMPS stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S37:CMPS:OK\n");
    STD_PRINTF("M5:T401:S16:CMPS-PROFILES:OK\n");
    return 0;
}
