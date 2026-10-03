#include "x86/core/machine.h"
#include "x86/ibmpc-common/dma_bus_interface.h"

static lib_status conflicting_write(void *owner, lib_u16 port, lib_u32 value)
{
    (void)port;
    (void)value;
    *(lib_u8 *)owner = 0x5au;
    return LIB_STATUS_OK;
}

/* Core owns allocation and sticky registration failure. DMA remains a public
 * caller of its atomic route batch, not a borrower of the private port table. */
lib_i32 main(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    const core_machine_port_provider provider = {LIB_NULL, conflicting_write};
    core_machine_port_test_allocation allocation = {3u, 0u};
    core_machine *machine = LIB_NULL;
    core_machine_dma_bus *bus = LIB_NULL;
    lib_u8 marker = 0u;
    lib_bool failed = LIB_FALSE;

    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    failed |= core_machine_install_port_provider(machine, 0xd4u, 0xd4u,
        &provider, &marker) != LIB_STATUS_OK;
    failed |= core_machine_dma_initialize(&bus, machine, 2u) != LIB_STATUS_INVALID_STATE;
    failed |= bus != LIB_NULL || core_machine_port_has_read(&machine->executor_port, 0u) ||
        core_machine_port_has_write(&machine->executor_port, 0u) ||
        !core_machine_port_has_write(&machine->executor_port, 0xd4u);
    core_machine_port_write(&machine->executor_port, 0xd4u, 0u);
    failed |= marker != 0x5au;
    failed |= core_machine_port_add_write_provider(&machine->executor_port,
        0xd4u, conflicting_write, &allocation) != LIB_STATUS_INVALID_STATE;
    failed |= core_machine_dma_initialize(&bus, machine, 1u) != LIB_STATUS_INVALID_STATE;
    failed |= bus != LIB_NULL;
    core_machine_destroy(machine);
    machine = LIB_NULL;

    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    core_machine_port_set_test_allocation(&machine->executor_port, &allocation);
    failed |= core_machine_dma_initialize(&bus, machine, 2u) != LIB_STATUS_NO_MEMORY;
    failed |= bus != LIB_NULL || core_machine_port_has_read(&machine->executor_port, 0u) ||
        core_machine_port_has_write(&machine->executor_port, 0u) ||
        core_machine_port_has_write(&machine->executor_port, 0x81u);
    allocation.fail_at = 0u;
    failed |= core_machine_dma_initialize(&bus, machine, 2u) != LIB_STATUS_OK;
    core_machine_destroy(machine);
    core_machine_dma_finalize(bus);
    return failed ? 1 : 0;
}
