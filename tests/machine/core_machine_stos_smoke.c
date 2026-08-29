#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct stos_machine {
    core_machine *machine;
} stos_machine;

static C_VOID stos_reset(C_VOID *opaque)
{
    stos_machine *state = (stos_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider stos_provider = {
    stos_reset, STD_NULL
};

static C_INT stos_prepare(core_machine_cpu_profile profile, stos_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &stos_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID stos_seed(stos_machine *state)
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
    cpu->data.cs.selector = 0x0000u;
    cpu->data.cs.base = 0x00000u;
    cpu->data.es.selector = 0x2000u;
    cpu->data.es.base = 0x20000u;
    cpu->data.fs.selector = 0x4000u;
    cpu->data.fs.base = 0x40000u;
}

static C_INT stos_boot_protected(stos_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x18u,0,0x8eu,0xd8u, 0xb8u,0x10u,0,0x8eu,0xc0u,
        0xb8u,0x18u,0,0x8eu,0xd0u, 0xbcu,0,0x80u,
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

static C_INT stos_run(stos_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes,
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

static C_INT stos_nonindexes_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.eflags == after->data.eflags;
}

static C_INT stos_case(core_machine_cpu_profile profile, const type_unsigned_8 *code,
    type_unsigned_8 bytes, type_unsigned_8 width, C_INT address32, C_INT decrement,
    type_unsigned_32 destination)
{
    stos_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_status status;
    type_unsigned_32 image = 0xa5a5a5a5u;
    type_unsigned_32 expected = width == 1u ? 0xa5a5a544u :
        width == 2u ? 0xa5a53344u : 0xaabb3344u;
    type_unsigned_32 index = address32 ? 0x00001020u : 0x00000020u;
    C_INT failed = !stos_prepare(profile, &state);

    if (!failed) {
        stos_seed(&state);
        if (decrement) state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
        if (address32)
            state.machine->executor_cpu.data.edi = index;
        else
            state.machine->executor_cpu.data.edi =
                (state.machine->executor_cpu.data.edi & 0xffff0000u) | index;
        failed |= core_machine_memory_write(state.machine, destination, &image,
            width) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !stos_run(&state, code, bytes, &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != bytes || !stos_nonindexes_same(&before, &after) ||
            after.data.edi != (address32 ? index + (decrement ?
            -(type_signed_32)width : width) : ((before.data.edi & 0xffff0000u) |
            (type_unsigned_16)(index + (decrement ? -(type_signed_32)width : width)))) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            destination, TYPE_REFERENCE_OF(image), width) != TYPE_STATUS_OK ||
            image != expected;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT stos_test_defaults(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386};
    static const type_unsigned_8 codes[] = {0xaau, 0xabu};
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile)
        for (form = 0u; form != sizeof(codes); ++form)
            if (!stos_case(profiles[profile], &codes[form], 1u,
                form == 0u ? 1u : 2u, 0, 0, 0x20020u)) return 0;
    return 1;
}

static C_INT stos_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 stosd[] = {0x66u, 0xabu};
    static const type_unsigned_8 address32[] = {0x67u, 0xaau};
    static const type_unsigned_8 combined[] = {0x66u, 0x67u, 0xabu};
    static const type_unsigned_8 cs_override[] = {0x2eu, 0xaau};
    static const type_unsigned_8 fs_override[] = {0x64u, 0xaau};

    return stos_case(CORE_MACHINE_CPU_PROFILE_80386, stosd, sizeof(stosd),
        4u, 0, 0, 0x20020u) && stos_case(CORE_MACHINE_CPU_PROFILE_80386,
        address32, sizeof(address32), 1u, 1, 0, 0x21020u) &&
        stos_case(CORE_MACHINE_CPU_PROFILE_80386, combined, sizeof(combined),
        4u, 1, 0, 0x21020u) && stos_case(CORE_MACHINE_CPU_PROFILE_80386,
        cs_override, sizeof(cs_override), 1u, 0, 0, 0x20020u) &&
        stos_case(CORE_MACHINE_CPU_PROFILE_80386, fs_override, sizeof(fs_override),
        1u, 0, 0, 0x20020u) && stos_case(CORE_MACHINE_CPU_PROFILE_80386,
        (type_unsigned_8[]){0xabu}, 1u, 2u, 0, 1, 0x20020u);
}

static type_unsigned_32 stos_replace_low(type_unsigned_32 original, type_unsigned_8 width)
{
    if (width == 1u) return (original & 0xffffff00u) | 0x44u;
    if (width == 2u) return (original & 0xffff0000u) | 0x3344u;
    return 0xaabb3344u;
}

static C_INT stos_rep_case(core_machine_cpu_profile profile, const type_unsigned_8 *code,
    type_unsigned_8 bytes, type_unsigned_8 width, C_INT address32, type_unsigned_8 count, C_INT decrement)
{
    stos_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 image = 0xa5a5a5a5u;
    type_unsigned_32 observed;
    type_unsigned_32 index = address32 ? 0x1020u : 0x20u;
    type_unsigned_8 slots = count == 0u ? 1u : count;
    type_unsigned_8 item;
    C_INT failed = !stos_prepare(profile, &state);

    if (!failed) {
        stos_seed(&state);
        if (decrement) {
            index += (count - 1u) * width;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
        }
        if (address32) state.machine->executor_cpu.data.edi = index;
        else state.machine->executor_cpu.data.edi =
            (state.machine->executor_cpu.data.edi & 0xffff0000u) | index;
        state.machine->executor_cpu.data.ecx = address32 ? count :
            (state.machine->executor_cpu.data.ecx & 0xffff0000u) | count;
        for (item = 0u; item != slots; ++item) {
            type_unsigned_32 location = 0x20000u + index - (decrement ?
                (count - 1u) * width : 0u) + item * width;
            failed |= core_machine_memory_write(state.machine, location, &image,
                width) != TYPE_STATUS_OK;
        }
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine, 0u, code, bytes) !=
            TYPE_STATUS_OK;
        status = core_machine_run(state.machine,
            (core_machine_run_budget){count == 0u ? 1u : count, 0u}, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != bytes || after.data.eax != before.data.eax ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.eflags != before.data.eflags ||
            after.data.ecx != (address32 ? 0u : (before.data.ecx & 0xffff0000u)) ||
            after.data.edi != (address32 ? index + (decrement ? -(type_signed_32)(count * width) : count * width) :
            ((before.data.edi & 0xffff0000u) | (type_unsigned_16)(index + (decrement ? -(type_signed_32)(count * width) : count * width))));
        for (item = 0u; !failed && item != slots; ++item) {
            type_unsigned_32 location = 0x20000u + index - (decrement ?
                (count - 1u) * width : 0u) + item * width;
            observed = image;
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                location, TYPE_REFERENCE_OF(observed), width) != TYPE_STATUS_OK ||
                observed != (count == 0u ? image : stos_replace_low(image, width));
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT stos_test_rep(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386};
    static const type_unsigned_8 repb[] = {0xf3u, 0xaau};
    static const type_unsigned_8 repw[] = {0xf3u, 0xabu};
    static const type_unsigned_8 repd[] = {0xf3u, 0x66u, 0xabu};
    static const type_unsigned_8 rep67[] = {0xf3u, 0x67u, 0xaau};
    static const type_unsigned_8 rep66_67[] = {0xf3u, 0x66u, 0x67u, 0xabu};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
        if (!stos_rep_case(profiles[profile], repb, 2u, 1u, 0, 0u, 0) ||
            !stos_rep_case(profiles[profile], repb, 2u, 1u, 0, 1u, 0) ||
            !stos_rep_case(profiles[profile], repw, 2u, 2u, 0, 3u, 0)) return 0;
    return stos_rep_case(CORE_MACHINE_CPU_PROFILE_80386, repd, 3u, 4u, 0, 3u, 0) &&
        stos_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep67, 3u, 1u, 1, 3u, 0) &&
        stos_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep66_67, 4u, 4u, 1, 3u, 0) &&
        stos_rep_case(CORE_MACHINE_CPU_PROFILE_80386, repw, 2u, 2u, 0, 3u, 1);
}

static C_INT stos_test_rejections(C_VOID)
{
    static const type_unsigned_8 codes[][5] = {
        {0x66u,0xaau,0,0,0}, {0x67u,0xabu,0,0,0},
        {0x66u,0x67u,0xabu,0,0}, {0xf0u,0xaau,0,0,0},
        {0xf0u,0xf3u,0xabu,0,0}, {0xf0u,0x66u,0xabu,0,0},
        {0xf0u,0x67u,0xaau,0,0}, {0xf0u,0x66u,0x67u,0xabu,0},
        {0xf0u,0xf3u,0x66u,0xabu,0}, {0xf0u,0xf3u,0x67u,0xaau,0},
        {0xf0u,0xf3u,0x66u,0x67u,0xabu}
    };
    static const type_unsigned_8 bytes[] = {2u,2u,3u,2u,3u,3u,3u,4u,4u,4u,5u};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286};
    type_unsigned_8 form;
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile) {
        for (form = 0u; form != 3u; ++form) {
            stos_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_16 image = 0xa5a5u;
            C_INT failed = !stos_prepare(legacy[profile], &state);

            if (!failed) {
                stos_seed(&state);
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= core_machine_memory_write(state.machine, 0x20020u, &image,
                    sizeof(image)) != TYPE_STATUS_OK || !stos_run(&state,
                    codes[form], bytes[form], &after, &diagnostic, &status) ||
                    status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                    !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD) || after.data.eip != 0u ||
                    !stos_nonindexes_same(&before, &after) || after.data.edi !=
                    before.data.edi || core_machine_memory_read_physical(
                    &state.machine->executor_memory, 0x20020u,
                    TYPE_REFERENCE_OF(image), sizeof(image)) != TYPE_STATUS_OK ||
                    image != 0xa5a5u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }

    for (form = 3u; form != sizeof(bytes); ++form) {
        stos_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_16 image = 0xa5a5u;
        C_INT failed = !stos_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            stos_seed(&state);
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine, 0x20020u, &image,
                sizeof(image)) != TYPE_STATUS_OK || !stos_run(&state, codes[form],
                bytes[form], &after, &diagnostic, &status) ||
                status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD) || after.data.eip != 0u ||
                !stos_nonindexes_same(&before, &after) || after.data.edi !=
                before.data.edi || core_machine_memory_read_physical(
                &state.machine->executor_memory, 0x20020u, TYPE_REFERENCE_OF(image),
                sizeof(image)) != TYPE_STATUS_OK || image != 0xa5a5u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT stos_test_protected_single_limit(C_VOID)
{
    static const type_unsigned_8 codes[][2] = {{0xaau, 0u}, {0x66u, 0xabu}};
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        stos_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu_data_sreg cs;
        t_cpu_data_sreg ds;
        t_cpu_data_sreg es;
        t_cpu_data_sreg ss;
        type_unsigned_32 image = 0xa5a5a5a5u;
        type_unsigned_8 bytes = form == 0u ? 1u : 2u;
        type_unsigned_8 width = form == 0u ? 1u : 4u;
        C_INT failed = !stos_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) failed |= !stos_boot_protected(&state);
        if (!failed) {
            cs = state.machine->executor_cpu.data.cs;
            ds = state.machine->executor_cpu.data.ds;
            es = state.machine->executor_cpu.data.es;
            ss = state.machine->executor_cpu.data.ss;
            stos_seed(&state);
            state.machine->executor_cpu.data.cs = cs;
            state.machine->executor_cpu.data.ds = ds;
            state.machine->executor_cpu.data.es = es;
            state.machine->executor_cpu.data.ss = ss;
            state.machine->executor_cpu.data.edi = 0x10u;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &image,
                width) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x2000u, codes[form], bytes) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u, 0u}, &result) != TYPE_STATUS_FAULT ||
                result.reason != CORE_MACHINE_STOP_FAULT || core_machine_get_cpu_diagnostic(
                state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || !stos_nonindexes_same(&before, &after) ||
                after.data.edi != before.data.edi || STD_MEMCMP(&before.data.es,
                &after.data.es, sizeof(before.data.es)) != 0 ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x3010u, TYPE_REFERENCE_OF(image), width) != TYPE_STATUS_OK ||
                image != 0xa5a5a5a5u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT stos_test_protected_rep_limit(C_VOID)
{
    static const type_unsigned_8 code[] = {0xf3u, 0xaau};
    stos_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu_data_sreg cs;
    t_cpu_data_sreg ds;
    t_cpu_data_sreg es;
    t_cpu_data_sreg ss;
    type_unsigned_8 first = 0xa5u;
    type_unsigned_8 second = 0xa5u;
    C_INT failed = !stos_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) failed |= !stos_boot_protected(&state);
    if (!failed) {
        cs = state.machine->executor_cpu.data.cs;
        ds = state.machine->executor_cpu.data.ds;
        es = state.machine->executor_cpu.data.es;
        ss = state.machine->executor_cpu.data.ss;
        stos_seed(&state);
        state.machine->executor_cpu.data.cs = cs;
        state.machine->executor_cpu.data.ds = ds;
        state.machine->executor_cpu.data.es = es;
        state.machine->executor_cpu.data.ss = ss;
        state.machine->executor_cpu.data.es.limit = 0x10u;
        state.machine->executor_cpu.data.edi = 0x10u;
        state.machine->executor_cpu.data.ecx = 0x11220003u;
        failed |= core_machine_memory_write(state.machine, 0x3010u, &first,
            sizeof(first)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x3011u, &second, sizeof(second)) != TYPE_STATUS_OK ||
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
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.eflags != before.data.eflags ||
            after.data.ecx != 0x11220002u || after.data.edi != 0x11u ||
            STD_MEMCMP(&before.data.es, &after.data.es, sizeof(before.data.es)) != 0 ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x3010u, TYPE_REFERENCE_OF(first), sizeof(first)) != TYPE_STATUS_OK ||
            first != 0x44u || core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x3011u, TYPE_REFERENCE_OF(second),
            sizeof(second)) != TYPE_STATUS_OK || second != 0xa5u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT stos_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 codes[][2] = {{0xaau, 0x90u}, {0xabu, 0x90u}};
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        stos_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_16 offset = 0x100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_16 image = 0xa5a5u;
        type_unsigned_8 width = form == 0u ? 1u : 2u;
        C_INT failed = !stos_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u) || core_machine_memory_write(state.machine,
                0x20020u, &image, width) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, codes[form],
                sizeof(codes[form])) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x80u, &offset,
                sizeof(offset)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x82u, &segment, sizeof(segment)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &hlt,
                sizeof(hlt)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            stos_seed(&state);
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u, 0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != 1u ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                after.data.edi != 0x20u + width || core_machine_memory_read_physical(
                &state.machine->executor_memory, 0x20020u, TYPE_REFERENCE_OF(image),
                width) != TYPE_STATUS_OK || image != (form == 0u ? 0xa544u :
                0x3344u);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT stos_test_irq_rep_restart(C_VOID)
{
    static const type_unsigned_8 code[] = {0xf3u, 0xaau, 0x90u};
    static const type_unsigned_8 hlt = 0xf4u;
    stos_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_16 offset = 0x100u;
    type_unsigned_16 segment = 0u;
    type_unsigned_16 frame_ip = 0u;
    type_unsigned_8 image[] = {0xa5u, 0xa5u, 0xa5u};
    C_INT failed = !stos_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u) || core_machine_memory_write(state.machine,
            0x20020u, image, sizeof(image)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0u, code, sizeof(code)) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x80u,
            &offset, sizeof(offset)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x82u, &segment, sizeof(segment)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x100u, &hlt,
            sizeof(hlt)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        stos_seed(&state);
        state.machine->executor_cpu.data.ecx = 0x11220003u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){4u, 0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp, TYPE_REFERENCE_OF(frame_ip),
            sizeof(frame_ip)) != TYPE_STATUS_OK || after.data.eip != 0x101u ||
            frame_ip != 0u || after.data.ecx != 0x11220002u ||
            after.data.edi != 0x21u || !TYPE_GET_BIT(
            state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u)) || core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x20020u, (type_virtual_address)image,
            sizeof(image)) != TYPE_STATUS_OK || image[0] != 0x44u ||
            image[1] != 0xa5u || image[2] != 0xa5u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!stos_test_defaults()) {
        STD_PRINTF("STOS stage=defaults\n");
        return 1;
    }
    if (!stos_test_386_attributes()) {
        STD_PRINTF("STOS stage=attributes\n");
        return 1;
    }
    if (!stos_test_rep()) {
        STD_PRINTF("STOS stage=rep\n");
        return 1;
    }
    if (!stos_test_rejections()) {
        STD_PRINTF("STOS stage=rejections\n");
        return 1;
    }
    if (!stos_test_protected_single_limit()) {
        STD_PRINTF("STOS stage=protected-single\n");
        return 1;
    }
    if (!stos_test_protected_rep_limit()) {
        STD_PRINTF("STOS stage=protected-rep\n");
        return 1;
    }
    if (!stos_test_irq_no_shadow()) {
        STD_PRINTF("STOS stage=irq\n");
        return 1;
    }
    if (!stos_test_irq_rep_restart()) {
        STD_PRINTF("STOS stage=irq-rep\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S34:STOS:OK\n");
    STD_PRINTF("M5:T401:S17:STOS-PROFILES:OK\n");
    return 0;
}
