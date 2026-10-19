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
    MYRAIDENII_CORE_PROGRAM_BANK_SELECT_ADDRESS = 0x006cbu,
    MYRAIDENII_CORE_SOUND_START = 0x00700u,
    MYRAIDENII_CORE_SOUND_BYTES = 0x00020u,
    MYRAIDENII_CORE_DIP_SWITCHES_ADDRESS = 0x00740u,
    MYRAIDENII_CORE_PLAYER_INPUTS_ADDRESS = 0x00744u,
    MYRAIDENII_CORE_SYSTEM_INPUTS_ADDRESS = 0x0074cu,
    MYRAIDENII_CORE_MAIN_INTERRUPT_VECTOR = 0x30u
};

typedef enum myraidenii_core_board_control {
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_UP = 0,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_DOWN,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_LEFT,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_RIGHT,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_BUTTON_1,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_BUTTON_2,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_2_UP,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_2_DOWN,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_2_LEFT,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_2_RIGHT,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_2_BUTTON_1,
    MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_2_BUTTON_2,
    MYRAIDENII_CORE_BOARD_CONTROL_START_1,
    MYRAIDENII_CORE_BOARD_CONTROL_START_2,
    MYRAIDENII_CORE_BOARD_CONTROL_SERVICE,
    MYRAIDENII_CORE_BOARD_CONTROL_COIN_1,
    MYRAIDENII_CORE_BOARD_CONTROL_COIN_2,
    MYRAIDENII_CORE_BOARD_CONTROL_COUNT
} myraidenii_core_board_control;

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

/* Board controls are active-low electrical inputs. The caller supplies a
 * semantic pressed state; this component alone owns the register polarity. */
lib_status myraidenii_core_board_set_control(myraidenii_core_board *board,
    myraidenii_core_board_control control, lib_bool pressed);
lib_status myraidenii_core_board_set_dip_switches(myraidenii_core_board *board,
    lib_u16 value);
/* Coin inputs are consumed by the later sound component, not the main V30 map. */
lib_status myraidenii_core_board_get_coin_inputs(const myraidenii_core_board *board,
    lib_u8 *out_value);

/* Video will request this one-shot board interrupt once it owns a qualified
 * raster source. The existing Core attachment performs delivery and acknowledgement. */
lib_status myraidenii_core_board_request_main_interrupt(myraidenii_core_board *board);

/* These are the board-owned sides of the future sound-CPU handoff. They do not
 * instantiate or schedule a sound CPU. */
lib_status myraidenii_core_board_sound_command_get(const myraidenii_core_board *board,
    lib_u8 index, lib_u8 *out_value);
lib_status myraidenii_core_board_sound_command_acknowledge(myraidenii_core_board *board);
lib_status myraidenii_core_board_sound_interrupt_is_pending(
    const myraidenii_core_board *board, lib_bool *out_pending);
lib_status myraidenii_core_board_sound_interrupt_acknowledge(myraidenii_core_board *board);
lib_status myraidenii_core_board_sound_response_set(myraidenii_core_board *board,
    lib_u8 index, lib_u8 value);

#ifdef __cplusplus
}
#endif

#endif
