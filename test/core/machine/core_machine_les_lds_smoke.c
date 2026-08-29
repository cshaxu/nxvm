#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct lld_machine {
    core_machine *machine;
} lld_machine;

static C_VOID lld_reset(C_VOID *opaque)
{
    lld_machine *state = (lld_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider lld_provider = {
    lld_reset, STD_NULL
};

static C_INT lld_prepare(core_machine_cpu_profile profile, lld_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &lld_provider, state, &state->machine);
}

static C_INT lld_run_prepared(lld_machine *state, const type_unsigned_8 *code,
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

static C_INT lld_test_real(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xc4u, 0xc5u };
    static const core_machine_cpu_profile legacy_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 pointer16[] = { 0x44u, 0x33u, 0x34u, 0x12u };
    static const type_unsigned_8 pointer32[] = {
        0x44u, 0x33u, 0x22u, 0x11u, 0x34u, 0x12u
    };
    type_unsigned_8 opcode;
    type_unsigned_8 profile;
    type_unsigned_8 operand32;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (profile = 0u; profile != sizeof(legacy_profiles) /
                sizeof(legacy_profiles[0]); ++profile) {
            for (operand32 = 0u; operand32 != 2u; ++operand32) {
                lld_machine state;
                t_cpu after;
                core_machine_cpu_diagnostic diagnostic;
                type_status status;
                type_unsigned_8 code[] = { opcodes[opcode], 0x06u, 0x00u, 0x10u, 0u };
                const type_unsigned_8 *pointer = operand32 ? pointer32 : pointer16;
                type_unsigned_8 code_bytes = operand32 ? 5u : 4u;
                type_unsigned_8 pointer_bytes = operand32 ? 6u : 4u;
                C_INT failed = !lld_prepare(legacy_profiles[profile], &state);

                if (operand32 && profile != 2u)
                    continue;
                if (operand32) {
                    code[0] = 0x66u;
                    code[1] = opcodes[opcode];
                    code[2] = 0x06u;
                    code[3] = 0x00u;
                    code[4] = 0x10u;
                }
                if (!failed) {
                    failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                        state.machine, 0u);
                    state.machine->executor_cpu.data.eflags =
                        VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                    state.machine->executor_cpu.data.es.selector = 0x1111u;
                    state.machine->executor_cpu.data.ds.selector = 0x2222u;
                    failed |= core_machine_memory_write(state.machine, 0x1000u,
                            pointer, pointer_bytes) != TYPE_STATUS_OK ||
                        !lld_run_prepared(&state, code, code_bytes, &after,
                            &diagnostic, &status) || status != TYPE_STATUS_OK ||
                        diagnostic.first_fault.valid || after.data.eip != code_bytes ||
                        after.data.eflags !=
                            (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                        (operand32 ? after.data.eax : after.data.ax) !=
                            (operand32 ? 0x11223344u : 0x3344u) ||
                        (opcode == 0u ? after.data.es.selector :
                            after.data.ds.selector) != 0x1234u;
                }
                core_machine_destroy(state.machine);
                if (failed)
                    return 0;
            }
        }
    }
    return 1;
}

static C_INT lld_test_reg_direct_ud(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xc4u, 0xc5u };
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lld_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_status status;
        type_unsigned_8 code[] = { opcodes[opcode], 0xc0u };
        C_INT failed = !lld_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            state.machine->executor_cpu.data.es.selector = 0x1111u;
            state.machine->executor_cpu.data.ds.selector = 0x2222u;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !lld_run_prepared(&state, code, sizeof(code), &after,
                    &diagnostic, &status) || status != TYPE_STATUS_FAULT ||
                !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags ||
                (opcode == 0u ? after.data.es.selector : after.data.ds.selector) !=
                    (opcode == 0u ? before.data.es.selector :
                        before.data.ds.selector);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT lld_test_80286_operand32_ud(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xc4u, 0xc5u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 opcode;
    type_unsigned_8 profile;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
                ++profile) {
            lld_machine state;
            t_cpu before;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_status status;
            type_unsigned_8 code[] = { 0x66u, opcodes[opcode], 0x06u, 0x00u, 0x10u };
            C_INT failed = !lld_prepare(profiles[profile], &state);

            if (!failed) {
                failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u);
                state.machine->executor_cpu.data.es.selector = 0x1111u;
                state.machine->executor_cpu.data.ds.selector = 0x2222u;
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !lld_run_prepared(&state, code, sizeof(code), &after,
                        &diagnostic, &status) || status != TYPE_STATUS_FAULT ||
                    !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                        diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_UD) || after.data.eip != before.data.eip ||
                    after.data.eax != before.data.eax ||
                    after.data.eflags != before.data.eflags ||
                    (opcode == 0u ? after.data.es.selector :
                        after.data.ds.selector) != (opcode == 0u ?
                            before.data.es.selector : before.data.ds.selector);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT lld_prepare_protected(lld_machine *state)
{
    static const type_unsigned_8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const type_unsigned_8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0xffu, 0xffu, 0, 0, 0, 0x92u, 0, 0, 0xffu, 0xffu, 0, 0x40u, 0, 0x92u,
        0, 0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u,
        0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u, 0xb8u, 0x18u,
        0x00u, 0x8eu, 0xd0u, 0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u,
        0x00u
    };
    static const type_unsigned_8 hlt[] = { 0xf4u };
    core_machine_run_result result;

    return lld_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer,
            sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
            sizeof(bootstrap)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, hlt,
            sizeof(hlt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 96u, 0u },
            &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT lld_test_protected(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xc4u, 0xc5u };
    static const type_unsigned_8 pointer16[] = { 0x44u, 0x33u, 0x10u, 0x00u };
    static const type_unsigned_8 pointer32[] = {
        0x44u, 0x33u, 0x22u, 0x11u, 0x10u, 0x00u
    };
    type_unsigned_8 opcode;
    type_unsigned_8 operand32;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        for (operand32 = 0u; operand32 != 2u; ++operand32) {
            lld_machine state;
            core_machine_run_result result;
            t_cpu after;
            type_unsigned_8 code[] = { opcodes[opcode], 0x06u, 0x00u, 0x10u, 0u };
            const type_unsigned_8 *pointer = operand32 ? pointer32 : pointer16;
            type_unsigned_8 code_bytes = operand32 ? 5u : 4u;
            type_unsigned_8 pointer_bytes = operand32 ? 6u : 4u;
            C_INT failed = !lld_prepare_protected(&state);

            if (operand32) {
                code[0] = 0x66u;
                code[1] = opcodes[opcode];
                code[2] = 0x06u;
                code[3] = 0x00u;
                code[4] = 0x10u;
            }
            if (!failed) {
                state.machine->executor_cpu.data.es.selector = 0x1111u;
                state.machine->executor_cpu.data.ds.selector = 0x2222u;
                state.machine->executor_cpu.data.eflags =
                    VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
                failed |= core_machine_memory_write(state.machine, 0x1000u,
                        pointer, pointer_bytes) != TYPE_STATUS_OK ||
                    core_machine_memory_write(state.machine, 0x2000u, code,
                        code_bytes) != TYPE_STATUS_OK;
                test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
                failed |= core_machine_run(state.machine,
                        (core_machine_run_budget){ 1u, 0u }, &result) !=
                            TYPE_STATUS_OK ||
                    result.reason != CORE_MACHINE_STOP_BUDGET;
                after = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= after.data.eip != code_bytes ||
                    (operand32 ? after.data.eax : after.data.ax) !=
                        (operand32 ? 0x11223344u : 0x3344u) ||
                    after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF) ||
                    (opcode == 0u ? after.data.es.selector :
                        after.data.ds.selector) != 0x0010u;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT lld_test_source_fault_atomicity(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xc4u, 0xc5u };
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lld_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_unsigned_8 code[] = { opcodes[opcode], 0x06u, 0x00u, 0x10u };
        C_INT failed = !lld_prepare_protected(&state);

        if (!failed) {
            state.machine->executor_cpu.data.ds.limit = 0x1001u;
            state.machine->executor_cpu.data.eax = 0x55557777u;
            state.machine->executor_cpu.data.es.selector = 0x1111u;
            state.machine->executor_cpu.data.ds.selector = 0x2222u;
            state.machine->executor_cpu.data.eflags =
                VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            failed |= core_machine_memory_write(state.machine, 0x2000u, code,
                    sizeof(code)) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 1u, 0u }, &result) !=
                        TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags ||
                (opcode == 0u ? after.data.es.selector : after.data.ds.selector) !=
                    (opcode == 0u ? before.data.es.selector :
                        before.data.ds.selector);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT lld_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0xc4u, 0xc5u };
    static const type_unsigned_8 pointer[] = { 0x44u, 0x33u, 0x00u, 0x00u };
    static const type_unsigned_8 hlt = 0xf4u;
    type_unsigned_8 opcode;

    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        lld_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_16 vector_offset = 0x0100u;
        type_unsigned_16 vector_segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_8 code[] = { opcodes[opcode], 0x06u, 0x00u, 0x10u, 0x90u };
        C_INT failed = !lld_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u) ||
                core_machine_memory_write(state.machine, 0x1000u, pointer,
                    sizeof(pointer)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, code,
                    sizeof(code)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u,
                    &vector_offset, sizeof(vector_offset)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x20u * 4u + 2u,
                    &vector_segment, sizeof(vector_segment)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x0100u, &hlt,
                    sizeof(hlt)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 2u, 0u }, &result) !=
                        TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    after.data.ss.base + (type_unsigned_16)after.data.esp,
                    (type_virtual_address)&frame_ip, sizeof(frame_ip)) !=
                        TYPE_STATUS_OK || after.data.eip != 0x0101u ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                    state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                frame_ip != 4u;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!lld_test_real() || !lld_test_reg_direct_ud() ||
            !lld_test_80286_operand32_ud() || !lld_test_protected() ||
            !lld_test_source_fault_atomicity() || !lld_test_irq_no_shadow())
        return 1;
    STD_PRINTF("M5:T316:S25:LES-LDS:OK\n");
    return 0;
}
