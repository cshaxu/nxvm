#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

/* Reuse the retained protected/VM86/fault fixture as executable S47 evidence. */
#define main pushf_popf_s21_main
#include "core_machine_pushf_popf_smoke.c"
#undef main

typedef struct pushf_popf_s47_machine
{
    core_machine *machine;
} pushf_popf_s47_machine;

static C_VOID pushf_popf_s47_reset(C_VOID *opaque)
{
    pushf_popf_s47_machine *state = (pushf_popf_s47_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider pushf_popf_s47_provider = {
    pushf_popf_s47_reset, STD_NULL
};

static type_unsigned_16 pushf_popf_s47_real_flags_image(
    core_machine_cpu_profile profile, type_unsigned_16 flags)
{
    if (profile < CORE_MACHINE_CPU_PROFILE_80286) flags &= 0x0fffu;
    return (type_unsigned_16)((flags & ~VCPU_EFLAGS_RESERVED) | 0x02u);
}

static type_unsigned_16 pushf_popf_s47_real_flags_load(
    core_machine_cpu_profile profile, type_unsigned_16 flags)
{
    if (profile < CORE_MACHINE_CPU_PROFILE_80286) flags &= 0x0fffu;
    return (type_unsigned_16)((flags & ~VCPU_EFLAGS_RESERVED) | 0x02u);
}

static type_unsigned_16 pushf_popf_s47_real_flags_known_mask(
    core_machine_cpu_profile profile)
{
    if (profile < CORE_MACHINE_CPU_PROFILE_80286) return 0x0fd5u;
    if (profile == CORE_MACHINE_CPU_PROFILE_80286) return 0x7fd5u;
    return 0xffffu;
}

static C_INT pushf_popf_s47_prepare(core_machine_cpu_profile profile,
    pushf_popf_s47_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &pushf_popf_s47_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_INT pushf_popf_s47_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT pushf_popf_s47_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT pushf_popf_s47_step(pushf_popf_s47_machine *state,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_status *status,
    core_machine_cpu_diagnostic *diagnostic, t_cpu *after)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine, (core_machine_run_budget){1u, 0u},
        &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK;
}

static C_INT pushf_popf_s47_test_defaults(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 opcodes[] = {0x9cu, 0x9du};
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
        VCPU_EFLAGS_OF | 0x02u;
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    {
        type_unsigned_8 form;

        for (form = 0u; form != sizeof(opcodes); ++form)
        {
            pushf_popf_s47_machine state;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            t_cpu before;
            t_cpu after;
            type_unsigned_32 image = form == 0u ? 0u : VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_NT | 0x8000u;
            type_unsigned_32 observed = 0u;
            C_INT failed = !pushf_popf_s47_prepare(profiles[profile], &state);

            if (!failed)
            {
                state.machine->executor_cpu.data.esp = 0x12348000u;
                state.machine->executor_cpu.data.eflags = flags;
                if (form != 0u)
                    failed |= core_machine_memory_write(state.machine, 0x8000u, &image,
                        2u) != TYPE_STATUS_OK;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !pushf_popf_s47_step(&state, &opcodes[form], 1u, &status,
                    &diagnostic, &after) || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid || after.data.eip != 1u ||
                    after.data.esp != (form == 0u ? 0x12347ffeu : 0x12348002u) ||
                    !pushf_popf_s47_gprs_same(&before, &after) ||
                    !pushf_popf_s47_sregs_same(&before, &after);
                if (form == 0u)
                    failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                        0x7ffeu, TYPE_REFERENCE_OF(observed), 2u) != TYPE_STATUS_OK ||
                        (observed & pushf_popf_s47_real_flags_known_mask(profiles[profile])) !=
                        (pushf_popf_s47_real_flags_image(profiles[profile],
                            (type_unsigned_16)flags) &
                            pushf_popf_s47_real_flags_known_mask(profiles[profile]));
                else
                    failed |= (after.data.eflags &
                        pushf_popf_s47_real_flags_known_mask(profiles[profile])) !=
                        (pushf_popf_s47_real_flags_load(profiles[profile],
                            (type_unsigned_16)image) &
                            pushf_popf_s47_real_flags_known_mask(profiles[profile])) ||
                        (after.data.eflags & 0xffff0000u) !=
                        (flags & 0xffff0000u);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT pushf_popf_s47_test_attributes_and_rejects(C_VOID)
{
    static const type_unsigned_8 prefixes[][2] = {{0x66u, 0u}, {0x67u, 0u},
        {0x66u, 0x67u}};
    static const core_machine_cpu_profile legacy[] = {CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286};
    type_unsigned_8 attribute;

    for (attribute = 0u; attribute != sizeof(prefixes) / sizeof(prefixes[0]);
        ++attribute)
    {
        type_unsigned_8 opcode;

        for (opcode = 0x9cu; opcode <= 0x9du; ++opcode)
        {
            pushf_popf_s47_machine state;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            t_cpu before;
            t_cpu after;
            type_unsigned_8 code[] = {prefixes[attribute][0], opcode, 0u};
            type_unsigned_8 bytes = attribute == 2u ? 3u : 2u;
            type_unsigned_8 width = attribute == 0u || attribute == 2u ? 4u : 2u;
            type_unsigned_32 image = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF;
            type_unsigned_32 observed = 0u;
            C_INT failed = !pushf_popf_s47_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);

            if (attribute == 2u)
            {
                code[1] = prefixes[attribute][1];
                code[2] = opcode;
            }
            if (!failed)
            {
                state.machine->executor_cpu.data.esp = 0x12348000u;
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                    VCPU_EFLAGS_IF | VCPU_EFLAGS_RF | VCPU_EFLAGS_VM | 0x02u;
                if (opcode == 0x9du)
                    failed |= core_machine_memory_write(state.machine, 0x8000u, &image,
                        width) != TYPE_STATUS_OK;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !pushf_popf_s47_step(&state, code, bytes, &status, &diagnostic,
                    &after) || status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                    after.data.eip != bytes || after.data.esp !=
                    (opcode == 0x9cu ? 0x12348000u - width : 0x12348000u + width) ||
                    !pushf_popf_s47_gprs_same(&before, &after) ||
                    !pushf_popf_s47_sregs_same(&before, &after);
                if (opcode == 0x9cu)
                    failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                        (0x12348000u - width) & 0xffffu, TYPE_REFERENCE_OF(observed),
                        width) != TYPE_STATUS_OK || (width == 4u ? observed !=
                        ((before.data.eflags & ~(VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_VM |
                        VCPU_EFLAGS_RF)) | 0x02u) : (observed & 0xffffu) !=
                        (((before.data.eflags & ~VCPU_EFLAGS_RESERVED) | 0x02u) &
                        0xffffu)) || after.data.eflags !=
                        (before.data.eflags & ~VCPU_EFLAGS_RF);
                else
                    failed |= after.data.eflags != (width == 4u ?
                        ((image & ~(VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_RF |
                        VCPU_EFLAGS_VM)) | (before.data.eflags &
                        (VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_VM)) | 0x02u) :
                        ((image & ~(VCPU_EFLAGS_RESERVED | 0xffff0000u)) |
                        (before.data.eflags & (VCPU_EFLAGS_RESERVED |
                        (0xffff0000u & ~VCPU_EFLAGS_RF))) | 0x02u));
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    for (attribute = 0u; attribute != sizeof(legacy) / sizeof(legacy[0]); ++attribute)
    {
        type_unsigned_8 prefix;
        for (prefix = 0u; prefix != sizeof(prefixes) / sizeof(prefixes[0]); ++prefix)
        {
            type_unsigned_8 opcode;
            for (opcode = 0x9cu; opcode <= 0x9du; ++opcode)
            {
                pushf_popf_s47_machine state;
                core_machine_cpu_diagnostic diagnostic;
                type_status status;
                t_cpu before;
                t_cpu after;
                type_unsigned_8 code[] = {prefixes[prefix][0], opcode, 0u};
                type_unsigned_8 bytes = prefix == 2u ? 3u : 2u;
                C_INT failed = !pushf_popf_s47_prepare(legacy[attribute], &state);

                if (prefix == 2u)
                {
                    code[1] = prefixes[prefix][1];
                    code[2] = opcode;
                }
                if (!failed)
                {
                    failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                        state.machine);
                    before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                    failed |= !pushf_popf_s47_step(&state, code, bytes, &status,
                        &diagnostic, &after) || status != TYPE_STATUS_FAULT ||
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

static C_INT pushf_popf_s47_test_lock(C_VOID)
{
    static const type_unsigned_8 prefixes[][2] = {{0u, 0u}, {0x66u, 0u}, {0x67u, 0u},
        {0x66u, 0x67u}};
    type_unsigned_8 attribute;

    for (attribute = 0u; attribute != sizeof(prefixes) / sizeof(prefixes[0]);
        ++attribute)
    {
        type_unsigned_8 opcode;
        for (opcode = 0x9cu; opcode <= 0x9du; ++opcode)
        {
            pushf_popf_s47_machine state;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            t_cpu before;
            t_cpu after;
            type_unsigned_8 code[] = {0xf0u, opcode, 0u, 0u};
            type_unsigned_8 bytes = attribute == 0u ? 2u : attribute == 3u ? 4u : 3u;
            type_unsigned_32 sentinel = 0xa55aa55au;
            type_unsigned_32 observed = 0u;
            C_INT failed = !pushf_popf_s47_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);
            if (attribute != 0u)
            {
                code[1] = prefixes[attribute][0];
                code[2] = opcode;
            }
            if (attribute == 3u)
            {
                code[2] = prefixes[attribute][1];
                code[3] = opcode;
            }
            if (!failed)
            {
                state.machine->executor_cpu.data.esp = 0x8000u;
                failed |= core_machine_memory_write(state.machine, 0x7ffcu, &sentinel,
                    sizeof(sentinel)) != TYPE_STATUS_OK || core_machine_memory_write(
                    state.machine, 0x8000u, &sentinel, sizeof(sentinel)) != TYPE_STATUS_OK;
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !pushf_popf_s47_step(&state, code, bytes, &status, &diagnostic,
                    &after) || status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                    !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    STD_MEMCMP(&before, &after, sizeof(before)) != 0 ||
                    core_machine_memory_read_physical(&state.machine->executor_memory, 0x7ffcu,
                    TYPE_REFERENCE_OF(observed), sizeof(observed)) != TYPE_STATUS_OK ||
                    observed != sentinel || core_machine_memory_read_physical(
                    &state.machine->executor_memory, 0x8000u, TYPE_REFERENCE_OF(observed),
                    sizeof(observed)) != TYPE_STATUS_OK || observed != sentinel;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT pushf_popf_s47_test_irq(C_VOID)
{
    static const type_unsigned_8 codes[][2] = {{0x9cu, 0x90u}, {0x9du, 0x90u}};
    static const type_unsigned_8 halt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        pushf_popf_s47_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_unsigned_16 offset = 0x100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame = 0u;
        type_unsigned_16 image = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        type_unsigned_16 observed = 0u;
        C_INT failed = !pushf_popf_s47_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            state.machine->executor_cpu.data.esp = 0x8000u;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            if (form != 0u)
                failed |= core_machine_memory_write(state.machine, 0x8000u, &image,
                    2u) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0u, codes[form], 2u) !=
                TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x80u,
                &offset, 2u) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x82u, &segment, 2u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &halt, 1u) !=
                TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){3u, 0u},
                &result) != TYPE_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp, TYPE_REFERENCE_OF(frame),
                2u) != TYPE_STATUS_OK || after.data.eip != 0x101u || frame != 1u ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u)) || !pushf_popf_s47_gprs_same(&before, &after) ||
                !pushf_popf_s47_sregs_same(&before, &after) ||
                (form == 0u && (after.data.esp != 0x7ff8u ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x7ffeu, TYPE_REFERENCE_OF(observed), 2u) != TYPE_STATUS_OK ||
                (observed & 0xffffu) != ((before.data.eflags &
                ~VCPU_EFLAGS_RESERVED) | 0x02u))) ||
                (form != 0u && (after.data.esp != 0x7ffcu ||
                (after.data.eflags & 0xffffu) != ((image &
                ~(VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_IF)) | 0x02u)));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!pushf_popf_s47_test_defaults())
    {
        STD_PRINTF("PUSHF-POPF-S47 stage=defaults\n");
        return 1;
    }
    if (!pushf_popf_s47_test_attributes_and_rejects())
    {
        STD_PRINTF("PUSHF-POPF-S47 stage=attributes\n");
        return 1;
    }
    if (!pushf_popf_s47_test_lock())
    {
        STD_PRINTF("PUSHF-POPF-S47 stage=lock\n");
        return 1;
    }
    if (pushf_popf_s21_main() != 0)
    {
        STD_PRINTF("PUSHF-POPF-S47 stage=protected-vm86-stack\n");
        return 1;
    }
    if (!pushf_popf_s47_test_irq())
    {
        STD_PRINTF("PUSHF-POPF-S47 stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S47:PUSHF-POPF:OK\n");
    STD_PRINTF("M5:T401:S39:PUSHF-POPF-PROFILES:OK\n");
    return 0;
}
