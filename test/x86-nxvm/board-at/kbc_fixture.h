#include "lib/types/test.h"
#ifndef TEST_X86_NXVM_AT_KBC_FIXTURE_H
#define TEST_X86_NXVM_AT_KBC_FIXTURE_H
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-at/kbc_interface.h"

/* Guest-visible controller status bits. */
#define TEST_KBC_STATUS_OBF 0x01u
#define TEST_KBC_STATUS_SYS 0x04u
#define TEST_KBC_STATUS_CD 0x08u
#define TEST_KBC_STATUS_INHIBIT 0x10u
#define TEST_KBC_STATUS_AUX 0x20u
#define TEST_KBC_COMMAND_DISABLE_AUX 0x20u

/* Protocol tests compose the real adapter and an opaque executor. Keep
 * construction open for IRQ wiring, then freeze before guest bus access. */
static inline x86_core_machine *test_kbc_create_executor(void)
{
    const x86_core_machine_executor_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .memory_bytes = (2u * 1024u * 1024u)
    };
    x86_core_machine *machine = LIB_NULL;
    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return machine;
}

static inline x86_core_machine *test_kbc_create(t_kbc **adapter)
{
    x86_core_machine *machine = test_kbc_create_executor();
    if (x86_nxvm_board_kbc_create(machine, adapter) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return machine;
}

static inline void test_kbc_ready(x86_core_machine *machine)
{
    if (x86_core_machine_configuration_is_open(machine) &&
        (x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
         x86_core_machine_reset(machine) != LIB_STATUS_OK)) lib_test_assert(LIB_FALSE);
}

static inline lib_u32 test_kbc_port_read(x86_core_machine *machine, lib_u16 port)
{
    lib_u32 value = 0u;
    test_kbc_ready(machine);
    if (x86_core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return value;
}

static inline void test_kbc_port_write(x86_core_machine *machine,
    lib_u16 port, lib_u32 value)
{
    test_kbc_ready(machine);
    if (x86_core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}
#endif
