#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* T337_REAL_UD_TERMINAL_CPU_OWNER: rejection and shutdown remain distinct. */
static lib_i32 sreg_mov_prepare(cpu_instruction_fixture *state,
    core_machine_cpu_profile profile)
{
    cpu_instruction_prepare(state, profile);
    return 1;
}

static void sreg_mov_seed(cpu_instruction_fixture *state)
{
    state->cpu.data.eax = 0xaabb3344u;
    state->cpu.data.ecx = 0x11225566u;
    state->cpu.data.edx = 0x778899aau;
    state->cpu.data.ebx = 0xbbccddeeU;
    state->cpu.data.esp = 0x00008000u;
    state->cpu.data.ebp = 0x00000120u;
    state->cpu.data.esi = 0x00000010u;
    state->cpu.data.edi = 0x00000020u;
    state->cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    state->cpu.data.es.selector = 0x1111u;
    state->cpu.data.es.base = 0x11110u;
    state->cpu.data.ss.selector = 0x2222u;
    state->cpu.data.ss.base = 0x22220u;
    state->cpu.data.ds.selector = 0x3333u;
    state->cpu.data.ds.base = 0x33330u;
    state->cpu.data.fs.selector = 0x4444u;
    state->cpu.data.fs.base = 0x44440u;
    state->cpu.data.gs.selector = 0x5555u;
    state->cpu.data.gs.base = 0x55550u;
}

static lib_i32 sreg_mov_run(cpu_instruction_fixture *state, const lib_u8 *code,
    lib_u8 bytes, t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    lib_status *status)
{
    *status = cpu_instruction_run(state, code, bytes, after);
    *diagnostic = (core_machine_cpu_diagnostic){ .first_fault = state->fault,
        .last_delivered_exception = state->delivered_exception };
    return 1;
}

static const t_cpu_data_sreg *sreg_mov_sreg(const t_cpu *cpu, lib_u8 index)
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

static lib_i32 sreg_mov_gprs_same(const t_cpu *before, const t_cpu *after,
    lib_u8 changed)
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

static lib_i32 sreg_mov_test_real_forms(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;
    lib_u8 sreg;
    lib_i32 failed = 0;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile) {
        for (sreg = 0u; sreg != 4u; ++sreg) {
            const lib_u8 store_register[] = {0x8cu,
                (lib_u8)(0xc0u | (sreg << 3u))};
            const lib_u8 store_memory[] = {0x8cu,
                (lib_u8)(0x06u | (sreg << 3u)),0x00u,0x10u};
            const lib_u8 load_register[] = {0x8eu,
                (lib_u8)(0xc0u | (sreg << 3u))};
            const lib_u8 load_memory[] = {0x8eu,
                (lib_u8)(0x06u | (sreg << 3u)),0x00u,0x10u};
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            lib_status status;
            lib_u16 image = 0xbe5au;
            const t_cpu_data_sreg *source;

            if (!sreg_mov_prepare(&state, profiles[profile])) return 0;
            sreg_mov_seed(&state);
            before = state.cpu;
            source = sreg_mov_sreg(&before, sreg);
            failed |= !sreg_mov_run(&state, store_register,
                sizeof(store_register), &after, &diagnostic, &status) ||
                status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(store_register) ||
                !sreg_mov_gprs_same(&before, &after, 0u) ||
                after.data.eax != ((before.data.eax & 0xffff0000u) |
                    source->selector);

            if (!sreg_mov_prepare(&state, profiles[profile])) return 0;
            sreg_mov_seed(&state);
            before = state.cpu;
            failed |= cpu_instruction_write(&state, before.data.ds.base + 0x1000u, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !sreg_mov_run(&state,
                store_memory, sizeof(store_memory), &after, &diagnostic, &status) ||
                status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(store_memory) ||
                !sreg_mov_gprs_same(&before, &after, 8u) ||
                cpu_instruction_read(&state, before.data.ds.base + 0x1000u, (void *)((lib_uptr)&image), sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK || image != sreg_mov_sreg(&before, sreg)->selector;

            if (sreg == 1u) continue;
            if (!sreg_mov_prepare(&state, profiles[profile])) return 0;
            sreg_mov_seed(&state);
            before = state.cpu;
            failed |= !sreg_mov_run(&state, load_register,
                sizeof(load_register), &after, &diagnostic, &status) ||
                status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(load_register) ||
                !sreg_mov_gprs_same(&before, &after, 8u) ||
                sreg_mov_sreg(&after, sreg)->selector != 0x3344u ||
                sreg_mov_sreg(&after, sreg)->base != 0x33440u;

            if (!sreg_mov_prepare(&state, profiles[profile])) return 0;
            sreg_mov_seed(&state);
            before = state.cpu;
            failed |= cpu_instruction_write(&state, before.data.ds.base + 0x1000u, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !sreg_mov_run(&state,
                load_memory, sizeof(load_memory), &after, &diagnostic, &status) ||
                status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(load_memory) ||
                !sreg_mov_gprs_same(&before, &after, 8u) ||
                sreg_mov_sreg(&after, sreg)->selector != image ||
                sreg_mov_sreg(&after, sreg)->base != ((lib_u32)image << 4u);
        }
    }
    return !failed;
}

static lib_i32 sreg_mov_test_386_extensions(void)
{
    lib_u8 sreg;
    lib_i32 failed = 0;

    for (sreg = 4u; sreg != 6u; ++sreg) {
        const lib_u8 store[] = {0x8cu, (lib_u8)(0xc0u | (sreg << 3u))};
        const lib_u8 store_memory[] = {0x8cu,
            (lib_u8)(0x06u | (sreg << 3u)), 0x00u, 0x10u};
        const lib_u8 load[] = {0x8eu, (lib_u8)(0xc0u | (sreg << 3u))};
        const lib_u8 load_memory[] = {0x8eu,
            (lib_u8)(0x06u | (sreg << 3u)), 0x00u, 0x10u};
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status;
        lib_u16 image = 0xbe5au;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        failed |= !sreg_mov_run(&state, store, sizeof(store), &after,
            &diagnostic, &status) || status != LIB_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(store) ||
            !sreg_mov_gprs_same(&before, &after, 0u) ||
            after.data.eax != ((before.data.eax & 0xffff0000u) |
                sreg_mov_sreg(&before, sreg)->selector);

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        failed |= cpu_instruction_write(&state, before.data.ds.base + 0x1000u, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
            LIB_STATUS_OK || !sreg_mov_run(&state, store_memory,
            sizeof(store_memory), &after, &diagnostic, &status) || status !=
            LIB_STATUS_OK || diagnostic.first_fault.valid || after.data.eip !=
            sizeof(store_memory) || !sreg_mov_gprs_same(&before, &after, 8u) ||
            cpu_instruction_read(&state, before.data.ds.base + 0x1000u, (void *)((lib_uptr)&image), sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || image !=
                sreg_mov_sreg(&before, sreg)->selector;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        failed |= !sreg_mov_run(&state, load, sizeof(load), &after,
            &diagnostic, &status) || status != LIB_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(load) ||
            !sreg_mov_gprs_same(&before, &after, 8u) ||
            sreg_mov_sreg(&after, sreg)->selector != 0x3344u ||
            sreg_mov_sreg(&after,
                sreg)->base != 0x33440u;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        image = 0xbe5au;
        failed |= cpu_instruction_write(&state, before.data.ds.base + 0x1000u, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
            LIB_STATUS_OK || !sreg_mov_run(&state, load_memory,
            sizeof(load_memory), &after, &diagnostic, &status) || status !=
            LIB_STATUS_OK || diagnostic.first_fault.valid || after.data.eip !=
            sizeof(load_memory) || !sreg_mov_gprs_same(&before, &after, 8u) ||
            sreg_mov_sreg(&after, sreg)->selector != image ||
            sreg_mov_sreg(&after, sreg)->base != ((lib_u32)image << 4u);
    }
    return !failed;
}

static lib_i32 sreg_mov_test_rejections_and_attributes(void)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 rejected[][3] = {
        {0x8eu, 0xc8u},
        {0x8cu, 0xf0u},
        {0x8eu, 0xf0u}
    };
    /* SS-null #GP, DS non-present #NP, DS code/type #GP, DS RPL/DPL #GP. */
    lib_u8 profile;
    lib_u8 form;
    lib_i32 failed = 0;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
         ++profile) {
        for (form = 0u; form != 2u; ++form) {
            const lib_u8 fs[] = {
                (lib_u8)(form ? 0x8eu : 0x8cu), 0xe0u
            };
            const lib_u8 attr[] = {form ? 0x66u : 0x67u,
                (lib_u8)(form ? 0x8eu : 0x8cu), 0xc0u};
            cpu_instruction_fixture state;

            if (!sreg_mov_prepare(&state, legacy[profile])) return 0;
            sreg_mov_seed(&state);
            failed |= !cpu_instruction_expect_real_fault(&state, fs, sizeof(fs), 6u);
            if (!sreg_mov_prepare(&state, legacy[profile])) return 0;
            sreg_mov_seed(&state);
            failed |= !cpu_instruction_expect_real_fault(&state, attr, sizeof(attr), 6u);
        }
    }
    for (form = 0u; form != sizeof(rejected) / sizeof(rejected[0]); ++form) {
        cpu_instruction_fixture state;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        failed |= !cpu_instruction_expect_real_fault(&state, rejected[form], 2u, 6u);
    }
    for (form = 0u; form != 2u; ++form) {
        const lib_u8 code[] = {0xf0u,
            (lib_u8)(form ? 0x8eu : 0x8cu), 0xc0u};
        cpu_instruction_fixture state;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        failed |= !cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u);
    }
    for (form = 0u; form != 2u; ++form) {
        const lib_u8 code[] = {0xf0u, (lib_u8)(form ? 0x8eu : 0x8cu),
            0x06u, 0x00u, 0x10u};
        cpu_instruction_fixture state;
        t_cpu before;
        lib_u16 image = 0xbe5au;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        failed |= cpu_instruction_write(&state, before.data.ds.base + 0x1000u, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
            LIB_STATUS_OK || !cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u) || cpu_instruction_read(&state, before.data.ds.base + 0x1000u, (void *)((lib_uptr)&image), sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            image != 0xbe5au;
    }
    {
        static const lib_u8 code[] = {0x8eu, 0x0eu, 0x00u, 0x10u};
        cpu_instruction_fixture state;
        t_cpu before;
        lib_u16 image = 0xbe5au;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        failed |= cpu_instruction_write(&state, before.data.ds.base + 0x1000u, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
            LIB_STATUS_OK || !cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u) || cpu_instruction_read(&state, before.data.ds.base + 0x1000u, (void *)((lib_uptr)&image), sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            image != 0xbe5au;
    }
    {
        static const lib_u8 store[] = {0x66u, 0x8cu, 0xc0u};
        static const lib_u8 load[] = {0x66u, 0x8eu, 0xc0u};
        static const lib_u8 store67[] = {
            0x67u, 0x8cu, 0x05u, 0x00u, 0x10u, 0, 0
        };
        static const lib_u8 load6766[] = {
            0x66u, 0x67u, 0x8eu, 0x05u, 0x00u, 0x10u, 0, 0
        };
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status;
        lib_u16 image = 0xbe5au;

        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        failed |= !sreg_mov_run(&state, store, sizeof(store), &after,
            &diagnostic, &status) || status != LIB_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(store) ||
            !sreg_mov_gprs_same(&before, &after, 0u) || after.data.eax !=
            ((before.data.eax & 0xffff0000u) | before.data.es.selector);
        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        failed |= !sreg_mov_run(&state, load, sizeof(load), &after,
            &diagnostic, &status) || status != LIB_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(load) ||
            !sreg_mov_gprs_same(&before, &after, 8u) ||
            sreg_mov_sreg(&after, 0u)->selector != 0x3344u ||
            sreg_mov_sreg(&after, 0u)->base != 0x33440u;
        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        failed |= cpu_instruction_write(&state, before.data.ds.base + 0x1000u, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
            LIB_STATUS_OK || !sreg_mov_run(&state, store67, sizeof(store67),
            &after, &diagnostic, &status) || status != LIB_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != sizeof(store67) ||
            !sreg_mov_gprs_same(&before, &after, 8u) ||
            cpu_instruction_read(&state, before.data.ds.base + 0x1000u, (void *)((lib_uptr)&image), sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || image != before.data.es.selector;
        if (!sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386))
            return 0;
        sreg_mov_seed(&state);
        before = state.cpu;
        image = 0xbe5au;
        failed |= cpu_instruction_write(&state, before.data.ds.base + 0x1000u, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
            LIB_STATUS_OK || !sreg_mov_run(&state, load6766,
            sizeof(load6766), &after, &diagnostic, &status) || status !=
            LIB_STATUS_OK || diagnostic.first_fault.valid || after.data.eip !=
            sizeof(load6766) || !sreg_mov_gprs_same(&before, &after, 8u) ||
            sreg_mov_sreg(&after, 0u)->selector != image ||
            sreg_mov_sreg(&after, 0u)->base != ((lib_u32)image << 4u);
    }
    return !failed;
}

static lib_i32 sreg_mov_boot_protected(cpu_instruction_fixture *state)
{
    static const lib_u8 pointer[] = {0x3fu, 0, 0, 0x03u, 0, 0};
    static const lib_u8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0xffu, 0xffu, 0, 0x30u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x30u, 0, 0x12u, 0, 0,
        0xffu, 0xffu, 0, 0x30u, 0, 0x98u, 0, 0,
        0xffu, 0xffu, 0, 0x50u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x50u, 0, 0x92u, 0, 0,
        0x0fu, 0, 0, 0x50u, 0, 0x92u, 0, 0
    };
    static const lib_u8 boot[] = {
        0x0fu, 0x01u, 0x16u, 0, 1u,
        0xb8u, 1u, 0, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0x8eu, 0xd0u, 0xbcu, 0, 0x80u,
        0xeau, 0, 0, 8u, 0
    };

    lib_memory_copy(state->memory + 0x100u, pointer, sizeof(pointer));
    lib_memory_copy(state->memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(state->memory, boot, sizeof(boot));
    for (lib_u8 step = 0u; step != 9u; ++step) {
        core_machine_cpu_execution_refresh(&state->execution);
        if (state->execution.stop_requested || state->fault.valid) return 0;
    }
    return state->cpu.data.cs.selector == 8u && state->cpu.data.eip == 0u;
}

static lib_i32 sreg_mov_protected_step(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, lib_status *status)
{
    if (cpu_instruction_write(state, 0x2000u, code, bytes,
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK) return 0;
    state->cpu.data.eip = 0u;
    core_machine_cpu_execution_refresh(&state->execution);
    *status = state->execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
    *after = state->cpu;
    *diagnostic = (core_machine_cpu_diagnostic){ .first_fault = state->fault,
        .last_delivered_exception = state->delivered_exception };
    return 1;
}

static lib_i32 sreg_mov_protected_fault(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, const t_cpu *before,
    lib_u32 address, lib_u16 image)
{
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_status status;

    return sreg_mov_protected_step(state, code, bytes, &after, &diagnostic,
        &status) && status == LIB_STATUS_OK && core_machine_cpu_is_shutdown(&state->execution) &&
        !diagnostic.first_fault.valid && diagnostic.last_delivered_exception.valid &&
        diagnostic.last_delivered_exception.exception_mask == VCPUINS_EXCEPT_SHUTDOWN &&
        after.data.eip == 0u && sreg_mov_gprs_same(before, &after, 8u) &&
        lib_memory_compare(&before->data.es, &after.data.es, sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.ss, &after.data.ss, sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after.data.ds, sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after.data.fs, sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after.data.gs, sizeof(before->data.gs)) == 0 &&
        cpu_instruction_read(state, address, (void *)((lib_uptr)&image), sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) == LIB_STATUS_OK &&
        image == 0xbe5au;
}

static lib_i32 sreg_mov_test_protected(void)
{
    static const lib_u8 load_codes[] = {0xc0u, 0xd8u, 0xd0u, 0xe0u, 0xe8u};
    static const lib_u8 null_codes[] = {0xc0u, 0xd8u, 0xe0u, 0xe8u};
    static const lib_u8 store_limit[] = {0x8cu, 0x06u, 0x10u, 0};
    static const lib_u8 load_limit[] = {0x8eu, 0x1eu, 0x10u, 0};
    lib_u8 form;

    for (form = 0u; form != sizeof(load_codes); ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status;
        t_cpu_data_sreg *target;
        lib_u8 access = 0u;
        lib_i32 failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) failed |= !sreg_mov_boot_protected(&state);
        if (!failed) {
            state.cpu.data.eax = 0xaabb0030u;
            before = state.cpu;
            failed |= !sreg_mov_protected_step(&state,
                (lib_u8[]){0x8eu, load_codes[form]}, 2u, &after,
                &diagnostic, &status) || status != LIB_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != 2u ||
                !sreg_mov_gprs_same(&before, &after, 8u);
            target = (t_cpu_data_sreg *)sreg_mov_sreg(&after, form == 0u ? 0u :
                form == 1u ? 3u : form == 2u ? 2u : form == 3u ? 4u : 5u);
            failed |= target->selector != 0x30u || target->base != 0x5000u ||
                target->limit != 0xffffu || !target->flagValid ||
                !target->seg.data.writable ||
                cpu_instruction_read(&state, 0x335u, (void *)((lib_uptr)&access), sizeof(access), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK || access != 0x93u;
        }
        if (failed) return 0;
    }
    for (form = 0u; form != sizeof(null_codes); ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status;
        const t_cpu_data_sreg *target;
        lib_u8 index = form == 0u ? 0u : form == 1u ? 3u :
            form == 2u ? 4u : 5u;
        lib_i32 failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) failed |= !sreg_mov_boot_protected(&state);
        if (!failed) {
            state.cpu.data.eax = 0xaabb0000u;
            before = state.cpu;
            failed |= !sreg_mov_protected_step(&state,
                (lib_u8[]){0x8eu, null_codes[form]}, 2u, &after,
                &diagnostic, &status) || status != LIB_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != 2u ||
                !sreg_mov_gprs_same(&before, &after, 8u);
            target = sreg_mov_sreg(&after, index);
            failed |= target->selector != 0u || target->flagValid;
        }
        if (failed) return 0;
    }
    for (form = 0u; form != 4u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        lib_u16 image = 0xbe5au;
        lib_u16 selector = form == 0u ? 0u : form == 1u ? 0x18u :
            form == 2u ? 0x20u : 0x2bu;
        lib_u32 address = 0x3010u;
        lib_u8 modrm = form == 0u ? 0xd0u : 0xd8u;
        lib_i32 failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) failed |= !sreg_mov_boot_protected(&state);
        if (!failed) {
            state.cpu.data.eax = 0xaabb0000u | selector;
            before = state.cpu;
            failed |= cpu_instruction_write(&state, address, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !sreg_mov_protected_fault(
                &state, (lib_u8[]){0x8eu, modrm}, 2u, &before, address, image);
        }
        if (failed) return 0;
    }
    for (form = 0u; form != 2u; ++form) {
        cpu_instruction_fixture state;
        t_cpu before;
        lib_u16 image = 0xbe5au;
        lib_i32 failed = !sreg_mov_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) failed |= !sreg_mov_boot_protected(&state);
        if (!failed) {
            state.cpu.data.eax = 0xaabb0038u;
            failed |= !sreg_mov_protected_step(&state, (lib_u8[]){0x8eu, 0xd8u},
                2u, &before, &(core_machine_cpu_diagnostic){0},
                &(lib_status){0});
            before = state.cpu;
            failed |= cpu_instruction_write(&state, 0x5010u, &image, sizeof(image), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !sreg_mov_protected_fault(
                &state, form == 0u ? load_limit : store_limit, 4u, &before,
                0x5010u, image);
        }
        if (failed) return 0;
    }
    return 1;
}



lib_i32 main(void)
{
    if (!sreg_mov_test_real_forms()) {
        lib_c_printf("SREG-MOV stage=real\n");
        return 1;
    }
    if (!sreg_mov_test_386_extensions()) {
        lib_c_printf("SREG-MOV stage=extensions\n");
        return 1;
    }
    if (!sreg_mov_test_rejections_and_attributes()) {
        lib_c_printf("SREG-MOV stage=reject\n");
        return 1;
    }
    if (!sreg_mov_test_protected()) {
        lib_c_printf("SREG-MOV stage=protected\n");
        return 1;
    }
    lib_c_printf("M5:T316:S32:SREG-MOV:OK\n");
    lib_c_printf("M5:T401:S48:SREG-MOV-PROFILES:OK\n");
    return 0;
}
