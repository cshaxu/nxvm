#ifndef MYPC110_ES488_H
#define MYPC110_ES488_H

#include "x86-core/machine/machine_interface.h"

typedef struct mypc110_es488 {
    lib_bool reset_asserted;
    lib_u8 response[2];
    lib_u8 response_count;
    lib_u8 response_index;
} mypc110_es488;

void mypc110_es488_reset(mypc110_es488 *device);
lib_status mypc110_es488_install_routes(x86_core_machine *machine,
    mypc110_es488 *device);

#endif
