#ifndef MYRAIDENII_CORE_SOUND_H
#define MYRAIDENII_CORE_SOUND_H

#include "lib/types/types_interface.h"
#include "myraidenii/core/audio_interface.h"

/* The Board's guest axis is 16 MHz. The selected sound Z80 is clocked at
 * 28.636363 MHz / 8. This private bridge preserves the fractional phase until
 * the later Z80 owner consumes whole sound clocks. */
enum {
    MYRAIDENII_CORE_SOUND_BOARD_TICKS_PER_SECOND = 16000000u,
    MYRAIDENII_CORE_SOUND_PCM_FRAME_CAPACITY = 4096u,
    MYRAIDENII_CORE_SOUND_PROGRAM_BYTES = 65536u,
    MYRAIDENII_CORE_SOUND_SAMPLE_BYTES = 0x40000u,
    MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR = 28636363u,
    MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR =
        8u * MYRAIDENII_CORE_SOUND_BOARD_TICKS_PER_SECOND
};

typedef struct myraidenii_core_sound myraidenii_core_sound;

/* These are the Board-owned sides of the fixed Seibu handoff. Sound does not
 * retain a Board pointer outside this explicitly supplied local contract. */
typedef struct myraidenii_core_sound_transport {
    void *context;
    lib_status (*command_get)(void *context, lib_u8 index, lib_u8 *out_value);
    lib_status (*command_is_pending)(void *context, lib_bool *out_pending);
    lib_status (*command_acknowledge)(void *context);
    lib_status (*interrupt_acknowledge)(void *context);
    lib_status (*response_set)(void *context, lib_u8 index, lib_u8 value);
    lib_status (*coin_get)(void *context, lib_u8 *out_value);
} myraidenii_core_sound_transport;

typedef struct myraidenii_core_sound_config {
    const lib_u8 *program;
    lib_size program_bytes;
    const lib_u8 *samples_1;
    lib_size samples_1_bytes;
    const lib_u8 *samples_2;
    lib_size samples_2_bytes;
    myraidenii_core_sound_transport transport;
} myraidenii_core_sound_config;

lib_status myraidenii_core_sound_create(
    const myraidenii_core_sound_config *config, myraidenii_core_sound **out_sound);
void myraidenii_core_sound_destroy(myraidenii_core_sound *sound);
void myraidenii_core_sound_reset(myraidenii_core_sound *sound);
lib_status myraidenii_core_sound_advance(myraidenii_core_sound *sound,
    lib_u64 board_ticks);
/* Copies and consumes up to out_frame_capacity interleaved stereo frames from
 * the sole bounded Sound queue. A null output is allowed only for zero
 * capacity. Dropped frames are reported separately so a later Machine owner
 * can distinguish a discontinuity from an empty queue. */
lib_status myraidenii_core_sound_pcm_copy(myraidenii_core_sound *sound,
    lib_i16 *out_samples, lib_u32 out_frame_capacity, lib_u32 *out_frames,
    lib_u64 *out_dropped_frames);

#endif
