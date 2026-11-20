#ifndef MYOB300_BOARD_INTERFACE_H
#define MYOB300_BOARD_INTERFACE_H

#include "x86-nxvm/board-base/machine_board_interface.h"

extern const x86_nxvm_board_config myob300_board_configuration;

#define MYOB300_CARD_BOOT_BYTES 0x800000u

typedef struct myob300_board myob300_board;

typedef struct myob300_card_image {
    const lib_u8 *data;
    lib_size bytes;
} myob300_card_image;

/* The plan borrows this setup through construction. The resulting Core
 * machine owns the profile and borrows the immutable card images for
 * its lifetime. The profile is usable only after construction succeeds. */
typedef struct myob300_board_setup {
    myob300_card_image common;
    myob300_card_image attribute;
    myob300_board *profile;
} myob300_board_setup;

lib_status myob300_board_configure(x86_nxvm_board_plan *plan, myob300_board_setup *setup);
lib_status myob300_board_capture_display(void *context, x86_video_snapshot *out_snapshot);

#endif
