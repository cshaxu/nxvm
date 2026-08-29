#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct sign_extend_machine {
    core_machine *machine;
} sign_extend_machine;

static C_VOID sign_extend_reset(C_VOID *opaque)
{
    sign_extend_machine *state = (sign_extend_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider sign_extend_provider = {
    sign_extend_reset, STD_NULL
};

static C_INT sign_extend_prepare(core_machine_cpu_profile profile,
    sign_extend_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &sign_extend_provider, state, &state->machine);
}

static C_INT sign_extend_run(sign_extend_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    type_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
            TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_VOID sign_extend_set_registers(sign_extend_machine *state)
{
    state->machine->executor_cpu.data.eax = 0xaabb0000u;
    state->machine->executor_cpu.data.ecx = 0x11223344u;
    state->machine->executor_cpu.data.edx = 0x55660000u;
    state->machine->executor_cpu.data.ebx = 0x778899aau;
    state->machine->executor_cpu.data.esp = 0xbbbb8000u;
    state->machine->executor_cpu.data.ebp = 0xccccddddu;
    state->machine->executor_cpu.data.esi = 0xeeeeffffu;
    state->machine->executor_cpu.data.edi = 0x10203040u;
    state->machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
}

static C_INT sign_extend_state_equal(const t_cpu *before, const t_cpu *after)
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
        before->data.eip == after->data.eip;
}

static C_INT sign_extend_nonparticipants_equal(const t_cpu *before,
    const t_cpu *after, type_unsigned_8 opcode)
{
    return before->data.ecx == after->data.ecx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags &&
        (opcode == 0x98u || before->data.eax == after->data.eax) &&
        (opcode == 0x99u || before->data.edx == after->data.edx);
}

static C_INT sign_extend_test_default(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 opcodes[] = { 0x98u, 0x99u };
    type_unsigned_8 profile;
    type_unsigned_8 opcode;
    type_unsigned_8 sign;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile)
    {
        for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
        {
            for (sign = 0u; sign != 2u; ++sign)
            {
                sign_extend_machine state;
                t_cpu before;
                t_cpu after;
                core_machine_cpu_diagnostic diagnostic;
                type_status status;
                type_unsigned_8 code[] = { opcodes[opcode] };
                type_unsigned_32 expected_eax;
                type_unsigned_32 expected_edx;
                C_INT failed;

                STD_MEMSET(&state, 0, sizeof(state));
                STD_MEMSET(&before, 0, sizeof(before));
                STD_MEMSET(&after, 0, sizeof(after));
                STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
                status = TYPE_STATUS_INVALID_ARGUMENT;
                failed = !sign_extend_prepare(profiles[profile], &state);
                if (!failed)
                {
                    failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                        state.machine, 0u);
                    sign_extend_set_registers(&state);
                    state.machine->executor_cpu.data.eax =
                        opcodes[opcode] == 0x98u ?
                        (sign == 0u ? 0xaabb007fu : 0xaabb0080u) :
                        (sign == 0u ? 0xaabb007fu : 0xaabb8000u);
                    before = test_core_machine_fixture_capture_cpu_after_run(
                        state.machine);
                    expected_eax = opcodes[opcode] == 0x98u ?
                        (sign == 0u ? 0xaabb007fu : 0xaabbff80u) :
                        before.data.eax;
                    expected_edx = opcodes[opcode] == 0x99u ?
                        (sign == 0u ? 0x55660000u : 0x5566ffffu) :
                        before.data.edx;
                    failed |= !sign_extend_run(&state, code, sizeof(code),
                        &after, &diagnostic, &status) ||
                        status != TYPE_STATUS_OK ||
                        diagnostic.first_fault.valid ||
                        after.data.eip != 1u ||
                        after.data.eax != expected_eax ||
                        after.data.edx != expected_edx ||
                        !sign_extend_nonparticipants_equal(&before, &after,
                            opcodes[opcode]);
                }
                core_machine_destroy(state.machine);
                if (failed)
                    return 0;
            }
        }
    }
    return 1;
}

static C_INT sign_extend_test_operand32(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0x98u, 0x99u };
    type_unsigned_8 opcode;
    type_unsigned_8 sign;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
    {
        for (sign = 0u; sign != 2u; ++sign)
        {
            sign_extend_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_8 code[] = { 0x66u, opcodes[opcode] };
            type_unsigned_32 expected_eax;
            type_unsigned_32 expected_edx;
            C_INT failed;

            STD_MEMSET(&state, 0, sizeof(state));
            STD_MEMSET(&before, 0, sizeof(before));
            STD_MEMSET(&after, 0, sizeof(after));
            STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
            status = TYPE_STATUS_INVALID_ARGUMENT;
            failed = !sign_extend_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);
            if (!failed)
            {
                failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u);
                sign_extend_set_registers(&state);
                state.machine->executor_cpu.data.eax =
                    opcodes[opcode] == 0x98u ?
                    (sign == 0u ? 0xabcd7f00u : 0xabcd8000u) :
                    (sign == 0u ? 0x12347f00u : 0xabcd8000u);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                expected_eax = opcodes[opcode] == 0x98u ?
                    (sign == 0u ? 0x00007f00u : 0xffff8000u) :
                    before.data.eax;
                expected_edx = opcodes[opcode] == 0x99u ?
                    (sign == 0u ? 0x00000000u : 0xffffffffu) :
                    before.data.edx;
                failed |= !sign_extend_run(&state, code, sizeof(code), &after,
                    &diagnostic, &status) ||
                    status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid ||
                    after.data.eip != 2u ||
                    after.data.eax != expected_eax ||
                    after.data.edx != expected_edx ||
                    !sign_extend_nonparticipants_equal(&before, &after,
                        opcodes[opcode]);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT sign_extend_test_address_prefix(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0x98u, 0x99u };
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
    {
        sign_extend_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 code[] = { 0x67u, opcodes[opcode] };
        C_INT failed;

        STD_MEMSET(&state, 0, sizeof(state));
        STD_MEMSET(&before, 0, sizeof(before));
        STD_MEMSET(&after, 0, sizeof(after));
        STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
        status = TYPE_STATUS_INVALID_ARGUMENT;
        failed = !sign_extend_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed)
        {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            sign_extend_set_registers(&state);
            state.machine->executor_cpu.data.eax = opcodes[opcode] == 0x98u ?
                0xaabb0080u : 0xaabb8000u;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !sign_extend_run(&state, code, sizeof(code), &after,
                &diagnostic, &status) ||
                status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid ||
                after.data.eip != 2u ||
                after.data.eax != (opcodes[opcode] == 0x98u ?
                    0xaabbff80u : before.data.eax) ||
                after.data.edx != (opcodes[opcode] == 0x99u ?
                    0x5566ffffu : before.data.edx) ||
                !sign_extend_nonparticipants_equal(&before, &after,
                    opcodes[opcode]);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT sign_extend_test_prefix_reject(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 prefixes[] = { 0x66u, 0x67u };
    static const type_unsigned_8 opcodes[] = { 0x98u, 0x99u };
    type_unsigned_8 profile;
    type_unsigned_8 prefix;
    type_unsigned_8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile)
    {
        for (prefix = 0u; prefix != sizeof(prefixes); ++prefix)
        {
            for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
            {
                sign_extend_machine state;
                t_cpu before;
                t_cpu after;
                core_machine_cpu_diagnostic diagnostic;
                type_status status;
                type_unsigned_8 code[] = { prefixes[prefix], opcodes[opcode] };
                C_INT failed;

                STD_MEMSET(&state, 0, sizeof(state));
                STD_MEMSET(&before, 0, sizeof(before));
                STD_MEMSET(&after, 0, sizeof(after));
                STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
                status = TYPE_STATUS_INVALID_ARGUMENT;
                failed = !sign_extend_prepare(profiles[profile], &state);
                if (!failed)
                {
                    failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                        state.machine, 0u);
                    sign_extend_set_registers(&state);
                    failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                        state.machine);
                    before = test_core_machine_fixture_capture_cpu_after_run(
                        state.machine);
                    failed |= !sign_extend_run(&state, code, sizeof(code),
                        &after, &diagnostic, &status) ||
                        status != TYPE_STATUS_FAULT ||
                        !diagnostic.first_fault.valid ||
                        !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                            VCPUINS_EXCEPT_UD) ||
                        !sign_extend_state_equal(&before, &after);
                }
                core_machine_destroy(state.machine);
                if (failed)
                    return 0;
            }
        }
    }
    return 1;
}

static C_INT sign_extend_test_irq(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0x98u, 0x99u };
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
    {
        sign_extend_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_8 code[] = { opcodes[opcode], 0x90u };
        type_unsigned_16 offset = 0x0100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame = 0u;
        C_INT failed;

        STD_MEMSET(&state, 0, sizeof(state));
        STD_MEMSET(&source, 0, sizeof(source));
        STD_MEMSET(&result, 0, sizeof(result));
        STD_MEMSET(&after, 0, sizeof(after));
        failed = !sign_extend_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed)
        {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u) ||
                core_machine_memory_write(state.machine, 0u, code,
                    sizeof(code)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x80u, &offset,
                    2u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment,
                    2u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &hlt,
                    1u) != TYPE_STATUS_OK;
            sign_extend_set_registers(&state);
            state.machine->executor_cpu.data.eax = opcodes[opcode] == 0x98u ?
                0xaabb0080u : 0xaabb8000u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(
                &state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)&frame, 2u) != TYPE_STATUS_OK ||
                after.data.eip != 0x101u ||
                frame != 1u ||
                after.data.eax != (opcodes[opcode] == 0x98u ?
                    0xaabbff80u : 0xaabb8000u) ||
                after.data.edx != (opcodes[opcode] == 0x99u ?
                    0x5566ffffu : 0x55660000u) ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT sign_extend_test_lock_diagnostic(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0x98u, 0x99u };
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode)
    {
        sign_extend_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 code[] = { 0xf0u, opcodes[opcode] };
        C_INT failed;

        STD_MEMSET(&state, 0, sizeof(state));
        STD_MEMSET(&before, 0, sizeof(before));
        STD_MEMSET(&after, 0, sizeof(after));
        STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
        status = TYPE_STATUS_INVALID_ARGUMENT;
        failed = !sign_extend_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed)
        {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            sign_extend_set_registers(&state);
            state.machine->executor_cpu.data.eax = 0xaabb0080u;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed |= !sign_extend_run(&state, code, sizeof(code), &after,
                &diagnostic, &status) ||
                status != TYPE_STATUS_FAULT ||
                !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD) ||
                !sign_extend_state_equal(&before, &after);
        }
        if (failed)
        {
            STD_PRINTF(
                "SIGN-EXT lock opcode=%02x status=%d fault=%08x "
                "before=%08x/%08x/%08x after=%08x/%08x/%08x\n",
                opcodes[opcode],
                status,
                diagnostic.first_fault.exception_mask,
                before.data.eip,
                before.data.eax,
                before.data.edx,
                after.data.eip,
                after.data.eax,
                after.data.edx);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!sign_extend_test_default())
    {
        STD_PRINTF("SIGN-EXT stage=default\n");
        return 1;
    }
    if (!sign_extend_test_operand32())
    {
        STD_PRINTF("SIGN-EXT stage=operand32\n");
        return 1;
    }
    if (!sign_extend_test_address_prefix())
    {
        STD_PRINTF("SIGN-EXT stage=address-prefix\n");
        return 1;
    }
    if (!sign_extend_test_prefix_reject())
    {
        STD_PRINTF("SIGN-EXT stage=prefix-reject\n");
        return 1;
    }
    if (!sign_extend_test_irq())
    {
        STD_PRINTF("SIGN-EXT stage=irq\n");
        return 1;
    }
    if (!sign_extend_test_lock_diagnostic())
    {
        STD_PRINTF("SIGN-EXT stage=lock\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S29:SIGN-EXTEND:OK\n");
    STD_PRINTF("M5:T401:S45:SIGN-EXTEND-PROFILES:OK\n");
    return 0;
}
