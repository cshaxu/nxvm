#include "lib/types/file.h"
#include "../core_machine_board_fixture.h"
#include "support/protected_16_bootstrap_fixture.h"
#include "x86-core/cpu/cpu_interface.h"
#define S47_VECTOR 0x30u
#define S47_HANDLER 0x0100u

static lib_i32 s47_patch_register(test_protected_16_machine *state,
    x86_core_cpu_debug_register register_id, lib_u32 value)
{
    x86_core_cpu_debug_register_patch patch = {0};

    patch.mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(register_id);
    patch.values[register_id] = value;
    return x86_core_machine_debug_patch_registers(state->machine, &patch) ==
        LIB_STATUS_OK;
}

static lib_i32 s47_install_gate(test_protected_16_machine *state,
    lib_u8 vector, lib_u8 type, lib_u8 present)
{
    static const lib_u8 hlt[] = {0xf4u};

    return test_protected_16_install_gate(state, vector, S47_HANDLER,
        0x0008u, type, 0u, present) &&
        test_protected_16_write(state, TEST_PROTECTED_16_CODE_BASE + S47_HANDLER,
            hlt, sizeof(hlt));
}

static lib_i32 s47_gate_entry(x86_core_cpu_profile profile, lib_u8 type)
{
    static const lib_u8 interrupt[] = {0xcdu,S47_VECTOR};
    test_protected_16_machine state;
    x86_core_machine_run_result result = {0};
    x86_core_cpu_debug_cpu_snapshot snapshot = {0};
    lib_u16 frame[3u] = {0};
    lib_i32 failed = !test_protected_16_prepare(&state, profile);

    if (!failed) {
        failed = !s47_install_gate(&state, S47_VECTOR, type, LIB_TRUE) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
                interrupt, sizeof(interrupt)) || !s47_patch_register(&state,
                X86_CORE_CPU_DEBUG_EFLAGS, X86_CORE_CPU_DEBUG_EFLAGS_CF | X86_CORE_CPU_DEBUG_EFLAGS_IF |
                X86_CORE_CPU_DEBUG_EFLAGS_TF) || x86_core_machine_run(state.machine,
                (x86_core_machine_run_budget){32u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            !test_protected_16_snapshot(&state, &snapshot) ||
            snapshot.eip != S47_HANDLER + 1u ||
            snapshot.esp != TEST_PROTECTED_16_STACK_TOP - sizeof(frame) ||
            !test_protected_16_read(&state, TEST_PROTECTED_16_STACK_TOP -
                sizeof(frame), frame, sizeof(frame)) || frame[0] != 2u ||
            frame[1] != 0x0008u || frame[2] != (X86_CORE_CPU_DEBUG_EFLAGS_CF |
                X86_CORE_CPU_DEBUG_EFLAGS_IF | X86_CORE_CPU_DEBUG_EFLAGS_TF);
    }
    if (failed) {
        lib_c_printf("profile=%u type=%u reason=%u ip=%08x sp=%08x frame=%04x/%04x/%04x\n",
            profile, type, result.reason, snapshot.eip, snapshot.esp,
            frame[0], frame[1], frame[2]);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

static lib_i32 s47_rejected_encoding(x86_core_cpu_profile profile,
    const lib_u8 *code, lib_size code_bytes)
{
    test_protected_16_machine state;
    x86_core_machine_run_result result = {0};
    lib_i32 failed = !test_protected_16_prepare(&state, profile);

    if (!failed) {
        failed = !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
            code, code_bytes) || !test_core_machine_fixture_shutdown_wait(
            x86_core_machine_run(state.machine, (x86_core_machine_run_budget){8u,0u},
                &result), &result);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

static lib_i32 s47_not_present_gate(void)
{
    static const lib_u8 interrupt[] = {0xcdu,S47_VECTOR};
    test_protected_16_machine state;
    x86_core_machine_run_result result = {0};
    lib_i32 failed = !test_protected_16_prepare(&state,
        X86_CORE_CPU_PROFILE_80386);

    if (!failed) {
        failed = !s47_install_gate(&state, S47_VECTOR,
            TEST_PROTECTED_INTERRUPT_GATE_16, LIB_FALSE) ||
            !test_protected_16_write(&state, TEST_PROTECTED_16_CODE_BASE,
                interrupt, sizeof(interrupt)) || !test_core_machine_fixture_shutdown_wait(
                x86_core_machine_run(state.machine, (x86_core_machine_run_budget){8u,0u},
                    &result), &result);
    }
    test_protected_16_destroy(&state);
    return !failed;
}

static lib_i32 s47_outer_entry(x86_core_cpu_profile profile)
{
    test_protected_16_machine state;
    lib_i32 passed = test_protected_16_prepare(&state, profile) &&
        test_protected_16_enter_user(&state, profile);

    test_protected_16_destroy(&state);
    return passed;
}

int main(void)
{
    static const lib_u8 prefix66[] = {0x66u,0xcdu,S47_VECTOR};
    static const lib_u8 prefix67[] = {0x67u,0xcdu,S47_VECTOR};
    static const lib_u8 lock[] = {0xf0u,0xcdu,S47_VECTOR};

    if (!s47_gate_entry(X86_CORE_CPU_PROFILE_80286,
            TEST_PROTECTED_INTERRUPT_GATE_16)) return 1;
    if (!s47_gate_entry(X86_CORE_CPU_PROFILE_80286,
            TEST_PROTECTED_TRAP_GATE_16)) return 2;
    if (!s47_gate_entry(X86_CORE_CPU_PROFILE_80386,
            TEST_PROTECTED_INTERRUPT_GATE_16)) return 3;
    if (!s47_gate_entry(X86_CORE_CPU_PROFILE_80386,
            TEST_PROTECTED_TRAP_GATE_16)) return 4;
    if (!s47_outer_entry(X86_CORE_CPU_PROFILE_80286)) return 5;
    if (!s47_outer_entry(X86_CORE_CPU_PROFILE_80386)) return 6;
    if (!s47_rejected_encoding(X86_CORE_CPU_PROFILE_80286, prefix66,
            sizeof(prefix66))) return 7;
    if (!s47_rejected_encoding(X86_CORE_CPU_PROFILE_80286, prefix67,
            sizeof(prefix67))) return 8;
    if (!s47_rejected_encoding(X86_CORE_CPU_PROFILE_80386, lock,
            sizeof(lock))) return 9;
    if (!s47_not_present_gate()) return 10;
    lib_c_printf("PROTECTED-16-GATE:OK\n");
    return 0;
}
