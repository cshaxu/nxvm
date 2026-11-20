#ifndef MYOB300_BOARD_INTERFACE_H
#define MYOB300_BOARD_INTERFACE_H

#include "x86-nxvm/board-base/machine_board_interface.h"

extern const x86_nxvm_board_config myob300_board_configuration;

#define MYOB300_SYSTEM_ROM_BYTES 0x8000u

typedef struct myob300_board myob300_board;

/* The plan borrows this setup and its optional reset image through
 * construction. The profile is usable only after construction succeeds
 * and is owned by the resulting Core machine. */
typedef struct myob300_board_setup {
    const lib_u8 *reset_image;
    lib_size reset_image_bytes;
    myob300_board *profile;
} myob300_board_setup;

lib_status myob300_board_configure(x86_nxvm_board_plan *plan, myob300_board_setup *setup);
lib_status myob300_board_capture_display(void *context, x86_video_snapshot *out_snapshot);

#endif
