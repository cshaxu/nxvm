#include "lib/types/test.h"
#ifndef TEST_CORE_MACHINE_BUS_FIXTURE_H
#define TEST_CORE_MACHINE_BUS_FIXTURE_H
#include "core/x86/port_interface.h"

static inline lib_u32 test_core_machine_fixture_read_bus(
    core_machine *machine, lib_u16 address)
{
    lib_u32 value = 0u;
    if (machine != LIB_NULL &&
        core_machine_bus_read(machine, address, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return value;
}

static inline lib_u8 test_core_machine_fixture_read_port(
    core_machine *machine, lib_u16 address)
{
    return (lib_u8)test_core_machine_fixture_read_bus(machine, address);
}

static inline void test_core_machine_fixture_write_port(
    core_machine *machine, lib_u16 address, lib_u32 value)
{
    if (core_machine_bus_write(machine, address, value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}
#endif
