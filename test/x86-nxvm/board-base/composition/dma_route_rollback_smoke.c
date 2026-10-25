#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"

static lib_bool reject_routes;

static lib_status dma_route_install(x86_core_machine *machine,
    const x86_core_machine_port_route *routes, lib_size count)
{
    return reject_routes ? LIB_STATUS_NO_MEMORY :
        x86_core_machine_install_port_routes(machine, routes, count);
}

/* Same-owner failure at the declared Machine boundary. Machine's own tests
 * retain per-allocation rollback and sticky single-entry failure assertions. */
#define x86_core_machine_install_port_routes dma_route_install
#include "x86-nxvm/board-base/dma_bus.c"
#undef x86_core_machine_install_port_routes

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
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    const x86_core_machine_port_provider provider = {LIB_NULL, conflicting_write};
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_dma_bus *bus = LIB_NULL;
    lib_u8 marker = 0u;
    lib_bool failed = LIB_FALSE;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    failed |= x86_core_machine_install_port_provider(machine, 0xd4u, 0xd4u,
        &provider, &marker) != LIB_STATUS_OK;
    failed |= x86_nxvm_board_dma_initialize(&bus, machine, 2u, LIB_FALSE) != LIB_STATUS_INVALID_STATE;
    failed |= bus != LIB_NULL ||
        x86_nxvm_board_dma_initialize(&bus, machine, 2u, LIB_FALSE) != LIB_STATUS_INVALID_STATE ||
        bus != LIB_NULL ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0u, &(lib_u32){0u}) != LIB_STATUS_UNSUPPORTED ||
        x86_core_machine_bus_write(machine, 0u, 0u) != LIB_STATUS_UNSUPPORTED ||
        x86_core_machine_bus_write(machine, 0xd4u, 0u) != LIB_STATUS_OK ||
        marker != 0x5au;
    x86_core_machine_destroy(machine);
    machine = LIB_NULL;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) return 1;
    reject_routes = LIB_TRUE;
    failed |= x86_nxvm_board_dma_initialize(&bus, machine, 2u, LIB_FALSE) != LIB_STATUS_NO_MEMORY;
    failed |= bus != LIB_NULL;
    reject_routes = LIB_FALSE;
    failed |= x86_nxvm_board_dma_initialize(&bus, machine, 2u, LIB_FALSE) != LIB_STATUS_OK;
    failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0u, &(lib_u32){0u}) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x81u, 0u) != LIB_STATUS_OK;
    x86_core_machine_destroy(machine);
    x86_nxvm_board_dma_finalize(bus);
    return failed ? 1 : 0;
}
