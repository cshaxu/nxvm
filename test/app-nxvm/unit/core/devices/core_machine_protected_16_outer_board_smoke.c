#include "support/protected_16_bootstrap_fixture.h"
#include "x86/devices/cpu/cpu.h"
#include <stdio.h>

#define S47_S5_HANDLER 0x0320u

static lib_i32 s47_s5_patch_eflags(test_protected_16_machine *state,
    lib_u32 eflags)
{
    core_machine_debug_register_patch patch = {0};

    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS);
    patch.values[CORE_MACHINE_DEBUG_EFLAGS] = eflags;
    return core_machine_debug_patch_registers(state->machine, &patch) ==
        LIB_STATUS_OK;
}

static lib_i32 s47_s5_install_handler(test_protected_16_machine *state,
    lib_u8 vector, lib_u8 type)
{
    static const lib_u8 hlt[] = {0xf4u};

    return test_protected_16_install_gate(state, vector, S47_S5_HANDLER,
        0x0008u, type, 0u, LIB_TRUE) && test_protected_16_write(state,
        TEST_PROTECTED_16_CODE_BASE + S47_S5_HANDLER, hlt, sizeof(hlt));
}

static lib_i32 s47_s5_outer_nmi(core_machine_cpu_profile profile,
    lib_i32 tss32, lib_u8 type)
{
    static const lib_u8 nop[] = {0x90u};
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    core_machine_planar_parity_observation parity = {0};
    lib_u16 frame[5u] = {0};
    lib_i32 failed = !test_protected_16_prepare_with_planar_parity(&state,
        profile, LIB_TRUE, LIB_FALSE) || !test_protected_16_enter_user_with_tss(&state,
        profile, tss32);

    if (!failed) {
        failed = !s47_s5_install_handler(&state, 0x02u, type) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE +
                0x0100u, nop, sizeof(nop)) || !s47_s5_patch_eflags(&state,
                VCPU_EFLAGS_CF | VCPU_EFLAGS_IF) ||
            core_machine_report_planar_parity_fault(state.machine) !=
                LIB_STATUS_OK || core_machine_get_planar_parity_observation(
                state.machine, &parity) != LIB_STATUS_OK || !parity.nmi_signaled ||
            core_machine_run(state.machine, (core_machine_run_budget){32u,0u},
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.cs.selector != 0x0008u || snapshot.cs.dpl != 0u ||
            snapshot.ss.selector != 0x0010u || snapshot.ss.dpl != 0u ||
            snapshot.eip != S47_S5_HANDLER + 1u || snapshot.esp !=
                TEST_PROTECTED_16_STACK_TOP - sizeof(frame) ||
            (snapshot.eflags & VCPU_EFLAGS_CF) == 0u ||
            ((snapshot.eflags & VCPU_EFLAGS_IF) != 0u) !=
                (type == VCPU_DESC_SYS_TYPE_TRAPGATE_16) ||
            !test_protected_16_read(&state, TEST_PROTECTED_16_STACK_TOP -
                sizeof(frame), frame, sizeof(frame)) || frame[0] != 0x0101u ||
            frame[1] != 0x001bu || frame[2] != (VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF) || frame[3] != 0x7000u || frame[4] != 0x0023u;
    }
    test_protected_16_destroy(&state);
    return !failed;
}

int main(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 types[] = {
        VCPU_DESC_SYS_TYPE_INTGATE_16, VCPU_DESC_SYS_TYPE_TRAPGATE_16
    };
    lib_size profile;
    lib_size type;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (type = 0u; type < sizeof(types) / sizeof(types[0]); ++type) {
            if (!s47_s5_outer_nmi(profiles[profile], LIB_FALSE, types[type]))
                return 1;
            if (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80386 &&
                !s47_s5_outer_nmi(profiles[profile], LIB_TRUE, types[type]))
                return 2;
        }
    }
    printf("M5:T323:S5:PROTECTED-16-OUTER:OK\n");
    return 0;
}
