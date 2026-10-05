#include "ibmpc/board-common/pit_bus_interface.h"

static lib_status pit_read_counter(void *owner, lib_u8 counter,
    lib_u32 *out_value)
{
    lib_u8 value = 0u;
    lib_status status;
    if (owner == LIB_NULL || out_value == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_pit_read_counter(owner, counter, &value);
    if (status == LIB_STATUS_OK) *out_value = value;
    return status;
}

static lib_status pit_read_0(void *owner, lib_u16 address, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)address; (void)tick;
    return pit_read_counter(owner, 0u, out_value);
}

static lib_status pit_read_1(void *owner, lib_u16 address, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)address; (void)tick;
    return pit_read_counter(owner, 1u, out_value);
}

static lib_status pit_read_2(void *owner, lib_u16 address, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)address; (void)tick;
    return pit_read_counter(owner, 2u, out_value);
}

static lib_status pit_write_0(void *owner, lib_u16 address, lib_u32 value)
{
    (void)address;
    return x86_pit_write_register(owner, 0u, (lib_u8)value);
}

static lib_status pit_write_1(void *owner, lib_u16 address, lib_u32 value)
{
    (void)address;
    return x86_pit_write_register(owner, 1u, (lib_u8)value);
}

static lib_status pit_write_2(void *owner, lib_u16 address, lib_u32 value)
{
    (void)address;
    return x86_pit_write_register(owner, 2u, (lib_u8)value);
}

static lib_status pit_write_3(void *owner, lib_u16 address, lib_u32 value)
{
    (void)address;
    return x86_pit_write_register(owner, 3u, (lib_u8)value);
}

lib_status core_machine_pit_install_ports(core_machine *machine,
    x86_pit *device, lib_u16 base_port)
{
    const core_machine_port_read_provider readers[4] = {
        pit_read_0, pit_read_1, pit_read_2, LIB_NULL
    };
    const core_machine_port_write_provider writers[4] = {
        pit_write_0, pit_write_1, pit_write_2, pit_write_3
    };
    core_machine_port_route routes[4];
    lib_u8 index;

    if (machine == LIB_NULL || device == LIB_NULL || base_port > 0xfffcu)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < 4u; ++index) {
        routes[index] = (core_machine_port_route) {
            (lib_u16)(base_port + index), readers[index], writers[index],
            device, LIB_FALSE, 0u
        };
    }
    return core_machine_install_port_routes(machine, routes, 4u);
}
