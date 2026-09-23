#ifndef CORE_CPU_H
#define CORE_CPU_H

#include "core/machine.h"

lib_bool core_cpu_flag_is_set(const core_machine *machine, lib_u8 flag);
void core_cpu_set_nz(core_machine *machine, lib_u8 value);

#endif
