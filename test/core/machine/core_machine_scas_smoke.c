#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define SCAS_CMP_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

typedef struct scas_machine {
    core_machine *machine;
} scas_machine;

static C_VOID scas_reset(C_VOID *opaque)
{
    scas_machine *state = (scas_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider scas_provider = {
    scas_reset, STD_NULL
};

static C_INT scas_prepare(core_machine_cpu_profile profile, scas_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &scas_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID scas_seed(scas_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xaabb0010u;
    cpu->data.ecx = 0x11220003u;
    cpu->data.edx = 0x778899aau;
    cpu->data.ebx = 0xbbccddeeU;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF;
    cpu->data.cs.base = 0u;
    cpu->data.es.base = 0x20000u;
    cpu->data.fs.base = 0x40000u;
}

static C_INT scas_nonparticipants_same(const t_cpu *before, const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi;
}

static type_unsigned_32 scas_real_flags_known_mask(
    core_machine_cpu_profile profile)
{
    if (profile < CORE_MACHINE_CPU_PROFILE_80286) return 0x0fd5u;
    if (profile == CORE_MACHINE_CPU_PROFILE_80286) return 0x7fd5u;
    return 0xffd5u;
}

static C_INT scas_run(scas_machine *state, const type_unsigned_8 *code, type_unsigned_8 bytes,
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

static C_INT scas_single_case(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width, C_INT address32,
    C_INT decrement, type_unsigned_32 physical)
{
    scas_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 image = width == 4u ? 0xaabb0001u : 1u;
    type_unsigned_32 observed;
    type_unsigned_32 index = address32 ? 0x1020u : 0x20u;
    type_unsigned_32 expected_index = index + (decrement ? -(type_signed_32)width : width);
    C_INT failed = !scas_prepare(profile, &state);

    if (!failed) {
        scas_seed(&state);
        if (address32)
            state.machine->executor_cpu.data.edi = index;
        else
            state.machine->executor_cpu.data.edi =
                (state.machine->executor_cpu.data.edi & 0xffff0000u) | index;
        if (decrement)
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
        failed |= core_machine_memory_write(state.machine, physical, &image,
            width) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !scas_run(&state, code, bytes, 1u, &after, &diagnostic,
            &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            !scas_nonparticipants_same(&before, &after) ||
            after.data.ecx != before.data.ecx ||
            after.data.edi != (address32 ? expected_index :
            (before.data.edi & 0xffff0000u) | (type_unsigned_16)expected_index) ||
            (after.data.eflags & SCAS_CMP_FLAGS) !=
            (VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) ||
            (after.data.eflags & (scas_real_flags_known_mask(profile) &
            ~SCAS_CMP_FLAGS)) != (before.data.eflags &
            (scas_real_flags_known_mask(profile) & ~SCAS_CMP_FLAGS)) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            physical, TYPE_REFERENCE_OF(observed), width) != TYPE_STATUS_OK ||
            (width == 1u ? (observed & 0xffu) != 1u : width == 2u ?
            (observed & 0xffffu) != 1u : observed != 0xaabb0001u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT scas_test_single(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 scasb = 0xaeu;
    static const type_unsigned_8 scasw = 0xafu;
    static const type_unsigned_8 scasd[] = {0x66u, 0xafu};
    static const type_unsigned_8 address32[] = {0x67u, 0xaeu};
    static const type_unsigned_8 combined[] = {0x66u, 0x67u, 0xafu};
    static const type_unsigned_8 cs[] = {0x2eu, 0xaeu};
    static const type_unsigned_8 fs[] = {0x64u, 0xaeu};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        if (!scas_single_case(profiles[profile], &scasb, 1u, 1u, 0, 0,
            0x20020u)) {
            STD_PRINTF("SCAS single profile=%u form=AE\n", profile);
            return 0;
        }
        if (!scas_single_case(profiles[profile], &scasw, 1u, 2u, 0, 0,
            0x20020u)) {
            STD_PRINTF("SCAS single profile=%u form=AF\n", profile);
            return 0;
        }
    }
    if (!scas_single_case(CORE_MACHINE_CPU_PROFILE_80386, scasd,
        sizeof(scasd), 4u, 0, 0, 0x20020u)) {
        return 0;
    }
    return scas_single_case(CORE_MACHINE_CPU_PROFILE_80386, address32,
        sizeof(address32), 1u, 1, 0, 0x21020u) &&
        scas_single_case(CORE_MACHINE_CPU_PROFILE_80386, combined,
        sizeof(combined), 4u, 1, 0, 0x21020u) &&
        scas_single_case(CORE_MACHINE_CPU_PROFILE_80386, &scasw, 1u, 2u,
        0, 1, 0x20020u) && scas_single_case(CORE_MACHINE_CPU_PROFILE_80386,
        cs, sizeof(cs), 1u, 0, 0, 0x20020u) &&
        scas_single_case(CORE_MACHINE_CPU_PROFILE_80386, fs, sizeof(fs),
        1u, 0, 0, 0x20020u);
}

static C_INT scas_flag_case(type_unsigned_8 accumulator, type_unsigned_8 image,
    type_unsigned_32 expected_flags)
{
    static const type_unsigned_8 code = 0xaeu;
    scas_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_8 observed;
    C_INT failed = !scas_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        scas_seed(&state);
        state.machine->executor_cpu.data.eax =
            (state.machine->executor_cpu.data.eax & 0xffffff00u) | accumulator;
        failed |= core_machine_memory_write(state.machine, 0x20020u, &image,
            sizeof(image)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !scas_run(&state, &code, sizeof(code), 1u, &after,
            &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != 1u ||
            !scas_nonparticipants_same(&before, &after) ||
            after.data.ecx != before.data.ecx || after.data.edi != 0x21u ||
            (after.data.eflags & SCAS_CMP_FLAGS) != expected_flags ||
            (after.data.eflags & (scas_real_flags_known_mask(
            CORE_MACHINE_CPU_PROFILE_80386) & ~SCAS_CMP_FLAGS)) !=
            (before.data.eflags & (scas_real_flags_known_mask(
            CORE_MACHINE_CPU_PROFILE_80386) & ~SCAS_CMP_FLAGS)) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x20020u, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
            TYPE_STATUS_OK || observed != image;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT scas_test_flags(C_VOID)
{
    return scas_flag_case(0x10u, 0x01u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) &&
        scas_flag_case(0x10u, 0x10u, VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) &&
        scas_flag_case(0x00u, 0x01u, VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_SF) && scas_flag_case(0x80u, 0x01u,
        VCPU_EFLAGS_AF | VCPU_EFLAGS_OF);
}

static C_INT scas_rep_case(core_machine_cpu_profile profile, const type_unsigned_8 *code, type_unsigned_8 bytes, C_INT repz,
    type_unsigned_16 count, const type_unsigned_8 *image, type_unsigned_16 expected_count,
    type_unsigned_16 expected_di, type_unsigned_32 expected_flags)
{
    scas_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_8 observed[3] = {0u, 0u, 0u};
    C_INT failed = !scas_prepare(profile, &state);

    if (!failed) {
        scas_seed(&state);
        state.machine->executor_cpu.data.ecx = 0x11220000u | count;
        failed |= core_machine_memory_write(state.machine, 0x20020u, image,
            3u) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !scas_run(&state, code, bytes, count == 0u ? 1u :
            (type_unsigned_8)(count - expected_count),
            &after, &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            !scas_nonparticipants_same(&before, &after) ||
            after.data.ecx != ((before.data.ecx & 0xffff0000u) | expected_count) ||
            after.data.edi != ((before.data.edi & 0xffff0000u) | expected_di) ||
            (after.data.eflags & SCAS_CMP_FLAGS) != expected_flags ||
            (after.data.eflags & (scas_real_flags_known_mask(profile) &
            ~SCAS_CMP_FLAGS)) != (before.data.eflags &
            (scas_real_flags_known_mask(profile) & ~SCAS_CMP_FLAGS)) ||
            core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x20020u,
            (type_virtual_address)observed, sizeof(observed)) != TYPE_STATUS_OK ||
            STD_MEMCMP(observed, image, sizeof(observed)) != 0;
    }
    core_machine_destroy(state.machine);
    (C_VOID)repz;
    return !failed;
}

static C_INT scas_test_rep(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 repe[] = {0xf3u, 0xaeu};
    static const type_unsigned_8 repne[] = {0xf2u, 0xaeu};
    static const type_unsigned_8 equals[] = {0x10u, 1u, 0x10u};
    static const type_unsigned_8 unequal[] = {1u, 0x10u, 1u};
    static const type_unsigned_8 zero[] = {1u, 1u, 1u};
    static const type_unsigned_8 one[] = {0x10u, 1u, 1u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        if (!scas_rep_case(profiles[profile], repe, sizeof(repe), 1, 0u,
            zero, 0u, 0x20u, 0u) || !scas_rep_case(profiles[profile], repe,
            sizeof(repe), 1, 1u, one, 0u, 0x21u,
            VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) || !scas_rep_case(
            profiles[profile], repne, sizeof(repne), 0, 0u, zero, 0u, 0x20u,
            0u) || !scas_rep_case(profiles[profile], repne, sizeof(repne),
            0, 1u, unequal, 0u, 0x21u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) ||
            !scas_rep_case(profiles[profile], repe, sizeof(repe), 1, 3u,
            equals, 1u, 0x22u, VCPU_EFLAGS_PF | VCPU_EFLAGS_AF) ||
            !scas_rep_case(profiles[profile], repne, sizeof(repne), 0, 3u,
            unequal, 1u, 0x22u, VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF)) {
            STD_PRINTF("SCAS rep profile=%u\n", profile);
            return 0;
        }
    }
    return 1;
}

static C_INT scas_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    scas_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 image = 0x11223344u;
    type_unsigned_32 observed;
    C_INT failed = !scas_prepare(profile, &state);

    if (!failed) {
        scas_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x20020u, &image,
            sizeof(image)) != TYPE_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !scas_run(&state, code, bytes, 1u, &after, &diagnostic,
            &status, &result) || status != TYPE_STATUS_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || STD_MEMCMP(&before.data.es, &after.data.es,
            sizeof(before.data.es)) != 0 || core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x20020u,
            TYPE_REFERENCE_OF(observed), sizeof(observed)) != TYPE_STATUS_OK ||
            observed != image;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT scas_test_rejections(C_VOID)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 attr[][4] = {
        {0x66u, 0xaeu, 0u, 0u}, {0x67u, 0xafu, 0u, 0u},
        {0x66u, 0x67u, 0xafu, 0u}, {0xf3u, 0x66u, 0xaeu, 0u},
        {0xf2u, 0x67u, 0xafu, 0u}, {0xf3u, 0x66u, 0x67u, 0xafu}
    };
    static const type_unsigned_8 attr_bytes[] = {2u, 2u, 3u, 3u, 3u, 4u};
    static const type_unsigned_8 lock[][5] = {
        {0xf0u, 0xaeu, 0u, 0u, 0u}, {0xf0u, 0xafu, 0u, 0u, 0u},
        {0xf0u, 0xf3u, 0xaeu, 0u, 0u}, {0xf0u, 0xf2u, 0xafu, 0u, 0u},
        {0xf0u, 0x66u, 0xafu, 0u, 0u}, {0xf0u, 0x67u, 0xaeu, 0u, 0u},
        {0xf0u, 0x66u, 0x67u, 0xafu, 0u},
        {0xf0u, 0xf3u, 0x66u, 0x67u, 0xafu}
    };
    static const type_unsigned_8 lock_bytes[] = {2u, 2u, 3u, 3u, 3u, 3u, 4u, 5u};
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
        for (form = 0u; form != sizeof(attr_bytes); ++form)
            if (!scas_expect_ud(legacy[profile], attr[form], attr_bytes[form]))
                return 0;
    for (form = 0u; form != sizeof(lock_bytes); ++form)
        if (!scas_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock[form],
            lock_bytes[form]))
            return 0;
    return 1;
}

static C_INT scas_boot_protected(scas_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x18u,0,0x8eu,0xd8u, 0xb8u,0x10u,0,0x8eu,0xc0u,
        0xb8u,0x18u,0,0x8eu,0xd0u,0xbcu,0,0x80u, 0xeau,0,0,8u,0
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

static C_INT scas_test_protected_limit(C_VOID)
{
    static const type_unsigned_8 code[] = {0xaeu};
    scas_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_8 image = 1u;
    type_unsigned_8 observed;
    C_INT failed = !scas_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !scas_boot_protected(&state);
    if (!failed) {
        t_cpu_data_sreg cs = state.machine->executor_cpu.data.cs;
        t_cpu_data_sreg ds = state.machine->executor_cpu.data.ds;
        t_cpu_data_sreg es = state.machine->executor_cpu.data.es;
        t_cpu_data_sreg ss = state.machine->executor_cpu.data.ss;

        scas_seed(&state);
        state.machine->executor_cpu.data.cs = cs;
        state.machine->executor_cpu.data.ds = ds;
        state.machine->executor_cpu.data.es = es;
        state.machine->executor_cpu.data.ss = ss;
        state.machine->executor_cpu.data.edi = 0x10u;
        failed |= core_machine_memory_write(state.machine, 0x3010u, &image,
            sizeof(image)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, code, sizeof(code)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u, 0u}, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT || core_machine_get_cpu_diagnostic(
            state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || STD_MEMCMP(&before.data.es, &after.data.es,
            sizeof(before.data.es)) != 0 || core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x3010u,
            TYPE_REFERENCE_OF(observed), sizeof(observed)) != TYPE_STATUS_OK ||
            observed != image;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT scas_test_protected_rep_limit(C_VOID)
{
    static const type_unsigned_8 code[] = {0xf3u, 0xaeu};
    scas_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_8 first = 0x10u;
    type_unsigned_8 second = 1u;
    type_unsigned_8 observed;
    C_INT failed = !scas_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !scas_boot_protected(&state);
    if (!failed) {
        t_cpu_data_sreg cs = state.machine->executor_cpu.data.cs;
        t_cpu_data_sreg ds = state.machine->executor_cpu.data.ds;
        t_cpu_data_sreg es = state.machine->executor_cpu.data.es;
        t_cpu_data_sreg ss = state.machine->executor_cpu.data.ss;

        scas_seed(&state);
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
            after.data.ecx != 0x11220002u || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != 0x11u || after.data.eflags !=
            (VCPU_EFLAGS_IF | VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF) ||
            STD_MEMCMP(&before.data.es, &after.data.es, sizeof(before.data.es)) != 0 ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x3010u, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
            TYPE_STATUS_OK || observed != first || core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x3011u,
            TYPE_REFERENCE_OF(observed), sizeof(observed)) != TYPE_STATUS_OK ||
            observed != second;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT scas_test_irq(C_VOID)
{
    static const type_unsigned_8 single[] = {0xaeu, 0x90u};
    static const type_unsigned_8 repeat[] = {0xf3u, 0xaeu, 0x90u};
    static const type_unsigned_8 hlt = 0xf4u;
    scas_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    t_cpu after;
    type_unsigned_16 offset = 0x100u;
    type_unsigned_16 segment = 0u;
    type_unsigned_16 frame_ip = 0u;
    type_unsigned_8 image[] = {0x10u, 1u, 1u};
    C_INT failed = !scas_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x20020u, image,
            sizeof(image)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0u, single, sizeof(single)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x80u, &offset,
            sizeof(offset)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x82u, &segment, sizeof(segment)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x100u, &hlt, sizeof(hlt)) !=
            TYPE_STATUS_OK;
    }
    if (!failed) {
        scas_seed(&state);
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        STD_MEMSET(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){2u, 0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
            after.data.eip != 0x101u || frame_ip != 1u || after.data.edi != 0x21u ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !scas_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x20020u, image,
            sizeof(image)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0u, repeat, sizeof(repeat)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x80u, &offset,
            sizeof(offset)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x82u, &segment, sizeof(segment)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x100u, &hlt, sizeof(hlt)) !=
            TYPE_STATUS_OK;
    }
    if (!failed) {
        scas_seed(&state);
        state.machine->executor_cpu.data.ecx = 0x11220003u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        STD_MEMSET(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){4u, 0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
            after.data.eip != 0x101u || frame_ip != 0u ||
            after.data.ecx != 0x11220002u || after.data.edi != 0x21u ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!scas_test_single()) {
        STD_PRINTF("SCAS stage=single\n");
        return 1;
    }
    if (!scas_test_flags())
        return 1;
    if (!scas_test_rep()) {
        STD_PRINTF("SCAS stage=rep\n");
        return 1;
    }
    if (!scas_test_rejections())
        return 1;
    if (!scas_test_protected_limit()) {
        STD_PRINTF("SCAS stage=protected\n");
        return 1;
    }
    if (!scas_test_protected_rep_limit())
        return 1;
    if (!scas_test_irq())
        return 1;
    STD_PRINTF("M5:T316:S36:SCAS:OK\n");
    STD_PRINTF("M5:T401:S19:SCAS-PROFILES:OK\n");
    return 0;
}
