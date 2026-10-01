#include "support/protected_16_bootstrap_fixture.h"
#include "x86/devices/cpu/cpu.h"
#include <stdio.h>

#define S47_S6_STACK_POINTER 0x12348000u
#define S47_S6_USER_STACK 0x4000u

static lib_i32 s47_s6_patch(test_protected_16_machine *state, lib_u32 mask,
    lib_u32 esp, lib_u32 eflags)
{
    core_machine_debug_register_patch patch = {0};

    patch.mask = mask;
    patch.values[CORE_MACHINE_DEBUG_ESP] = esp;
    patch.values[CORE_MACHINE_DEBUG_EFLAGS] = eflags;
    return core_machine_debug_patch_registers(state->machine, &patch) ==
        LIB_STATUS_OK;
}

static lib_i32 s47_s6_run_same(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_size code_bytes, const lib_u16 *frame,
    lib_size frame_words, lib_u32 expected_esp, lib_u32 expected_eflags)
{
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_i32 failed = !test_protected_16_prepare(&state, profile);

    if (!failed) {
        failed = !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
            code, code_bytes) || !test_protected_16_write(&state,
            TEST_PROTECTED_16_STACK_TOP, frame, frame_words * sizeof(*frame)) ||
            !s47_s6_patch(&state, CORE_MACHINE_DEBUG_REGISTER_MASK(
                CORE_MACHINE_DEBUG_ESP) | CORE_MACHINE_DEBUG_REGISTER_MASK(
                CORE_MACHINE_DEBUG_EFLAGS), S47_S6_STACK_POINTER,
                VCPU_EFLAGS_CF) || core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.eip != 0x0010u || snapshot.cs.selector != 0x0008u ||
            snapshot.cs.dpl != 0u || snapshot.ss.selector != 0x0010u ||
            snapshot.ss.dpl != 0u || snapshot.esp != expected_esp ||
            snapshot.eflags != expected_eflags;
    }
    if (failed) {
        (void)test_protected_16_snapshot(&state, &snapshot);
        printf("same profile=%u ip=%08x cs=%04x ss=%04x sp=%08x fl=%08x reason=%u\n",
            profile, snapshot.eip, snapshot.cs.selector, snapshot.ss.selector,
            snapshot.esp, snapshot.eflags, result.reason);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

static lib_i32 s47_s6_run_outer_retf(lib_u8 immediate)
{
    static const lib_u8 retf[] = {0xcbu};
    static const lib_u8 retf_immediate[] = {0xcau,0x04u,0u};
    static const lib_u16 frame[] = {0x0010u,0x001bu,S47_S6_USER_STACK,0x0023u};
    static const lib_u16 frame_immediate[] = {
        0x0010u,0x001bu,0x1357u,0x2468u,S47_S6_USER_STACK,0x0023u
    };
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_i32 failed = !test_protected_16_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed = !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
            immediate ? retf_immediate : retf,
            immediate ? sizeof(retf_immediate) : sizeof(retf)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_STACK_TOP,
                immediate ? frame_immediate : frame,
                immediate ? sizeof(frame_immediate) : sizeof(frame)) ||
            !s47_s6_patch(&state, CORE_MACHINE_DEBUG_REGISTER_MASK(
                CORE_MACHINE_DEBUG_ESP), S47_S6_STACK_POINTER, 0u) ||
            core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_BUDGET || !test_protected_16_snapshot(&state,
                &snapshot) || snapshot.eip != 0x0010u ||
            snapshot.cs.selector != 0x001bu || snapshot.cs.dpl != 3u ||
            snapshot.ss.selector != 0x0023u || snapshot.ss.dpl != 3u ||
            snapshot.esp != (immediate ? 0x12344004u : 0x12344000u);
    }
    if (failed) {
        (void)test_protected_16_snapshot(&state, &snapshot);
        printf("retf immediate=%d ip=%08x cs=%04x ss=%04x sp=%08x reason=%u\n",
            immediate, snapshot.eip, snapshot.cs.selector, snapshot.ss.selector,
            snapshot.esp, result.reason);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

static lib_i32 s47_s6_run_outer_iret(core_machine_cpu_profile profile,
    lib_i32 address_prefix)
{
    static const lib_u8 iret[] = {0xcfu};
    static const lib_u8 iret_address[] = {0x67u,0xcfu};
    static const lib_u16 frame[] = {0x0010u,0x001bu,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL,
        S47_S6_USER_STACK,0x0023u};
    test_protected_16_machine state;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot snapshot = {0};
    lib_i32 failed = !test_protected_16_prepare(&state, profile);

    if (!failed) {
        failed = !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
            address_prefix ? iret_address : iret,
            address_prefix ? sizeof(iret_address) : sizeof(iret)) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_STACK_TOP,
                frame, sizeof(frame)) || !s47_s6_patch(&state,
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
                S47_S6_STACK_POINTER, VCPU_EFLAGS_CF) ||
            core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_BUDGET || !test_protected_16_snapshot(&state,
                &snapshot) || snapshot.eip != 0x0010u ||
            snapshot.cs.selector != 0x001bu || snapshot.cs.dpl != 3u ||
            snapshot.ss.selector != 0x0023u || snapshot.ss.dpl != 3u ||
            snapshot.esp != 0x12344000u || snapshot.eflags !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL | 0x02u);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

int main(void)
{
    static const lib_u8 iret[] = {0xcfu};
    static const lib_u8 retf[] = {0xcbu};
    static const lib_u16 iret_frame[] = {0x0010u,0x0008u,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL};
    static const lib_u16 retf_frame[] = {0x0010u,0x0008u};

    if (!s47_s6_run_same(CORE_MACHINE_CPU_PROFILE_80286, iret,
            sizeof(iret), iret_frame, sizeof(iret_frame) / sizeof(iret_frame[0]),
            0x12348006u, VCPU_EFLAGS_CF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_IOPL | 0x02u)) return 1;
    if (!s47_s6_run_same(CORE_MACHINE_CPU_PROFILE_80286, retf,
            sizeof(retf), retf_frame, sizeof(retf_frame) / sizeof(retf_frame[0]),
            0x12348004u, VCPU_EFLAGS_CF)) return 2;
    if (!s47_s6_run_outer_retf(LIB_FALSE)) return 3;
    if (!s47_s6_run_outer_retf(LIB_TRUE)) return 4;
    if (!s47_s6_run_outer_iret(CORE_MACHINE_CPU_PROFILE_80286, LIB_FALSE)) return 5;
    if (!s47_s6_run_outer_iret(CORE_MACHINE_CPU_PROFILE_80386, LIB_FALSE)) return 6;
    if (!s47_s6_run_outer_iret(CORE_MACHINE_CPU_PROFILE_80386, LIB_TRUE)) return 7;
    printf("M5:T323:S6:PROTECTED-16-OUTER-IRET:OK\n");
    return 0;
}
