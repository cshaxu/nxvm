#include "lib/types/test.h"
#ifndef TEST_IBMPC_AT_KBC_FIXTURE_H
#define TEST_IBMPC_AT_KBC_FIXTURE_H
#include "lib/types/types_interface.h"
#include "core/x86/machine_interface.h"
#include "core/board-at/kbc.h"

/* Protocol tests compose the real adapter and an opaque executor. Keep
 * construction open for IRQ wiring, then freeze before guest bus access. */
static inline core_machine *test_kbc_create_executor(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    core_machine *machine = LIB_NULL;
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return machine;
}

static inline core_machine *test_kbc_create(t_kbc *adapter)
{
    core_machine *machine = test_kbc_create_executor();
    if (core_machine_kbc_initialize(adapter, machine) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return machine;
}

static inline void test_kbc_ready(core_machine *machine)
{
    if (core_machine_configuration_is_open(machine) &&
        (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
         core_machine_reset(machine) != LIB_STATUS_OK)) lib_test_assert(LIB_FALSE);
}

static inline lib_u32 test_kbc_port_read(core_machine *machine, lib_u16 port)
{
    lib_u32 value = 0u;
    test_kbc_ready(machine);
    if (core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return value;
}

static inline void test_kbc_port_write(core_machine *machine,
    lib_u16 port, lib_u32 value)
{
    test_kbc_ready(machine);
    if (core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}
#endif
