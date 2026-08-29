#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct gpr_mov_machine { core_machine *machine; } gpr_mov_machine;

static C_VOID gpr_mov_reset(C_VOID *opaque)
{
    gpr_mov_machine *state = (gpr_mov_machine *)opaque;
    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider gpr_mov_provider = {
    gpr_mov_reset, STD_NULL
};

static C_INT gpr_mov_prepare(core_machine_cpu_profile profile, gpr_mov_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &gpr_mov_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID gpr_mov_seed(gpr_mov_machine *state)
{
    state->machine->executor_cpu.data.eax = 0xaabb3344u;
    state->machine->executor_cpu.data.ecx = 0x11225566u;
    state->machine->executor_cpu.data.edx = 0x778899aau;
    state->machine->executor_cpu.data.ebx = 0xbbccddeeU;
    state->machine->executor_cpu.data.esp = 0x00008000u;
    state->machine->executor_cpu.data.ebp = 0x00000120u;
    state->machine->executor_cpu.data.esi = 0x00000010u;
    state->machine->executor_cpu.data.edi = 0x00000020u;
    state->machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
}

static C_INT gpr_mov_run(gpr_mov_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes,
    t_cpu *after, core_machine_cpu_diagnostic *diagnostic, type_status *status)
{
    core_machine_run_result result;
    if (core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine, (core_machine_run_budget){1u,0u}, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK;
}

static type_unsigned_32 gpr_mov_gpr(const t_cpu *cpu, type_unsigned_8 index)
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

static C_INT gpr_mov_nonparticipants(const t_cpu *before, const t_cpu *after,
    type_unsigned_8 destination)
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

static C_INT gpr_mov_test_defaults(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 memory_codes[][4] = {
        {0x88u,0x06u,0x00u,0x10u}, {0x89u,0x0eu,0x00u,0x10u},
        {0x8au,0x06u,0x00u,0x10u}, {0x8bu,0x0eu,0x00u,0x10u}
    };
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile) {
        for (form = 0u; form != sizeof(memory_codes) / sizeof(memory_codes[0]);
             ++form) {
            gpr_mov_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_16 image = 0xbe5au;
            C_INT failed = !gpr_mov_prepare(profiles[profile], &state);
            if (!failed) {
                gpr_mov_seed(&state);
                failed |= core_machine_memory_write(state.machine, 0x1000u,
                    &image, 2u) != TYPE_STATUS_OK;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !gpr_mov_run(&state, memory_codes[form], 4u, &after, &diagnostic, &status) ||
                    status != TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eip != 4u ||
                    !gpr_mov_nonparticipants(&before, &after,
                        form == 3u ? 1u :
                        (form == 0u || form == 1u) ? 8u : 0u);
                if (form == 0u) failed |= (after.data.eax & 0xffu) != 0x44u;
                if (form == 1u) failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x1000u, TYPE_REFERENCE_OF(image), 2u) != TYPE_STATUS_OK || image != 0x5566u;
                if (form == 2u)
                    failed |= after.data.eax !=
                        ((before.data.eax & 0xffffff00u) | 0x5au);
                if (form == 3u)
                    failed |= after.data.ecx !=
                        ((before.data.ecx & 0xffff0000u) | 0xbe5au);
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
        for (form = 0u; form != 8u; ++form) {
            gpr_mov_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_8 code[] = {(type_unsigned_8)(0xb0u + form),
                (type_unsigned_8)(0x80u + form)};
            type_unsigned_8 target = form & 3u;
            type_unsigned_32 expected;
            C_INT failed = !gpr_mov_prepare(profiles[profile], &state);
            if (!failed) {
                gpr_mov_seed(&state);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                expected = gpr_mov_gpr(&before, target);
                expected = form < 4u ?
                    (expected & 0xffffff00u) | (0x80u + form) :
                    (expected & 0xffff00ffu) |
                        ((type_unsigned_32)(0x80u + form) << 8u);
                failed |= !gpr_mov_run(&state, code, sizeof(code), &after,
                    &diagnostic, &status) || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
                    !gpr_mov_nonparticipants(&before, &after, target) ||
                    gpr_mov_gpr(&after, target) != expected;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
        for (form = 0u; form != 8u; ++form) {
            gpr_mov_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_8 code[] = {(type_unsigned_8)(0xb8u + form), 0x34u,
                (type_unsigned_8)(0x12u + form)};
            type_unsigned_32 expected;
            C_INT failed = !gpr_mov_prepare(profiles[profile], &state);

            if (!failed) {
                gpr_mov_seed(&state);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                expected = (gpr_mov_gpr(&before, form) & 0xffff0000u) |
                    (type_unsigned_16)(0x1234u + (form << 8u));
                failed |= !gpr_mov_run(&state, code, sizeof(code), &after,
                    &diagnostic, &status) || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
                    !gpr_mov_nonparticipants(&before, &after, form) ||
                    gpr_mov_gpr(&after, form) != expected;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    {
        static const type_unsigned_8 codes[][2] = {
            {0x88u,0xcbu}, {0x89u,0xcbu}, {0x8au,0xcbu}, {0x8bu,0xcbu}
        };
        type_unsigned_8 direction;

        for (direction = 0u; direction != sizeof(codes) / sizeof(codes[0]);
             ++direction) {
            gpr_mov_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_8 destination = direction < 2u ? 3u : 1u;
            C_INT failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);

            if (!failed) {
                gpr_mov_seed(&state);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !gpr_mov_run(&state, codes[direction],
                    sizeof(codes[direction]), &after, &diagnostic, &status) ||
                    status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
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
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT gpr_mov_test_immediate_and_reject(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 valid[][6] = {
        {0xc6u,0x06u,0,0x10u,0x5au,0},
        {0xc7u,0x06u,0,0x10u,0x34u,0x12u}
    };
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != 2u; ++form) {
        gpr_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_16 image = 0u;
        type_unsigned_8 bytes = form == 0u ? 5u : 6u;
        C_INT failed = !gpr_mov_prepare(profiles[profile],
            &state);

        if (!failed) {
            gpr_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed |= !gpr_mov_run(&state, valid[form], bytes, &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != bytes ||
                !gpr_mov_nonparticipants(&before, &after, 8u) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x1000u, TYPE_REFERENCE_OF(image), 2u) != TYPE_STATUS_OK ||
                image != (form == 0u ? 0x005au : 0x1234u);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != 2u; ++form) {
        type_unsigned_8 extension;

        for (extension = 1u; extension != 8u; ++extension) {
            gpr_mov_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_16 image = 0xbeefu;
            type_unsigned_8 code[] = {(type_unsigned_8)(form ? 0xc7u : 0xc6u),
                (type_unsigned_8)(0x06u | (extension << 3u)),0,0x10u,0,0};
            C_INT failed = !gpr_mov_prepare(profiles[profile],
                &state);

            if (!failed) {
                gpr_mov_seed(&state);
                failed |= core_machine_memory_write(state.machine, 0x1000u,
                    &image, sizeof(image)) != TYPE_STATUS_OK;
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !gpr_mov_run(&state, code, form ? 6u : 5u,
                    &after, &diagnostic, &status) || status != TYPE_STATUS_FAULT ||
                    !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                        diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    after.data.eip != before.data.eip ||
                    !gpr_mov_nonparticipants(&before, &after, 8u) ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                        0x1000u, TYPE_REFERENCE_OF(image), sizeof(image)) !=
                        TYPE_STATUS_OK || image != 0xbeefu;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT gpr_mov_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 codes[][12] = {
        {0x66u, 0x89u, 0x0eu, 0, 0x10u},
        {0x67u, 0x8bu, 0x05u, 0, 0x10u, 0, 0},
        {0x66u, 0x67u, 0x8bu, 0x05u, 0, 0x10u, 0, 0},
        {0x66u, 0xb8u, 0x44u, 0x33u, 0x22u, 0x11u},
        {0x67u, 0xb0u, 0x5au}
    };
    static const type_unsigned_8 bytes[] = {5u, 7u, 8u, 6u, 3u};
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(bytes); ++form) {
        gpr_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_32 image = 0xaabbccddU;
        type_unsigned_8 destination = form == 0u ? 8u : 0u;
        C_INT failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            gpr_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed |= core_machine_memory_write(state.machine, 0x1000u,
                &image, 4u) != TYPE_STATUS_OK ||
                !gpr_mov_run(&state, codes[form], bytes[form], &after,
                    &diagnostic, &status) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != bytes[form] ||
                !gpr_mov_nonparticipants(&before, &after, destination);
            if (form == 0u)
                failed |= core_machine_memory_read_physical(
                    &state.machine->executor_memory, 0x1000u,
                    TYPE_REFERENCE_OF(image), 4u) != TYPE_STATUS_OK ||
                    image != 0x11225566u;
            if (form == 1u)
                failed |= after.data.eax != 0xaabbccddu;
            if (form == 2u)
                failed |= after.data.eax != 0xaabbccddu;
            if (form == 3u)
                failed |= after.data.eax != 0x11223344u;
            if (form == 4u)
                failed |= after.data.eax != 0xaabb335au;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT gpr_mov_test_immediate_register_386_attributes(C_VOID)
{
    type_unsigned_8 form;

    for (form = 0u; form != 8u; ++form) {
        gpr_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 code[] = {0x66u, (type_unsigned_8)(0xb0u + form),
            (type_unsigned_8)(0x80u + form)};
        type_unsigned_8 target = form & 3u;
        type_unsigned_32 expected;
        C_INT failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            gpr_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            expected = gpr_mov_gpr(&before, target);
            expected = form < 4u ?
                (expected & 0xffffff00u) | (0x80u + form) :
                (expected & 0xffff00ffu) |
                    ((type_unsigned_32)(0x80u + form) << 8u);
            failed |= !gpr_mov_run(&state, code, sizeof(code), &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
                !gpr_mov_nonparticipants(&before, &after, target) ||
                gpr_mov_gpr(&after, target) != expected;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (form = 0u; form != 8u; ++form) {
        gpr_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 code[] = {0x66u, (type_unsigned_8)(0xb8u + form),
            0x44u, 0x33u, 0x22u, (type_unsigned_8)(0x11u + form)};
        type_unsigned_32 expected = 0x11223344u + ((type_unsigned_32)form << 24u);
        C_INT failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            gpr_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !gpr_mov_run(&state, code, sizeof(code), &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
                !gpr_mov_nonparticipants(&before, &after, form) ||
                gpr_mov_gpr(&after, form) != expected;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT gpr_mov_test_prefix_lock(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 prefix_codes[][7] = {
        {0x66u,0x8bu,0x06u,0,0x10u},
        {0x67u,0x89u,0x06u,0,0x10u},
        {0x66u,0xc7u,0x06u,0,0x10u,0},
        {0x67u,0xb8u,0,0,0},
        {0x66u,0xb0u,0},
        {0x66u,0xb8u,0,0,0}
    };
    static const type_unsigned_8 prefix_sizes[] = {5u,5u,6u,5u,3u,5u};
    static const type_unsigned_8 lock_codes[][7] = {
        {0xf0u,0x88u,0x06u,0,0x10u},
        {0xf0u,0x89u,0x0eu,0,0x10u},
        {0xf0u,0x8au,0x06u,0,0x10u},
        {0xf0u,0x8bu,0x0eu,0,0x10u},
        {0xf0u,0xc6u,0x06u,0,0x10u,0x5au},
        {0xf0u,0xc7u,0x06u,0,0x10u,0x34u,0x12u},
        {0xf0u,0xb0u,0x5au}, {0xf0u,0xb8u,0x34u,0x12u}
    };
    static const type_unsigned_8 lock_sizes[] = {5u,5u,5u,5u,6u,7u,3u,4u};
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile) {
        for (form = 0u; form != sizeof(prefix_sizes); ++form) {
            gpr_mov_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_16 image = 0xbeefu;
            C_INT failed = !gpr_mov_prepare(profiles[profile], &state);

            if (!failed) {
                gpr_mov_seed(&state);
                failed |= core_machine_memory_write(state.machine, 0x1000u,
                    &image, sizeof(image)) != TYPE_STATUS_OK;
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !gpr_mov_run(&state, prefix_codes[form],
                    prefix_sizes[form], &after, &diagnostic, &status) ||
                    status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                    !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_UD) || after.data.eip != before.data.eip ||
                    !gpr_mov_nonparticipants(&before, &after, 8u) ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                        0x1000u, TYPE_REFERENCE_OF(image), sizeof(image)) !=
                        TYPE_STATUS_OK || image != 0xbeefu;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    for (form = 0u; form != sizeof(lock_sizes); ++form) {
        gpr_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_16 image = 0xbeefu;
        C_INT failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            gpr_mov_seed(&state);
            failed |= core_machine_memory_write(state.machine, 0x1000u,
                &image, sizeof(image)) != TYPE_STATUS_OK;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !gpr_mov_run(&state, lock_codes[form], lock_sizes[form],
                &after, &diagnostic, &status) || status != TYPE_STATUS_FAULT ||
                !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != before.data.eip ||
                !gpr_mov_nonparticipants(&before, &after, 8u) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x1000u, TYPE_REFERENCE_OF(image), sizeof(image)) !=
                    TYPE_STATUS_OK || image != 0xbeefu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT gpr_mov_test_segments_and_irq(C_VOID)
{
    static const type_unsigned_8 codes[][5] = {
        {0x8au, 0x06u, 0x10u, 0, 0},
        {0x8au, 0x46u, 0, 0, 0},
        {0x26u, 0x88u, 0x06u, 0x10u, 0},
        {0x64u, 0x8au, 0x06u, 0x10u, 0},
        {0x65u, 0x88u, 0x06u, 0x10u, 0},
        {0x67u, 0x8au, 0x45u, 0, 0}
    };
    static const type_unsigned_8 bytes[] = {4u, 3u, 5u, 5u, 5u, 4u};
    static const type_unsigned_8 values[] = {0x11u, 0x22u, 0, 0x44u, 0, 0x66u};
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(bytes); ++form) {
        gpr_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 image = values[form];
        type_unsigned_32 address = form == 0u ? 0x10u : form == 1u ? 0x110u :
            form == 2u ? 0x110u : form == 3u ? 0x210u :
            form == 4u ? 0x310u : 0x110u;
        C_INT failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);
        if (!failed) {
            if (form == 1u || form == 5u) {
                failed |= core_machine_cpu_execution_load_segment(
                    &state.machine->executor_cpu_execution,
                    &state.machine->executor_cpu.data.ss, 0x10u) != 0;
            }
            if (form == 2u) {
                failed |= core_machine_cpu_execution_load_segment(
                    &state.machine->executor_cpu_execution,
                    &state.machine->executor_cpu.data.es, 0x10u) != 0;
            }
            if (form == 3u) {
                failed |= core_machine_cpu_execution_load_segment(
                    &state.machine->executor_cpu_execution,
                    &state.machine->executor_cpu.data.fs, 0x20u) != 0;
            }
            if (form == 4u) {
                failed |= core_machine_cpu_execution_load_segment(
                    &state.machine->executor_cpu_execution,
                    &state.machine->executor_cpu.data.gs, 0x30u) != 0;
            }
            gpr_mov_seed(&state);
            state.machine->executor_cpu.data.ebp = 0x10u;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed |= core_machine_memory_write(state.machine, address,
                &image, 1u) != TYPE_STATUS_OK ||
                !gpr_mov_run(&state, codes[form], bytes[form], &after,
                    &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != bytes[form] ||
                !gpr_mov_nonparticipants(&before, &after,
                    (form == 2u || form == 4u) ? 8u : 0u);
            if (form == 0u || form == 1u || form == 3u || form == 5u) {
                failed |= after.data.eax !=
                    ((before.data.eax & 0xffffff00u) | values[form]);
            }
            if (form == 2u || form == 4u) {
                image = 0u;
                failed |= core_machine_memory_read_physical(
                    &state.machine->executor_memory, address,
                    TYPE_REFERENCE_OF(image), 1u) != TYPE_STATUS_OK ||
                    image != 0x44u;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT gpr_mov_test_protected_limit(C_VOID)
{
    static const type_unsigned_8 pointer[] = {0x1fu, 0, 0, 0x03u, 0, 0};
    static const type_unsigned_8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0x0fu, 0, 0, 0x30u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x40u, 0, 0x92u, 0, 0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu, 0x01u, 0x16u, 0, 1u, 0xb8u, 1u, 0,
        0x0fu, 0x01u, 0xf0u, 0xb8u, 0x10u, 0, 0x8eu, 0xd8u,
        0x8eu, 0xc0u, 0xb8u, 0x18u, 0, 0x8eu, 0xd0u, 0xbcu,
        0, 0x80u, 0xeau, 0, 0, 8u, 0
    };
    static const type_unsigned_8 halt[] = {0xf4u};
    static const type_unsigned_8 codes[][10] = {
        {0x8au, 0x06u, 0x10u, 0},
        {0x66u, 0xc7u, 0x06u, 0x10u, 0, 0x78u, 0x56u, 0x34u, 0x12u}
    };
    static const type_unsigned_8 bytes[] = {4u, 9u};
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        gpr_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_32 image = 0x11223344u;
        C_INT failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x100u,
                pointer, sizeof(pointer)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x300u, gdt,
                    sizeof(gdt)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0, boot,
                    sizeof(boot)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, halt, 1u) !=
                    TYPE_STATUS_OK || core_machine_run(state.machine,
                    (core_machine_run_budget){96u, 0u}, &result) !=
                    TYPE_STATUS_OK || result.reason !=
                    CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        }
        if (!failed) {
            gpr_mov_seed(&state);
            failed |= core_machine_memory_write(state.machine, 0x3010u,
                &image, 4u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u,
                    codes[form], bytes[form]) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u, 0u}, &result) !=
                TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.edx !=
                before.data.edx || after.data.ebx != before.data.ebx ||
                after.data.esp != before.data.esp || after.data.ebp !=
                before.data.ebp || after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi || after.data.eflags !=
                before.data.eflags || core_machine_memory_read_physical(
                &state.machine->executor_memory, 0x3010u,
                TYPE_REFERENCE_OF(image), 4u) != TYPE_STATUS_OK || image !=
                0x11223344u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT gpr_mov_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 codes[][5] = {
        {0x8au, 0x06u, 0, 0x10u, 0x90u},
        {0x88u, 0x06u, 0, 0x10u, 0x90u}
    };
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        gpr_mov_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_16 offset = 0x100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame = 0u;
        type_unsigned_8 image = form ? 0u : 0x5au;
        C_INT failed = !gpr_mov_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x1000u,
                &image, 1u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, codes[form],
                    sizeof(codes[form])) !=
                TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                    0x80u, &offset, 2u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment, 2u) !=
                TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                    0x100u, &hlt, 1u) != TYPE_STATUS_OK;
        }
        if (!failed) {
            gpr_mov_seed(&state);
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u, 0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(
                &state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame), 2u) != TYPE_STATUS_OK ||
                after.data.eip != 0x101u || frame != 4u || !TYPE_GET_BIT(
                    state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
                TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                    VPIC_IRR_IRQ(0u)) || (form == 0u && after.data.al != 0x5au) ||
                (form == 1u && (core_machine_memory_read_physical(
                    &state.machine->executor_memory, 0x1000u,
                    TYPE_REFERENCE_OF(image), 1u) != TYPE_STATUS_OK ||
                    image != 0x44u));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!gpr_mov_test_defaults()) {
        STD_PRINTF("GPR-MOV stage=defaults\n");
        return 1;
    }
    if (!gpr_mov_test_immediate_and_reject()) {
        STD_PRINTF("GPR-MOV stage=immediate\n");
        return 1;
    }
    if (!gpr_mov_test_386_attributes() ||
        !gpr_mov_test_immediate_register_386_attributes()) {
        STD_PRINTF("GPR-MOV stage=attributes\n");
        return 1;
    }
    if (!gpr_mov_test_prefix_lock()) {
        STD_PRINTF("GPR-MOV stage=reject\n");
        return 1;
    }
    if (!gpr_mov_test_segments_and_irq()) {
        STD_PRINTF("GPR-MOV stage=segments\n");
        return 1;
    }
    if (!gpr_mov_test_protected_limit()) {
        STD_PRINTF("GPR-MOV stage=limit\n");
        return 1;
    }
    if (!gpr_mov_test_irq_no_shadow()) {
        STD_PRINTF("GPR-MOV stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S31:GPR-MOV:OK\n");
    STD_PRINTF("M5:T401:S13:IMMEDIATE-REGISTER-MOV-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S47:GPR-MOV-MODRM-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S58:RM-IMMEDIATE-MOV-PROFILES:OK\n");
    return 0;
}
