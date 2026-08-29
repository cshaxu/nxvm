#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct legacy_lock_s1_machine {
    core_machine *machine;
    type_unsigned_32 writes;
    type_unsigned_16 last_port;
    type_unsigned_32 last_value;
} legacy_lock_s1_machine;

static type_status legacy_lock_s1_port_read(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 *value)
{
    (C_VOID)owner;
    (C_VOID)port;
    if (value == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    *value = 0u;
    return TYPE_STATUS_OK;
}

static type_status legacy_lock_s1_port_write(C_VOID *owner,
    type_unsigned_16 port, type_unsigned_32 value)
{
    legacy_lock_s1_machine *state = (legacy_lock_s1_machine *)owner;

    if (state == STD_NULL)
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++state->writes;
    state->last_port = port;
    state->last_value = value;
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider legacy_lock_s1_port_provider = {
    legacy_lock_s1_port_read, legacy_lock_s1_port_write
};

static C_VOID legacy_lock_s1_reset(C_VOID *opaque)
{
    legacy_lock_s1_machine *state = (legacy_lock_s1_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider legacy_lock_s1_provider = {
    legacy_lock_s1_reset, STD_NULL
};

static C_INT legacy_lock_s1_prepare(core_machine_cpu_profile profile,
    legacy_lock_s1_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL)
        return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine) == TYPE_STATUS_OK &&
        core_machine_install_port_provider(state->machine, 0x005au, 0x005au,
            &legacy_lock_s1_port_provider, state) == TYPE_STATUS_OK &&
        test_core_machine_fixture_bind_freeze_reset(state->machine,
        &legacy_lock_s1_provider, state) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine,
            0u);
}

static C_INT legacy_lock_s1_sregs_same(const t_cpu *before,
    const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 && STD_MEMCMP(&before->data.cs,
        &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
        sizeof(before->data.ss)) == 0 && STD_MEMCMP(&before->data.ds,
        &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 && STD_MEMCMP(&before->data.gs,
        &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT legacy_lock_s1_cpu_same(const t_cpu *before, const t_cpu *after)
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

static C_INT legacy_lock_s1_run(legacy_lock_s1_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T count, type_unsigned_32 budget,
    t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    type_status *status, core_machine_run_result *result)
{
    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        after == STD_NULL || diagnostic == STD_NULL || status == STD_NULL ||
        result == STD_NULL || core_machine_memory_write(state->machine,
            state->machine->executor_cpu.data.cs.base +
            state->machine->executor_cpu.data.eip, code, count) !=
            TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ budget, 0u }, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT legacy_lock_s1_test_transparent_real(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 cbw[] = { 0xf0u, 0x98u };
    static const type_unsigned_8 add_memory[] = {
        0xf0u, 0x01u, 0x06u, 0x00u, 0x01u
    };
    static const type_unsigned_8 rep_movs[] = { 0xf0u, 0xf3u, 0xa4u };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        legacy_lock_s1_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_16 image = 3u;
        type_unsigned_8 source[] = { 0x31u, 0x42u };
        type_unsigned_8 target[] = { 0u, 0u };
        C_INT failed = !legacy_lock_s1_prepare(profiles[profile], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabb0080u;
            before = state.machine->executor_cpu;
            failed |= !legacy_lock_s1_run(&state, cbw, sizeof(cbw), 1u,
                &after, &diagnostic, &status, &result) ||
                status != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
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
                sizeof(image)) != TYPE_STATUS_OK || !legacy_lock_s1_run(&state,
                add_memory, sizeof(add_memory), 1u, &after, &diagnostic,
                &status, &result) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != sizeof(add_memory) ||
                core_machine_memory_read(state.machine, 0x100u, &image,
                sizeof(image)) != TYPE_STATUS_OK || image != 5u;
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
                sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x200u, target, sizeof(target)) != TYPE_STATUS_OK ||
                !legacy_lock_s1_run(&state, rep_movs, sizeof(rep_movs), 2u,
                &after, &diagnostic, &status, &result) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != sizeof(rep_movs) || after.data.ecx != 0x11220000u ||
                after.data.esi != 0x0102u || after.data.edi != 0x0202u ||
                core_machine_memory_read(state.machine, 0x200u, target,
                sizeof(target)) != TYPE_STATUS_OK || STD_MEMCMP(source, target,
                sizeof(source)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_lock_s1_test_port_output(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 code[] = { 0xf0u, 0xe6u, 0x5au };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        legacy_lock_s1_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_status status;
        C_INT failed = !legacy_lock_s1_prepare(profiles[profile], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabbcc44u;
            before = state.machine->executor_cpu;
            failed |= !legacy_lock_s1_run(&state, code, sizeof(code), 1u,
                &after, &diagnostic, &status, &result) ||
                status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
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

static C_INT legacy_lock_s1_test_legacy_ud(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 code[] = { 0xf0u, 0xf1u };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        legacy_lock_s1_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_status status;
        C_INT failed = !legacy_lock_s1_prepare(profiles[profile], &state);

        if (!failed) {
            before = state.machine->executor_cpu;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine) || !legacy_lock_s1_run(&state, code, sizeof(code), 1u,
                &after, &diagnostic, &status, &result) ||
                status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD) || !legacy_lock_s1_cpu_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_lock_s1_prepare_80286_protected(
    legacy_lock_s1_machine *state, type_unsigned_8 cpl,
    type_unsigned_32 eflags)
{
    static const type_unsigned_8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0xfau,0u,0u,
        0xffu,0xffu,0u,0u,0u,0xf2u,0u,0u
    };
    static const type_unsigned_8 handler[] = { 0xf4u };
    type_unsigned_8 gate[8] = { 0u };
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
    cpu->data.gdtr.flagValid = TYPE_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = 0x0300u;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = TYPE_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = 0x0400u;
    cpu->data.idtr.limit = 0x006fu;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.selector = 0x000bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = 0x2000u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.cs.seg.exec.defsize = TYPE_FALSE;
    cpu->data.cs.seg.exec.readable = TYPE_TRUE;
    cpu->data.ss.flagValid = TYPE_TRUE;
    cpu->data.ss.selector = 0x0013u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffu;
    cpu->data.ss.dpl = cpl;
    cpu->data.ss.seg.data.writable = TYPE_TRUE;
    cpu->data.ss.seg.data.big = TYPE_FALSE;
    cpu->data.ds = cpu->data.ss;
    cpu->data.ds.sregtype = SREG_DATA;
    return core_machine_memory_write(state->machine, 0x0300u, gdt,
        sizeof(gdt)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x0400u + 0x0du * 8u, gate, sizeof(gate)) ==
        TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0x2100u,
        handler, sizeof(handler)) == TYPE_STATUS_OK;
}

static C_INT legacy_lock_s1_test_80286_iopl(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xf0u, 0x98u };
    legacy_lock_s1_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu after;
    type_status status;
    type_unsigned_16 frame[4] = { 0u, 0u, 0u, 0u };
    C_INT failed = !legacy_lock_s1_prepare_80286_protected(&state, 0u, 0u);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabb0080u;
        failed |= !legacy_lock_s1_run(&state, code, sizeof(code), 1u, &after,
            &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
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
            &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
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
            &diagnostic, &status, &result) || status != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_GP) || diagnostic.last_delivered_exception.exception_code != 0u ||
            after.data.cs.selector != 0x000bu || after.data.eip != 0x0100u ||
            after.data.eax != 0xaabb0080u ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
            frame[0] != 0u || frame[1] != 0u || frame[2] != 0x000bu ||
            frame[3] != VCPU_EFLAGS_CF;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT legacy_lock_s1_test_80386_regression(C_VOID)
{
    static const type_unsigned_8 legal[] = {
        0xf0u, 0x01u, 0x06u, 0x00u, 0x01u
    };
    static const type_unsigned_8 invalid[] = { 0xf0u, 0x01u, 0xc0u };
    legacy_lock_s1_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status status;
    type_unsigned_16 image = 1u;
    C_INT failed = !legacy_lock_s1_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 2u;
        failed |= core_machine_memory_write(state.machine, 0x100u, &image,
            sizeof(image)) != TYPE_STATUS_OK || !legacy_lock_s1_run(&state,
            legal, sizeof(legal), 1u, &after, &diagnostic, &status, &result) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            core_machine_memory_read(state.machine, 0x100u, &image,
            sizeof(image)) != TYPE_STATUS_OK || image != 3u;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !legacy_lock_s1_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
    if (!failed) {
        before = state.machine->executor_cpu;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine) || !legacy_lock_s1_run(&state, invalid, sizeof(invalid), 1u,
            &after, &diagnostic, &status, &result) || status != TYPE_STATUS_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            !legacy_lock_s1_cpu_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

int main(C_VOID)
{
    if (!legacy_lock_s1_test_transparent_real() ||
        !legacy_lock_s1_test_port_output() ||
        !legacy_lock_s1_test_legacy_ud() || !legacy_lock_s1_test_80286_iopl() ||
        !legacy_lock_s1_test_80386_regression()) {
        STD_FPRINTF(STD_STDERR, "M5:T328:S1:LEGACY-LOCK failed\n");
        return 1;
    }
    STD_PRINTF("M5:T328:S1:LEGACY-LOCK:OK\n");
    return 0;
}
