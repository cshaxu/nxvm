#include "video_topology_fixture.h"
#include "../board-base/composition/composition_fixture.h"

lib_bool test_video_missing_write_port(x86_core_machine *machine, lib_u16 port)
{
    return !test_core_port_accepts_write(machine, port);
}

lib_i32 test_video_cga_ports(x86_core_machine *machine)
{
    return (test_core_port_accepts_read(machine, 0x03d4u) << 1) |
        (test_core_port_accepts_read(machine, 0x03d8u) << 2) |
        (test_core_port_accepts_read(machine, 0x03d9u) << 3) |
        (!test_core_port_accepts_write(machine, 0x03d8u) << 4) |
        (!test_core_port_accepts_read(machine, 0x03dau) << 5) |
        (test_core_port_accepts_write(machine, 0x03c0u) << 6) |
        (test_core_port_accepts_read(machine, 0x03c4u) << 7) |
        (test_core_port_accepts_write(machine, 0x03cfu) << 8);
}

lib_i32 test_video_ega_ports(x86_core_machine *machine)
{
    return (!test_core_port_accepts_write(machine, 0x03c0u) << 1) |
        (!test_core_port_accepts_read(machine, 0x03d8u) << 2) |
        (!test_core_port_accepts_read(machine, 0x03d9u) << 3) |
        (test_core_port_accepts_read(machine, 0x03c4u) << 4) |
        (test_core_port_accepts_read(machine, 0x03c5u) << 5) |
        (test_core_port_accepts_read(machine, 0x03ceu) << 6) |
        (test_core_port_accepts_read(machine, 0x03cfu) << 7) |
        (test_core_port_accepts_read(machine, 0x03c1u) << 8) |
        (test_core_port_accepts_read(machine, 0x03c0u) << 9);
}

lib_i32 test_video_memory_route(x86_core_machine *machine, lib_u32 physical,
    x86_core_machine_memory_route expected)
{
    x86_core_machine_memory_route route;
    return x86_core_machine_memory_query(machine,
        physical, 1u, X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) == LIB_STATUS_OK &&
        route == expected;
}
