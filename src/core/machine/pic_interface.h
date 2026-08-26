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

#endif
