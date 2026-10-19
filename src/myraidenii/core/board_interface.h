#ifndef MYRAIDENII_CORE_BOARD_INTERFACE_H
#define MYRAIDENII_CORE_BOARD_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct myraidenii_core_board myraidenii_core_board;

enum {
    MYRAIDENII_CORE_PROGRAM_LANE_BYTES = 0x80000u,
    MYRAIDENII_CORE_PROGRAM_WINDOW_START = 0x20000u,
    MYRAIDENII_CORE_PROGRAM_WINDOW_BYTES = 0x20000u,
    MYRAIDENII_CORE_STATIC_PROGRAM_START = 0x40000u,
    MYRAIDENII_CORE_STATIC_PROGRAM_BYTES = 0xc0000u,
    MYRAIDENII_CORE_PROGRAM_BANK_SELECT_ADDRESS = 0x006cbu
};

/* The two immutable V30 program lanes are copied while the board is created.
 * The caller may release its input after a successful return. */
typedef struct myraidenii_core_board_config {
    const lib_u8 *program_even;
    const lib_u8 *program_odd;
    lib_size program_lane_bytes;
} myraidenii_core_board_config;

lib_status myraidenii_core_board_create(const myraidenii_core_board_config *config,
    myraidenii_core_board **out_board);
void myraidenii_core_board_destroy(myraidenii_core_board *board);

/* Composition-time borrowed Core executor. The board retains ownership. */
x86_core_machine *myraidenii_core_board_machine_get(myraidenii_core_board *board);

#ifdef __cplusplus
}
#endif

#endif
