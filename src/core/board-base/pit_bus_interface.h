#ifndef CORE_MACHINE_PIT_BUS_INTERFACE_H
#define CORE_MACHINE_PIT_BUS_INTERFACE_H
#include "core/chips/pit825x/pit825x_interface.h"
#include "core/x86/port_interface.h"

/* Construction only. The board owns the opaque timer; Core copies all four
 * routes atomically. The timer must outlive route dispatch. Serialize access
 * and remove its owner routes or destroy the whole Core before reusing them.
 * Serialized Core attachment finalization may destroy the chip immediately
 * before Core discards its routes; no dispatch runs during that teardown. */
lib_status core_machine_pit_install_ports(core_machine *machine,
    x86_pit *device, lib_u16 base_port);
#endif
