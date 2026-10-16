/* Copyright 2012-2026 Neko. */
#ifndef X86_NXVM_BOARD_PIC_BUS_H
#define X86_NXVM_BOARD_PIC_BUS_H
#include "lib/types/types_interface.h"
#include "x86-nxvm/chips/pic8259/pic8259_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-core/machine/port_interface.h"

/* Board endpoint, not chip state. Multiple source leases resolve to one input.
 * Pair links belong to this board adapter and never enter the shared chip. */
struct x86_nxvm_board_pic_bus {
    x86_pic *device;
    struct x86_nxvm_board_pic_bus *master;
    struct x86_nxvm_board_pic_bus *slave;
    lib_u8 asserted[8];
    x86_nxvm_board_pic_irq_source *sources;
};

struct x86_nxvm_board_pic_irq_source {
    x86_nxvm_board_pic_bus *master;
    x86_nxvm_board_pic_bus *slave;
    lib_u8 irq;
    lib_bool asserted;
    x86_nxvm_board_pic_irq_source *next;
};

#endif
