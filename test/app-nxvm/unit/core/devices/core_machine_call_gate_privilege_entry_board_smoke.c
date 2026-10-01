#include "support/protected_16_bootstrap_fixture.h"
#include "x86/chips/cpu/cpu.h"
#include <stdio.h>

#define S48_GATE_SELECTOR 0x0033u
#define S48_TARGET 0x0320u
#define S48_GP_HANDLER 0x0360u

static lib_i32 s48_prepare_user32(test_protected_16_machine *state)
{
    return test_protected_32_prepare(state) && test_protected_32_enter_user(state);
}

static lib_i32 s48_install_call_gate(test_protected_16_machine *state,
    lib_u8 access, lib_u8 parameter_count, lib_u16 target_selector)
{
    lib_u8 descriptor[8u] = {0};
    static const lib_u8 hlt[] = {0xf4u};

    descriptor[0] = (lib_u8)S48_TARGET;
    descriptor[1] = (lib_u8)(S48_TARGET >> 8u);
    descriptor[2] = (lib_u8)target_selector;
    descriptor[3] = (lib_u8)(target_selector >> 8u);
    descriptor[4] = parameter_count;
    descriptor[5] = access;
    return test_protected_16_write(state, TEST_PROTECTED_16_GDT_BASE + 48u,
        descriptor, sizeof(descriptor)) && test_protected_16_write(state,
        TEST_PROTECTED_16_CODE_BASE + S48_TARGET, hlt, sizeof(hlt));
}

static lib_i32 s48_write_call(test_protected_16_machine *state)
{
    const lib_u8 call[] = {0x9au,(lib_u8)S48_TARGET,
        (lib_u8)(S48_TARGET >> 8u),0u,0u,0x33u,0u};

    return test_protected_16_write(state, TEST_PROTECTED_16_CODE_BASE +
        0x0100u, call, sizeof(call));
}

static lib_i32 s48_snapshot_equal(const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return before->cs.selector == after->cs.selector &&
        before->ss.selector == after->ss.selector && before->eip == after->eip &&
        before->esp == after->esp && before->eflags == after->eflags;
}

static lib_i32 s48_outer_parameter_copy(lib_u8 parameter_count)
{
    static const lib_u32 parameters[] = {0x11223344u,0x55667788u};
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_u32 frame[6u] = {0};
    lib_i32 failed = !s48_prepare_user32(&state);

    if (!failed) {
        failed = !s48_install_call_gate(&state, (lib_u8)(0x80u | (3u << 5u) |
            VCPU_DESC_SYS_TYPE_CALLGATE_32), parameter_count, 0x0008u) ||
            !s48_write_call(&state) || (parameter_count != 0u &&
            !test_protected_16_write(&state, 0x7000u, parameters,
                (lib_size)parameter_count * sizeof(parameters[0]))) ||
            core_machine_run(state.machine, (core_machine_run_budget){32u,0u},
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.cs.selector != 0x0008u || snapshot.ss.selector != 0x0010u ||
            snapshot.eip != S48_TARGET + 1u || snapshot.esp !=
                TEST_PROTECTED_16_STACK_TOP - (lib_u32)(4u + parameter_count) * 4u ||
            !test_protected_16_read(&state, snapshot.esp, frame,
                (lib_size)(4u + parameter_count) * sizeof(frame[0])) ||
            frame[0] != 0x0107u || frame[1] != 0x0000001bu ||
            frame[2u + parameter_count] != 0x7000u ||
            frame[3u + parameter_count] != 0x00000023u ||
            (parameter_count > 0u && frame[2] != parameters[0]) ||
            (parameter_count > 1u && frame[3] != parameters[1]);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

static lib_i32 s48_install_gp_handler(test_protected_16_machine *state)
{
    static const lib_u8 hlt[] = {0xf4u};

    return test_protected_16_install_gate(state, 0x0du, S48_GP_HANDLER,
        0x0008u, VCPU_DESC_SYS_TYPE_INTGATE_32, 0u, LIB_TRUE) &&
        test_protected_16_write(state, TEST_PROTECTED_16_CODE_BASE +
            S48_GP_HANDLER, hlt, sizeof(hlt));
}

static lib_i32 s48_gate_rejection(lib_u8 gate_access, lib_u8 target_access)
{
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !s48_prepare_user32(&state);

    if (!failed) {
        failed = !s48_install_call_gate(&state, gate_access, 0u,
            target_access != 0u ? 0x0028u : 0x0008u) ||
            !s48_write_call(&state) || !s48_install_gp_handler(&state) ||
            !test_protected_16_snapshot(&state, &before) ||
            core_machine_run(state.machine, (core_machine_run_budget){32u,0u},
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK || !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
            !test_protected_16_snapshot(&state, &after) ||
            after.cs.selector != 0x0008u || after.eip != S48_GP_HANDLER ||
            s48_snapshot_equal(&before, &after);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

int main(void)
{
    if (!s48_outer_parameter_copy(0u)) return 1;
    if (!s48_outer_parameter_copy(2u)) return 2;
    if (!s48_gate_rejection((lib_u8)(0x80u | VCPU_DESC_SYS_TYPE_CALLGATE_32),
            0u)) return 3;
    if (!s48_gate_rejection((lib_u8)(0x80u | (3u << 5u) |
            VCPU_DESC_SYS_TYPE_CALLGATE_16), 0u)) return 4;
    if (!s48_gate_rejection((lib_u8)(0x80u | (3u << 5u) |
            VCPU_DESC_SYS_TYPE_CALLGATE_32), 0x92u)) return 5;
    printf("M5:T539:S48:CALL-GATE-PRIVILEGE-ENTRY:OK\n");
    return 0;
}
