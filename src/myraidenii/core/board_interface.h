#ifndef MYRAIDENII_CORE_BOARD_INTERFACE_H
#define MYRAIDENII_CORE_BOARD_INTERFACE_H

#include "lib/types/types_interface.h"
#include "myraidenii/core/audio_interface.h"
#include "myraidenii/core/cop.h"
#include "myraidenii/core/graphics_interface.h"
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

/* All seven selected-set graphics roles are one immutable input bundle. */
typedef struct myraidenii_core_board_graphics_images {
    const lib_u8 *characters;
    const lib_u8 *background_1;
    const lib_u8 *background_2;
    const lib_u8 *sprites_1;
    const lib_u8 *sprites_2;
    const lib_u8 *sprites_3;
    const lib_u8 *sprites_4;
} myraidenii_core_board_graphics_images;

/* Program lanes are required and copied while the board is created. Graphics
 * and the sound program are optional only for bounded Core diagnostics; a
 * supplied graphics bundle must be complete. The caller may release all input
 * buffers after success. */
typedef struct myraidenii_core_board_config {
    const lib_u8 *program_even;
    const lib_u8 *program_odd;
    lib_size program_lane_bytes;
    const myraidenii_core_board_graphics_images *graphics_images;
    const lib_u8 *sound_program;
    lib_size sound_program_bytes;
    const lib_u8 *sound_samples_1;
    lib_size sound_samples_1_bytes;
    const lib_u8 *sound_samples_2;
    lib_size sound_samples_2_bytes;
} myraidenii_core_board_config;

/* Reset clears this bounded fallback observation. It reports only how an
 * unowned Board control-window access arrived; it never retains guest data. */
typedef struct myraidenii_core_board_control_failure_observation {
    lib_bool valid;
    lib_u32 physical;
    lib_u8 bytes;
    x86_core_machine_memory_access access;
} myraidenii_core_board_control_failure_observation;

/* Reset clears this aggregate Sound-path observation. It reports reachability,
 * not guest payload, private device layout or host PCM delivery. */
typedef struct myraidenii_core_board_sound_observation {
    lib_u64 ym_address_writes;
    lib_u64 ym_data_writes;
    lib_u64 oki_writes[2u];
    lib_u64 oki_voice_starts[2u];
    lib_u64 pcm_nonzero_frames;
} myraidenii_core_board_sound_observation;

lib_status myraidenii_core_board_create(const myraidenii_core_board_config *config,
    myraidenii_core_board **out_board);
void myraidenii_core_board_destroy(myraidenii_core_board *board);

/* Composition-time borrowed Core executor. The board retains ownership. */
x86_core_machine *myraidenii_core_board_machine_get(myraidenii_core_board *board);

/* Reset clears this COP-owned observation. Board exposes it for integration
 * diagnostics without exposing guest payload or execution internals. */
lib_status myraidenii_core_board_cop_failure_get(const myraidenii_core_board *board,
    myraidenii_core_cop_failure_observation *out_observation);

lib_status myraidenii_core_board_control_failure_get(const myraidenii_core_board *board,
    myraidenii_core_board_control_failure_observation *out_observation);

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

/* Copies the bounded native frame. It is a read-only Board operation; later
 * Machine presentation owns conversion and publication. */
lib_status myraidenii_core_board_frame_copy(const myraidenii_core_board *board,
    lib_u32 *out_pixels, lib_size pixel_count);

/* The Board only forwards the one Sound-owned, copied PCM queue. A later
 * Machine is responsible for adapting these frames to the host stream. */
lib_status myraidenii_core_board_pcm_copy(myraidenii_core_board *board,
    lib_i16 *out_samples, lib_u32 out_frame_capacity, lib_u32 *out_frames,
    lib_u64 *out_dropped_frames);
lib_status myraidenii_core_board_sound_observation_get(const myraidenii_core_board *board,
    myraidenii_core_board_sound_observation *out_observation);

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
