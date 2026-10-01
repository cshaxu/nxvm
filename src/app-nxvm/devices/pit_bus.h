#ifndef CORE_MACHINE_PIT_BUS_H
#define CORE_MACHINE_PIT_BUS_H
#include "x86/chips/pit825x/pit825x_interface.h"
#include "app-nxvm/devices/port.h"

/* Board-owned port attachment; the timer itself remains opaque. */
typedef struct core_machine_pit_bus {
    x86_pit *device;
    lib_u16 base_port;
} core_machine_pit_bus;

/* Construction only: bus must not already own a chip or registered routes. */
lib_status core_machine_pit_bus_create(core_machine_pit_bus *bus, t_port *ports,
    x86_pit_personality personality, lib_u16 base_port);
/* No bus access may run during or after destruction. The owning machine
 * removes its port routes before releasing the attachment storage. */
void core_machine_pit_bus_destroy(core_machine_pit_bus *bus);
#endif
