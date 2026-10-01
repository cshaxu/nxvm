#include "app-nxvm/devices/pit_bus.h"

static lib_status core_machine_pit_bus_read(void *owner, lib_u16 address,
    lib_u32 *out_value)
{
    core_machine_pit_bus *bus = owner;
    lib_u8 value = 0u;
    lib_status status;

    if (bus == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_pit_read_counter(bus->device,
        (lib_u8)(address - bus->base_port), &value);
    if (status == LIB_STATUS_OK) *out_value = value;
    return status;
}

static lib_status core_machine_pit_bus_write(void *owner, lib_u16 address,
    lib_u32 value)
{
    core_machine_pit_bus *bus = owner;
    if (bus == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_pit_write_register(bus->device,
        (lib_u8)(address - bus->base_port), (lib_u8)value);
}

lib_status core_machine_pit_bus_create(core_machine_pit_bus *bus,
    core_machine *machine,
    x86_pit_personality personality, lib_u16 base_port)
{
    core_machine_port_route routes[4];
    lib_status status;
    lib_u8 index;
    if (bus == LIB_NULL || machine == LIB_NULL || base_port > 0xfffcu)
        return LIB_STATUS_INVALID_ARGUMENT;
    bus->device = LIB_NULL;
    status = x86_pit_create(personality, &bus->device);
    if (status != LIB_STATUS_OK) return status;
    bus->base_port = base_port;
    for (index = 0u; index < 4u; ++index) {
        routes[index] = (core_machine_port_route) {(lib_u16)(base_port + index),
            index < 3u ? core_machine_pit_bus_read : LIB_NULL,
            core_machine_pit_bus_write, bus, LIB_FALSE};
    }
    status = core_machine_install_port_routes(machine, routes, 4u);
    if (status != LIB_STATUS_OK) {
        core_machine_pit_bus_destroy(bus);
    }
    return status;
}

void core_machine_pit_bus_destroy(core_machine_pit_bus *bus)
{
    if (bus == LIB_NULL) return;
    x86_pit_destroy(bus->device);
    bus->device = LIB_NULL;
}
