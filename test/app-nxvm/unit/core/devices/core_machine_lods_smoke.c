#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct lods_machine { core_machine *machine; } lods_machine;

static void lods_reset(void *opaque)
{
    lods_machine *state = (lods_machine *)opaque;
    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider lods_provider = {
    lods_reset, LIB_NULL
};

static lib_i32 lods_prepare(core_machine_cpu_profile profile, lods_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    lib_memory_set(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &lods_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static void lods_seed(lods_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;
    cpu->data.eax = 0xaabb3344u; cpu->data.ecx = 0x11225566u;
    cpu->data.edx = 0x778899aau; cpu->data.ebx = 0xbbccddeeU;
    cpu->data.esp = 0x8000u; cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u; cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    cpu->data.ds.base = 0x10000u; cpu->data.cs.base = 0u;
    cpu->data.ss.base = 0x30000u; cpu->data.es.base = 0x20000u;
    cpu->data.fs.base = 0x40000u; cpu->data.gs.base = 0x50000u;
}

static lib_i32 lods_case(core_machine_cpu_profile profile, const lib_u8 *code,
    lib_u8 bytes, lib_u8 width, lib_i32 address32, lib_i32 decrement,
    lib_u32 address, lib_u32 source)
{
    lods_machine state; t_cpu before, after; core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result; lib_status status; lib_u32 source_after;
    lib_u32 expected = width == 1u ? 0xaabb3300u | (source & 0xffu) :
        width == 2u ? 0xaabb0000u | (source & 0xffffu) : source;
    lib_i32 failed = !lods_prepare(profile, &state);
    if (!failed) {
        lods_seed(&state);
        if (address32) state.machine->executor_cpu.data.esi = 0x1010u;
        if (decrement) state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
        failed |= core_machine_memory_write(state.machine, address, &source, width) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0u, code, bytes) != LIB_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        status = core_machine_run(state.machine, (core_machine_run_budget){1u,0u}, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            status != LIB_STATUS_OK || diagnostic.first_fault.valid || after.data.eip != bytes ||
            after.data.eax != expected || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp || after.data.ebp != before.data.ebp ||
            after.data.edi != before.data.edi || after.data.eflags != before.data.eflags ||
            after.data.esi != (address32 ? before.data.esi + (decrement ? -(lib_i32)width : width) :
            ((before.data.esi & 0xffff0000u) | (lib_u16)((lib_u16)before.data.esi + (decrement ? -(lib_i32)width : width)))) ||
            core_machine_memory_read_physical(&state.machine->executor_memory, address,
            CORE_MACHINE_REFERENCE_OF(source_after), width) != LIB_STATUS_OK ||
            (width == 1u ? (source_after & 0xffu) != (source & 0xffu) :
            width == 2u ? (source_after & 0xffffu) != (source & 0xffffu) :
            source_after != source);
    }
    core_machine_destroy(state.machine); return !failed;
}

static lib_i32 lods_rep_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_i32 address32,
    lib_i32 decrement, lib_u16 count, const lib_u32 *source)
{
    lods_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_status status;
    lib_u32 source_after;
    lib_u32 address;
    lib_u32 expected_eax;
    lib_u32 expected_esi;
    lib_u32 expected_ecx;
    lib_u16 index;
    lib_i32 failed = !lods_prepare(profile, &state);

    if (!failed) {
        lods_seed(&state);
        state.machine->executor_cpu.data.ecx = address32 ? count :
            0x11220000u | count;
        state.machine->executor_cpu.data.esi = address32 ? 0x1010u : 0x10u;
        if (decrement)
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_DF;
        for (index = 0u; index != count; ++index) {
            address = 0x10000u + (address32 ? 0x1010u : 0x10u) +
                (decrement ? -(lib_i32)(index * width) : index * width);
            failed |= core_machine_memory_write(state.machine, address,
                &source[index], width) != LIB_STATUS_OK;
        }
        failed |= core_machine_memory_write(state.machine, 0u, code, bytes) !=
            LIB_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        status = core_machine_run(state.machine,
            (core_machine_run_budget){count == 0u ? 1u : count, 0u}, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        expected_eax = count == 0u ? before.data.eax : width == 1u ?
            (before.data.eax & 0xffffff00u) | (source[count - 1u] & 0xffu) :
            width == 2u ? (before.data.eax & 0xffff0000u) |
            (source[count - 1u] & 0xffffu) : source[count - 1u];
        expected_esi = address32 ? before.data.esi + (decrement ?
            -(lib_i32)(count * width) : count * width) :
            (before.data.esi & 0xffff0000u) | (lib_u16)((lib_u16)
            before.data.esi + (decrement ? -(lib_i32)(count * width) :
            count * width));
        expected_ecx = address32 ? 0u : before.data.ecx & 0xffff0000u;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK || status != LIB_STATUS_OK ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            after.data.eax != expected_eax || after.data.ecx != expected_ecx ||
            after.data.esi != expected_esi || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags;
        for (index = 0u; index != count; ++index) {
            address = 0x10000u + (address32 ? 0x1010u : 0x10u) +
                (decrement ? -(lib_i32)(index * width) : index * width);
            failed |= core_machine_memory_read_physical(
                &state.machine->executor_memory, address,
                CORE_MACHINE_REFERENCE_OF(source_after), width) != LIB_STATUS_OK ||
                (width == 1u ? (source_after & 0xffu) !=
                (source[index] & 0xffu) : width == 2u ?
                (source_after & 0xffffu) != (source[index] & 0xffffu) :
                source_after != source[index]);
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 lods_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    static const lib_u32 source = 0x12345678u;
    lods_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_status status;
    lib_u32 source_after;
    lib_i32 failed = !lods_prepare(profile, &state);

    if (!failed) {
        lods_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x10010u, &source,
            sizeof(source)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0u, code, bytes) != LIB_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        status = core_machine_run(state.machine,
            (core_machine_run_budget){1u, 0u}, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK || status != LIB_STATUS_INTERNAL_ERROR ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x10010u,
            CORE_MACHINE_REFERENCE_OF(source_after), sizeof(source_after)) !=
            LIB_STATUS_OK || source_after != source;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 lods_test_rejections(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 prefix_66_ac[] = {0x66u, 0xacu};
    static const lib_u8 prefix_67_ad[] = {0x67u, 0xadu};
    static const lib_u8 prefix_66_67_ad[] = {0x66u, 0x67u, 0xadu};
    static const lib_u8 rep_prefix_66_ac[] = {0xf3u, 0x66u, 0xacu};
    static const lib_u8 rep_prefix_67_ad[] = {0xf3u, 0x67u, 0xadu};
    static const lib_u8 rep_prefix_66_67_ad[] = {0xf3u, 0x66u, 0x67u, 0xadu};
    static const lib_u8 lock_ac[] = {0xf0u, 0xacu};
    static const lib_u8 lock_ad[] = {0xf0u, 0xadu};
    static const lib_u8 lock_rep_ac[] = {0xf0u, 0xf3u, 0xacu};
    static const lib_u8 lock_rep_ad[] = {0xf0u, 0xf3u, 0xadu};
    static const lib_u8 lock_66_ac[] = {0xf0u, 0x66u, 0xacu};
    static const lib_u8 lock_rep_66_ad[] = {0xf0u, 0xf3u, 0x66u, 0xadu};
    static const lib_u8 lock_67_ac[] = {0xf0u, 0x67u, 0xacu};
    static const lib_u8 lock_rep_67_ad[] = {0xf0u, 0xf3u, 0x67u, 0xadu};
    static const lib_u8 lock_66_67_ad[] = {0xf0u, 0x66u, 0x67u, 0xadu};
    static const lib_u8 lock_rep_66_67_ad[] = {
        0xf0u, 0xf3u, 0x66u, 0x67u, 0xadu
    };
    lib_u8 index;

    for (index = 0u; index != sizeof(profiles) / sizeof(profiles[0]); ++index) {
        if (!lods_expect_ud(profiles[index], prefix_66_ac,
            sizeof(prefix_66_ac)) || !lods_expect_ud(profiles[index],
            prefix_67_ad, sizeof(prefix_67_ad)) || !lods_expect_ud(
            profiles[index], prefix_66_67_ad, sizeof(prefix_66_67_ad)) ||
            !lods_expect_ud(profiles[index], rep_prefix_66_ac,
            sizeof(rep_prefix_66_ac)) || !lods_expect_ud(profiles[index],
            rep_prefix_67_ad, sizeof(rep_prefix_67_ad)) || !lods_expect_ud(
            profiles[index], rep_prefix_66_67_ad,
            sizeof(rep_prefix_66_67_ad)))
            return 0;
    }
    return lods_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock_ac,
        sizeof(lock_ac)) && lods_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
        lock_ad, sizeof(lock_ad)) && lods_expect_ud(
        CORE_MACHINE_CPU_PROFILE_80386, lock_rep_ac, sizeof(lock_rep_ac)) &&
        lods_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock_rep_ad,
        sizeof(lock_rep_ad)) && lods_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
        lock_66_ac, sizeof(lock_66_ac)) && lods_expect_ud(
        CORE_MACHINE_CPU_PROFILE_80386, lock_rep_66_ad,
        sizeof(lock_rep_66_ad)) && lods_expect_ud(
        CORE_MACHINE_CPU_PROFILE_80386, lock_67_ac, sizeof(lock_67_ac)) &&
        lods_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, lock_rep_67_ad,
        sizeof(lock_rep_67_ad)) && lods_expect_ud(
        CORE_MACHINE_CPU_PROFILE_80386, lock_66_67_ad,
        sizeof(lock_66_67_ad)) && lods_expect_ud(
        CORE_MACHINE_CPU_PROFILE_80386, lock_rep_66_67_ad,
        sizeof(lock_rep_66_67_ad));
}

static lib_i32 lods_boot_protected(lods_machine *state)
{
    static const lib_u8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    static const lib_u8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0x0fu, 0, 0, 0x30u, 0, 0x92u, 0, 0,
        0xffu, 0xffu, 0, 0x40u, 0, 0x92u, 0, 0
    };
    static const lib_u8 boot[] = {
        0x0fu, 0x01u, 0x16u, 0, 1u,
        0xb8u, 1u, 0,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x18u, 0,
        0x8eu, 0xd8u,
        0xb8u, 0x10u, 0,
        0x8eu, 0xc0u,
        0xb8u, 0x18u, 0,
        0x8eu, 0xd0u,
        0xbcu, 0, 0x80u,
        0xeau, 0, 0, 8u, 0
    };
    static const lib_u8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == LIB_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, boot, sizeof(boot)) ==
        LIB_STATUS_OK && core_machine_memory_write(state->machine, 0x2000u,
        &halt, sizeof(halt)) == LIB_STATUS_OK && core_machine_run(
        state->machine, (core_machine_run_budget){96u, 0u}, &result) ==
        LIB_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static void lods_seed_protected(lods_machine *state, lib_u32 ds_limit)
{
    lods_seed(state);
    state->machine->executor_cpu.data.cs.base = 0x2000u;
    state->machine->executor_cpu.data.ds.selector = 0x10u;
    state->machine->executor_cpu.data.ds.base = 0x3000u;
    state->machine->executor_cpu.data.ds.limit = ds_limit;
}

static lib_i32 lods_test_protected_limits(void)
{
    static const lib_u8 single_codes[][2] = {
        {0xacu, 0u},
        {0x66u, 0xadu}
    };
    static const lib_u8 rep_ac[] = {0xf3u, 0xacu};
    lib_u8 form;

    for (form = 0u; form != 2u; ++form) {
        lods_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        lib_u32 source = 0x11223344u;
        lib_u32 source_after;
        lib_u8 bytes = form == 0u ? 1u : 2u;
        lib_u8 width = form == 0u ? 1u : 4u;
        lib_i32 failed = !lods_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
            failed |= !lods_boot_protected(&state);
        if (!failed) {
            lods_seed_protected(&state, 0x0fu);
            failed |= core_machine_memory_write(state.machine, 0x3010u, &source,
                width) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x2000u, single_codes[form], bytes) !=
                LIB_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
                after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi || after.data.eflags !=
                before.data.eflags || lib_memory_compare(&before.data.ds, &after.data.ds,
                sizeof(before.data.ds)) != 0 || core_machine_memory_read_physical(
                &state.machine->executor_memory, 0x3010u,
                CORE_MACHINE_REFERENCE_OF(source_after), width) != LIB_STATUS_OK ||
                (width == 1u ? (source_after & 0xffu) != (source & 0xffu) :
                source_after != source);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    {
        lods_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        lib_u8 first = 0x51u;
        lib_u8 second = 0x62u;
        lib_u8 first_after;
        lib_u8 second_after;
        lib_i32 failed = !lods_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed)
            failed |= !lods_boot_protected(&state);
        if (!failed) {
            lods_seed_protected(&state, 0x10u);
            state.machine->executor_cpu.data.ecx = 0x11220003u;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &first,
                sizeof(first)) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x3011u, &second, sizeof(second)) !=
                LIB_STATUS_OK || core_machine_memory_write(state.machine,
                0x2000u, rep_ac, sizeof(rep_ac)) != LIB_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || after.data.eax != 0xaabb3351u ||
                after.data.ecx != 0x11220002u || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
                after.data.ebp != before.data.ebp || after.data.esi != 0x11u ||
                after.data.edi != before.data.edi || after.data.eflags !=
                before.data.eflags || lib_memory_compare(&before.data.ds, &after.data.ds,
                sizeof(before.data.ds)) != 0 || core_machine_memory_read_physical(
                &state.machine->executor_memory, 0x3010u,
                CORE_MACHINE_REFERENCE_OF(first_after), sizeof(first_after)) !=
                LIB_STATUS_OK || first_after != first ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x3011u, CORE_MACHINE_REFERENCE_OF(second_after), sizeof(second_after)) !=
                LIB_STATUS_OK || second_after != second;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 lods_test_irq_no_shadow(void)
{
    static const lib_u8 codes[][2] = {
        {0xacu, 0x90u},
        {0xadu, 0x90u}
    };
    static const lib_u8 hlt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != 2u; ++form) {
        lods_machine state;
        core_machine_pic_irq_source irq;
        core_machine_run_result result;
        t_cpu after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u32 source = form == 0u ? 0x51u : 0x2262u;
        lib_u32 source_after;
        lib_u8 width = form == 0u ? 1u : 2u;
        lib_i32 failed = !lods_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u) || core_machine_memory_write(state.machine,
                0x10010u, &source, width) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, codes[form],
                sizeof(codes[form])) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x80u, &offset,
                sizeof(offset)) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x82u, &segment, sizeof(segment)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            lods_seed(&state);
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            lib_memory_set(&irq, 0, sizeof(irq));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&irq);
            core_machine_pic_irq_source_deassert(&irq);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (lib_u16)after.data.esp,
                CORE_MACHINE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != LIB_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != 1u ||
                !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(
                state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                after.data.eax != (form == 0u ? 0xaabb3351u : 0xaabb2262u) ||
                after.data.esi != 0x10u + width || core_machine_memory_read_physical(
                &state.machine->executor_memory, 0x10010u,
                CORE_MACHINE_REFERENCE_OF(source_after), width) != LIB_STATUS_OK ||
                (width == 1u ? (source_after & 0xffu) != (source & 0xffu) :
                (source_after & 0xffffu) != (source & 0xffffu));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 lods_test_irq_rep_restart(void)
{
    static const lib_u8 code[] = {0xf3u, 0xacu, 0x90u};
    static const lib_u8 hlt = 0xf4u;
    lods_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    t_cpu after;
    lib_u16 offset = 0x100u;
    lib_u16 segment = 0u;
    lib_u16 frame_ip = 0u;
    lib_u8 source[] = {0x51u, 0x62u, 0x73u};
    lib_u8 source_after[sizeof(source)] = {0u, 0u, 0u};
    lib_i32 failed = !lods_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u) || core_machine_memory_write(state.machine, 0x10010u,
            source, sizeof(source)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x80u, &offset,
            sizeof(offset)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x82u, &segment, sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x100u, &hlt, sizeof(hlt)) !=
            LIB_STATUS_OK;
    }
    if (!failed) {
        lods_seed(&state);
        state.machine->executor_cpu.data.ecx = 0x11220003u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        lib_memory_set(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){4u, 0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (lib_u16)after.data.esp,
            CORE_MACHINE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != LIB_STATUS_OK ||
            after.data.eip != 0x101u || frame_ip != 0u ||
            !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(
            state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            after.data.eax != 0xaabb3351u || after.data.ecx != 0x11220002u ||
            after.data.esi != 0x11u || core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x10010u,
            (lib_uptr)source_after, sizeof(source_after)) !=
            LIB_STATUS_OK || lib_memory_compare(source_after, source, sizeof(source)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    static const core_machine_cpu_profile profiles[] = {CORE_MACHINE_CPU_PROFILE_8086,CORE_MACHINE_CPU_PROFILE_80186,CORE_MACHINE_CPU_PROFILE_80286,CORE_MACHINE_CPU_PROFILE_80386};
    static const lib_u8 ac = 0xacu, ad = 0xadu, dword[] = {0x66u,0xadu}, addr[] = {0x67u,0xacu}, both[] = {0x66u,0x67u,0xadu};
    static const lib_u8 cs[] = {0x2eu,0xacu}, ss[] = {0x36u,0xacu}, es[] = {0x26u,0xacu}, fs[] = {0x64u,0xacu}, gs[] = {0x65u,0xacu};
    lib_u8 profile;
    for (profile=0u; profile != sizeof(profiles)/sizeof(profiles[0]); ++profile)
        if (!lods_case(profiles[profile], &ac,1u,1u,0,0,0x10010u,0x12345678u) || !lods_case(profiles[profile], &ad,1u,2u,0,0,0x10010u,0x12345678u)) return 1;
    if (!lods_case(CORE_MACHINE_CPU_PROFILE_80386,dword,2u,4u,0,0,0x10010u,0x12345678u) || !lods_case(CORE_MACHINE_CPU_PROFILE_80386,addr,2u,1u,1,0,0x11010u,0x12345678u) || !lods_case(CORE_MACHINE_CPU_PROFILE_80386,both,3u,4u,1,0,0x11010u,0x12345678u) || !lods_case(CORE_MACHINE_CPU_PROFILE_80386,&ad,1u,2u,0,1,0x10010u,0x12345678u) || !lods_case(CORE_MACHINE_CPU_PROFILE_80386,cs,2u,1u,0,0,0x10u,0x51u) || !lods_case(CORE_MACHINE_CPU_PROFILE_80386,ss,2u,1u,0,0,0x30010u,0x52u) || !lods_case(CORE_MACHINE_CPU_PROFILE_80386,es,2u,1u,0,0,0x20010u,0x53u) || !lods_case(CORE_MACHINE_CPU_PROFILE_80386,fs,2u,1u,0,0,0x40010u,0x54u) || !lods_case(CORE_MACHINE_CPU_PROFILE_80386,gs,2u,1u,0,0,0x50010u,0x55u)) return 1;
    {
        static const lib_u8 rep_ac[] = {0xf3u, 0xacu};
        static const lib_u8 rep_ad[] = {0xf3u, 0xadu};
        static const lib_u8 rep_66_ad[] = {0xf3u, 0x66u, 0xadu};
        static const lib_u8 rep_67_ac[] = {0xf3u, 0x67u, 0xacu};
        static const lib_u8 rep_66_67_ad[] = {0xf3u, 0x66u, 0x67u, 0xadu};
        static const lib_u32 byte_source[] = {0x51u, 0x62u, 0x73u};
        static const lib_u32 word_source[] = {0x1151u, 0x2262u, 0x3373u};
        static const lib_u32 dword_source[] = {0x10203040u, 0x50607080u,
            0x90a0b0c0u};

        for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
            if (!lods_rep_case(profiles[profile], rep_ac, sizeof(rep_ac), 1u,
                0, 0, 0u, byte_source) || !lods_rep_case(profiles[profile],
                rep_ac, sizeof(rep_ac), 1u, 0, 0, 1u, byte_source) ||
                !lods_rep_case(profiles[profile], rep_ad, sizeof(rep_ad), 2u,
                0, 0, 3u, word_source))
                return 1;
        }
        if (!lods_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_66_ad,
            sizeof(rep_66_ad), 4u, 0, 0, 3u, dword_source) ||
            !lods_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_67_ac,
            sizeof(rep_67_ac), 1u, 1, 0, 3u, byte_source) ||
            !lods_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_66_67_ad,
            sizeof(rep_66_67_ad), 4u, 1, 0, 3u, dword_source) ||
            !lods_rep_case(CORE_MACHINE_CPU_PROFILE_80386, rep_ac,
            sizeof(rep_ac), 1u, 0, 1, 3u, byte_source))
            return 1;
    }
    if (!lods_test_rejections())
        return 1;
    if (!lods_test_protected_limits())
        return 1;
    if (!lods_test_irq_no_shadow())
        return 1;
    if (!lods_test_irq_rep_restart())
        return 1;
    printf("M5:T316:S35:LODS:OK\n");
    printf("M5:T401:S18:LODS-PROFILES:OK\n");
    return 0;
}
