#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct direct_flags_machine {
    core_machine *machine;
} direct_flags_machine;

typedef struct direct_flags_case {
    type_unsigned_8 opcode;
    type_unsigned_32 initial_bits;
} direct_flags_case;

static C_VOID direct_flags_reset(C_VOID *opaque)
{
    direct_flags_machine *state = (direct_flags_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider direct_flags_provider = {
    direct_flags_reset, STD_NULL
};

static C_INT direct_flags_prepare(core_machine_cpu_profile profile,
    direct_flags_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &direct_flags_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID direct_flags_seed(direct_flags_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xaabbccddU;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccU;
    cpu->data.esp = 0x8000u;
    cpu->data.ebp = 0x120u;
    cpu->data.esi = 0x10u;
    cpu->data.edi = 0x20u;
    cpu->data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_OF;
}

static C_INT direct_flags_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.esp == before->data.esp &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi;
}

static C_INT direct_flags_irq_gprs_same(const t_cpu *before,
    const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi;
}

static type_unsigned_32 direct_flags_expected(type_unsigned_8 opcode, type_unsigned_32 flags)
{
    switch (opcode) {
    case 0xf5u:
        return flags ^ VCPU_EFLAGS_CF;
    case 0xf8u:
        return flags & ~VCPU_EFLAGS_CF;
    case 0xf9u:
        return flags | VCPU_EFLAGS_CF;
    case 0xfcu:
        return flags & ~VCPU_EFLAGS_DF;
    case 0xfdu:
        return flags | VCPU_EFLAGS_DF;
    default:
        return flags;
    }
}

static C_INT direct_flags_run(direct_flags_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    type_status *status, core_machine_run_result *result)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){1u,0u}, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT direct_flags_test_default(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const direct_flags_case cases[] = {
        {0xf5u,0u}, {0xf5u,VCPU_EFLAGS_CF}, {0xf8u,VCPU_EFLAGS_CF},
        {0xf9u,0u}, {0xfcu,VCPU_EFLAGS_DF}, {0xfdu,0u}
    };
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (form = 0u; form != sizeof(cases) / sizeof(cases[0]); ++form) {
            direct_flags_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            type_status status;
            const type_unsigned_32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_PF |
                VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
                VCPU_EFLAGS_OF | cases[form].initial_bits;
            C_INT failed = !direct_flags_prepare(profiles[profile], &state);

            if (!failed) {
                direct_flags_seed(&state);
                state.machine->executor_cpu.data.eflags = flags;
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !direct_flags_run(&state, &cases[form].opcode, 1u,
                    &after, &diagnostic, &status, &result) ||
                    status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                    after.data.eip != 1u ||
                    !direct_flags_gprs_same(&before, &after) ||
                    after.data.eflags != direct_flags_expected(
                    cases[form].opcode, before.data.eflags);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT direct_flags_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    direct_flags_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    C_INT failed = !direct_flags_prepare(profile, &state);

    if (!failed) {
        direct_flags_seed(&state);
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !direct_flags_run(&state, code, bytes, &after, &diagnostic,
            &status, &result) || status != TYPE_STATUS_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || STD_MEMCMP(&before.data, &after.data,
            sizeof(before.data)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT direct_flags_test_attributes(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;
    type_unsigned_8 prefix;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
            for (prefix = 0u; prefix != 3u; ++prefix) {
                type_unsigned_8 code[] = {0x66u,opcodes[opcode],0u};
                type_unsigned_8 bytes = prefix == 2u ? 3u : 2u;

                if (prefix == 1u)
                    code[0] = 0x67u;
                if (prefix == 2u) {
                    code[1] = 0x67u;
                    code[2] = opcodes[opcode];
                }
                if (!direct_flags_expect_ud(legacy[profile], code, bytes))
                    return 0;
            }
        }
    }
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        type_unsigned_8 code[] = {0xf0u,opcodes[opcode]};

        if (!direct_flags_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, code,
            sizeof(code)))
            return 0;
    }
    return 1;
}

static C_INT direct_flags_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};
    type_unsigned_8 opcode;
    type_unsigned_8 prefix;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (prefix = 0u; prefix != 3u; ++prefix) {
            direct_flags_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            type_status status;
            type_unsigned_8 code[] = {0x66u,opcodes[opcode],0u};
            type_unsigned_8 bytes = prefix == 2u ? 3u : 2u;
            C_INT failed = !direct_flags_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);

            if (prefix == 1u)
                code[0] = 0x67u;
            if (prefix == 2u) {
                code[1] = 0x67u;
                code[2] = opcodes[opcode];
            }
            if (!failed) {
                direct_flags_seed(&state);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !direct_flags_run(&state, code, bytes, &after,
                    &diagnostic, &status, &result) ||
                    status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                    after.data.eip != bytes ||
                    !direct_flags_gprs_same(&before, &after) ||
                    after.data.eflags != direct_flags_expected(opcodes[opcode],
                    before.data.eflags);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT direct_flags_boot_protected(direct_flags_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x1fu,0u,0u,0x03u,0u,0u};
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0, 0xffu,0xffu,0,0,0,0x92u,0,0
    };
    static const type_unsigned_8 boot[] = {
        0x0fu,0x01u,0x16u,0,1u, 0xb8u,1u,0,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u, 0xb8u,0x18u,0,0x8eu,
        0xd0u,0xbcu,0,0x80u, 0xeau,0,0,8u,0
    };
    static const type_unsigned_8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, boot, sizeof(boot)) ==
        TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0x2000u,
        &halt, sizeof(halt)) == TYPE_STATUS_OK && core_machine_run(
        state->machine, (core_machine_run_budget){96u,0u}, &result) ==
        TYPE_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT direct_flags_test_protected(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        direct_flags_machine state;
        t_cpu before;
        t_cpu after = { 0 };
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        const type_unsigned_32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF |
            VCPU_EFLAGS_DF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
            VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF |
            VCPU_EFLAGS_IOPL;
        C_INT failed = !direct_flags_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !direct_flags_boot_protected(&state);
        if (!failed) {
            const t_cpu_data_sreg cs = state.machine->executor_cpu.data.cs;
            const t_cpu_data_sreg ds = state.machine->executor_cpu.data.ds;
            const t_cpu_data_sreg es = state.machine->executor_cpu.data.es;
            const t_cpu_data_sreg ss = state.machine->executor_cpu.data.ss;

            direct_flags_seed(&state);
            state.machine->executor_cpu.data.cs = cs;
            state.machine->executor_cpu.data.ds = ds;
            state.machine->executor_cpu.data.es = es;
            state.machine->executor_cpu.data.ss = ss;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x2000u,
                &opcodes[opcode], 1u) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= diagnostic.first_fault.valid || after.data.eip != 1u ||
                !direct_flags_gprs_same(&before, &after) ||
                after.data.eflags != direct_flags_expected(opcodes[opcode],
                before.data.eflags) || STD_MEMCMP(&before.data.cs,
                &after.data.cs, sizeof(before.data.cs)) != 0 ||
                STD_MEMCMP(&before.data.ds, &after.data.ds,
                sizeof(before.data.ds)) != 0 || STD_MEMCMP(&before.data.es,
                &after.data.es, sizeof(before.data.es)) != 0 ||
                STD_MEMCMP(&before.data.ss, &after.data.ss,
                sizeof(before.data.ss)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT direct_flags_test_vm86(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        direct_flags_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        const type_unsigned_32 flags = VCPU_EFLAGS_VM | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_CF | VCPU_EFLAGS_DF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
            VCPU_EFLAGS_OF | VCPU_EFLAGS_IOPL;
        C_INT failed = !direct_flags_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            direct_flags_seed(&state);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
            state.machine->executor_cpu.data.eflags = flags;
            state.machine->executor_cpu.data.cs.selector = 0u;
            state.machine->executor_cpu.data.cs.base = 0u;
            state.machine->executor_cpu.data.cs.limit = 0xffffu;
            state.machine->executor_cpu.data.cs.dpl = 3u;
            state.machine->executor_cpu.data.cs.flagValid = TYPE_TRUE;
            state.machine->executor_cpu.data.cs.seg.exec.defsize = TYPE_FALSE;
            state.machine->executor_cpu.data.ss.selector = 0u;
            state.machine->executor_cpu.data.ss.base = 0u;
            state.machine->executor_cpu.data.ss.limit = 0xffffu;
            state.machine->executor_cpu.data.ss.dpl = 3u;
            state.machine->executor_cpu.data.ss.flagValid = TYPE_TRUE;
            state.machine->executor_cpu.data.ss.seg.data.big = TYPE_FALSE;
            failed |= core_machine_memory_write(state.machine, 0u,
                &opcodes[opcode], 1u) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= diagnostic.first_fault.valid || after.data.eip != 1u ||
                !direct_flags_gprs_same(&before, &after) ||
                after.data.eflags != direct_flags_expected(opcodes[opcode],
                before.data.eflags);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT direct_flags_test_irq(C_VOID)
{
    static const type_unsigned_8 opcodes[] = {0xf5u,0xf8u,0xf9u,0xfcu,0xfdu};
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        direct_flags_machine state;
        core_machine_pic_irq_source irq;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_unsigned_16 offset = 0x100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_8 code[] = {opcodes[opcode],0x90u};
        const type_unsigned_32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF |
            VCPU_EFLAGS_DF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
            VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
        C_INT failed = !direct_flags_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x80u, &offset, sizeof(offset)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment,
                sizeof(segment)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x100u, &hlt, sizeof(hlt)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            direct_flags_seed(&state);
            state.machine->executor_cpu.data.eflags = flags;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            STD_MEMSET(&irq, 0, sizeof(irq));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&irq);
            core_machine_pic_irq_source_deassert(&irq);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u,0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != 1u ||
                !direct_flags_irq_gprs_same(&before, &after) ||
                after.data.eflags != (direct_flags_expected(opcodes[opcode],
                before.data.eflags) & ~VCPU_EFLAGS_IF) || !TYPE_GET_BIT(
                state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
                TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT direct_flags_test_real_identity(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0xb8u,0x00u,0xf0u,0x50u,0x9du,0x9cu,0x58u,0xf4u
    };
    static const struct {
        core_machine_cpu_profile profile;
        type_unsigned_16 known_mask;
        type_unsigned_16 expected_image;
    } cases[] = {
        { CORE_MACHINE_CPU_PROFILE_8086, 0x0fd7u, 0x0002u },
        { CORE_MACHINE_CPU_PROFILE_8088, 0x0fd7u, 0x0002u },
        { CORE_MACHINE_CPU_PROFILE_80186, 0x0fd7u, 0x0002u },
        /* I286 defines IOPL and NT, but labels bit 15 undefined. */
        { CORE_MACHINE_CPU_PROFILE_80286, 0x7fd7u, 0x7002u },
        { CORE_MACHINE_CPU_PROFILE_80386, 0xffd7u, 0x7002u }
    };
    type_unsigned_8 index;

    for (index = 0u; index != sizeof(cases) / sizeof(cases[0]); ++index) {
        direct_flags_machine state;
        core_machine_run_result result;
        t_cpu after;
        type_status status = TYPE_STATUS_OK;
        C_INT eax_matches = 0;
        C_INT flags_match = 0;
        C_INT failed = !direct_flags_prepare(cases[index].profile, &state);

        if (!failed) {
            direct_flags_seed(&state);
            failed |= core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != TYPE_STATUS_OK ||
                ((status = core_machine_run(state.machine,
                (core_machine_run_budget){16u,0u}, &result)) != TYPE_STATUS_OK);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            eax_matches = (after.data.eax & cases[index].known_mask) ==
                cases[index].expected_image;
            flags_match = (after.data.eflags & cases[index].known_mask) ==
                cases[index].expected_image;
            failed |= !eax_matches || !flags_match;
        }
        core_machine_destroy(state.machine);
        if (failed) {
            STD_PRINTF("DIRECT-FLAGS real identity profile=%u eax=%08x flags=%08x eip=%08x esp=%08x status=%u reason=%u eax-match=%u flags-match=%u expected=%04x\n",
                (unsigned int)cases[index].profile, (unsigned int)after.data.eax,
                (unsigned int)after.data.eflags, (unsigned int)after.data.eip,
                (unsigned int)after.data.esp, (unsigned int)status,
                (unsigned int)result.reason, (unsigned int)eax_matches,
                (unsigned int)flags_match, (unsigned int)cases[index].expected_image);
            return 0;
        }
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!direct_flags_test_default()) {
        STD_PRINTF("DIRECT-FLAGS stage=default\n");
        return 1;
    }
    if (!direct_flags_test_attributes()) {
        STD_PRINTF("DIRECT-FLAGS stage=attributes\n");
        return 1;
    }
    if (!direct_flags_test_386_attributes()) {
        STD_PRINTF("DIRECT-FLAGS stage=386-attributes\n");
        return 1;
    }
    if (!direct_flags_test_protected()) {
        STD_PRINTF("DIRECT-FLAGS stage=protected\n");
        return 1;
    }
    if (!direct_flags_test_vm86()) {
        STD_PRINTF("DIRECT-FLAGS stage=vm86\n");
        return 1;
    }
    if (!direct_flags_test_irq()) {
        STD_PRINTF("DIRECT-FLAGS stage=irq\n");
        return 1;
    }
    if (!direct_flags_test_real_identity()) {
        STD_PRINTF("DIRECT-FLAGS stage=real-identity\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S40:DIRECT-FLAGS:OK\n");
    STD_PRINTF("M5:T401:S42:DIRECT-FLAGS-PROFILES:OK\n");
    return 0;
}
