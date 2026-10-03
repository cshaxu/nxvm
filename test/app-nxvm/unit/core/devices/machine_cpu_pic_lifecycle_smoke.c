#include "support/pic_fixture.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "app-nxvm/devices/debug_interface.h"

static lib_i32 cpu_pic_binding_is_owned(core_machine *machine,
    const core_machine_cpu_execution_context *cpu)
{
    const lib_u8 program[] = { 0xb0u, 0x0au, 0xe6u, 0x20u, 0xe4u, 0x20u, 0xf4u };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    core_machine_cpu_state state;
    core_machine_run_result result;
    lib_u32 eax;

    /* Assert the same owned instance survives reset, then exercise its memory
     * and PIC port wiring instead of inspecting private callback pointers. */
    return machine == LIB_NULL || cpu == LIB_NULL ||
        machine->executor_cpu_execution != cpu ||
        core_machine_get_cpu_state(machine, &state) != LIB_STATUS_OK ||
        state.cs != 0xf000u || state.eip != 0xfff0u || state.halted ||
        core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, program, sizeof(program)) != LIB_STATUS_OK ||
        core_machine_run(machine, (core_machine_run_budget){8u, 0u}, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EAX, &eax) !=
            LIB_STATUS_OK || (eax & 0xffu) != 0u;
}

lib_i32 main(void)
{
    core_machine_config config = { .memory_bytes = 0u };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_cpu_execution_context *cpu;
    lib_i32 failed = 0;

    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;
    if (failed || machine == LIB_NULL) return 1;
    cpu = machine->executor_cpu_execution;
    failed |= cpu == LIB_NULL;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= cpu_pic_binding_is_owned(machine, cpu);
    if (machine != LIB_NULL) {
        x86_pic_set_inputs(board->shared_pic_master.device, 0u, 0xffu, 0u);
        x86_pic_set_inputs(board->shared_pic_slave.device, 0u, 0xffu, 0u);
    }
    failed |= core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= cpu_pic_binding_is_owned(machine, cpu);
    failed |= machine == LIB_NULL || test_pic_read(&board->shared_pic_master, 0x0au) != 0u ||
        test_pic_read(&board->shared_pic_slave, 0x0au) != 0u;

    core_machine_destroy(machine);
    if (failed != 0) return 1;
    printf("M5:T295:S3:CORE-CPU-PIC-LIFECYCLE:OK\n");
    return 0;
}
