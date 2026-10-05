#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "../core_machine_board_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "x86/core/device_support_interface.h"
typedef struct fs_gs_machine { core_machine *machine; } fs_gs_machine;

static lib_i32 fs_gs_prepare(core_machine_cpu_profile profile, fs_gs_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile, .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    lib_memory_set(state, 0, sizeof(*state));
    return core_machine_create(&config, &state->machine, LIB_NULL) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(state->machine) == LIB_STATUS_OK &&
        core_machine_reset(state->machine) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &entry) == LIB_STATUS_OK;
}

static lib_i32 fs_gs_prepare_protected(fs_gs_machine *state)
{
    static const lib_u8 pointer[] = { 0x1fu,0,0,0x03u,0,0 };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    core_machine_run_result result;
    lib_i32 ready = fs_gs_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer, sizeof(pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap, sizeof(bootstrap)) == LIB_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 10u,0u }, &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_BUDGET && result.executed == 10u;
    const lib_u8 limit[] = {0xffu, 0x7fu};
    const core_machine_debug_register_patch registers = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_FS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_GS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_SS] = 0x18u,
            [CORE_MACHINE_DEBUG_FS] = 0x10u,
            [CORE_MACHINE_DEBUG_GS] = 0x18u,
            [CORE_MACHINE_DEBUG_EFLAGS] = 0x41u
        }
    };

    /* CPU tests retain the original deliberately inconsistent selector/cache
     * pairs. This receiver uses valid descriptors through public board setup. */
    return ready && core_machine_memory_write(state->machine, 0x318u, limit,
        sizeof(limit)) == LIB_STATUS_OK &&
        core_machine_debug_patch_registers(state->machine, &registers) == LIB_STATUS_OK;
}

static lib_i32 fs_gs_test_pop_stack_fault(void)
{
    static const lib_u8 opcodes[] = { 0xa1u, 0xa9u };
    lib_u8 opcode;
    for (opcode = 0u; opcode != sizeof(opcodes); ++opcode) {
        fs_gs_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_debug_cpu_snapshot before;
        core_machine_debug_cpu_snapshot after;
        lib_i32 failed = !fs_gs_prepare_protected(&state);
        if (!failed) {
            failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u,
                    (lib_u8[]){0x0fu,opcodes[opcode]}, 2u) != LIB_STATUS_OK ||
                core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
                    &result) != LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
                core_machine_debug_capture_cpu_snapshot(state.machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
                !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, (1u << 8)) ||
                after.eip != 0u || after.esp != 0x8000u ||
                after.eflags != before.eflags ||
                (opcode == 0u ? after.fs.selector : after.gs.selector) !=
                    (opcode == 0u ? before.fs.selector : before.gs.selector);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!fs_gs_test_pop_stack_fault()) return 1;
    lib_c_printf("M5:T539:S26:FS-GS-BOARD-FAULT:OK\n");
    return 0;
}
