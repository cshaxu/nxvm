/* Copyright 2012-2026 Neko. */
#ifndef CORE_MACHINE_PIC_BUS_H
#define CORE_MACHINE_PIC_BUS_H
#include "lib/types/types_interface.h"
#include "core/chips/pic8259/pic8259_interface.h"
#include "core/board-base/pic_bus_interface.h"
#include "core/x86/port_interface.h"

/* Board endpoint, not chip state. Multiple source leases resolve to one input.
 * Pair links belong to this board adapter and never enter the shared chip. */
struct core_machine_pic_bus {
    x86_pic *device;
    struct core_machine_pic_bus *master;
    struct core_machine_pic_bus *slave;
    lib_u8 asserted[8];
    core_machine_pic_irq_source *sources;
};

struct core_machine_pic_irq_source {
    core_machine_pic_bus *master;
    core_machine_pic_bus *slave;
    lib_u8 irq;
    lib_bool asserted;
    core_machine_pic_irq_source *next;
};

#endif
