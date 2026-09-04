#ifndef CORE_MACHINE_PIC_INTERFACE_H
#define CORE_MACHINE_PIC_INTERFACE_H

#include "type.h"

/* Zero preserves the PC/AT cascaded pair; a selected single-PIC board omits
 * the slave's guest-visible port decode while retaining one private Core
 * owner. */
typedef enum core_machine_pic_topology {
    CORE_MACHINE_PIC_TOPOLOGY_CASCADED = 0,
    CORE_MACHINE_PIC_TOPOLOGY_SINGLE = 1
} core_machine_pic_topology;

/* Immutable board timing for already-pending IRQs released by an IMR write.
 * The 8259A remains the sole owner of IRR, IMR and interrupt selection. */
typedef struct core_machine_pic_irq_timing {
    type_unsigned_32 unmask_delivery_ticks[16];
} core_machine_pic_irq_timing;

#endif
