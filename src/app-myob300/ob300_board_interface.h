#ifndef MYOB300_BOARD_INTERFACE_H
#define MYOB300_BOARD_INTERFACE_H

#include "x86-nxvm/board-base/machine_board_interface.h"

extern const x86_nxvm_board_config myob300_board_configuration;

#define MYOB300_SYSTEM_ROM_BYTES 0x8000u

/* The optional reset image is borrowed through board construction, when
 * the immutable firmware mappings take their own copies. */
lib_status myob300_board_configure(x86_nxvm_board_plan *plan,
    const lib_u8 *reset_image, lib_size bytes);

#endif
