#ifndef TEST_CPU_PIC_LIFECYCLE_FIXTURE_H
#define TEST_CPU_PIC_LIFECYCLE_FIXTURE_H

#include "core/board-base/machine_board_interface.h"
#include "core/x86/debug_interface.h"

static inline lib_i32 test_cpu_pic_binding_after_reset(core_machine *machine)
{
    const lib_u8 program[] = { 0xb0u, 0x0au, 0xe6u, 0x20u, 0xe4u, 0x20u, 0xf4u };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    x86_core_cpu_state state;
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
        core_machine_debug_read_register(machine, X86_CORE_CPU_DEBUG_EAX, &eax) !=
            LIB_STATUS_OK || (eax & 0xffu) != 0u;
}

#endif
