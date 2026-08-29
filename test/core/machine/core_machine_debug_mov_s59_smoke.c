#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

/* T337_REAL_UD_VECTOR6_DELIVERY: this owner installs and observes vector 6. */

#define DEBUG_MOV_S59_GDT_POINTER 0x0100u
#define DEBUG_MOV_S59_GDT 0x0300u
#define DEBUG_MOV_S59_CODE 0x2000u

typedef struct debug_mov_s59_machine {
    core_machine *machine;
} debug_mov_s59_machine;

static C_VOID debug_mov_s59_reset(C_VOID *opaque)
{
    debug_mov_s59_machine *state = (debug_mov_s59_machine *)opaque;

    if (state != STD_NULL) {
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider debug_mov_s59_execution_provider = {
    debug_mov_s59_reset, STD_NULL
};

static C_INT debug_mov_s59_prepare(debug_mov_s59_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &debug_mov_s59_execution_provider, state, &state->machine)) {
        if (state->machine != STD_NULL) core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT debug_mov_s59_boot_protected(debug_mov_s59_machine *state)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x17u, 0x00u, 0x00u, 0x03u, 0u, 0u };
    static const type_unsigned_8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0u, 0u, 0x92u, 0u, 0u
    };
    static const type_unsigned_8 code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u,
        0x8eu, 0xd0u, 0xbcu, 0x00u, 0x80u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;

    if (state == STD_NULL || state->machine == STD_NULL ||
        core_machine_memory_write(state->machine, DEBUG_MOV_S59_GDT_POINTER,
            gdt_pointer, sizeof(gdt_pointer)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, DEBUG_MOV_S59_GDT, gdt,
            sizeof(gdt)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, 0u, code, sizeof(code)) !=
            TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, DEBUG_MOV_S59_CODE, halt,
            sizeof(halt)) != TYPE_STATUS_OK ||
        core_machine_run(state->machine, (core_machine_run_budget){ 64u, 0u },
            &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) return 0;
    return 1;
}

static C_INT debug_mov_s59_run(debug_mov_s59_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T bytes, t_cpu *out_cpu,
    core_machine_cpu_diagnostic *out_diagnostic, type_status *out_status)
{
    core_machine_run_result result;
    type_status status;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        out_cpu == STD_NULL || out_diagnostic == STD_NULL || out_status == STD_NULL ||
        core_machine_memory_write(state->machine, state->machine->executor_cpu.data.cs.base, code,
            bytes) != TYPE_STATUS_OK) return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    status = core_machine_run(state->machine, (core_machine_run_budget){ 16u, 0u },
        &result);
    *out_status = status;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT debug_mov_s59_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT debug_mov_s59_install_real_ud_vector(debug_mov_s59_machine *state)
{
    static const type_unsigned_8 handler[] = { 0xf4u };
    static const type_unsigned_8 vector[] = { 0x00u, 0x01u, 0x00u, 0x00u };

    return core_machine_memory_write(state->machine, 0x18u, vector,
        sizeof(vector)) == TYPE_STATUS_OK && core_machine_memory_write(
            state->machine, 0x0100u, handler, sizeof(handler)) == TYPE_STATUS_OK;
}

static C_INT debug_mov_s59_ud_delivered(const t_cpu *before, const t_cpu *after,
    const core_machine_cpu_diagnostic *diagnostic)
{
    return !diagnostic->first_fault.valid &&
        diagnostic->last_delivered_exception.valid && TYPE_GET_BIT(
            diagnostic->last_delivered_exception.exception_mask, VCPUINS_EXCEPT_UD) &&
        after->data.eip == 0x0101u && after->data.esp ==
            ((before->data.esp & 0xffff0000u) |
                (type_unsigned_16)(before->data.esp - 6u)) &&
        after->data.eax == before->data.eax && after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx && after->data.ebx == before->data.ebx &&
        after->data.ebp == before->data.ebp && after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi && after->data.eflags ==
            (before->data.eflags & ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF)) &&
        after->data.dr0 == before->data.dr0 && after->data.dr1 == before->data.dr1 &&
        after->data.dr2 == before->data.dr2 && after->data.dr3 == before->data.dr3 &&
        after->data.dr6 == before->data.dr6 && after->data.dr7 == before->data.dr7 &&
        debug_mov_s59_sregs_same(before, after);
}

static C_INT debug_mov_s59_reject_state_same(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags &&
        before->data.dr0 == after->data.dr0 &&
        before->data.dr1 == after->data.dr1 &&
        before->data.dr2 == after->data.dr2 &&
        before->data.dr3 == after->data.dr3 &&
        before->data.dr6 == after->data.dr6 &&
        before->data.dr7 == after->data.dr7 &&
        debug_mov_s59_sregs_same(before, after);
}
static type_unsigned_32 *debug_mov_s59_dr(t_cpu *cpu, type_unsigned_8 index)
{
    switch (index) {
    case 0: return &cpu->data.dr0;
    case 1: return &cpu->data.dr1;
    case 2: return &cpu->data.dr2;
    case 3: return &cpu->data.dr3;
    case 6: return &cpu->data.dr6;
    case 7: return &cpu->data.dr7;
    default: return STD_NULL;
    }
}

static C_INT debug_mov_s59_test_round_trips(C_VOID)
{
    static const type_unsigned_8 indices[] = { 0u, 1u, 2u, 3u, 6u, 7u };
    debug_mov_s59_machine state;
    C_INT failed = 0;
    STD_SIZE_T i;

    if (!debug_mov_s59_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
        !debug_mov_s59_boot_protected(&state)) return 1;
    for (i = 0u; i < (sizeof(indices) / sizeof(indices[0])); ++i) {
        type_unsigned_8 code[] = { 0x0fu, 0x23u, (type_unsigned_8)(0xc1u | (indices[i] << 3u)),
            0x0fu, 0x21u, (type_unsigned_8)(0xc0u | (indices[i] << 3u)), 0xf4u };
        t_cpu before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;
        type_unsigned_32 value = 0x10203040u + (type_unsigned_32)indices[i];

        state.machine->executor_cpu.data.ecx = value;
        state.machine->executor_cpu.data.eax = 0xaabbccddu;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !debug_mov_s59_run(&state, code, sizeof(code), &after,
            &diagnostic, &status);
        failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid;
        failed |= after.data.eip != sizeof(code) || after.data.eax != value;
        failed |= *debug_mov_s59_dr(&after, indices[i]) != value;
        failed |= after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags ||
            !debug_mov_s59_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT debug_mov_s59_test_real_profiles(C_VOID)
{
    static const core_machine_cpu_profile rejected[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 reject_code[] = { 0x0fu, 0x21u, 0xc0u };
    static const type_unsigned_8 success_code[] = { 0x0fu, 0x23u, 0xf9u,
        0x0fu, 0x21u, 0xf8u, 0xf4u };
    C_INT failed = 0;
    STD_SIZE_T i;

    for (i = 0u; i < (sizeof(rejected) / sizeof(rejected[0])); ++i) {
        debug_mov_s59_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;

        if (!debug_mov_s59_prepare(&state, rejected[i]) ||
            !debug_mov_s59_install_real_ud_vector(&state)) return 1;
        state.machine->executor_cpu.data.eax = 0xaabbccddu;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !debug_mov_s59_run(&state, reject_code, sizeof(reject_code),
            &after, &diagnostic, &status);
        failed |= status != TYPE_STATUS_OK || !debug_mov_s59_ud_delivered(&before,
            &after, &diagnostic);
        core_machine_destroy(state.machine);
    }
    {
        debug_mov_s59_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;

        if (!debug_mov_s59_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 1;
        state.machine->executor_cpu.data.ecx = 0x5a5aa5a5u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !debug_mov_s59_run(&state, success_code, sizeof(success_code),
            &after, &diagnostic, &status);
        failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(success_code) ||
            after.data.eax != before.data.ecx || after.data.dr7 != before.data.ecx ||
            after.data.eflags != before.data.eflags ||
            !debug_mov_s59_sregs_same(&before, &after);
        core_machine_destroy(state.machine);
    }
    return failed;
}
static C_INT debug_mov_s59_test_rejections(C_VOID)
{
    static const type_unsigned_8 forms[][3] = {
        { 0x0fu, 0x21u, 0xe0u }, { 0x0fu, 0x23u, 0xe8u },
        { 0x0fu, 0x21u, 0x00u }, { 0x0fu, 0x23u, 0x00u }
    };
    C_INT failed = 0;
    STD_SIZE_T i;

    for (i = 0u; i < (sizeof(forms) / sizeof(forms[0])); ++i) {
        debug_mov_s59_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;

        if (!debug_mov_s59_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !debug_mov_s59_boot_protected(&state)) return 1;
        state.machine->executor_cpu.data.eax = 0xaabbccddu;
        state.machine->executor_cpu.data.ecx = 0x11223344u;
        state.machine->executor_cpu.data.dr0 = 0x55667788u;
        state.machine->executor_cpu.data.dr7 = 0x99aabbccu;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !debug_mov_s59_run(&state, forms[i], sizeof(forms[i]), &after,
            &diagnostic, &status);
        failed |= status != TYPE_STATUS_FAULT || after.data.eip != 0u ||
            !debug_mov_s59_reject_state_same(&before, &after);
        core_machine_destroy(state.machine);
    }
    return failed;
}

static C_INT debug_mov_s59_test_attributes(C_VOID)
{
    static const type_unsigned_8 prefixes[][2] = {
        { 0x66u, 0u }, { 0x67u, 0u }, { 0x66u, 0x67u }
    };
    C_INT failed = 0;
    STD_SIZE_T i;

    for (i = 0u; i < (sizeof(prefixes) / sizeof(prefixes[0])); ++i) {
        debug_mov_s59_machine state;
        type_unsigned_8 code[9] = { 0u };
        STD_SIZE_T length = prefixes[i][1] == 0u ? 7u : 8u;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;

        if (!debug_mov_s59_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !debug_mov_s59_boot_protected(&state)) return 1;
        code[0] = prefixes[i][0];
        if (prefixes[i][1] != 0u) code[1] = prefixes[i][1];
        code[length - 6u] = 0x0fu;
        code[length - 5u] = 0x23u;
        code[length - 4u] = 0xc1u;
        code[length - 3u] = 0x0fu;
        code[length - 2u] = 0x21u;
        code[length - 1u] = 0xc0u;
        code[length] = 0xf4u;
        ++length;
        state.machine->executor_cpu.data.ecx = 0x5a5aa5a5u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !debug_mov_s59_run(&state, code, length, &after, &diagnostic,
            &status);
        failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != length || after.data.eax != before.data.ecx ||
            after.data.dr0 != before.data.ecx || after.data.eflags != before.data.eflags ||
            !debug_mov_s59_sregs_same(&before, &after);
        core_machine_destroy(state.machine);
    }
    return failed;
}
static C_INT debug_mov_s59_test_privilege_and_lock(C_VOID)
{
    static const type_unsigned_8 cpl3_forms[][3] = {
        { 0x0fu, 0x21u, 0xc0u }, { 0x0fu, 0x23u, 0xc1u }
    };
    static const type_unsigned_8 lock_forms[][4] = {
        { 0xf0u, 0x0fu, 0x21u, 0xc0u },
        { 0xf0u, 0x0fu, 0x23u, 0xc1u }
    };
    C_INT failed = 0;
    STD_SIZE_T i;

    for (i = 0u; i < (sizeof(cpl3_forms) / sizeof(cpl3_forms[0])); ++i) {
        debug_mov_s59_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;

        if (!debug_mov_s59_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !debug_mov_s59_boot_protected(&state)) return 1;
        state.machine->executor_cpu.data.cs.dpl = 3u;
        state.machine->executor_cpu.data.eax = 0xaabbccddu;
        state.machine->executor_cpu.data.ecx = 0x11223344u;
        state.machine->executor_cpu.data.dr0 = 0x55667788u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !debug_mov_s59_run(&state, cpl3_forms[i], sizeof(cpl3_forms[i]),
            &after, &diagnostic, &status);
        failed |= status != TYPE_STATUS_FAULT || after.data.eip != 0u ||
            !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            !debug_mov_s59_reject_state_same(&before, &after);
        core_machine_destroy(state.machine);
    }
    for (i = 0u; i < (sizeof(lock_forms) / sizeof(lock_forms[0])); ++i) {
        debug_mov_s59_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;

        if (!debug_mov_s59_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !debug_mov_s59_boot_protected(&state)) return 1;
        state.machine->executor_cpu.data.eax = 0xaabbccddu;
        state.machine->executor_cpu.data.ecx = 0x11223344u;
        state.machine->executor_cpu.data.dr0 = 0x55667788u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !debug_mov_s59_run(&state, lock_forms[i], sizeof(lock_forms[i]),
            &after, &diagnostic, &status);
        failed |= status != TYPE_STATUS_FAULT || after.data.eip != 0u ||
            !debug_mov_s59_reject_state_same(&before, &after);
        core_machine_destroy(state.machine);
    }
    return failed;
}
static C_INT debug_mov_s59_test_pic_no_shadow(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x0fu, 0x23u, 0xc1u, 0x90u };
    static const type_unsigned_8 halt[] = { 0xf4u };
    debug_mov_s59_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_unsigned_32 frame[3u] = { 0u, 0u, 0u };
    type_unsigned_8 gate[8u] = { 0u };
    C_INT failed = !debug_mov_s59_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed = !debug_mov_s59_boot_protected(&state);
    if (!failed) {
        gate[0] = 0x00u;
        gate[1] = 0x01u;
        gate[2] = 0x08u;
        gate[5] = 0x8eu;
        state.machine->executor_cpu.data.idtr.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.idtr.sregtype = SREG_IDTR;
        state.machine->executor_cpu.data.idtr.base = 0x0400u;
        state.machine->executor_cpu.data.idtr.limit = 0x0107u;
        state.machine->executor_cpu.data.ecx = 0x11223344u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine, 0x0500u, gate,
            sizeof(gate)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, DEBUG_MOV_S59_CODE + 0x100u,
                halt, sizeof(halt)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, DEBUG_MOV_S59_CODE, code,
                sizeof(code)) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){ 2u, 0u },
            &result) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            after.data.eip != 0x101u || after.data.dr0 != before.data.ecx ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
            frame[0] != 3u || frame[1] != before.data.cs.selector ||
            frame[2] != before.data.eflags;
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}
C_INT main(C_VOID)
{
    C_INT round = debug_mov_s59_test_round_trips();
    C_INT real = debug_mov_s59_test_real_profiles();
    C_INT reject = debug_mov_s59_test_rejections();
    C_INT attributes = debug_mov_s59_test_attributes();
    C_INT boundary = debug_mov_s59_test_privilege_and_lock();
    C_INT pic = debug_mov_s59_test_pic_no_shadow();
    if (round || real || reject || attributes || boundary || pic) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S59:DEBUG-MOV failed round=%d profiles=%d reject=%d attributes=%d boundary=%d pic=%d\n", round, real, reject, attributes, boundary, pic);
        return 1;
    }
    STD_PRINTF("M5:T316:S59:DEBUG-MOV:OK\n");
    STD_PRINTF("M5:T401:S66:DEBUG-MOV-PROFILES:OK\n");
    return 0;
}
