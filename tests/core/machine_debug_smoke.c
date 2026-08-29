#include "type.h"



#include "core/machine/debug_interface.h"
#include "../support/core_machine_executor_fixture.h"

typedef struct debug_port_probe { type_status status; type_unsigned_32 value; } debug_port_probe;
static type_status debug_port_read(C_VOID *owner, type_unsigned_16 port, type_unsigned_32 *out)
{ debug_port_probe *probe = owner; (C_VOID)port; if (probe->status != TYPE_STATUS_OK) return probe->status; *out = probe->value; return TYPE_STATUS_OK; }
static type_status debug_port_write(C_VOID *owner, type_unsigned_16 port, type_unsigned_32 value)
{ debug_port_probe *probe = owner; (C_VOID)port; if (probe->status != TYPE_STATUS_OK) return probe->status; probe->value = value; return TYPE_STATUS_OK; }

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_cpu_state cpu;
    core_machine_debug_instruction_observation observation;
    core_machine_debug_register_patch patch = {0};
    core_machine_run_result result;
    core_machine_run_budget budget = { 2u, 0u };
    type_unsigned_32 value;
    C_UCHAR byte = 0x5au;
    C_UCHAR nop = 0x90u;
    debug_port_probe port_probe = {TYPE_STATUS_OK, 0x11u};
    core_machine_port_provider port_provider = {debug_port_read, debug_port_write};

    if (test_core_machine_create_executor(0u, &machine) != TYPE_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
            &port_provider, &port_probe) != TYPE_STATUS_OK ||
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
            &observation) != TYPE_STATUS_OK || observation.cs != 0xf000u ||
        observation.cs_base != 0xffff0000u || observation.eip != 0xfff0u ||
        observation.instruction_byte_count >
            CORE_MACHINE_DEBUG_INSTRUCTION_BYTES ||
        observation.memory_access_count >
            CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY ||
        core_machine_debug_step(machine, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_debug_continue(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET) {
        core_machine_destroy(machine);
        return 1;
    }

    port_probe.status = TYPE_STATUS_FAULT;
    value = 0xdeadbeefu;
    if (core_machine_debug_read_port(machine, 0x00e0u, &value) !=
            TYPE_STATUS_FAULT || value != 0xdeadbeefu ||
        core_machine_debug_write_port(machine, 0x00e0u, 0x55u) !=
            TYPE_STATUS_FAULT) {
        core_machine_destroy(machine);
        return 1;
    }
    port_probe.status = TYPE_STATUS_OK;
    if (core_machine_debug_read_port(machine, 0x00e0u, &value) !=
            TYPE_STATUS_OK || value != 0x11u ||
        core_machine_debug_write_port(machine, 0x00e0u, 0x55u) !=
            TYPE_STATUS_OK || port_probe.value != 0x55u) {
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
