#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define DTTR_S61_GDT_POINTER 0x0100u
#define DTTR_S61_GDT 0x0300u
#define DTTR_S61_CODE 0x2000u

typedef struct dttr_s61_machine { core_machine *machine; } dttr_s61_machine;

static C_VOID dttr_s61_reset(C_VOID *opaque)
{
    dttr_s61_machine *state = (dttr_s61_machine *)opaque;
    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider dttr_s61_provider = {
    dttr_s61_reset, STD_NULL
};

static C_INT dttr_s61_prepare(dttr_s61_machine *state, core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &dttr_s61_provider, state, &state->machine);
}

static C_INT dttr_s61_boot(dttr_s61_machine *state)
{
    static const type_unsigned_8 pointer[] = { 0x27u, 0x00u, 0x00u, 0x03u, 0u, 0u };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0,0,0x92,0,0,
        0x1f,0,0,0x50,0,0x82,0,0,
        0x67,0,0,0x60,0,0x89,0,0
    };
    static const type_unsigned_8 code[] = {
        0x0f,0x01,0x16,0x00,0x01, 0xb8,0x01,0,0x0f,0x01,0xf0,
        0xb8,0x10,0,0x8e,0xd8,0x8e,0xc0,0x8e,0xd0,0xbc,0,0x80,
        0xea,0,0,0x08,0
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;
    return core_machine_memory_write(state->machine, DTTR_S61_GDT_POINTER,
            pointer, sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, DTTR_S61_GDT, gdt, sizeof(gdt)) ==
            TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0u, code,
            sizeof(code)) == TYPE_STATUS_OK && core_machine_memory_write(state->machine,
            DTTR_S61_CODE, halt, sizeof(halt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){64u,0u}, &result) ==
            TYPE_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT dttr_s61_run(dttr_s61_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T bytes, t_cpu *after)
{
    core_machine_run_result result;
    if (core_machine_memory_write(state->machine, DTTR_S61_CODE, code, bytes) !=
        TYPE_STATUS_OK) return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    if (core_machine_run(state->machine, (core_machine_run_budget){16u,0u}, &result) !=
        TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) return 0;
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT dttr_s61_case(core_machine_cpu_profile profile, const type_unsigned_8 *code, STD_SIZE_T bytes, type_unsigned_16 ax,
    type_unsigned_16 ldtr, type_unsigned_16 tr, type_unsigned_8 busy)
{
    dttr_s61_machine state;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 descriptor_access = 0;
    C_INT failed = !dttr_s61_prepare(&state, profile);

    if (!failed) failed = !dttr_s61_boot(&state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xa1a10000u | ax;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = !dttr_s61_run(&state, code, bytes, &after);
        failed |= (type_unsigned_16)after.data.eax != ax || after.data.eip != bytes ||
            after.data.eflags != before.data.eflags || after.data.ldtr.selector != ldtr ||
            after.data.tr.selector != tr || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.esp != before.data.esp || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.edi != before.data.edi;
        if (busy) {
            failed |= core_machine_memory_read(state.machine, DTTR_S61_GDT + 4u * 8u + 5u,
                &descriptor_access, sizeof(descriptor_access)) != TYPE_STATUS_OK ||
                descriptor_access != 0x8bu || after.data.tr.sys.type !=
                    VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
        }
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}

static C_INT dttr_s61_test_attributes(C_VOID)
{
    static const type_unsigned_8 codes[][6] = {
        { 0x66u,0x0fu,0u,0xd0u,0xf4u },
        { 0x67u,0x0fu,0u,0xd0u,0xf4u },
        { 0x66u,0x67u,0x0fu,0u,0xd0u,0xf4u }
    };
    static const type_unsigned_8 lengths[] = { 5u, 5u, 6u };
    C_INT failed = 0;
    STD_SIZE_T i;

    for (i = 0u; i < 3u; ++i) {
        dttr_s61_machine state;
        t_cpu after;
        C_INT local = !dttr_s61_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (!local) local = !dttr_s61_boot(&state);
        if (!local) {
            state.machine->executor_cpu.data.eax = 0xa1a10018u;
            local = !dttr_s61_run(&state, codes[i], lengths[i], &after);
            local |= after.data.eax != 0xa1a10018u ||
                after.data.ldtr.selector != 0x18u || after.data.eip != lengths[i];
        }
        if (state.machine != STD_NULL) core_machine_destroy(state.machine);
        failed |= local;
    }
    return failed;
}static C_INT dttr_s61_expect_fault(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, STD_SIZE_T bytes, C_INT protected)
{
    dttr_s61_machine state;
    core_machine_run_result result;
    t_cpu after;
    type_status status;
    C_INT failed = !dttr_s61_prepare(&state, profile);

    if (!failed && protected) failed = !dttr_s61_boot(&state);
    if (!failed) {
        failed |= core_machine_memory_write(state.machine,
            protected ? DTTR_S61_CODE : 0u, code, bytes) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        if (!protected) failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        status = core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
            &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= status != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
            after.data.eip == 0x0100u;
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}

static C_INT dttr_s61_test_rejections(C_VOID)
{
    static const type_unsigned_8 form[] = { 0x0fu,0x00u,0xc0u };
    static const type_unsigned_8 lock[] = { 0xf0u,0x0fu,0x00u,0xc0u };
    static const type_unsigned_8 prefix66[] = { 0x66u,0x0fu,0x00u,0xc0u };
    static const type_unsigned_8 prefix67[] = { 0x67u,0x0fu,0x00u,0xc0u };
    C_INT a = dttr_s61_expect_fault(CORE_MACHINE_CPU_PROFILE_80186, form, sizeof(form), 0);
    C_INT b = dttr_s61_expect_fault(CORE_MACHINE_CPU_PROFILE_80286, form, sizeof(form), 0);
    C_INT c = dttr_s61_expect_fault(CORE_MACHINE_CPU_PROFILE_80386, form, sizeof(form), 0);
    C_INT d = dttr_s61_expect_fault(CORE_MACHINE_CPU_PROFILE_80386, lock, sizeof(lock), 1);
    C_INT e = dttr_s61_expect_fault(CORE_MACHINE_CPU_PROFILE_80286, prefix66, sizeof(prefix66), 1);
    C_INT f = dttr_s61_expect_fault(CORE_MACHINE_CPU_PROFILE_80286, prefix67, sizeof(prefix67), 1);
    return a || b || c || d || e || f;
}static C_INT dttr_s61_test_null_ldtr(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xb8, 0, 0, 0x0f, 0, 0xd0, 0xf4 };
    dttr_s61_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !dttr_s61_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed = !dttr_s61_boot(&state);
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = !dttr_s61_run(&state, code, sizeof(code), &after);
        failed |= after.data.ldtr.selector != 0u || after.data.ldtr.flagValid ||
            after.data.eip != sizeof(code) || after.data.eflags != before.data.eflags ||
            after.data.tr.selector != before.data.tr.selector;
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}
static C_INT dttr_s61_test_memory_forms(C_VOID)
{
    static const type_unsigned_8 sldt[] = { 0xb8,0x18,0,0x0f,0,0xd0,0x0f,0,0x06,0,0x40,0xf4 };
    static const type_unsigned_8 ltr[] = { 0x0f,0,0x1e,0,0x40,0xf4 };
    dttr_s61_machine state;
    t_cpu after;
    type_unsigned_16 selector = 0;
    C_INT failed = !dttr_s61_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed = !dttr_s61_boot(&state);
    if (!failed) {
        failed = !dttr_s61_run(&state, sldt, sizeof(sldt), &after);
        failed |= core_machine_memory_read(state.machine, 0x4000u, &selector,
            sizeof(selector)) != TYPE_STATUS_OK || selector != 0x18u ||
            after.data.eip != sizeof(sldt) || after.data.ldtr.selector != 0x18u;
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    failed |= !dttr_s61_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed = !dttr_s61_boot(&state);
    if (!failed) {
        selector = 0x20u;
        failed |= core_machine_memory_write(state.machine, 0x4000u, &selector,
            sizeof(selector)) != TYPE_STATUS_OK;
        failed |= !dttr_s61_run(&state, ltr, sizeof(ltr), &after);
        failed |= after.data.tr.selector != 0x20u ||
            after.data.tr.sys.type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    }
    if (state.machine != STD_NULL) core_machine_destroy(state.machine);
    return failed;
}int main(void)
{
    static const type_unsigned_8 lldt[] = { 0xb8,0x18,0,0x0f,0,0xd0,0xf4 };
    static const type_unsigned_8 sldt[] = { 0x0f,0,0xc0,0xf4 };
    static const type_unsigned_8 ltr[] = { 0xb8,0x20,0,0x0f,0,0xd8,0xf4 };
    static const type_unsigned_8 str[] = { 0x0f,0,0xc8,0xf4 };
    C_INT a = dttr_s61_case(CORE_MACHINE_CPU_PROFILE_80386, lldt, sizeof(lldt), 0x18u, 0x18u, 0u, 0u);
    C_INT b = dttr_s61_case(CORE_MACHINE_CPU_PROFILE_80386, sldt, sizeof(sldt), 0u, 0u, 0u, 0u);
    C_INT c = dttr_s61_case(CORE_MACHINE_CPU_PROFILE_80386, ltr, sizeof(ltr), 0x20u, 0u, 0x20u, 1u);
    C_INT d = dttr_s61_case(CORE_MACHINE_CPU_PROFILE_80386, str, sizeof(str), 0u, 0u, 0u, 0u);
    C_INT e = dttr_s61_case(CORE_MACHINE_CPU_PROFILE_80286, lldt, sizeof(lldt), 0x18u, 0x18u, 0u, 0u);
    C_INT f = dttr_s61_case(CORE_MACHINE_CPU_PROFILE_80286, sldt, sizeof(sldt), 0u, 0u, 0u, 0u);
    C_INT g = dttr_s61_case(CORE_MACHINE_CPU_PROFILE_80286, ltr, sizeof(ltr), 0x20u, 0u, 0x20u, 1u);
    C_INT h = dttr_s61_case(CORE_MACHINE_CPU_PROFILE_80286, str, sizeof(str), 0u, 0u, 0u, 0u);
    C_INT failed = a || b || c || d || e || f || g || h ||
        dttr_s61_test_memory_forms() || dttr_s61_test_rejections() ||
        dttr_s61_test_attributes() || dttr_s61_test_null_ldtr();
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T316:S61:DTTR failed\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S61:DTTR:OK\n");
    return 0;
}
