#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct setcc_machine {
    core_machine *machine;
} setcc_machine;

static C_VOID setcc_reset(C_VOID *opaque)
{
    setcc_machine *state = (setcc_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider setcc_provider = {
    setcc_reset, STD_NULL
};

static C_INT setcc_prepare(core_machine_cpu_profile profile,
    setcc_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &setcc_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT setcc_run_one(setcc_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T code_size, t_cpu *out_cpu, core_machine_cpu_diagnostic *out_diagnostic,
    C_INT expect_fault)
{
    const core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    type_status status;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        out_cpu == STD_NULL || out_diagnostic == STD_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, code_size) !=
            TYPE_STATUS_OK) return 0;
    if (expect_fault && !test_core_machine_fixture_preflight_real_ud_terminal(
            state->machine)) return 0;
    status = core_machine_run(state->machine, budget, &result);
    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        TYPE_STATUS_OK || status != (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) ||
        result.reason != (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_BUDGET)) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static type_unsigned_32 setcc_flags(type_unsigned_8 condition, C_INT truth)
{
    static const type_unsigned_32 true_flags[16] = {
        VCPU_EFLAGS_OF, 0u, VCPU_EFLAGS_CF, 0u,
        VCPU_EFLAGS_ZF, 0u, VCPU_EFLAGS_CF, 0u,
        VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_PF, 0u,
        VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_ZF, 0u
    };
    static const type_unsigned_32 false_flags[16] = {
        0u, VCPU_EFLAGS_OF, 0u, VCPU_EFLAGS_CF,
        0u, VCPU_EFLAGS_ZF, 0u, VCPU_EFLAGS_CF,
        0u, VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_PF,
        0u, VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_ZF
    };

    return truth ? true_flags[condition] : false_flags[condition];
}

static C_INT setcc_test_register_conditions(C_VOID)
{
    type_unsigned_8 condition;
    C_INT truth;
    C_INT failed = 0;

    for (condition = 0u; condition != 16u; ++condition) {
        for (truth = 0; truth != 2; ++truth) {
            const type_unsigned_8 code[] = {0x0fu, (type_unsigned_8)(0x90u + condition), 0xc0u};
            const type_unsigned_32 flags = setcc_flags(condition, truth);
            setcc_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;

            if (!setcc_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state)) return 0;
            state.machine->executor_cpu.data.eax = 0x123456a5u;
            state.machine->executor_cpu.data.eflags = flags;
            if (!setcc_run_one(&state, code, sizeof(code), &after, &diagnostic, 0) ||
                diagnostic.first_fault.valid || after.data.eax !=
                    (0x12345600u | (type_unsigned_32)truth) ||
                after.data.eflags != flags || after.data.eip != sizeof(code)) failed = 1;
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT setcc_test_memory_conditions(C_VOID)
{
    const type_unsigned_16 destination = 0x1200u;
    type_unsigned_8 condition;
    C_INT truth;

    for (condition = 0u; condition != 16u; ++condition) {
        for (truth = 0; truth != 2; ++truth) {
            const type_unsigned_8 code[] = {0x0fu, (type_unsigned_8)(0x90u + condition), 0x06u,
                (type_unsigned_8)destination, (type_unsigned_8)(destination >> 8u)};
            const type_unsigned_32 flags = setcc_flags(condition, truth);
            const type_unsigned_8 initial = 0xa5u;
            type_unsigned_8 value = 0u;
            setcc_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            C_INT failed = !setcc_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

            if (!failed) {
                state.machine->executor_cpu.data.eax = 0x87654321u;
                state.machine->executor_cpu.data.eflags = flags;
                failed |= core_machine_memory_write(state.machine, destination, &initial,
                    sizeof(initial)) != TYPE_STATUS_OK || !setcc_run_one(&state, code,
                    sizeof(code), &after, &diagnostic, 0) || diagnostic.first_fault.valid ||
                    core_machine_memory_read(state.machine, destination, &value,
                    sizeof(value)) != TYPE_STATUS_OK || value != (type_unsigned_8)truth ||
                    after.data.eax != 0x87654321u || after.data.eflags != flags ||
                    after.data.eip != sizeof(code);
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT setcc_test_prefix_forms(C_VOID)
{
    static const type_unsigned_8 operand_prefix[] = {0x66u, 0x0fu, 0x94u, 0xc0u};
    static const type_unsigned_8 address_prefix[] = {0x67u, 0x0fu, 0x94u, 0x06u};
    const type_unsigned_32 flags = VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF;
    const type_unsigned_32 address = 0x00002345u;
    const type_unsigned_8 initial = 0xa5u;
    type_unsigned_8 value = 0u;
    setcc_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !setcc_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x112233a5u;
        state.machine->executor_cpu.data.eflags = flags;
        failed |= !setcc_run_one(&state, operand_prefix, sizeof(operand_prefix), &after,
            &diagnostic, 0) || diagnostic.first_fault.valid ||
            after.data.eax != 0x11223301u || after.data.eflags != flags ||
            after.data.eip != sizeof(operand_prefix);
    }
    if (!failed) {
        state.machine->executor_cpu.data.esi = address;
        state.machine->executor_cpu.data.eflags = flags;
        failed |= core_machine_memory_write(state.machine, address, &initial,
            sizeof(initial)) != TYPE_STATUS_OK || !setcc_run_one(&state, address_prefix,
            sizeof(address_prefix), &after, &diagnostic, 0) || diagnostic.first_fault.valid ||
            core_machine_memory_read(state.machine, address, &value, sizeof(value)) !=
                TYPE_STATUS_OK || value != 1u || after.data.esi != address ||
            after.data.eflags != flags || after.data.eip != sizeof(address_prefix);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT setcc_prepare_protected_limit(setcc_machine *state)
{
    static const type_unsigned_8 gdt_pointer[] = {0x1fu,0,0,0x03u,0,0};
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0x40u,0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[] = {0xf4u};
    const core_machine_run_budget budget = {96u, 0u};
    core_machine_run_result result;

    return setcc_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) ==
            TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0u,
            bootstrap, sizeof(bootstrap)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, halt, sizeof(halt)) ==
            TYPE_STATUS_OK && core_machine_run(state->machine, budget, &result) ==
            TYPE_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT setcc_test_pre_fault_nonpublication(C_VOID)
{
    static const type_unsigned_8 ud_code[] = {0x0fu, 0x94u, 0xc0u};
    static const type_unsigned_8 limit_code[] = {0x67u, 0x0fu, 0x94u, 0x05u,
        0x10u, 0x00u, 0x00u, 0x00u};
    const type_unsigned_32 flags = VCPU_EFLAGS_ZF | VCPU_EFLAGS_OF;
    const type_unsigned_32 out_of_limit = 0x00003010u;
    const type_unsigned_8 initial = 0xa5u;
    const core_machine_run_budget budget = {1u, 0u};
    type_unsigned_8 value = 0u;
    setcc_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    C_INT failed = !setcc_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x556677a5u;
        state.machine->executor_cpu.data.eflags = flags;
        failed |= !setcc_run_one(&state, ud_code, sizeof(ud_code), &after, &diagnostic, 1) ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x556677a5u || after.data.eflags != flags ||
            after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !setcc_prepare_protected_limit(&state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x99aabbccu;
        state.machine->executor_cpu.data.eflags = flags;
        failed |= core_machine_memory_write(state.machine, out_of_limit, &initial,
            sizeof(initial)) != TYPE_STATUS_OK ||
            core_machine_memory_read(state.machine, out_of_limit, &value,
                sizeof(value)) != TYPE_STATUS_OK || value != initial ||
            core_machine_memory_write(state.machine, 0x2000u, limit_code,
                sizeof(limit_code)) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                out_of_limit, TYPE_REFERENCE_OF(value),
                sizeof(value)) != TYPE_STATUS_OK || value != initial ||
            after.data.eax != 0x99aabbccu || after.data.eflags != flags ||
            after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!setcc_test_register_conditions() || !setcc_test_memory_conditions() ||
        !setcc_test_prefix_forms() || !setcc_test_pre_fault_nonpublication()) return 1;
    STD_PRINTF("M5:T310:S3:SETCC:OK\n");
    STD_PRINTF("M5:T401:S60:SETCC-PROFILES:OK\n");
    return 0;
}
