#ifndef MYPC110_OPL2_H
#define MYPC110_OPL2_H

#include "x86-core/machine/machine_interface.h"

typedef struct mypc110_opl2 {
    x86_core_machine *core;
    void *output_context;
    lib_status (*before_write)(void *context, lib_u64 tick);
    lib_u64 deadline[2];
    lib_u8 registers[256];
    lib_u8 address;
    lib_u8 control;
    lib_u8 overflow;
    lib_u32 phase[9][2];
    double level[9][2];
    double feedback[9];
    lib_u8 envelope[9][2];
} mypc110_opl2;

void mypc110_opl2_reset(mypc110_opl2 *device);
void mypc110_opl2_render(mypc110_opl2 *device, lib_i16 *samples, lib_u32 count);
lib_status mypc110_opl2_install_routes(x86_core_machine *core, mypc110_opl2 *device);

#endif
