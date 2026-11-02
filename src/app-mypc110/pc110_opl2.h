#ifndef MYPC110_OPL2_H
#define MYPC110_OPL2_H

#include "x86-core/machine/machine_interface.h"

typedef struct mypc110_opl2 {
    x86_core_machine *core;
    lib_u64 deadline[2];
    lib_u8 registers[256];
    lib_u8 address;
    lib_u8 control;
    lib_u8 overflow;
} mypc110_opl2;

void mypc110_opl2_reset(mypc110_opl2 *device);
lib_status mypc110_opl2_install_routes(x86_core_machine *core, mypc110_opl2 *device);

#endif
