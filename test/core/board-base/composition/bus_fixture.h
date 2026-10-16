#include "lib/types/test.h"
#ifndef TEST_CORE_MACHINE_BUS_FIXTURE_H
#define TEST_CORE_MACHINE_BUS_FIXTURE_H
#include "x86-core/machine/port_interface.h"
#include "x86-core/machine/machine_interface.h"

static inline lib_status test_core_machine_fixture_probe_read(void *owner,
    lib_u16 port, lib_u64 tick, lib_u32 *value)
{
    (void)owner;
    (void)port;
    (void)tick;
    (void)value;
    return LIB_STATUS_UNSUPPORTED;
}

static inline lib_status test_core_machine_fixture_probe_write(void *owner,
    lib_u16 port, lib_u32 value)
{
    (void)owner;
    (void)port;
    (void)value;
    return LIB_STATUS_UNSUPPORTED;
}

/* Construction-only conflict probe, not an I/O operation or private lookup.
 * An unused endpoint is revoked before its local token leaves scope. */
static inline lib_i32 test_core_machine_fixture_expect_port(x86_core_machine *machine,
    lib_u16 port, lib_bool write, lib_bool present)
{
    lib_u8 owner = 0u;
    const x86_core_machine_port_route probe = {
        .address = port,
        .read = write ? LIB_NULL : test_core_machine_fixture_probe_read,
        .write = write ? test_core_machine_fixture_probe_write : LIB_NULL,
        .owner = &owner
    };

    if (!x86_core_machine_configuration_is_open(machine)) return 1;
    const lib_status status = x86_core_machine_install_port_routes(machine, &probe, 1u);
    lib_i32 failed = status != (present ? LIB_STATUS_INVALID_STATE : LIB_STATUS_OK);
    if (status == LIB_STATUS_OK)
        failed |= x86_core_machine_remove_port_routes(machine, &owner) != LIB_STATUS_OK;
    return failed;
}

static inline lib_u32 test_core_machine_fixture_read_bus(
    x86_core_machine *machine, lib_u16 address)
{
    lib_u32 value = 0u;
    if (machine != LIB_NULL &&
        x86_core_machine_bus_read(machine, address, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return value;
}

static inline lib_u8 test_core_machine_fixture_read_port(
    x86_core_machine *machine, lib_u16 address)
{
    return (lib_u8)test_core_machine_fixture_read_bus(machine, address);
}

static inline void test_core_machine_fixture_write_port(
    x86_core_machine *machine, lib_u16 address, lib_u32 value)
{
    if (x86_core_machine_bus_write(machine, address, value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}
#endif
