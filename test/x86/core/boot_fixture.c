#include "boot_fixture.h"
#include "x86/core/machine.h"

test_core_boot_observation test_core_boot_capture(const core_machine *machine)
{
    return (test_core_boot_observation){
        .memory_bytes = machine->executor_memory.connect.installed_bytes,
        .a20 = machine->executor_memory.data.flagA20,
        .cpu_profile = machine->cpu_profile,
        .elapsed_ticks = machine->elapsed_ticks,
        .rom_mapping_count = machine->immutable_rom_mapping_count,
        .transaction_owner = machine->transaction.owner,
        .hold_owner = machine->transaction.hold_owner,
        .hold_acknowledged = machine->transaction.hold_acknowledged
    };
}

test_core_boot_rom_mapping test_core_boot_rom_at(const core_machine *machine,
    lib_size index)
{
    return (test_core_boot_rom_mapping){
        machine->immutable_rom_mappings[index].physical_start,
        machine->immutable_rom_mappings[index].bytes
    };
}

void test_core_boot_capture_write_cpu(const core_machine *machine,
    core_machine_cpu_state *cpu)
{
    core_machine_cpu_capture_state(machine->executor_cpu_execution, cpu);
}

lib_status test_core_boot_read_reset(core_machine *machine, lib_u32 address,
    lib_uptr destination, lib_uptr bytes)
{
    return core_machine_memory_read_reset_physical(&machine->executor_memory,
        address, destination, bytes);
}

void test_core_boot_capture_diagnostic(const core_machine *machine,
    core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_cpu_diagnostic_capture(machine, diagnostic);
}

lib_status test_core_boot_query(const core_machine *machine, lib_u32 address,
    lib_uptr bytes, core_machine_memory_access access,
    core_machine_memory_route *route)
{
    return core_machine_memory_query_physical(&machine->executor_memory,
        address, bytes, access, route);
}

lib_bool test_core_boot_bind_write_observer(core_machine *machine,
    core_machine_memory_write_observer observer, void *context)
{
    t_ram *memory = &machine->executor_memory;
    if (memory->connect.write_observer_count >=
            CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY) return LIB_FALSE;
    memory->connect.write_observers[memory->connect.write_observer_count++] =
        (core_machine_memory_write_observer_slot){observer, context};
    return LIB_TRUE;
}
