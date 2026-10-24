#ifndef MYPC110_LPT_INTERFACE_H
#define MYPC110_LPT_INTERFACE_H

#include "x86-core/machine/machine_interface.h"

typedef struct mypc110_lpt {
    lib_u8 control;
} mypc110_lpt;

lib_status mypc110_lpt_install_routes(x86_core_machine *machine,
    mypc110_lpt *lpt);

#endif
