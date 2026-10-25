#ifndef MYPC110_EC_BLOCK_INTERFACE_H
#define MYPC110_EC_BLOCK_INTERFACE_H

#include "x86-core/machine/machine_interface.h"

/* L2: documented PC110 software selects one of the captured EC block-B
 * registers at 35EAh, then reads its value at 35EBh.  The capture establishes
 * a fixed 32-register index file, not an EC program or a live power model. */
typedef struct mypc110_ec_block {
    lib_u8 index;
} mypc110_ec_block;

lib_status mypc110_ec_block_install_routes(x86_core_machine *machine,
    mypc110_ec_block *block);

#endif
