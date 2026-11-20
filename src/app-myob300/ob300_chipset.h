#ifndef MYOB300_CHIPSET_H
#define MYOB300_CHIPSET_H

#include "app-myob300/ob300_card.h"

#define MYOB300_CHIPSET_STARTUP_REGISTER_COUNT 33u
#define MYOB300_KEYBOARD_ROWS 8u
#define MYOB300_KEYBOARD_COLUMNS 16u
#define MYOB300_CHIPSET_AB_SHADOW_BYTES 0x20000u

typedef struct myob300_chipset {
    x86_core_machine *core;
    myob300_card card;
    lib_u8 configuration_index;
    lib_u8 miscellaneous;
    lib_u8 management_control;
    lib_u8 startup_registers[MYOB300_CHIPSET_STARTUP_REGISTER_COUNT];
    lib_u8 keyboard_row_select;
    lib_u16 keyboard_columns[MYOB300_KEYBOARD_ROWS];
    lib_u8 ab_access;
    lib_u8 ab_shadow[MYOB300_CHIPSET_AB_SHADOW_BYTES];
    lib_bool configuration_enabled;
    lib_bool management_active;
} myob300_chipset;

/* The board retains chip storage until Core releases its routes. */
lib_status myob300_chipset_install(myob300_chipset *chipset, x86_core_machine *core);
void myob300_chipset_reset(myob300_chipset *chipset);

#endif
