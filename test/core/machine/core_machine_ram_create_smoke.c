#include "type.h"

#include "core/machine/machine.h"

static C_INT ram_create_success(STD_SIZE_T memory_bytes)
{
    core_machine_config config = { .memory_bytes = memory_bytes };
    core_machine_memory_test_allocation allocation = {0};
    core_machine *machine = STD_NULL;
    STD_SIZE_T installed_bytes = 0u;
    C_INT failed = 0;

    failed |= core_machine_create_with_test_memory_allocation(&config, &machine,
        &allocation) != TYPE_STATUS_OK;
    failed |= machine == STD_NULL || allocation.attempts != 1u;
    failed |= !failed && core_machine_get_memory_bytes(machine, &installed_bytes) !=
        TYPE_STATUS_OK;
    failed |= !failed && installed_bytes != (memory_bytes == 0u ?
        CORE_MACHINE_DEFAULT_MEMORY_BYTES : memory_bytes);
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    core_machine_destroy(machine);
    return failed;
}

static C_INT ram_create_failure(STD_SIZE_T memory_bytes)
{
    core_machine_config config = { .memory_bytes = memory_bytes };
    core_machine_memory_test_allocation allocation = { TYPE_TRUE, 0u };
    core_machine *machine = (core_machine *)(type_virtual_address)1u;
    type_status status = core_machine_create_with_test_memory_allocation(
        &config, &machine, &allocation);

    return status != TYPE_STATUS_NO_MEMORY || machine != STD_NULL ||
        allocation.attempts != 1u;
}

static C_INT ram_fixture_retained(C_VOID)
{
    t_ram ram = {0};
    type_unsigned_8 value = 0x5au;
    type_unsigned_8 observed = 0u;
    C_INT failed = 0;

    core_machine_memory_initialize(&ram);
    failed |= ram.connect.installed_bytes != CORE_MACHINE_DEFAULT_MEMORY_BYTES;
    failed |= core_machine_memory_allocate_for(&ram,
        CORE_MACHINE_MINIMUM_MEMORY_BYTES) != TYPE_STATUS_OK;
    failed |= ram.connect.installed_bytes != CORE_MACHINE_MINIMUM_MEMORY_BYTES;
    failed |= core_machine_memory_write_physical(&ram, 0u,
        (type_virtual_address)&value, sizeof(value)) != TYPE_STATUS_OK;
    failed |= core_machine_memory_read_physical(&ram, 0u,
        (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK;
    failed |= observed != value;
    core_machine_memory_finalize(&ram);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT failed = 0;

    failed |= ram_create_success(0u);
    failed |= ram_create_success(CORE_MACHINE_MINIMUM_MEMORY_BYTES);
    failed |= ram_create_failure(0u);
    failed |= ram_create_failure(CORE_MACHINE_MINIMUM_MEMORY_BYTES);
    failed |= ram_fixture_retained();
    if (failed) return 1;
    puts("M5:T313:S2:RAM-CREATE:OK");
    return 0;
}
