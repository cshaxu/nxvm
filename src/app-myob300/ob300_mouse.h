#ifndef MYOB300_MOUSE_H
#define MYOB300_MOUSE_H

#include "x86-nxvm/board-base/board_profile_interface.h"

typedef struct myob300_mouse {
    x86_nxvm_board_pic_irq_source *irq;
    lib_u16 x;
    lib_u16 y;
    lib_u8 buttons;
    lib_u8 control;
    lib_bool sensing;
} myob300_mouse;

lib_status myob300_mouse_install(myob300_mouse *mouse,
    const x86_nxvm_board_profile_services *services);
void myob300_mouse_reset(myob300_mouse *mouse);
void myob300_mouse_receive(myob300_mouse *mouse, lib_i32 dx, lib_i32 dy, lib_u32 buttons);

#endif
