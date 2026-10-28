#ifndef MYPC110_INKING_INTERFACE_H
#define MYPC110_INKING_INTERFACE_H

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-core/chips/vl82c420/vl82c420_interface.h"

typedef struct mypc110_inking_point {
    lib_u8 x, y;
    lib_bool down;
} mypc110_inking_point;

/* L2 bounded receiver: consecutive motion samples coalesce, while pen
 * transitions stay ordered until the guest acknowledges the packet. */
typedef struct mypc110_inking {
    x86_nxvm_board_pic_irq_source *irq;
    x86_nxvm_board_pic_bus *pic_master;
    x86_nxvm_board_pic_bus *pic_slave;
    x86_vl82c420 *configuration;
    lib_u16 base;
    lib_u8 control;
    lib_u8 packet[3];
    lib_u8 position;
    lib_bool pending;
    mypc110_inking_point waiting[8];
    lib_u8 waiting_count;
} mypc110_inking;

lib_status mypc110_inking_install_routes(x86_core_machine *machine,
    mypc110_inking *inking, lib_u16 base);
void mypc110_inking_reset(mypc110_inking *inking);
lib_status mypc110_inking_sample(mypc110_inking *inking,
    lib_u8 x, lib_u8 y, lib_bool down);

#endif
