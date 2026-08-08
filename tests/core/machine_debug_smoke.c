#include "type.h"



#include "core/machine/debug_interface.h"
#include "../support/core_machine_executor_fixture.h"

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_cpu_state cpu;
    core_machine_debug_instruction_observation observation;
    core_machine_debug_register_patch patch = {0};
    core_machine_run_result result;
    core_machine_run_budget budget = { 2u, 0u };
    uint32_t value;
    C_UCHAR byte = 0x5au;
    C_UCHAR nop = 0x90u;

    if (test_core_machine_create_executor(0u, &machine) != TYPE_STATUS_OK ||
        core_machine_debug_read_cpu(machine, &cpu) != TYPE_STATUS_INVALID_STATE ||
        core_machine_debug_capture_instruction_observation(machine,
            &observation) != TYPE_STATUS_INVALID_STATE ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0u, &byte, 1u) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0xffff0u, &nop, 1u) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0xffff1u, &nop, 1u) != TYPE_STATUS_OK ||
        core_machine_debug_read_cpu(machine, &cpu) != TYPE_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0xfff0u ||
        core_machine_debug_read_memory(machine, 0u, &byte, 1u) != TYPE_STATUS_OK ||
        byte != 0x5au ||
        core_machine_debug_capture_instruction_observation(machine,
            &observation) != TYPE_STATUS_OK ||
        core_machine_debug_step(machine, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_debug_continue(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET) {
        core_machine_destroy(machine);
        return 1;
    }

    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX);
    patch.values[CORE_MACHINE_DEBUG_EAX] = 0x12345678u;
    patch.values[CORE_MACHINE_DEBUG_EBX] = 0x87654321u;
    if (core_machine_debug_patch_registers(machine, &patch) != TYPE_STATUS_OK ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EAX,
            &value) != TYPE_STATUS_OK || value != 0x12345678u ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EBX,
            &value) != TYPE_STATUS_OK || value != 0x87654321u) {
        core_machine_destroy(machine);
        return 1;
    }
    patch.mask |= 0x80000000u;
    patch.values[CORE_MACHINE_DEBUG_EAX] = 0u;
    if (core_machine_debug_patch_registers(machine, &patch) !=
            TYPE_STATUS_INVALID_ARGUMENT ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EAX,
            &value) != TYPE_STATUS_OK || value != 0x12345678u) {
        core_machine_destroy(machine);
        return 1;
    }

    core_machine_destroy(machine);
    puts("M3:T4:S2:DEBUG:OK");
    return 0;
}
