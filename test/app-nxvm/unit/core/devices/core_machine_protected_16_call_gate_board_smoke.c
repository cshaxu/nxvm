#include "support/protected_16_bootstrap_fixture.h"
#include "app-nxvm/devices/cpu.h"
#include <stdio.h>

#define S47_S7_GATE_SELECTOR 0x0033u
#define S47_S7_TARGET 0x0320u
#define S47_S7_GP_HANDLER 0x0340u

static lib_i32 s47_s7_install_interrupt_gate(test_protected_16_machine *state,
    lib_u8 vector, lib_u16 offset)
{
    static const lib_u8 hlt[] = {0xf4u};

    return test_protected_16_install_gate(state, vector, offset, 0x0008u,
        VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, LIB_TRUE) &&
        test_protected_16_write(state, TEST_PROTECTED_16_CODE_BASE + offset,
            hlt, sizeof(hlt));
}

static lib_i32 s47_s7_install_call_gate(test_protected_16_machine *state,
    lib_u8 dpl, lib_u8 parameter_count)
{
    lib_u8 descriptor[8u] = {0};
    static const lib_u8 hlt[] = {0xf4u};

    descriptor[0] = (lib_u8)S47_S7_TARGET;
    descriptor[1] = (lib_u8)(S47_S7_TARGET >> 8u);
    descriptor[2] = 0x08u;
    descriptor[4] = parameter_count;
    descriptor[5] = (lib_u8)(0x80u | (dpl << 5u) |
        VCPU_DESC_SYS_TYPE_CALLGATE_16);
    return test_protected_16_write(state, TEST_PROTECTED_16_GDT_BASE + 48u,
        descriptor, sizeof(descriptor)) && test_protected_16_write(state,
        TEST_PROTECTED_16_CODE_BASE + S47_S7_TARGET, hlt, sizeof(hlt));
}

static lib_i32 s47_s7_write_call(test_protected_16_machine *state,
    lib_u16 selector)
{
    const lib_u8 call[] = {0x9au,(lib_u8)S47_S7_TARGET,
        (lib_u8)(S47_S7_TARGET >> 8u),(lib_u8)selector,
        (lib_u8)(selector >> 8u)};

    return test_protected_16_write(state, TEST_PROTECTED_16_CODE_BASE +
        0x0100u, call, sizeof(call));
}

static lib_i32 s47_s7_outer_success(core_machine_cpu_profile profile,
    lib_i32 tss32)
{
    static const lib_u16 parameters[] = {0x1234u,0xabcdu};
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_u16 frame[6u] = {0};
    lib_i32 failed = !test_protected_16_prepare(&state, profile) ||
        !test_protected_16_enter_user_with_tss(&state, profile, tss32);

    if (!failed) {
        failed = !s47_s7_install_call_gate(&state, 3u, 2u) ||
            !s47_s7_write_call(&state, S47_S7_GATE_SELECTOR) ||
            !test_protected_16_write(&state, 0x7000u, parameters,
                sizeof(parameters)) || core_machine_run(state.machine,
                (core_machine_run_budget){32u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.cs.selector != 0x0008u || snapshot.cs.dpl != 0u ||
            snapshot.ss.selector != 0x0010u || snapshot.ss.dpl != 0u ||
            snapshot.eip != S47_S7_TARGET + 1u || snapshot.esp !=
                TEST_PROTECTED_16_STACK_TOP - sizeof(frame) ||
            !test_protected_16_read(&state, TEST_PROTECTED_16_STACK_TOP -
                sizeof(frame), frame, sizeof(frame)) || frame[0] != 0x0105u ||
            frame[1] != 0x001bu || frame[2] != parameters[0] ||
            frame[3] != parameters[1] || frame[4] != 0x7000u ||
            frame[5] != 0x0023u;
    }
    test_protected_16_destroy(&state);
    return !failed;
}

static lib_i32 s47_s7_same_cpl(void)
{
    const lib_u8 call[] = {0x9au,(lib_u8)S47_S7_TARGET,
        (lib_u8)(S47_S7_TARGET >> 8u),0x30u,0u};
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_u16 frame[2u] = {0};
    lib_i32 failed = !test_protected_16_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed = !s47_s7_install_call_gate(&state, 0u, 2u) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
                call, sizeof(call)) || core_machine_run(state.machine,
                (core_machine_run_budget){32u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.cs.selector != 0x0008u || snapshot.ss.selector != 0x0010u ||
            snapshot.eip != S47_S7_TARGET + 1u || snapshot.esp !=
                TEST_PROTECTED_16_STACK_TOP - sizeof(frame) ||
            !test_protected_16_read(&state, TEST_PROTECTED_16_STACK_TOP -
                sizeof(frame), frame, sizeof(frame)) || frame[0] != 5u ||
            frame[1] != 0x0008u;
    }
    test_protected_16_destroy(&state);
    return !failed;
}

static lib_i32 s47_s7_dpl_rejection(void)
{
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_i32 failed = !test_protected_16_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80386) || !test_protected_16_enter_user(&state,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed = !s47_s7_install_call_gate(&state, 0u, 2u) ||
            !s47_s7_write_call(&state, S47_S7_GATE_SELECTOR) ||
            !s47_s7_install_interrupt_gate(&state, 0x0du, S47_S7_GP_HANDLER) ||
            core_machine_run(state.machine, (core_machine_run_budget){32u,0u},
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK || !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.cs.selector != 0x0008u || snapshot.eip !=
                S47_S7_GP_HANDLER;
    }
    if (failed) {
        (void)core_machine_get_cpu_diagnostic(state.machine, &diagnostic);
        (void)test_protected_16_snapshot(&state, &snapshot);
        printf("dpl reason=%u last=%u/%08x cs=%04x ss=%04x ip=%08x\n",
            result.reason, diagnostic.last_delivered_exception.valid,
            diagnostic.last_delivered_exception.exception_mask, snapshot.cs.selector,
            snapshot.ss.selector, snapshot.eip);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

int main(void)
{
    if (!s47_s7_outer_success(CORE_MACHINE_CPU_PROFILE_80286, LIB_FALSE)) return 1;
    if (!s47_s7_outer_success(CORE_MACHINE_CPU_PROFILE_80386, LIB_FALSE)) return 2;
    if (!s47_s7_outer_success(CORE_MACHINE_CPU_PROFILE_80386, LIB_TRUE)) return 3;
    if (!s47_s7_same_cpl()) return 4;
    if (!s47_s7_dpl_rejection()) return 5;
    printf("M5:T323:S7:PROTECTED-16-CALL-GATE:OK\n");
    return 0;
}
