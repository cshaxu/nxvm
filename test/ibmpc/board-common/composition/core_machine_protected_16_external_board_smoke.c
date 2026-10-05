#include "support/protected_16_bootstrap_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "x86/chips/cpu/cpu_interface.h"
#include <stdio.h>

#define S47_S4_VECTOR 0x30u
#define S47_S4_HANDLER 0x0300u

static lib_i32 s47_s4_patch_eflags(test_protected_16_machine *state,
    lib_u32 eflags)
{
    core_machine_debug_register_patch patch = {0};

    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
    patch.values[CORE_MACHINE_DEBUG_EFLAGS] = eflags;
    return core_machine_debug_patch_registers(state->machine, &patch) ==
        LIB_STATUS_OK;
}

static lib_i32 s47_s4_install_handler(test_protected_16_machine *state,
    lib_u8 vector, lib_u8 type, lib_u8 dpl, lib_u16 offset)
{
    static const lib_u8 hlt[] = {0xf4u};

    return test_protected_16_install_gate(state, vector, offset, 0x0008u,
        type, dpl, LIB_TRUE) && test_protected_16_write(state,
        TEST_PROTECTED_16_CODE_BASE + offset, hlt, sizeof(hlt));
}

static lib_i32 s47_s4_outer_software(core_machine_cpu_profile profile,
    lib_u8 type)
{
    static const lib_u8 interrupt[] = {0xcdu,S47_S4_VECTOR};
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_u16 frame[5u] = {0};
    lib_status run_status = LIB_STATUS_OK;
    lib_i32 failed = !test_protected_16_prepare(&state, profile) ||
        !test_protected_16_enter_user(&state, profile);

    if (!failed) {
        failed = !s47_s4_install_handler(&state, S47_S4_VECTOR, type, 3u,
            S47_S4_HANDLER) || !s47_s4_patch_eflags(&state,
            CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_TF) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE + 0x0100u,
                interrupt, sizeof(interrupt)) || (run_status = core_machine_run(state.machine,
                (core_machine_run_budget){32u,0u}, &result)) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK || diagnostic.first_fault.valid ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.cs.selector != 0x0008u || snapshot.cs.dpl != 0u ||
            snapshot.ss.selector != 0x0010u || snapshot.ss.dpl != 0u ||
            snapshot.eip != S47_S4_HANDLER + 1u || snapshot.esp !=
                TEST_PROTECTED_16_STACK_TOP - sizeof(frame) ||
            (snapshot.eflags & CORE_MACHINE_DEBUG_EFLAGS_CF) == 0u ||
            (snapshot.eflags & CORE_MACHINE_DEBUG_EFLAGS_TF) != 0u ||
            ((snapshot.eflags & CORE_MACHINE_DEBUG_EFLAGS_IF) != 0u) !=
                (type == TEST_PROTECTED_TRAP_GATE_16) ||
            !test_protected_16_read(&state, TEST_PROTECTED_16_STACK_TOP -
                sizeof(frame), frame, sizeof(frame)) || frame[0] != 0x0102u ||
            frame[1] != 0x001bu || frame[2] != (CORE_MACHINE_DEBUG_EFLAGS_CF |
                CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_TF) || frame[3] != 0x7000u ||
            frame[4] != 0x0023u;
    }
    if (failed) {
        printf("outer profile=%u status=%u reason=%u first=%u/%08x cs=%04x ss=%04x ip=%08x sp=%08x frame=%04x/%04x/%04x/%04x/%04x\n",
        profile, run_status, result.reason, diagnostic.first_fault.valid,
        diagnostic.first_fault.exception_mask, snapshot.cs.selector,
        snapshot.ss.selector, snapshot.eip, snapshot.esp, frame[0], frame[1],
        frame[2], frame[3], frame[4]);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

static lib_i32 s47_s4_outer_dpl_rejection(core_machine_cpu_profile profile)
{
    static const lib_u8 interrupt[] = {0xcdu,S47_S4_VECTOR};
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_i32 failed = !test_protected_16_prepare(&state, profile) ||
        !test_protected_16_enter_user(&state, profile);

    if (!failed) {
        failed = !s47_s4_install_handler(&state, S47_S4_VECTOR,
            TEST_PROTECTED_INTERRUPT_GATE_16, 0u, S47_S4_HANDLER) ||
            !s47_s4_install_handler(&state, 0x0du,
                TEST_PROTECTED_INTERRUPT_GATE_16, 0u, S47_S4_HANDLER + 0x0010u) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE + 0x0100u,
                interrupt, sizeof(interrupt)) || core_machine_run(state.machine,
                (core_machine_run_budget){32u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK || !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_GP ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.cs.selector != 0x0008u || snapshot.eip !=
                S47_S4_HANDLER + 0x0010u;
    }
    test_protected_16_destroy(&state);
    return !failed;
}

/* The board is the only producer of this NMI.  The CPU test never reaches
 * into a CPU or PIC object to synthesize a delivery condition. */
static lib_i32 s47_s4_outer_nmi(core_machine_cpu_profile profile, lib_u8 type)
{
    static const lib_u8 nop[] = {0x90u};
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    core_machine_planar_parity_observation parity = {0};
    lib_u16 frame[5u] = {0};
    lib_i32 failed = !test_protected_16_prepare_with_planar_parity(&state,
        profile, LIB_TRUE, LIB_FALSE) ||
        !test_protected_16_enter_user(&state, profile);

    if (!failed) {
        failed = !s47_s4_install_handler(&state, 0x02u, type, 0u,
            S47_S4_HANDLER) || !s47_s4_patch_eflags(&state,
            CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_IF) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE + 0x0100u,
                nop, sizeof(nop));
    }
    if (!failed) {
        failed = core_machine_report_planar_parity_fault(state.board) !=
            LIB_STATUS_OK || core_machine_get_planar_parity_observation(
            state.board, &parity) != LIB_STATUS_OK || !parity.nmi_signaled;
    }
    if (!failed) {
        failed = core_machine_run(state.machine, (core_machine_run_budget){32u,0u},
            &result) != LIB_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.cs.selector != 0x0008u || snapshot.ss.selector != 0x0010u ||
            snapshot.eip != S47_S4_HANDLER + 1u || snapshot.esp !=
                TEST_PROTECTED_16_STACK_TOP - sizeof(frame) ||
            (snapshot.eflags & CORE_MACHINE_DEBUG_EFLAGS_CF) == 0u ||
            ((snapshot.eflags & CORE_MACHINE_DEBUG_EFLAGS_IF) != 0u) !=
                (type == TEST_PROTECTED_TRAP_GATE_16) ||
            !test_protected_16_read(&state, TEST_PROTECTED_16_STACK_TOP -
                sizeof(frame), frame, sizeof(frame)) || frame[0] != 0x0101u ||
            frame[1] != 0x001bu || frame[2] != (CORE_MACHINE_DEBUG_EFLAGS_CF |
                CORE_MACHINE_DEBUG_EFLAGS_IF) || frame[3] != 0x7000u || frame[4] != 0x0023u;
    }
    test_protected_16_destroy(&state);
    return !failed;
}

int main(void)
{
    if (!s47_s4_outer_software(CORE_MACHINE_CPU_PROFILE_80286,
            TEST_PROTECTED_INTERRUPT_GATE_16)) return 1;
    if (!s47_s4_outer_software(CORE_MACHINE_CPU_PROFILE_80286,
            TEST_PROTECTED_TRAP_GATE_16)) return 2;
    if (!s47_s4_outer_software(CORE_MACHINE_CPU_PROFILE_80386,
            TEST_PROTECTED_INTERRUPT_GATE_16)) return 3;
    if (!s47_s4_outer_software(CORE_MACHINE_CPU_PROFILE_80386,
            TEST_PROTECTED_TRAP_GATE_16)) return 4;
    if (!s47_s4_outer_dpl_rejection(CORE_MACHINE_CPU_PROFILE_80286)) return 5;
    if (!s47_s4_outer_dpl_rejection(CORE_MACHINE_CPU_PROFILE_80386)) return 6;
    if (!s47_s4_outer_nmi(CORE_MACHINE_CPU_PROFILE_80286,
            TEST_PROTECTED_INTERRUPT_GATE_16)) return 7;
    if (!s47_s4_outer_nmi(CORE_MACHINE_CPU_PROFILE_80286,
            TEST_PROTECTED_TRAP_GATE_16)) return 8;
    if (!s47_s4_outer_nmi(CORE_MACHINE_CPU_PROFILE_80386,
            TEST_PROTECTED_INTERRUPT_GATE_16)) return 9;
    if (!s47_s4_outer_nmi(CORE_MACHINE_CPU_PROFILE_80386,
            TEST_PROTECTED_TRAP_GATE_16)) return 10;
    printf("M5:T323:S4:PROTECTED-16-EXTERNAL:OK\n");
    return 0;
}
