#include "type.h"

#include "core/machine/machine_interface.h"
#include "../support/core_machine_executor_fixture.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct checked_memory_provider {
    C_UINT read_count;
    C_UINT write_count;
    C_UINT query_count;
    C_UCHAR value;
} checked_memory_provider;

static type_status checked_memory_read(C_VOID *owner, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned bytes)
{
    checked_memory_provider *provider = (checked_memory_provider *)owner;

    if (provider == STD_NULL || physical != 0x00180000u || bytes != 1u) {
        return TYPE_STATUS_FAULT;
    }
    ++provider->read_count;
    *(C_UCHAR *)destination = provider->value;
    return TYPE_STATUS_OK;
}

static type_status checked_memory_write(C_VOID *owner, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned bytes)
{
    checked_memory_provider *provider = (checked_memory_provider *)owner;

    if (provider == STD_NULL || physical != 0x00180000u || bytes != 1u) {
        return TYPE_STATUS_FAULT;
    }
    ++provider->write_count;
    provider->value = *(const C_UCHAR *)source;
    return TYPE_STATUS_OK;
}

static type_status checked_memory_query(C_VOID *owner, type_unsigned_32 physical,
    type_native_unsigned bytes, core_machine_memory_access access)
{
    checked_memory_provider *provider = (checked_memory_provider *)owner;

    if (provider == STD_NULL || physical != 0x00180000u || bytes != 1u ||
        (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
         access != CORE_MACHINE_MEMORY_ACCESS_WRITE)) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    ++provider->query_count;
    return TYPE_STATUS_OK;
}

static C_INT expect_status(type_status actual, type_status expected)
{
    return actual == expected ? 0 : 1;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    checked_memory_provider provider = { 0u, 0u, 0u, 0x5au };
    core_machine_memory_route route;
    C_UCHAR value = 0u;
    C_INT failed = 0;

    failed |= expect_status(test_core_machine_create_executor(
        CORE_MACHINE_MINIMUM_MEMORY_BYTES, &machine), TYPE_STATUS_OK);
    if (failed) return 1;

    failed |= expect_status(core_machine_memory_query(machine, 0u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), TYPE_STATUS_INVALID_STATE);
    failed |= expect_status(test_core_machine_fixture_register_memory_device_provider(machine,
        0x00180000u, 1u, checked_memory_read, checked_memory_write,
        checked_memory_query, &provider), TYPE_STATUS_OK);
    failed |= expect_status(core_machine_freeze_execution_providers(machine),
        TYPE_STATUS_OK);
    failed |= expect_status(core_machine_reset(machine), TYPE_STATUS_OK);
    failed |= expect_status(core_machine_set_a20(machine, 1), TYPE_STATUS_OK);

    failed |= expect_status(core_machine_memory_query(machine, 0x20u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), TYPE_STATUS_OK);
    failed |= route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    failed |= expect_status(core_machine_memory_query(machine, 0x00180000u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), TYPE_STATUS_OK);
    failed |= route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
    failed |= provider.query_count != 1u || provider.read_count != 0u ||
        provider.write_count != 0u;
    failed |= expect_status(core_machine_memory_query(machine, 0x00180000u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_WRITE, &route), TYPE_STATUS_OK);
    failed |= provider.query_count != 2u || provider.read_count != 0u ||
        provider.write_count != 0u;
    failed |= expect_status(core_machine_memory_read(machine, 0x00180000u,
        &value, 1u), TYPE_STATUS_OK);
    failed |= value != 0x5au || provider.read_count != 1u;
    value = 0x3cu;
    failed |= expect_status(core_machine_memory_write(machine, 0x00180000u,
        &value, 1u), TYPE_STATUS_OK);
    failed |= provider.value != 0x3cu || provider.write_count != 1u;

    failed |= expect_status(core_machine_memory_query(machine, 0u, 0u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), TYPE_STATUS_INVALID_ARGUMENT);
    failed |= expect_status(core_machine_memory_query(machine, 0u, 1u,
        (core_machine_memory_access)99, &route), TYPE_STATUS_INVALID_ARGUMENT);
    failed |= expect_status(core_machine_memory_query(machine, 0xffffffffu, 2u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), TYPE_STATUS_FAULT);
    failed |= expect_status(core_machine_memory_query(machine, 0x001fffffu, 2u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), TYPE_STATUS_FAULT);

    core_machine_destroy(machine);
    if (failed) return 1;
    puts("M5:T243:S2:CHECKED-MEMORY:OK");
    return 0;
}
