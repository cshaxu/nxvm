#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"

static lib_i32 check_bus(lib_u8 controllers, lib_bool page_ports_plain_scratch)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    const x86_nxvm_board_dma_channel_provider provider = {0};
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_dma_bus *bus = LIB_NULL;
    x86_nxvm_board_dma_request_binding request = {0};
    x86_nxvm_board_dma_request_binding duplicate = {0};
    lib_u32 value = 0u;
    lib_i32 failed = 1;
    lib_u8 owner = 0u;
    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_nxvm_board_dma_initialize(&bus, machine, 0u, LIB_FALSE) != LIB_STATUS_INVALID_ARGUMENT ||
        bus != LIB_NULL ||
        x86_nxvm_board_dma_initialize(&bus, machine, controllers,
            page_ports_plain_scratch) != LIB_STATUS_OK ||
        x86_nxvm_board_dma_bind_channel(bus, 2u, &provider, &owner, &request) != LIB_STATUS_OK ||
        x86_nxvm_board_dma_bind_channel(bus, 2u, &provider, &owner, &duplicate) != LIB_STATUS_INVALID_STATE ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    if (x86_core_machine_bus_write(machine, 0x81u, 0x12u) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x81u, &value) != LIB_STATUS_OK ||
        value != 0x12u ||
        x86_core_machine_bus_write(machine, 0x0bu, 0x46u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0au, 2u) != LIB_STATUS_OK) goto done;
    if (controllers == 2u &&
        (x86_core_machine_bus_write(machine, 0xd6u, 0xc0u) != LIB_STATUS_OK ||
         x86_core_machine_bus_write(machine, 0xd4u, 0u) != LIB_STATUS_OK)) goto done;
    if (x86_core_machine_bus_read(machine, 0xd0u, &value) !=
        (controllers == 2u ? LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED)) goto done;
    x86_nxvm_board_dma_request_assert(bus, &request);
    if (!x86_nxvm_board_dma_has_pending_request(bus) ||
        x86_nxvm_board_dma_get_signals(bus, 0u).requests != 4u) goto done;
    x86_nxvm_board_dma_request_deassert(bus, &request);
    if (x86_nxvm_board_dma_has_pending_request(bus)) goto done;
    if (x86_core_machine_bus_write(machine, 0x0du, 0u) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x81u, &value) != LIB_STATUS_OK ||
        value != 0x12u) goto done;
    if (controllers == 2u && !page_ports_plain_scratch &&
        (x86_core_machine_bus_write(machine, 0x89u, 0x34u) != LIB_STATUS_OK ||
         x86_core_machine_bus_write(machine, 0xdau, 0u) != LIB_STATUS_OK ||
         x86_core_machine_bus_read(machine, 0x89u, &value) != LIB_STATUS_OK ||
         value != 0x34u)) goto done;
    if (page_ports_plain_scratch) {
        for (lib_u16 port = 0x80u; port <= 0x8fu; ++port) {
            if (x86_core_machine_bus_write(machine, port, port) != LIB_STATUS_OK ||
                x86_core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK ||
                value != port) goto done;
        }
    }
    x86_nxvm_board_dma_reset(bus);
    if (x86_core_machine_bus_read(machine, 0x81u, &value) != LIB_STATUS_OK ||
        value != 0u || request.core_token == 0u) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    x86_nxvm_board_dma_finalize(bus);
    return failed;
}

lib_i32 main(void)
{
    return check_bus(1u, LIB_FALSE) | check_bus(2u, LIB_FALSE) |
        check_bus(2u, LIB_TRUE);
}
