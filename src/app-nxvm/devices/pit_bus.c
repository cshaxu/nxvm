#include "app-nxvm/devices/pit_bus.h"

static void core_machine_pit_bus_read(t_port *port, lib_u16 address, void *owner)
{
    core_machine_pit_bus *bus = owner;
    (void)x86_pit_read_counter(bus->device, (lib_u8)(address - bus->base_port),
        &port->data.ioByte);
}

static void core_machine_pit_bus_write(t_port *port, lib_u16 address, void *owner)
{
    core_machine_pit_bus *bus = owner;
    (void)x86_pit_write_register(bus->device, (lib_u8)(address - bus->base_port),
        port->data.ioByte);
}

lib_status core_machine_pit_bus_create(core_machine_pit_bus *bus, t_port *ports,
    x86_pit_personality personality, lib_u16 base_port)
{
    core_machine_port_provider_entry *checkpoint;
    lib_status status;
    if (bus == LIB_NULL || ports == LIB_NULL || base_port > 0xfffcu)
        return LIB_STATUS_INVALID_ARGUMENT;
    bus->device = LIB_NULL;
    status = core_machine_port_registration_status(ports);
    if (status != LIB_STATUS_OK) return status;
    status = x86_pit_create(personality, &bus->device);
    if (status != LIB_STATUS_OK) return status;
    bus->base_port = base_port;
    checkpoint = core_machine_port_registration_begin(ports);
    for (lib_u8 i = 0u; i < 4u; ++i) {
        if (i < 3u) core_machine_port_add_read(ports,
            (lib_u16)(base_port + i), core_machine_pit_bus_read, bus);
        core_machine_port_add_write(ports, (lib_u16)(base_port + i),
            core_machine_pit_bus_write, bus);
    }
    status = core_machine_port_registration_status(ports);
    if (status != LIB_STATUS_OK) {
        core_machine_port_rollback_registration(ports, checkpoint);
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
