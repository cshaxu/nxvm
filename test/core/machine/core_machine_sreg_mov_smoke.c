#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct sreg_mov_machine { core_machine *machine; } sreg_mov_machine;

static C_VOID sreg_mov_reset(C_VOID *opaque)
{
    sreg_mov_machine *state = (sreg_mov_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider sreg_mov_provider = {
    sreg_mov_reset, STD_NULL
};

static C_INT sreg_mov_prepare(sreg_mov_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &sreg_mov_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID sreg_mov_seed(sreg_mov_machine *state)
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
    state->machine->executor_cpu.data.es.selector = 0x1111u;
    state->machine->executor_cpu.data.es.base = 0x11110u;
    state->machine->executor_cpu.data.ss.selector = 0x2222u;
    state->machine->executor_cpu.data.ss.base = 0x22220u;
    state->machine->executor_cpu.data.ds.selector = 0x3333u;
    state->machine->executor_cpu.data.ds.base = 0x33330u;
    state->machine->executor_cpu.data.fs.selector = 0x4444u;
    state->machine->executor_cpu.data.fs.base = 0x44440u;
    state->machine->executor_cpu.data.gs.selector = 0x5555u;
    state->machine->executor_cpu.data.gs.base = 0x55550u;
}

static C_INT sreg_mov_run(sreg_mov_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    type_status *status)
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

static const t_cpu_data_sreg *sreg_mov_sreg(const t_cpu *cpu, type_unsigned_8 index)
{
    switch (index) {
    case 0u: return &cpu->data.es;
    case 1u: return &cpu->data.cs;
    case 2u: return &cpu->data.ss;
    case 3u: return &cpu->data.ds;
    case 4u: return &cpu->data.fs;
    default: return &cpu->data.gs;
    }
}

static C_INT sreg_mov_gprs_same(const t_cpu *before, const t_cpu *after,
    type_unsigned_8 changed)
{
    return before->data.eflags == after->data.eflags &&
        (changed == 0u || before->data.eax == after->data.eax) &&
        (changed == 1u || before->data.ecx == after->data.ecx) &&
        (changed == 2u || before->data.edx == after->data.edx) &&
        (changed == 3u || before->data.ebx == after->data.ebx) &&
        (changed == 4u || before->data.esp == after->data.esp) &&
        (changed == 5u || before->data.ebp == after->data.ebp) &&
        (changed == 6u || before->data.esi == after->data.esi) &&
        (changed == 7u || before->data.edi == after->data.edi);
}

static C_INT sreg_mov_all_same(const t_cpu *before, const t_cpu *after)
{
    return sreg_mov_gprs_same(before, after, 8u) &&
        before->data.eip == after->data.eip &&
        STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT sreg_mov_expect_ud(sreg_mov_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, const t_cpu *before)
{
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_status status;
    C_INT passed;

    passed = test_core_machine_fixture_preflight_real_ud_terminal(state->machine) &&
        sreg_mov_run(state, code, bytes, &after, &diagnostic, &status) &&
        status == TYPE_STATUS_FAULT && diagnostic.first_fault.valid &&
        TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) &&
        sreg_mov_all_same(before, &after);
    return passed;
}

static C_INT sreg_mov_test_real_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 sreg;
    C_INT failed = 0;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile) {
        for (sreg = 0u; sreg != 4u; ++sreg) {
            const type_unsigned_8 store_register[] = {0x8cu,
                (type_unsigned_8)(0xc0u | (sreg << 3u))};
            const type_unsigned_8 store_memory[] = {0x8cu,
                (type_unsigned_8)(0x06u | (sreg << 3u)),0x00u,0x10u};
            const type_unsigned_8 load_register[] = {0x8eu,
                (type_unsigned_8)(0xc0u | (sreg << 3u))};
            const type_unsigned_8 load_memory[] = {0x8eu,
                (type_unsigned_8)(0x06u | (sreg << 3u)),0x00u,0x10u};
            sreg_mov_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_16 image = 0xbe5au;
            const t_cpu_data_sreg *source;

            if (!sreg_mov_prepare(&state, profiles[profile])) return 0;
            sreg_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            source = sreg_mov_sreg(&before, sreg);
            failed |= !sreg_mov_run(&state, store_register,
                sizeof(store_register), &after, &diagnostic, &status) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(store_register) ||
                !sreg_mov_gprs_same(&before, &after, 0u) ||
                after.data.eax != ((before.data.eax & 0xffff0000u) |
                    source->selector);
            core_machine_destroy(state.machine);

            if (!sreg_mov_prepare(&state, profiles[profile])) return 0;
            sreg_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine,
                before.data.ds.base + 0x1000u, &image,
                sizeof(image)) != TYPE_STATUS_OK || !sreg_mov_run(&state,
                store_memory, sizeof(store_memory), &after, &diagnostic, &status) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(store_memory) ||
                !sreg_mov_gprs_same(&before, &after, 8u) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    before.data.ds.base + 0x1000u, TYPE_REFERENCE_OF(image),
                    sizeof(image)) !=
                    TYPE_STATUS_OK || image != sreg_mov_sreg(&before, sreg)->selector;
            core_machine_destroy(state.machine);

            if (sreg == 1u) continue;
            if (!sreg_mov_prepare(&state, profiles[profile])) return 0;
            sreg_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !sreg_mov_run(&state, load_register,
                sizeof(load_register), &after, &diagnostic, &status) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(load_register) ||
                !sreg_mov_gprs_same(&before, &after, 8u) ||
                sreg_mov_sreg(&after, sreg)->selector != 0x3344u ||
                sreg_mov_sreg(&after, sreg)->base != 0x33440u;
            core_machine_destroy(state.machine);

            if (!sreg_mov_prepare(&state, profiles[profile])) return 0;
            sreg_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine,
                before.data.ds.base + 0x1000u, &image,
                sizeof(image)) != TYPE_STATUS_OK || !sreg_mov_run(&state,
                load_memory, sizeof(load_memory), &after, &diagnostic, &status) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(load_memory) ||
                !sreg_mov_gprs_same(&before, &after, 8u) ||
                sreg_mov_sreg(&after, sreg)->selector != image ||
                sreg_mov_sreg(&after, sreg)->base != ((type_unsigned_32)image << 4u);
            core_machine_destroy(state.machine);
        }
    }
    return !failed;
}

static C_INT sreg_mov_test_386_extensions(C_VOID)
{
    type_unsigned_8 sreg;
    C_INT failed = 0;

    for (sreg = 4u; sreg != 6u; ++sreg) {
        const type_unsigned_8 store[] = {0x8cu, (type_unsigned_8)(0xc0u | (sreg << 3u))};
        const type_unsigned_8 store_memory[] = {0x8cu,
            (type_unsigned_8)(0x06u | (sreg << 3u)), 0x00u, 0x10u};
        const type_unsigned_8 load[] = {0x8eu, (type_unsigned_8)(0xc0u | (sreg << 3u))};
        const type_unsigned_8 load_memory[] = {0x8eu,
            (type_unsigned_8)(0x06u | (sreg << 3u)), 0x00u, 0x10u};
        sreg_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_16 image = 0xbe5au;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !sreg_mov_run(&state, store, sizeof(store), &after,
            &diagnostic, &status) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(store) ||
            !sreg_mov_gprs_same(&before, &after, 0u) ||
            after.data.eax != ((before.data.eax & 0xffff0000u) |
                sreg_mov_sreg(&before, sreg)->selector);
        core_machine_destroy(state.machine);

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine,
            before.data.ds.base + 0x1000u, &image, sizeof(image)) !=
            TYPE_STATUS_OK || !sreg_mov_run(&state, store_memory,
            sizeof(store_memory), &after, &diagnostic, &status) || status !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eip !=
            sizeof(store_memory) || !sreg_mov_gprs_same(&before, &after, 8u) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                before.data.ds.base + 0x1000u, TYPE_REFERENCE_OF(image),
                sizeof(image)) != TYPE_STATUS_OK || image !=
                sreg_mov_sreg(&before, sreg)->selector;
        core_machine_destroy(state.machine);

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !sreg_mov_run(&state, load, sizeof(load), &after,
            &diagnostic, &status) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(load) ||
            !sreg_mov_gprs_same(&before, &after, 8u) ||
            sreg_mov_sreg(&after, sreg)->selector != 0x3344u ||
            sreg_mov_sreg(&after,
                sreg)->base != 0x33440u;
        core_machine_destroy(state.machine);

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        image = 0xbe5au;
        failed |= core_machine_memory_write(state.machine,
            before.data.ds.base + 0x1000u, &image, sizeof(image)) !=
            TYPE_STATUS_OK || !sreg_mov_run(&state, load_memory,
            sizeof(load_memory), &after, &diagnostic, &status) || status !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eip !=
            sizeof(load_memory) || !sreg_mov_gprs_same(&before, &after, 8u) ||
            sreg_mov_sreg(&after, sreg)->selector != image ||
            sreg_mov_sreg(&after, sreg)->base != ((type_unsigned_32)image << 4u);
        core_machine_destroy(state.machine);
    }
    return !failed;
}

static C_INT sreg_mov_test_rejections_and_attributes(C_VOID)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 rejected[][3] = {
        {0x8eu, 0xc8u},
        {0x8cu, 0xf0u},
        {0x8eu, 0xf0u}
    };
    /* SS-null #GP, DS non-present #NP, DS code/type #GP, DS RPL/DPL #GP. */
    type_unsigned_8 profile;
    type_unsigned_8 form;
    C_INT failed = 0;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
         ++profile) {
        for (form = 0u; form != 2u; ++form) {
            const type_unsigned_8 fs[] = {
                (type_unsigned_8)(form ? 0x8eu : 0x8cu), 0xe0u
            };
            const type_unsigned_8 attr[] = {form ? 0x66u : 0x67u,
                (type_unsigned_8)(form ? 0x8eu : 0x8cu), 0xc0u};
            sreg_mov_machine state;
            t_cpu before;

            if (!sreg_mov_prepare(&state, legacy[profile])) return 0;
            sreg_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !sreg_mov_expect_ud(&state, fs, sizeof(fs), &before);
            core_machine_destroy(state.machine);
            if (!sreg_mov_prepare(&state, legacy[profile])) return 0;
            sreg_mov_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !sreg_mov_expect_ud(&state, attr, sizeof(attr), &before);
            core_machine_destroy(state.machine);
        }
    }
    for (form = 0u; form != sizeof(rejected) / sizeof(rejected[0]); ++form) {
        sreg_mov_machine state;
        t_cpu before;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !sreg_mov_expect_ud(&state, rejected[form], 2u, &before);
        core_machine_destroy(state.machine);
    }
    for (form = 0u; form != 2u; ++form) {
        const type_unsigned_8 code[] = {0xf0u,
            (type_unsigned_8)(form ? 0x8eu : 0x8cu), 0xc0u};
        sreg_mov_machine state;
        t_cpu before;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !sreg_mov_expect_ud(&state, code, sizeof(code), &before);
        core_machine_destroy(state.machine);
    }
    for (form = 0u; form != 2u; ++form) {
        const type_unsigned_8 code[] = {0xf0u, (type_unsigned_8)(form ? 0x8eu : 0x8cu),
            0x06u, 0x00u, 0x10u};
        sreg_mov_machine state;
        t_cpu before;
        type_unsigned_16 image = 0xbe5au;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine,
            before.data.ds.base + 0x1000u, &image, sizeof(image)) !=
            TYPE_STATUS_OK || !sreg_mov_expect_ud(&state, code, sizeof(code),
            &before) || core_machine_memory_read_physical(
            &state.machine->executor_memory, before.data.ds.base + 0x1000u,
            TYPE_REFERENCE_OF(image), sizeof(image)) != TYPE_STATUS_OK ||
            image != 0xbe5au;
        core_machine_destroy(state.machine);
    }
    {
        static const type_unsigned_8 code[] = {0x8eu, 0x0eu, 0x00u, 0x10u};
        sreg_mov_machine state;
        t_cpu before;
        type_unsigned_16 image = 0xbe5au;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine,
            before.data.ds.base + 0x1000u, &image, sizeof(image)) !=
            TYPE_STATUS_OK || !sreg_mov_expect_ud(&state, code, sizeof(code),
            &before) || core_machine_memory_read_physical(
            &state.machine->executor_memory, before.data.ds.base + 0x1000u,
            TYPE_REFERENCE_OF(image), sizeof(image)) != TYPE_STATUS_OK ||
            image != 0xbe5au;
        core_machine_destroy(state.machine);
    }
    {
        static const type_unsigned_8 store[] = {0x66u, 0x8cu, 0xc0u};
        static const type_unsigned_8 load[] = {0x66u, 0x8eu, 0xc0u};
        static const type_unsigned_8 store67[] = {
            0x67u, 0x8cu, 0x05u, 0x00u, 0x10u, 0, 0
        };
        static const type_unsigned_8 load6766[] = {
            0x66u, 0x67u, 0x8eu, 0x05u, 0x00u, 0x10u, 0, 0
        };
        sreg_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_16 image = 0xbe5au;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !sreg_mov_run(&state, store, sizeof(store), &after,
            &diagnostic, &status) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(store) ||
            !sreg_mov_gprs_same(&before, &after, 0u) || after.data.eax !=
            ((before.data.eax & 0xffff0000u) | before.data.es.selector);
        core_machine_destroy(state.machine);
        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !sreg_mov_run(&state, load, sizeof(load), &after,
            &diagnostic, &status) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(load) ||
            !sreg_mov_gprs_same(&before, &after, 8u) ||
            sreg_mov_sreg(&after, 0u)->selector != 0x3344u ||
            sreg_mov_sreg(&after, 0u)->base != 0x33440u;
        core_machine_destroy(state.machine);
        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine,
            before.data.ds.base + 0x1000u, &image, sizeof(image)) !=
            TYPE_STATUS_OK || !sreg_mov_run(&state, store67, sizeof(store67),
            &after, &diagnostic, &status) || status != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(store67) ||
            !sreg_mov_gprs_same(&before, &after, 8u) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                before.data.ds.base + 0x1000u, TYPE_REFERENCE_OF(image),
                sizeof(image)) != TYPE_STATUS_OK || image != before.data.es.selector;
        core_machine_destroy(state.machine);
        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        image = 0xbe5au;
        failed |= core_machine_memory_write(state.machine,
            before.data.ds.base + 0x1000u, &image, sizeof(image)) !=
            TYPE_STATUS_OK || !sreg_mov_run(&state, load6766,
            sizeof(load6766), &after, &diagnostic, &status) || status !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eip !=
            sizeof(load6766) || !sreg_mov_gprs_same(&before, &after, 8u) ||
            sreg_mov_sreg(&after, 0u)->selector != image ||
            sreg_mov_sreg(&after, 0u)->base != ((type_unsigned_32)image << 4u);
        core_machine_destroy(state.machine);
    }
    return !failed;
}

static C_INT sreg_mov_boot_protected(sreg_mov_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x3fu, 0, 0, 0x03u, 0, 0};
    static const type_unsigned_8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0xffu, 0xffu, 0, 0x30u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x30u, 0, 0x12u, 0, 0,
        0xffu, 0xffu, 0, 0x30u, 0, 0x98u, 0, 0,
        0xffu, 0xffu, 0, 0x50u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x50u, 0, 0x92u, 0, 0,
        0x0fu, 0, 0, 0x50u, 0, 0x92u, 0, 0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu, 0x01u, 0x16u, 0, 1u,
        0xb8u, 1u, 0, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0x8eu, 0xd0u, 0xbcu, 0, 0x80u,
        0xeau, 0, 0, 8u, 0
    };
    static const type_unsigned_8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
            sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x300u, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0, boot, sizeof(boot)) ==
            TYPE_STATUS_OK && core_machine_memory_write(state->machine,
            0x2000u, &halt, sizeof(halt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){96u, 0u},
            &result) == TYPE_STATUS_OK && result.reason ==
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT sreg_mov_protected_step(sreg_mov_machine *state,
    const type_unsigned_8 *code, type_unsigned_8 bytes, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, type_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0x2000u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){1u, 0u}, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT sreg_mov_protected_fault(sreg_mov_machine *state,
    const type_unsigned_8 *code, type_unsigned_8 bytes, const t_cpu *before,
    type_unsigned_32 address, type_unsigned_16 image)
{
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_status status;

    return sreg_mov_protected_step(state, code, bytes, &after, &diagnostic,
        &status) && status == TYPE_STATUS_FAULT && diagnostic.first_fault.valid &&
        TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) &&
        after.data.eip == 0u && sreg_mov_gprs_same(before, &after, 8u) &&
        STD_MEMCMP(&before->data.es, &after.data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after.data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after.data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after.data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after.data.gs, sizeof(before->data.gs)) == 0 &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, TYPE_REFERENCE_OF(image), sizeof(image)) == TYPE_STATUS_OK &&
        image == 0xbe5au;
}

static C_INT sreg_mov_test_protected(C_VOID)
{
    static const type_unsigned_8 load_codes[] = {0xc0u, 0xd8u, 0xd0u, 0xe0u, 0xe8u};
    static const type_unsigned_8 null_codes[] = {0xc0u, 0xd8u, 0xe0u, 0xe8u};
    static const type_unsigned_8 store_limit[] = {0x8cu, 0x06u, 0x10u, 0};
    static const type_unsigned_8 load_limit[] = {0x8eu, 0x1eu, 0x10u, 0};
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(load_codes); ++form) {
        sreg_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        t_cpu_data_sreg *target;
        type_unsigned_8 access = 0u;
        C_INT failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) failed |= !sreg_mov_boot_protected(&state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabb0030u;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !sreg_mov_protected_step(&state,
                (type_unsigned_8[]){0x8eu, load_codes[form]}, 2u, &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != 2u ||
                !sreg_mov_gprs_same(&before, &after, 8u);
            target = (t_cpu_data_sreg *)sreg_mov_sreg(&after, form == 0u ? 0u :
                form == 1u ? 3u : form == 2u ? 2u : form == 3u ? 4u : 5u);
            failed |= target->selector != 0x30u || target->base != 0x5000u ||
                target->limit != 0xffffu || !target->flagValid ||
                !target->seg.data.writable ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x335u, TYPE_REFERENCE_OF(access), sizeof(access)) !=
                    TYPE_STATUS_OK || access != 0x93u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (form = 0u; form != sizeof(null_codes); ++form) {
        sreg_mov_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        const t_cpu_data_sreg *target;
        type_unsigned_8 index = form == 0u ? 0u : form == 1u ? 3u :
            form == 2u ? 4u : 5u;
        C_INT failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) failed |= !sreg_mov_boot_protected(&state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabb0000u;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !sreg_mov_protected_step(&state,
                (type_unsigned_8[]){0x8eu, null_codes[form]}, 2u, &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != 2u ||
                !sreg_mov_gprs_same(&before, &after, 8u);
            target = sreg_mov_sreg(&after, index);
            failed |= target->selector != 0u || target->flagValid;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (form = 0u; form != 4u; ++form) {
        sreg_mov_machine state;
        t_cpu before;
        type_unsigned_16 image = 0xbe5au;
        type_unsigned_16 selector = form == 0u ? 0u : form == 1u ? 0x18u :
            form == 2u ? 0x20u : 0x2bu;
        type_unsigned_32 address = 0x3010u;
        type_unsigned_8 modrm = form == 0u ? 0xd0u : 0xd8u;
        C_INT failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) failed |= !sreg_mov_boot_protected(&state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabb0000u | selector;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine, address, &image,
                sizeof(image)) != TYPE_STATUS_OK || !sreg_mov_protected_fault(
                &state, (type_unsigned_8[]){0x8eu, modrm}, 2u, &before, address, image);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (form = 0u; form != 2u; ++form) {
        sreg_mov_machine state;
        t_cpu before;
        type_unsigned_16 image = 0xbe5au;
        C_INT failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) failed |= !sreg_mov_boot_protected(&state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabb0038u;
            failed |= !sreg_mov_protected_step(&state, (type_unsigned_8[]){0x8eu, 0xd8u},
                2u, &before, &(core_machine_cpu_diagnostic){0},
                &(type_status){0});
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine, 0x5010u, &image,
                sizeof(image)) != TYPE_STATUS_OK || !sreg_mov_protected_fault(
                &state, form == 0u ? load_limit : store_limit, 4u, &before,
                0x5010u, image);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT sreg_mov_test_irq_shadow(C_VOID)
{
    static const type_unsigned_8 modrms[] = {0xd0u, 0xd8u, 0xe0u};
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(modrms); ++form) {
        sreg_mov_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_16 offset = 0x0100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_8 code[] = {0x8eu, modrms[form], 0x90u};
        type_unsigned_16 expected_ip = form == 0u ? 3u : 2u;
        type_unsigned_8 target = form == 0u ? 2u : form == 1u ? 3u : 4u;
        C_INT failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u) || core_machine_memory_write(state.machine,
                0u, code, sizeof(code)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x80u, &offset,
                    sizeof(offset)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment,
                    sizeof(segment)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &hlt,
                    sizeof(hlt)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            sreg_mov_seed(&state);
            state.machine->executor_cpu.data.eax = 0xaabb2000u;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){form == 0u ? 3u : 2u, 0u},
                &result) != TYPE_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != expected_ip ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                    state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                sreg_mov_sreg(&after, target)->selector != 0x2000u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!sreg_mov_test_real_forms()) {
        STD_PRINTF("SREG-MOV stage=real\n");
        return 1;
    }
    if (!sreg_mov_test_386_extensions()) {
        STD_PRINTF("SREG-MOV stage=extensions\n");
        return 1;
    }
    if (!sreg_mov_test_rejections_and_attributes()) {
        STD_PRINTF("SREG-MOV stage=reject\n");
        return 1;
    }
    if (!sreg_mov_test_protected()) {
        STD_PRINTF("SREG-MOV stage=protected\n");
        return 1;
    }
    if (!sreg_mov_test_irq_shadow()) {
        STD_PRINTF("SREG-MOV stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S32:SREG-MOV:OK\n");
    STD_PRINTF("M5:T401:S48:SREG-MOV-PROFILES:OK\n");
    return 0;
}
