#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct setcc_machine {
    core_machine *machine;
} setcc_machine;

static void setcc_reset(void *opaque)
{
    setcc_machine *state = (setcc_machine *)opaque;

    if (state != LIB_NULL) (void)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider setcc_provider = {
    setcc_reset, LIB_NULL
};

static lib_i32 setcc_prepare(core_machine_cpu_profile profile,
    setcc_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != LIB_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &setcc_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 setcc_run_one(setcc_machine *state, const lib_u8 *code,
    lib_size code_size, t_cpu *out_cpu, core_machine_cpu_diagnostic *out_diagnostic,
    lib_i32 expect_fault)
{
    const core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    lib_status status;

    if (state == LIB_NULL || state->machine == LIB_NULL || code == LIB_NULL ||
        out_cpu == LIB_NULL || out_diagnostic == LIB_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, code_size) !=
            LIB_STATUS_OK) return 0;
    if (expect_fault && !test_core_machine_fixture_preflight_real_ud_terminal(
            state->machine)) return 0;
    status = core_machine_run(state->machine, budget, &result);
    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        LIB_STATUS_OK || status != (expect_fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
        result.reason != (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_BUDGET)) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static lib_u32 setcc_flags(lib_u8 condition, lib_i32 truth)
{
    static const lib_u32 true_flags[16] = {
        VCPU_EFLAGS_OF, 0u, VCPU_EFLAGS_CF, 0u,
        VCPU_EFLAGS_ZF, 0u, VCPU_EFLAGS_CF, 0u,
        VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_PF, 0u,
        VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_ZF, 0u
    };
    static const lib_u32 false_flags[16] = {
        0u, VCPU_EFLAGS_OF, 0u, VCPU_EFLAGS_CF,
        0u, VCPU_EFLAGS_ZF, 0u, VCPU_EFLAGS_CF,
        0u, VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_PF,
        0u, VCPU_EFLAGS_SF, 0u, VCPU_EFLAGS_ZF
    };

    return truth ? true_flags[condition] : false_flags[condition];
}

static lib_i32 setcc_test_register_conditions(void)
{
    lib_u8 condition;
    lib_i32 truth;
    lib_i32 failed = 0;

    for (condition = 0u; condition != 16u; ++condition) {
        for (truth = 0; truth != 2; ++truth) {
            const lib_u8 code[] = {0x0fu, (lib_u8)(0x90u + condition), 0xc0u};
            const lib_u32 flags = setcc_flags(condition, truth);
            setcc_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;

            if (!setcc_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state)) return 0;
            state.machine->executor_cpu.data.eax = 0x123456a5u;
            state.machine->executor_cpu.data.eflags = flags;
            if (!setcc_run_one(&state, code, sizeof(code), &after, &diagnostic, 0) ||
                diagnostic.first_fault.valid || after.data.eax !=
                    (0x12345600u | (lib_u32)truth) ||
                after.data.eflags != flags || after.data.eip != sizeof(code)) failed = 1;
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static lib_i32 setcc_test_memory_conditions(void)
{
    const lib_u16 destination = 0x1200u;
    lib_u8 condition;
    lib_i32 truth;

    for (condition = 0u; condition != 16u; ++condition) {
        for (truth = 0; truth != 2; ++truth) {
            const lib_u8 code[] = {0x0fu, (lib_u8)(0x90u + condition), 0x06u,
                (lib_u8)destination, (lib_u8)(destination >> 8u)};
            const lib_u32 flags = setcc_flags(condition, truth);
            const lib_u8 initial = 0xa5u;
            lib_u8 value = 0u;
            setcc_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            lib_i32 failed = !setcc_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

            if (!failed) {
                state.machine->executor_cpu.data.eax = 0x87654321u;
                state.machine->executor_cpu.data.eflags = flags;
                failed |= core_machine_memory_write(state.machine, destination, &initial,
                    sizeof(initial)) != LIB_STATUS_OK || !setcc_run_one(&state, code,
                    sizeof(code), &after, &diagnostic, 0) || diagnostic.first_fault.valid ||
                    core_machine_memory_read(state.machine, destination, &value,
                    sizeof(value)) != LIB_STATUS_OK || value != (lib_u8)truth ||
                    after.data.eax != 0x87654321u || after.data.eflags != flags ||
                    after.data.eip != sizeof(code);
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static lib_i32 setcc_test_prefix_forms(void)
{
    static const lib_u8 operand_prefix[] = {0x66u, 0x0fu, 0x94u, 0xc0u};
    static const lib_u8 address_prefix[] = {0x67u, 0x0fu, 0x94u, 0x06u};
    const lib_u32 flags = VCPU_EFLAGS_ZF | VCPU_EFLAGS_CF;
    const lib_u32 address = 0x00002345u;
    const lib_u8 initial = 0xa5u;
    lib_u8 value = 0u;
    setcc_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = !setcc_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

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
            sizeof(initial)) != LIB_STATUS_OK || !setcc_run_one(&state, address_prefix,
            sizeof(address_prefix), &after, &diagnostic, 0) || diagnostic.first_fault.valid ||
            core_machine_memory_read(state.machine, address, &value, sizeof(value)) !=
                LIB_STATUS_OK || value != 1u || after.data.esi != address ||
            after.data.eflags != flags || after.data.eip != sizeof(address_prefix);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 setcc_prepare_protected_limit(setcc_machine *state)
{
    static const lib_u8 gdt_pointer[] = {0x1fu,0,0,0x03u,0,0};
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0x40u,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const lib_u8 halt[] = {0xf4u};
    const core_machine_run_budget budget = {96u, 0u};
    core_machine_run_result result;

    return setcc_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) ==
            LIB_STATUS_OK && core_machine_memory_write(state->machine, 0u,
            bootstrap, sizeof(bootstrap)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, halt, sizeof(halt)) ==
            LIB_STATUS_OK && core_machine_run(state->machine, budget, &result) ==
            LIB_STATUS_OK && result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static lib_i32 setcc_test_pre_fault_nonpublication(void)
{
    static const lib_u8 ud_code[] = {0x0fu, 0x94u, 0xc0u};
    static const lib_u8 limit_code[] = {0x67u, 0x0fu, 0x94u, 0x05u,
        0x10u, 0x00u, 0x00u, 0x00u};
    const lib_u32 flags = VCPU_EFLAGS_ZF | VCPU_EFLAGS_OF;
    const lib_u32 out_of_limit = 0x00003010u;
    const lib_u8 initial = 0xa5u;
    const core_machine_run_budget budget = {1u, 0u};
    lib_u8 value = 0u;
    setcc_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_i32 failed = !setcc_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x556677a5u;
        state.machine->executor_cpu.data.eflags = flags;
        failed |= !setcc_run_one(&state, ud_code, sizeof(ud_code), &after, &diagnostic, 1) ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
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
            sizeof(initial)) != LIB_STATUS_OK ||
            core_machine_memory_read(state.machine, out_of_limit, &value,
                sizeof(value)) != LIB_STATUS_OK || value != initial ||
            core_machine_memory_write(state.machine, 0x2000u, limit_code,
                sizeof(limit_code)) != LIB_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                out_of_limit, CORE_MACHINE_REFERENCE_OF(value),
                sizeof(value)) != LIB_STATUS_OK || value != initial ||
            after.data.eax != 0x99aabbccu || after.data.eflags != flags ||
            after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!setcc_test_register_conditions() || !setcc_test_memory_conditions() ||
        !setcc_test_prefix_forms() || !setcc_test_pre_fault_nonpublication()) return 1;
    printf("M5:T310:S3:SETCC:OK\n");
    printf("M5:T401:S60:SETCC-PROFILES:OK\n");
    return 0;
}
