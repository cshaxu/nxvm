#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct legacy_lock_s1_machine {
    core_machine *machine;
    lib_u32 writes;
    lib_u16 last_port;
    lib_u32 last_value;
} legacy_lock_s1_machine;

static lib_status legacy_lock_s1_port_read(void *owner,
    lib_u16 port, lib_u32 *value)
{
    (void)owner;
    (void)port;
    if (value == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *value = 0u;
    return LIB_STATUS_OK;
}

static lib_status legacy_lock_s1_port_write(void *owner,
    lib_u16 port, lib_u32 value)
{
    legacy_lock_s1_machine *state = (legacy_lock_s1_machine *)owner;

    if (state == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    state->last_port = port;
    state->last_value = value;
    return LIB_STATUS_OK;
}

static const core_machine_port_provider legacy_lock_s1_port_provider = {
    legacy_lock_s1_port_read, legacy_lock_s1_port_write
};

static void legacy_lock_s1_reset(void *opaque)
{
    legacy_lock_s1_machine *state = (legacy_lock_s1_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider legacy_lock_s1_provider = {
    legacy_lock_s1_reset, LIB_NULL
};

static lib_i32 legacy_lock_s1_prepare(core_machine_cpu_profile profile,
    legacy_lock_s1_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == LIB_NULL)
        return 0;
    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine) == LIB_STATUS_OK &&
        core_machine_install_port_provider(state->machine, 0x005au, 0x005au,
            &legacy_lock_s1_port_provider, state) == LIB_STATUS_OK &&
        test_core_machine_fixture_bind_freeze_reset(state->machine,
        &legacy_lock_s1_provider, state) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine,
            0u);
}

static lib_i32 legacy_lock_s1_sregs_same(const t_cpu *before,
    const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 && lib_memory_compare(&before->data.cs,
        &after->data.cs, sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
        sizeof(before->data.ss)) == 0 && lib_memory_compare(&before->data.ds,
        &after->data.ds, sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 && lib_memory_compare(&before->data.gs,
        &after->data.gs, sizeof(before->data.gs)) == 0;
}

static lib_i32 legacy_lock_s1_cpu_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eip == after->data.eip &&
        before->data.eflags == after->data.eflags &&
        legacy_lock_s1_sregs_same(before, after);
}

static lib_i32 legacy_lock_s1_run(legacy_lock_s1_machine *state,
    const lib_u8 *code, lib_size count, lib_u32 budget,
    t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    lib_status *status, core_machine_run_result *result)
{
    if (state == LIB_NULL || state->machine == LIB_NULL || code == LIB_NULL ||
        after == LIB_NULL || diagnostic == LIB_NULL || status == LIB_NULL ||
        result == LIB_NULL || core_machine_memory_write(state->machine,
            state->machine->executor_cpu.data.cs.base +
            state->machine->executor_cpu.data.eip, code, count) !=
            LIB_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ budget, 0u }, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        LIB_STATUS_OK;
}

static lib_i32 legacy_lock_s1_test_transparent_real(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 cbw[] = { 0xf0u, 0x98u };
    static const lib_u8 add_memory[] = {
        0xf0u, 0x01u, 0x06u, 0x00u, 0x01u
    };
    static const lib_u8 rep_movs[] = { 0xf0u, 0xf3u, 0xa4u };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        legacy_lock_s1_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u16 image = 3u;
        lib_u8 source[] = { 0x31u, 0x42u };
        lib_u8 target[] = { 0u, 0u };
        lib_i32 failed = !legacy_lock_s1_prepare(profiles[profile], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabb0080u;
            before = state.machine->executor_cpu;
            failed |= !legacy_lock_s1_run(&state, cbw, sizeof(cbw), 1u,
                &after, &diagnostic, &status, &result) ||
                status != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
                diagnostic.first_fault.valid || after.data.eip != sizeof(cbw) ||
                after.data.eax != 0xaabbff80u ||
                after.data.eflags != before.data.eflags ||
                after.data.ecx != before.data.ecx ||
                after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx ||
                after.data.esp != before.data.esp ||
                after.data.ebp != before.data.ebp ||
                after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi ||
                !legacy_lock_s1_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;

        failed = !legacy_lock_s1_prepare(profiles[profile], &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabb0002u;
            failed |= core_machine_memory_write(state.machine, 0x100u, &image,
                sizeof(image)) != LIB_STATUS_OK || !legacy_lock_s1_run(&state,
                add_memory, sizeof(add_memory), 1u, &after, &diagnostic,
                &status, &result) || status != LIB_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != sizeof(add_memory) ||
                core_machine_memory_read(state.machine, 0x100u, &image,
                sizeof(image)) != LIB_STATUS_OK || image != 5u;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;

        failed = !legacy_lock_s1_prepare(profiles[profile], &state);
        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0x11220002u;
            state.machine->executor_cpu.data.esi = 0x0100u;
            state.machine->executor_cpu.data.edi = 0x0200u;
            failed |= core_machine_memory_write(state.machine, 0x100u, source,
                sizeof(source)) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x200u, target, sizeof(target)) != LIB_STATUS_OK ||
                !legacy_lock_s1_run(&state, rep_movs, sizeof(rep_movs), 2u,
                &after, &diagnostic, &status, &result) ||
                status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(rep_movs) || after.data.ecx != 0x11220000u ||
                after.data.esi != 0x0102u || after.data.edi != 0x0202u ||
                core_machine_memory_read(state.machine, 0x200u, target,
                sizeof(target)) != LIB_STATUS_OK || lib_memory_compare(source, target,
                sizeof(source)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_lock_s1_test_port_output(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 code[] = { 0xf0u, 0xe6u, 0x5au };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        legacy_lock_s1_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_i32 failed = !legacy_lock_s1_prepare(profiles[profile], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabbcc44u;
            before = state.machine->executor_cpu;
            failed |= !legacy_lock_s1_run(&state, code, sizeof(code), 1u,
                &after, &diagnostic, &status, &result) ||
                status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(code) ||
                after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
                after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi || after.data.eflags != before.data.eflags ||
                !legacy_lock_s1_sregs_same(&before, &after) || state.writes != 1u ||
                state.last_port != 0x005au || state.last_value != 0x44u;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_lock_s1_test_legacy_ud(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 code[] = { 0xf0u, 0xf1u };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        legacy_lock_s1_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_i32 failed = !legacy_lock_s1_prepare(profiles[profile], &state);

        if (!failed) {
            before = state.machine->executor_cpu;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine) || !legacy_lock_s1_run(&state, code, sizeof(code), 1u,
                &after, &diagnostic, &status, &result) ||
                status != LIB_STATUS_INTERNAL_ERROR || !diagnostic.first_fault.valid ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD) || !legacy_lock_s1_cpu_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_lock_s1_prepare_80286_protected(
    legacy_lock_s1_machine *state, lib_u8 cpl,
    lib_u32 eflags)
{
    static const lib_u8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0xfau,0u,0u,
        0xffu,0xffu,0u,0u,0u,0xf2u,0u,0u
    };
    static const lib_u8 handler[] = { 0xf4u };
    lib_u8 gate[8] = { 0u };
    t_cpu *cpu;

    if (!legacy_lock_s1_prepare(CORE_MACHINE_CPU_PROFILE_80286, state))
        return 0;
    cpu = &state->machine->executor_cpu;
    gate[0] = 0x00u;
    gate[1] = 0x01u;
    gate[2] = 0x0bu;
    gate[5] = 0x86u;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.eflags = eflags;
    cpu->data.eip = 0u;
    cpu->data.esp = 0x8000u;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = 0x0300u;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = 0x0400u;
    cpu->data.idtr.limit = 0x006fu;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x000bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = 0x2000u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.defsize = LIB_FALSE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x0013u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffu;
    cpu->data.ss.dpl = cpl;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.ss.seg.data.big = LIB_FALSE;
    cpu->data.ds = cpu->data.ss;
    cpu->data.ds.sregtype = SREG_DATA;
    return core_machine_memory_write(state->machine, 0x0300u, gdt,
        sizeof(gdt)) == LIB_STATUS_OK && core_machine_memory_write(
        state->machine, 0x0400u + 0x0du * 8u, gate, sizeof(gate)) ==
        LIB_STATUS_OK && core_machine_memory_write(state->machine, 0x2100u,
        handler, sizeof(handler)) == LIB_STATUS_OK;
}

static lib_i32 legacy_lock_s1_test_80286_iopl(void)
{
    static const lib_u8 code[] = { 0xf0u, 0x98u };
    legacy_lock_s1_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu after;
    lib_status status;
    lib_u16 frame[4] = { 0u, 0u, 0u, 0u };
    lib_i32 failed = !legacy_lock_s1_prepare_80286_protected(&state, 0u, 0u);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabb0080u;
        failed |= !legacy_lock_s1_run(&state, code, sizeof(code), 1u, &after,
            &diagnostic, &status, &result) || status != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(code) || after.data.eax != 0xaabbff80u;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !legacy_lock_s1_prepare_80286_protected(&state, 3u,
        VCPU_EFLAGS_IOPL | VCPU_EFLAGS_CF);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabb0080u;
        failed |= !legacy_lock_s1_run(&state, code, sizeof(code), 1u, &after,
            &diagnostic, &status, &result) || status != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(code) || after.data.eax != 0xaabbff80u ||
            after.data.eflags != (VCPU_EFLAGS_IOPL | VCPU_EFLAGS_CF);
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !legacy_lock_s1_prepare_80286_protected(&state, 3u,
        VCPU_EFLAGS_CF);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabb0080u;
        failed |= !legacy_lock_s1_run(&state, code, sizeof(code), 1u, &after,
            &diagnostic, &status, &result) || status != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_GP) || diagnostic.last_delivered_exception.exception_code != 0u ||
            after.data.cs.selector != 0x000bu || after.data.eip != 0x0100u ||
            after.data.eax != 0xaabb0080u ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (lib_u16)after.data.esp,
                (lib_uptr)frame, sizeof(frame)) != LIB_STATUS_OK ||
            frame[0] != 0u || frame[1] != 0u || frame[2] != 0x000bu ||
            frame[3] != VCPU_EFLAGS_CF;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 legacy_lock_s1_test_80386_regression(void)
{
    static const lib_u8 legal[] = {
        0xf0u, 0x01u, 0x06u, 0x00u, 0x01u
    };
    static const lib_u8 invalid[] = { 0xf0u, 0x01u, 0xc0u };
    legacy_lock_s1_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_u16 image = 1u;
    lib_i32 failed = !legacy_lock_s1_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 2u;
        failed |= core_machine_memory_write(state.machine, 0x100u, &image,
            sizeof(image)) != LIB_STATUS_OK || !legacy_lock_s1_run(&state,
            legal, sizeof(legal), 1u, &after, &diagnostic, &status, &result) ||
            status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
            core_machine_memory_read(state.machine, 0x100u, &image,
            sizeof(image)) != LIB_STATUS_OK || image != 3u;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !legacy_lock_s1_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
    if (!failed) {
        before = state.machine->executor_cpu;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine) || !legacy_lock_s1_run(&state, invalid, sizeof(invalid), 1u,
            &after, &diagnostic, &status, &result) || status != LIB_STATUS_INTERNAL_ERROR ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            !legacy_lock_s1_cpu_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

int main(void)
{
    if (!legacy_lock_s1_test_transparent_real() ||
        !legacy_lock_s1_test_port_output() ||
        !legacy_lock_s1_test_legacy_ud() || !legacy_lock_s1_test_80286_iopl() ||
        !legacy_lock_s1_test_80386_regression()) {
        fprintf(stderr, "M5:T328:S1:LEGACY-LOCK failed\n");
        return 1;
    }
    printf("M5:T328:S1:LEGACY-LOCK:OK\n");
    return 0;
}
