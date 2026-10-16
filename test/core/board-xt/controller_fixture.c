#include "controller_fixture.h"
#include "core/board-xt/xt_ppi_keyboard.h"
#include "x86-core/machine/machine_interface.h"

static lib_bool reject_xt_routes;

static lib_status xt_route_install(x86_core_machine *machine,
    const x86_core_machine_port_route *routes, lib_size count)
{
    return reject_xt_routes ? LIB_STATUS_NO_MEMORY :
        x86_core_machine_install_port_routes(machine, routes, count);
}

#define x86_core_machine_install_port_routes xt_route_install
#include "core/board-xt/xt_ppi_keyboard.c"
#undef x86_core_machine_install_port_routes

static lib_status xt_sentinel_read(void *owner, lib_u16 address, lib_u64 tick,
    lib_u32 *value)
{
    (void)owner;
    (void)address;
    (void)tick;
    *value = 0x5au;
    return LIB_STATUS_OK;
}

lib_status test_xt_ppi_initialize(core_machine_xt_ppi_keyboard *keyboard,
    const core_machine_xt_ppi_keyboard_config *config, x86_core_machine *machine)
{
    return core_machine_xt_ppi_keyboard_initialize(keyboard, config, machine);
}

void test_xt_ppi_finalize(core_machine_xt_ppi_keyboard *keyboard)
{
    core_machine_xt_ppi_keyboard_finalize(keyboard);
}

lib_bool test_xt_ppi_unpublished(const core_machine_xt_ppi_keyboard *keyboard)
{
    return keyboard->ppi == LIB_NULL;
}

lib_bool test_xt_ppi_expect_byte_ready(const core_machine_xt_ppi_keyboard *keyboard,
    lib_bool expected)
{
    return keyboard->byte_ready != expected;
}

lib_bool test_xt_ppi_expect_irq1(const core_machine_xt_ppi_keyboard *keyboard,
    lib_bool expected)
{
    return keyboard->irq1_asserted != expected;
}

lib_bool test_xt_ppi_expect_nmi(const core_machine_xt_ppi_keyboard *keyboard,
    lib_bool expected)
{
    return keyboard->nmi_signaled != expected;
}

lib_bool xt_construction_rollback(void)
{
    const core_machine_xt_ppi_keyboard_config config = {
        0x60u, 0x61u, 0x62u, 0x63u, 1u, 0u, 0u};
    const x86_core_machine_executor_config executor = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    const x86_core_machine_port_provider sentinel = {xt_sentinel_read, LIB_NULL};
    core_machine_xt_ppi_keyboard keyboard = {0};
    x86_core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    lib_bool failed = LIB_FALSE;

    if (x86_core_machine_neutral_create(&executor, &machine) != LIB_STATUS_OK)
        return LIB_TRUE;
    failed |= x86_core_machine_install_port_provider(machine, 0x80u, 0x80u,
        &sentinel, &value) != LIB_STATUS_OK;
    reject_xt_routes = LIB_TRUE;
    failed |= test_xt_ppi_initialize(&keyboard, &config, machine) != LIB_STATUS_NO_MEMORY ||
        !test_xt_ppi_unpublished(&keyboard);
    reject_xt_routes = LIB_FALSE;
    failed |= test_xt_ppi_initialize(&keyboard, &config, machine) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        failed |= x86_core_machine_bus_write(machine, 0x63u, 0x80u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x60u, 0xa5u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x60u, &value) != LIB_STATUS_OK || value != 0xa5u;
    }
    failed |= x86_core_machine_remove_port_routes(machine, &keyboard) != LIB_STATUS_OK;
    test_xt_ppi_finalize(&keyboard);
    test_xt_ppi_finalize(&keyboard);
    for (lib_u16 address = 0x60u; address <= 0x63u; ++address)
        failed |= x86_core_machine_bus_read(machine, address, &value) != LIB_STATUS_UNSUPPORTED ||
            x86_core_machine_bus_write(machine, address, 0u) != LIB_STATUS_UNSUPPORTED;
    failed |= x86_core_machine_bus_read(machine, 0x80u, &value) != LIB_STATUS_OK || value != 0x5au;
    x86_core_machine_destroy(machine);
    return failed;
}
