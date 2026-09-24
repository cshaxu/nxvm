#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"

static lib_i32 ram_create_success(lib_size memory_bytes)
{
    core_machine_config config = { .memory_bytes = memory_bytes };
    core_machine_memory_test_allocation allocation = {0};
    core_machine *machine = LIB_NULL;
    lib_size installed_bytes = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_create_with_test_memory_allocation(&config, &machine,
        &allocation) != LIB_STATUS_OK;
    failed |= machine == LIB_NULL || allocation.attempts != 1u;
    failed |= !failed && core_machine_get_memory_bytes(machine, &installed_bytes) !=
        LIB_STATUS_OK;
    failed |= !failed && installed_bytes != (memory_bytes == 0u ?
        CORE_MACHINE_DEFAULT_MEMORY_BYTES : memory_bytes);
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 ram_create_failure(lib_size memory_bytes)
{
    core_machine_config config = { .memory_bytes = memory_bytes };
    core_machine_memory_test_allocation allocation = { LIB_TRUE, 0u };
    core_machine *machine = (core_machine *)(lib_uptr)1u;
    lib_status status = core_machine_create_with_test_memory_allocation(
        &config, &machine, &allocation);

    return status != LIB_STATUS_NO_MEMORY || machine != LIB_NULL ||
        allocation.attempts != 1u;
}

static lib_i32 ram_fixture_retained(void)
{
    t_ram ram = {0};
    lib_u8 value = 0x5au;
    lib_u8 observed = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_memory_initialize_for(&ram,
        CORE_MACHINE_DEFAULT_MEMORY_BYTES, LIB_NULL) != LIB_STATUS_OK;
    failed |= ram.connect.installed_bytes != CORE_MACHINE_DEFAULT_MEMORY_BYTES;
    failed |= core_machine_memory_allocate_for(&ram,
        CORE_MACHINE_MINIMUM_MEMORY_BYTES) != LIB_STATUS_OK;
    failed |= ram.connect.installed_bytes != CORE_MACHINE_MINIMUM_MEMORY_BYTES;
    failed |= core_machine_memory_write_physical(&ram, 0u,
        (lib_uptr)&value, sizeof(value)) != LIB_STATUS_OK;
    failed |= core_machine_memory_read_physical(&ram, 0u,
        (lib_uptr)&observed, sizeof(observed)) != LIB_STATUS_OK;
    failed |= observed != value;
    core_machine_memory_finalize(&ram);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 failed = 0;

    failed |= ram_create_success(0u);
    failed |= ram_create_success(CORE_MACHINE_MINIMUM_MEMORY_BYTES);
    failed |= ram_create_failure(0u);
    failed |= ram_create_failure(CORE_MACHINE_MINIMUM_MEMORY_BYTES);
    failed |= ram_fixture_retained();
    if (failed) return 1;
    puts("M5:T313:S2:RAM-CREATE:OK");
    return 0;
}
