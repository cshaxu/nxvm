#ifndef CORE_BUS_H
#define CORE_BUS_H

#include "core/machine.h"

lib_status core_bus_read(core_machine *machine, lib_u16 address, lib_u8 *out_value);
lib_status core_bus_write(core_machine *machine, lib_u16 address, lib_u8 value);
void core_bus_trace_reset(core_machine *machine);

#endif
