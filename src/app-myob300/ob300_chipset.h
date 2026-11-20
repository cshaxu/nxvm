#ifndef MYOB300_CHIPSET_H
#define MYOB300_CHIPSET_H

#include "app-myob300/ob300_card.h"

typedef struct myob300_chipset {
    x86_core_machine *core;
    myob300_card card;
    lib_u8 configuration_index;
    lib_u8 miscellaneous;
    lib_u8 management_control;
    lib_bool configuration_enabled;
    lib_bool management_active;
} myob300_chipset;

/* The board retains chip storage until Core releases its routes. */
lib_status myob300_chipset_install(myob300_chipset *chipset, x86_core_machine *core);
void myob300_chipset_reset(myob300_chipset *chipset);

#endif
