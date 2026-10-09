#include "planar_parity_fixture.h"
#include "core/x86/machine.h"

lib_i32 test_planar_parity_memory_fault(core_machine *machine)
{
    lib_u8 written = 0x5au;
    lib_u8 read = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_memory_write(machine, 0x1234u, &written,
            sizeof(written)) != LIB_STATUS_OK ||
        machine->executor_memory.connect.parity == 0u ||
        core_machine_reconfigure_memory(machine, 512u * 1024u) != LIB_STATUS_INVALID_STATE;
    if (!failed) ((lib_u8 *)machine->executor_memory.connect.parity)[0x1234u] ^= 1u;
    if (!failed) failed |= core_machine_memory_read(machine, 0x1234u, &read,
            sizeof(read)) != LIB_STATUS_OK || read != written;
    return failed;
}

lib_i32 test_planar_parity_memory_binding(core_machine *machine,
    const void *expected)
{
    if (expected == LIB_NULL) {
        return machine->executor_memory.connect.parity != 0u ||
            machine->executor_memory.connect.parity_owner != LIB_NULL;
    }
    return machine->executor_memory.connect.parity == 0u ||
        machine->executor_memory.connect.parity_owner != expected;
}

lib_i32 test_planar_parity_unbound_reconfigure(void)
{
    core_machine_config config = {0};
    core_machine *machine = LIB_NULL;
    lib_i32 failed = 0;

    config.memory_bytes = 2u * 1024u * 1024u;
    if (core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK) failed = 1;
    else if (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK) failed = 2;
    else if (core_machine_reset(machine) != LIB_STATUS_OK) failed = 3;
    else if (core_machine_reconfigure_memory(machine, 512u * 1024u) != LIB_STATUS_INVALID_STATE) failed = 4;
    core_machine_destroy(machine);
    return failed;
}
