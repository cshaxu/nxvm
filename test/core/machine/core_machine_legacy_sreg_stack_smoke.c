#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct legacy_sreg_stack_machine
{
    core_machine *machine;
} legacy_sreg_stack_machine;

static C_VOID legacy_sreg_stack_reset(C_VOID *opaque)
{
    legacy_sreg_stack_machine *state = (legacy_sreg_stack_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider legacy_sreg_stack_provider = {
    legacy_sreg_stack_reset, STD_NULL
};

static C_INT legacy_sreg_stack_test_lock(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0x06u,0x07u,0x0eu,0x16u,0x17u,0x1eu,0x1fu};
    type_unsigned_8 index;

    for (index = 0u; index != sizeof(opcodes); ++index)
    {
        const core_machine_config config = {
            .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
            .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
            .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
        };
        legacy_sreg_stack_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_8 code[] = {0xf0u, opcodes[index]};
        type_unsigned_32 sentinel = 0xdeadbeefu;
        type_unsigned_32 observed = 0u;
        C_INT failed;

        STD_MEMSET(&state, 0, sizeof(state));
        failed = !test_core_machine_fixture_create_bind_freeze_reset(&config,
            &legacy_sreg_stack_provider, &state, &state.machine) ||
            !test_core_machine_fixture_prepare_real_mode_execution(state.machine, 0u);
        if (!failed)
        {
            state.machine->executor_cpu.data.esp = 0x12348000u;
            failed |= core_machine_memory_write(state.machine, 0x7ffcu, &sentinel,
                sizeof(sentinel)) != TYPE_STATUS_OK;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine, 0u, code, sizeof(code)) !=
                TYPE_STATUS_OK;
            status = core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
                &result);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || status != TYPE_STATUS_FAULT || result.reason !=
                CORE_MACHINE_STOP_FAULT || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                STD_MEMCMP(&before, &after, sizeof(before)) != 0 ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x7ffcu, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
                TYPE_STATUS_OK || observed != sentinel;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_sreg_stack_prepare(core_machine_cpu_profile profile,
    legacy_sreg_stack_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &legacy_sreg_stack_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_INT legacy_sreg_stack_gprs_same_except_esp(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT legacy_sreg_stack_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
        sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
        sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
        sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
        sizeof(before->data.gs)) == 0;
}

static const t_cpu_data_sreg *legacy_sreg_stack_target(const t_cpu *cpu,
    type_unsigned_8 target)
{
    if (target == 0u)
        return &cpu->data.es;
    if (target == 1u)
        return &cpu->data.ss;
    return &cpu->data.ds;
}

static C_INT legacy_sreg_stack_non_target_sregs_same(const t_cpu *before,
    const t_cpu *after, type_unsigned_8 target)
{
    return (target == 0u || STD_MEMCMP(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0) &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
        sizeof(before->data.cs)) == 0 &&
        (target == 1u || STD_MEMCMP(&before->data.ss, &after->data.ss,
        sizeof(before->data.ss)) == 0) &&
        (target == 2u || STD_MEMCMP(&before->data.ds, &after->data.ds,
        sizeof(before->data.ds)) == 0) &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
        sizeof(before->data.gs)) == 0;
}

static C_INT legacy_sreg_stack_real_cache(const t_cpu_data_sreg *sreg,
    type_unsigned_16 selector, type_unsigned_8 target)
{
    return sreg->selector == selector && sreg->flagValid &&
        sreg->base == (type_unsigned_32)selector << 4u && sreg->limit == 0xffffu &&
        sreg->sregtype == (target == 1u ? SREG_STACK : SREG_DATA) &&
        !sreg->seg.executable && sreg->seg.data.writable &&
        !sreg->seg.data.big && !sreg->seg.data.expdown;
}

static C_INT legacy_sreg_stack_protected_cache(const t_cpu_data_sreg *sreg,
    type_unsigned_16 selector, type_unsigned_8 target)
{
    return sreg->selector == selector && sreg->flagValid &&
        sreg->base == 0x5000u && sreg->limit == 0xffffu &&
        sreg->sregtype == (target == 1u ? SREG_STACK : SREG_DATA) &&
        !sreg->seg.executable && sreg->seg.data.writable &&
        !sreg->seg.data.big && !sreg->seg.data.expdown;
}

static C_INT legacy_sreg_stack_test_defaults(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 push_ops[] = {0x06u,0x0eu,0x16u,0x1eu};
    static const type_unsigned_8 pop_ops[] = {0x07u,0x17u,0x1fu};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    {
        type_unsigned_8 form;

        for (form = 0u; form != sizeof(push_ops); ++form)
        {
            legacy_sreg_stack_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            t_cpu before;
            t_cpu after;
            type_status status;
            type_unsigned_32 image = 0u;
            C_INT failed = !legacy_sreg_stack_prepare(profiles[profile], &state);

            if (!failed)
            {
                state.machine->executor_cpu.data.esp = 0x00008000u;
                state.machine->executor_cpu.data.es.selector = 0x1111u;
                state.machine->executor_cpu.data.cs.selector = 0x2222u;
                state.machine->executor_cpu.data.ss.selector = 0x3333u;
                state.machine->executor_cpu.data.ds.selector = 0x4444u;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= core_machine_memory_write(state.machine, 0u, &push_ops[form], 1u) != TYPE_STATUS_OK;
                status = core_machine_run(state.machine, (core_machine_run_budget){1u,0u}, &result);
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
                    status != TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eip != 1u ||
                    after.data.esp != 0x00007ffeu || after.data.eflags != before.data.eflags ||
                    !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                    !legacy_sreg_stack_sregs_same(&before, &after) ||
                    core_machine_memory_read_physical(&state.machine->executor_memory, 0x7ffeu,
                    TYPE_REFERENCE_OF(image), 2u) != TYPE_STATUS_OK || image !=
                    (form == 0u ? 0x1111u : form == 1u ? 0x2222u : form == 2u ? 0x3333u : 0x4444u);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
        for (form = 0u; form != sizeof(pop_ops); ++form)
        {
            legacy_sreg_stack_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            t_cpu before;
            t_cpu after;
            type_status status;
            type_unsigned_16 selector = (type_unsigned_16)(0x5555u + form);
            C_INT failed = !legacy_sreg_stack_prepare(profiles[profile], &state);

            if (!failed)
            {
                state.machine->executor_cpu.data.esp = 0x00008000u;
                failed |= core_machine_memory_write(state.machine, 0x8000u, &selector, 2u) != TYPE_STATUS_OK ||
                    core_machine_memory_write(state.machine, 0u, &pop_ops[form], 1u) != TYPE_STATUS_OK;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                status = core_machine_run(state.machine, (core_machine_run_budget){1u,0u}, &result);
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
                    status != TYPE_STATUS_OK || diagnostic.first_fault.valid || after.data.eip != 1u ||
                    after.data.esp != 0x00008002u || after.data.eflags != before.data.eflags ||
                    after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
                    after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
                    after.data.ebp != before.data.ebp || after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
                    !legacy_sreg_stack_non_target_sregs_same(&before, &after, form) ||
                    !legacy_sreg_stack_real_cache(legacy_sreg_stack_target(&after, form),
                    selector, form);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT legacy_sreg_stack_test_attributes(C_VOID)
{
    static const type_unsigned_8 prefixes[][2] = {
        {0x66u, 0u}, {0x67u, 0u}, {0x66u, 0x67u}
    };
    static const type_unsigned_8 push_ops[] = {0x06u, 0x0eu, 0x16u, 0x1eu};
    static const type_unsigned_8 push_selectors[] = {0x11u, 0x22u, 0x33u, 0x44u};
    static const type_unsigned_8 pop_ops[] = {0x07u, 0x17u, 0x1fu};
    static const core_machine_cpu_profile legacy[] = {CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286};
    type_unsigned_8 attribute;
    type_unsigned_8 form;
    type_unsigned_8 profile;

    for (attribute = 0u; attribute != sizeof(prefixes) / sizeof(prefixes[0]);
        ++attribute)
    {
        for (form = 0u; form != sizeof(push_ops); ++form)
        {
            legacy_sreg_stack_machine state;
            core_machine_run_result result;
            core_machine_cpu_diagnostic diagnostic;
            t_cpu before;
            t_cpu after;
            type_status status;
            type_unsigned_8 code[] = {prefixes[attribute][0], push_ops[form], 0u};
            type_unsigned_8 bytes = attribute == 2u ? 3u : 2u;
            type_unsigned_16 image = 0u;
            type_unsigned_16 selector = push_selectors[form];
            type_unsigned_8 width = attribute == 0u || attribute == 2u ? 4u : 2u;
            type_unsigned_32 expected_esp = 0x12348000u - width;
            C_INT failed = !legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);

            if (!failed)
            {
                if (attribute == 2u)
                {
                    code[1] = prefixes[attribute][1];
                    code[2] = push_ops[form];
                }
                state.machine->executor_cpu.data.esp = 0x12348000u;
                state.machine->executor_cpu.data.es.selector = 0x0011u;
                state.machine->executor_cpu.data.cs.selector = 0x0022u;
                state.machine->executor_cpu.data.ss.selector = 0x0033u;
                state.machine->executor_cpu.data.ds.selector = 0x0044u;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= core_machine_memory_write(state.machine, 0u, code,
                    bytes) != TYPE_STATUS_OK;
                status = core_machine_run(state.machine,
                    (core_machine_run_budget){1u, 0u}, &result);
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    TYPE_STATUS_OK || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid || after.data.eip != bytes ||
                    after.data.esp != expected_esp ||
                    after.data.eflags != before.data.eflags ||
                    !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                    !legacy_sreg_stack_sregs_same(&before, &after) ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                    expected_esp & 0xffffu, TYPE_REFERENCE_OF(image), 2u) !=
                    TYPE_STATUS_OK || image != selector;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
        for (form = 0u; form != sizeof(pop_ops); ++form)
        {
            legacy_sreg_stack_machine state;
            core_machine_run_result result;
            core_machine_cpu_diagnostic diagnostic;
            t_cpu before;
            t_cpu after;
            type_status status;
            type_unsigned_8 code[] = {prefixes[attribute][0], pop_ops[form], 0u};
            type_unsigned_8 bytes = attribute == 2u ? 3u : 2u;
            type_unsigned_8 width = attribute == 0u || attribute == 2u ? 4u : 2u;
            type_unsigned_16 selector = (type_unsigned_16)(0x1110u + attribute * 3u + form);
            C_INT failed = !legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);

            if (!failed)
            {
                if (attribute == 2u)
                {
                    code[1] = prefixes[attribute][1];
                    code[2] = pop_ops[form];
                }
                state.machine->executor_cpu.data.esp = 0x12348000u;
                failed |= core_machine_memory_write(state.machine, 0x8000u, &selector,
                    2u) != TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                    0u, code, bytes) != TYPE_STATUS_OK;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                status = core_machine_run(state.machine,
                    (core_machine_run_budget){1u, 0u}, &result);
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    TYPE_STATUS_OK || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid || after.data.eip != bytes ||
                    after.data.esp != 0x12348000u + width ||
                    after.data.eflags != before.data.eflags ||
                    !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                    !legacy_sreg_stack_non_target_sregs_same(&before, &after, form) ||
                    !legacy_sreg_stack_real_cache(legacy_sreg_stack_target(&after, form),
                    selector, form);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    for (profile = 0u; profile != sizeof(legacy)/sizeof(legacy[0]); ++profile)
    {
        for (attribute = 0u; attribute != sizeof(prefixes) / sizeof(prefixes[0]);
            ++attribute)
        {
            for (form = 0u; form != sizeof(push_ops) + sizeof(pop_ops); ++form)
            {
                legacy_sreg_stack_machine state;
                core_machine_run_result result;
                core_machine_cpu_diagnostic diagnostic;
                t_cpu before;
                t_cpu after;
                type_status status;
                type_unsigned_8 opcode = form < sizeof(push_ops) ? push_ops[form] :
                    pop_ops[form - sizeof(push_ops)];
                type_unsigned_8 code[] = {prefixes[attribute][0], opcode, 0u};
                type_unsigned_8 bytes = attribute == 2u ? 3u : 2u;
                C_INT failed = !legacy_sreg_stack_prepare(legacy[profile], &state);

                if (!failed)
                {
                    if (attribute == 2u)
                    {
                        code[1] = prefixes[attribute][1];
                        code[2] = opcode;
                    }
                    failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                        state.machine);
                    before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                    failed |= core_machine_memory_write(state.machine, 0u,
                        code, bytes) != TYPE_STATUS_OK;
                    status = core_machine_run(state.machine,
                        (core_machine_run_budget){1u, 0u}, &result);
                    after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                    failed |= core_machine_get_cpu_diagnostic(state.machine,
                        &diagnostic) != TYPE_STATUS_OK || status != TYPE_STATUS_FAULT ||
                        !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                        diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                        STD_MEMCMP(&before, &after, sizeof(before)) != 0;
                }
                core_machine_destroy(state.machine);
                if (failed)
                    return 0;
            }
        }
    }
    return 1;
}

static C_INT legacy_sreg_stack_boot_protected(legacy_sreg_stack_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x37u,0u,0u,0x03u,0u,0u};
    static const type_unsigned_8 gdt[] = {0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x98u,0,0,
        0xffu,0xffu,0,0x50u,0,0x12u,0,0};
    static const type_unsigned_8 boot[] = {0x0fu,1u,0x16u,0,1u,0xb8u,1u,0,
        0x0fu,1u,0xf0u,0xb8u,0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0,0x8eu,0xd0u,0xbcu,0,0x80u,0xeau,0,0,8u,0};
    static const type_unsigned_8 halt = 0xf4u;
    core_machine_run_result result;
    return core_machine_memory_write(state->machine,0x100u,pointer,sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine,0x300u,gdt,sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine,0u,boot,sizeof(boot)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine,0x2000u,&halt,1u) == TYPE_STATUS_OK &&
        core_machine_run(state->machine,(core_machine_run_budget){96u,0u},&result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT legacy_sreg_stack_test_protected_pop(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0x07u,0x17u,0x1fu};
    type_unsigned_8 form;

    for (form = 0u; form != 3u; ++form)
    {
        legacy_sreg_stack_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_16 selector = 0x20u;
        type_unsigned_8 access = 0u;
        C_INT failed = !legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !legacy_sreg_stack_boot_protected(&state);
        if (!failed)
        {
            failed |= core_machine_memory_write(state.machine, 0xc000u, &selector,
                2u) != TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                0x2000u, &opcodes[form], 1u) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            status = core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
                &result);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != 1u || after.data.esp != 0x00008002u ||
                after.data.eflags != before.data.eflags ||
                !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                !legacy_sreg_stack_non_target_sregs_same(&before, &after, form) ||
                !legacy_sreg_stack_protected_cache(
                legacy_sreg_stack_target(&after, form), selector, form) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x325u, TYPE_REFERENCE_OF(access), sizeof(access)) != TYPE_STATUS_OK ||
                access != 0x93u;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_sreg_stack_test_protected_null(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0x07u, 0x1fu};
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(opcodes); ++form)
    {
        legacy_sreg_stack_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_16 selector = 0u;
        C_INT failed = !legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !legacy_sreg_stack_boot_protected(&state);
        if (!failed)
        {
            failed |= core_machine_memory_write(state.machine, 0xc000u, &selector,
                2u) != TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                0x2000u, &opcodes[form], 1u) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            status = core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
                &result);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                after.data.eip != 1u || after.data.esp != 0x8002u ||
                after.data.eflags != before.data.eflags ||
                !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                !legacy_sreg_stack_non_target_sregs_same(&before, &after,
                form == 0u ? 0u : 2u) || legacy_sreg_stack_target(&after,
                form == 0u ? 0u : 2u)->selector != 0u ||
                legacy_sreg_stack_target(&after, form == 0u ? 0u : 2u)->flagValid;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_sreg_stack_test_protected_ss_null(C_VOID)
{
    legacy_sreg_stack_machine state; core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic; t_cpu before,after; type_status status;
    type_unsigned_16 selector=0u; C_INT failed=!legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
    if(!failed) failed|=!legacy_sreg_stack_boot_protected(&state);
    if(!failed) {
        failed|=core_machine_memory_write(state.machine,0xc000u,&selector,2u)!=TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine,0x2000u,(type_unsigned_8[]){0x17u},1u)!=TYPE_STATUS_OK;
        before=test_core_machine_fixture_capture_cpu_after_run(state.machine);
        test_core_machine_fixture_resume_after_halt_at(state.machine,0u);
        status=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result);
        after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed|=core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=TYPE_STATUS_OK ||
            status!=TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_DF) || after.data.eip!=0u ||
            after.data.esp!=before.data.esp || after.data.eflags!=before.data.eflags ||
            !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
            !legacy_sreg_stack_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine); return !failed;
}

static C_INT legacy_sreg_stack_test_protected_rejects(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0x07u,0x17u,0x1fu};
    static const type_unsigned_16 selectors[] = {0x28u,0x23u,0x30u};
    type_unsigned_8 target,kind;
    for(target=0u;target!=3u;++target) for(kind=0u;kind!=3u;++kind) {
        legacy_sreg_stack_machine state; core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic; t_cpu before,after; type_status status;
        type_unsigned_16 selector=selectors[kind], observed=0u;
        C_INT failed=!legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
        if(!failed) failed|=!legacy_sreg_stack_boot_protected(&state);
        if(!failed) {
            failed|=core_machine_memory_write(state.machine,0xc000u,&selector,2u)!=TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine,0x2000u,&opcodes[target],1u)!=TYPE_STATUS_OK;
            before=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine,0u);
            status=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result);
            after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed|=core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=TYPE_STATUS_OK ||
                status!=TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_DF) || after.data.eip!=0u ||
                after.data.esp!=before.data.esp || after.data.eax!=before.data.eax || after.data.ecx!=before.data.ecx ||
                after.data.edx!=before.data.edx || after.data.ebx!=before.data.ebx || after.data.ebp!=before.data.ebp ||
                after.data.esi!=before.data.esi || after.data.edi!=before.data.edi || after.data.eflags!=before.data.eflags ||
                !legacy_sreg_stack_sregs_same(&before, &after) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,0xc000u,TYPE_REFERENCE_OF(observed),2u)!=TYPE_STATUS_OK || observed!=selector;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }return 1;
}

static C_INT legacy_sreg_stack_test_protected_stack_limits(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0x06u,0x07u};
    type_unsigned_8 form;
    for(form=0u;form!=2u;++form) {
        legacy_sreg_stack_machine state; core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic; t_cpu before,after; type_status status;
        type_unsigned_16 image=0xbe5au; type_unsigned_32 candidate=form==0u?0xbffeu:0xc000u;
        C_INT failed=!legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
        if(!failed) failed|=!legacy_sreg_stack_boot_protected(&state);
        if(!failed) {
            state.machine->executor_cpu.data.ss.limit=form==0u?0xffffu:0x7fffu;
            state.machine->executor_cpu.data.ss.seg.data.expdown=form==0u?TYPE_TRUE:TYPE_FALSE;
            failed|=core_machine_memory_write(state.machine,candidate,&image,2u)!=TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine,0x2000u,&opcodes[form],1u)!=TYPE_STATUS_OK;
            before=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine,0u);
            status=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result);
            after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed|=core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=TYPE_STATUS_OK ||
                status!=TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_DF) ||
                after.data.eip!=0u || after.data.esp!=before.data.esp ||
                after.data.eflags!=before.data.eflags || STD_MEMCMP(&before.data.es,&after.data.es,sizeof(before.data.es))!=0 ||
                !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                !legacy_sreg_stack_sregs_same(&before, &after) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,candidate,TYPE_REFERENCE_OF(image),2u)!=TYPE_STATUS_OK || image!=0xbe5au;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

static C_INT legacy_sreg_stack_test_irq(C_VOID)
{
    static const type_unsigned_8 codes[][3]={{0x17u,0x90u},{0x07u,0x90u},{0x1fu,0x90u},{0x06u,0x90u}};
    static const type_unsigned_8 frame[] = {2u,1u,1u,1u}; static const type_unsigned_8 halt=0xf4u;
    type_unsigned_8 form;
    for(form=0u;form!=4u;++form) {
        legacy_sreg_stack_machine state; core_machine_pic_irq_source source;
        core_machine_run_result result; t_cpu before,after;
        type_unsigned_16 off=0x100u,seg=0u,ip=0u,sel=0u,image=0u;
        C_INT failed=!legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
        if(!failed) {
            state.machine->executor_cpu.data.esp=0x8000u;
            if(form!=3u) failed|=core_machine_memory_write(state.machine,0x8000u,&sel,2u)!=TYPE_STATUS_OK;
            failed|=core_machine_memory_write(state.machine,0u,codes[form],2u)!=TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine,0x80u,&off,2u)!=TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine,0x82u,&seg,2u)!=TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine,0x100u,&halt,1u)!=TYPE_STATUS_OK;
        }
        if(!failed) {
            state.machine->executor_cpu.data.eflags|=VCPU_EFLAGS_IF;STD_MEMSET(&source,0,sizeof(source));
            before=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            state.machine->shared_pic_master.data.icw2=0x20u;
            core_machine_pic_irq_source_bind(&source,&state.machine->shared_pic_master,&state.machine->shared_pic_slave,0u);
            core_machine_pic_irq_source_assert(&source);core_machine_pic_irq_source_deassert(&source);
            failed|=core_machine_run(state.machine,(core_machine_run_budget){3u,0u},&result)!=TYPE_STATUS_OK ||
                result.reason!=CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed|=core_machine_memory_read_physical(&state.machine->executor_memory,after.data.ss.base+(type_unsigned_16)after.data.esp,TYPE_REFERENCE_OF(ip),2u)!=TYPE_STATUS_OK ||
                after.data.eip!=0x101u || ip!=frame[form] || !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,VPIC_ISR_IRQ(0u)) ||
                TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,VPIC_IRR_IRQ(0u)) ||
                !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                (form==3u && (after.data.esp!=0x7ff8u ||
                !legacy_sreg_stack_sregs_same(&before, &after) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x7ffeu,TYPE_REFERENCE_OF(image),2u)!=TYPE_STATUS_OK ||
                image!=before.data.es.selector)) ||
                (form!=3u && (after.data.esp!=0x7ffcu ||
                !legacy_sreg_stack_non_target_sregs_same(&before,&after,
                form==0u?1u:form==1u?0u:2u) ||
                !legacy_sreg_stack_real_cache(legacy_sreg_stack_target(&after,
                form==0u?1u:form==1u?0u:2u),0u,form==0u?1u:form==1u?0u:2u)));
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }return 1;
}

static C_INT legacy_sreg_stack_test_fs_gs(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0xa0u,0xa1u,0xa8u,0xa9u};
    type_unsigned_8 form;
    for (form = 0u; form != sizeof(opcodes); ++form) {
        legacy_sreg_stack_machine state; core_machine_run_result result; core_machine_cpu_diagnostic diagnostic; t_cpu before, after; type_unsigned_16 image = (type_unsigned_16)(0x5500u + form); C_INT failed = !legacy_sreg_stack_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) { state.machine->executor_cpu.data.esp=0x8000u; state.machine->executor_cpu.data.fs.selector=0x1111u; state.machine->executor_cpu.data.gs.selector=0x2222u; if (form & 1u) failed |= core_machine_memory_write(state.machine,0x8000u,&image,2u)!=TYPE_STATUS_OK; failed |= core_machine_memory_write(state.machine,0u,(type_unsigned_8[]){0x0fu,opcodes[form]},2u)!=TYPE_STATUS_OK; before=test_core_machine_fixture_capture_cpu_after_run(state.machine); failed |= core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result)!=TYPE_STATUS_OK || result.reason!=CORE_MACHINE_STOP_BUDGET || core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=TYPE_STATUS_OK; after=test_core_machine_fixture_capture_cpu_after_run(state.machine); failed |= diagnostic.first_fault.valid || after.data.eip!=2u || after.data.eflags!=before.data.eflags || !legacy_sreg_stack_gprs_same_except_esp(&before,&after); if (!(form&1u)) { type_unsigned_16 observed=0u; failed |= after.data.esp!=0x7ffeu || core_machine_memory_read_physical(&state.machine->executor_memory,0x7ffeu,TYPE_REFERENCE_OF(observed),2u)!=TYPE_STATUS_OK || observed!=(form==0u?0x1111u:0x2222u); } else failed |= after.data.esp!=0x8002u || (form==1u ? after.data.fs.selector : after.data.gs.selector)!=image; } core_machine_destroy(state.machine); if (failed) return 0;
    } return 1;
}

C_INT main(C_VOID)
{
    if (!legacy_sreg_stack_test_defaults())
    {
        STD_PRINTF("LEGACY-SREG-STACK stage=defaults\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_attributes())
    {
        STD_PRINTF("LEGACY-SREG-STACK stage=attributes\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_pop())
    {
        STD_PRINTF("LEGACY-SREG-STACK stage=protected-pop\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_null())
    {
        STD_PRINTF("LEGACY-SREG-STACK stage=protected-null\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_ss_null())
    {
        STD_PRINTF("LEGACY-SREG-STACK stage=protected-ss-null\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_rejects())
    {
        STD_PRINTF("LEGACY-SREG-STACK stage=protected-rejects\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_stack_limits())
    {
        STD_PRINTF("LEGACY-SREG-STACK stage=protected-stack-limits\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_irq())
    {
        STD_PRINTF("LEGACY-SREG-STACK stage=irq\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_fs_gs())
        return 1;
    if (!legacy_sreg_stack_test_lock())
    {
        STD_PRINTF("LEGACY-SREG-STACK stage=lock\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S46:LEGACY-SREG-STACK:OK\n");
    STD_PRINTF("M5:T401:S41:SREG-PUSH-POP-PROFILES:OK\n");
    return 0;
}
