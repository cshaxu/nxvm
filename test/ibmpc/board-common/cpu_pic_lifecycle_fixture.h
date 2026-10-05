#ifndef TEST_CPU_PIC_LIFECYCLE_FIXTURE_H
#define TEST_CPU_PIC_LIFECYCLE_FIXTURE_H

#include "ibmpc/board-common/machine_board_interface.h"
#include "x86/core/debug_interface.h"

static inline lib_i32 test_cpu_pic_binding_after_reset(core_machine *machine)
{
    const lib_u8 program[] = { 0xb0u, 0x0au, 0xe6u, 0x20u, 0xe4u, 0x20u, 0xf4u };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    core_machine_cpu_state state;
    core_machine_run_result result;
    lib_u32 eax;

    /* Exercise reset CPU state and its real memory/PIC port wiring. */
    return machine == LIB_NULL ||
        core_machine_get_cpu_state(machine, &state) != LIB_STATUS_OK ||
        state.cs != 0xf000u || state.eip != 0xfff0u || state.halted ||
        core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, program, sizeof(program)) != LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){8u, 0u}, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EAX, &eax) !=
            LIB_STATUS_OK || (eax & 0xffu) != 0u;
}

#endif
