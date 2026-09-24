#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_executor_fixture.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct checked_memory_provider {
    lib_u32 read_count;
    lib_u32 write_count;
    lib_u32 query_count;
    lib_u8 value;
} checked_memory_provider;

static lib_status checked_memory_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes)
{
    checked_memory_provider *provider = (checked_memory_provider *)owner;

    if (provider == LIB_NULL || physical != 0x00180000u || bytes != 1u) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    ++provider->read_count;
    *(lib_u8 *)destination = provider->value;
    return LIB_STATUS_OK;
}

static lib_status checked_memory_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    checked_memory_provider *provider = (checked_memory_provider *)owner;

    if (provider == LIB_NULL || physical != 0x00180000u || bytes != 1u) {
        return LIB_STATUS_INTERNAL_ERROR;
    }
    ++provider->write_count;
    provider->value = *(const lib_u8 *)source;
    return LIB_STATUS_OK;
}

static lib_status checked_memory_query(void *owner, lib_u32 physical,
    lib_uptr bytes, core_machine_memory_access access)
{
    checked_memory_provider *provider = (checked_memory_provider *)owner;

    if (provider == LIB_NULL || physical != 0x00180000u || bytes != 1u ||
        (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
         access != CORE_MACHINE_MEMORY_ACCESS_WRITE)) {
        return LIB_STATUS_UNSUPPORTED;
    }
    ++provider->query_count;
    return LIB_STATUS_OK;
}

static lib_i32 expect_status(lib_status actual, lib_status expected)
{
    return actual == expected ? 0 : 1;
}

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    checked_memory_provider provider = { 0u, 0u, 0u, 0x5au };
    core_machine_memory_route route;
    lib_u8 value = 0u;
    lib_i32 failed = 0;

    failed |= expect_status(test_core_machine_create_executor(
        CORE_MACHINE_MINIMUM_MEMORY_BYTES, &machine), LIB_STATUS_OK);
    if (failed) return 1;

    failed |= expect_status(core_machine_memory_query(machine, 0u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), LIB_STATUS_INVALID_STATE);
    failed |= expect_status(test_core_machine_fixture_register_memory_device_provider(machine,
        0x00180000u, 1u, checked_memory_read, checked_memory_write,
        checked_memory_query, &provider), LIB_STATUS_OK);
    failed |= expect_status(core_machine_freeze_execution_providers(machine),
        LIB_STATUS_OK);
    failed |= expect_status(core_machine_reset(machine), LIB_STATUS_OK);
    failed |= expect_status(core_machine_set_a20(machine, 1), LIB_STATUS_OK);

    failed |= expect_status(core_machine_memory_query(machine, 0x20u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), LIB_STATUS_OK);
    failed |= route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    failed |= expect_status(core_machine_memory_query(machine, 0x00180000u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), LIB_STATUS_OK);
    failed |= route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
    failed |= provider.query_count != 1u || provider.read_count != 0u ||
        provider.write_count != 0u;
    failed |= expect_status(core_machine_memory_query(machine, 0x00180000u, 1u,
        CORE_MACHINE_MEMORY_ACCESS_WRITE, &route), LIB_STATUS_OK);
    failed |= provider.query_count != 2u || provider.read_count != 0u ||
        provider.write_count != 0u;
    failed |= expect_status(core_machine_memory_read(machine, 0x00180000u,
        &value, 1u), LIB_STATUS_OK);
    failed |= value != 0x5au || provider.read_count != 1u;
    value = 0x3cu;
    failed |= expect_status(core_machine_memory_write(machine, 0x00180000u,
        &value, 1u), LIB_STATUS_OK);
    failed |= provider.value != 0x3cu || provider.write_count != 1u;

    failed |= expect_status(core_machine_memory_query(machine, 0u, 0u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), LIB_STATUS_INVALID_ARGUMENT);
    failed |= expect_status(core_machine_memory_query(machine, 0u, 1u,
        (core_machine_memory_access)99, &route), LIB_STATUS_INVALID_ARGUMENT);
    failed |= expect_status(core_machine_memory_query(machine, 0xffffffffu, 2u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), LIB_STATUS_INTERNAL_ERROR);
    failed |= expect_status(core_machine_memory_query(machine, 0x001fffffu, 2u,
        CORE_MACHINE_MEMORY_ACCESS_READ, &route), LIB_STATUS_INTERNAL_ERROR);

    core_machine_destroy(machine);
    if (failed) return 1;
    puts("M5:T243:S2:CHECKED-MEMORY:OK");
    return 0;
}
