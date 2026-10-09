#include "video_topology_fixture.h"
#include "core/x86/machine.h"

lib_bool test_video_missing_write_port(core_machine *machine, lib_u16 port)
{
    return !core_machine_port_has_write(&machine->executor_port, port);
}

lib_i32 test_video_cga_ports(core_machine *machine)
{
    const t_port *port = &machine->executor_port;
    return (core_machine_port_has_read(port, 0x03d4u) << 1) |
        (core_machine_port_has_read(port, 0x03d8u) << 2) |
        (core_machine_port_has_read(port, 0x03d9u) << 3) |
        (!core_machine_port_has_write(port, 0x03d8u) << 4) |
        (!core_machine_port_has_read(port, 0x03dau) << 5) |
        (core_machine_port_has_write(port, 0x03c0u) << 6) |
        (core_machine_port_has_read(port, 0x03c4u) << 7) |
        (core_machine_port_has_write(port, 0x03cfu) << 8);
}

lib_i32 test_video_ega_ports(core_machine *machine)
{
    const t_port *port = &machine->executor_port;
    return (!core_machine_port_has_write(port, 0x03c0u) << 1) |
        (!core_machine_port_has_read(port, 0x03d8u) << 2) |
        (!core_machine_port_has_read(port, 0x03d9u) << 3) |
        (!core_machine_port_has_read(port, 0x03c4u) << 4);
}

lib_i32 test_video_memory_route(core_machine *machine, lib_u32 physical,
    core_machine_memory_route expected)
{
    core_machine_memory_route route;
    return core_machine_memory_query_physical(&machine->executor_memory,
        physical, 1u, CORE_MACHINE_MEMORY_ACCESS_READ, &route) == LIB_STATUS_OK &&
        route == expected;
}
