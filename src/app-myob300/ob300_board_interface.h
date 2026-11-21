#ifndef MYOB300_BOARD_INTERFACE_H
#define MYOB300_BOARD_INTERFACE_H

#include "x86-nxvm/board-base/machine_board_interface.h"

extern const x86_nxvm_board_config myob300_board_configuration;
extern const x86_nxvm_board_plan_topology myob300_board_topology;

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
    lib_bool fixed_disk_card_installed;
    myob300_board *profile;
} myob300_board_setup;

lib_status myob300_board_configure(void *owner, x86_nxvm_board_plan *plan);
lib_status myob300_board_capture_display(void *context, x86_video_snapshot *out_snapshot);
lib_status myob300_board_set_matrix_key(myob300_board *board, lib_u8 row,
    lib_u8 column, lib_bool pressed);
void myob300_board_set_power_key(myob300_board *board, lib_bool pressed);
void myob300_board_receive_mouse(myob300_board *board, lib_i32 dx, lib_i32 dy, lib_u32 buttons);

#endif
