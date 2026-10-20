#ifndef MYRAIDENII_CORE_SOUND_H
#define MYRAIDENII_CORE_SOUND_H

#include "lib/types/types_interface.h"

/* The Board's guest axis is 16 MHz. The selected sound Z80 is clocked at
 * 28.636363 MHz / 8. This private bridge preserves the fractional phase until
 * the later Z80 owner consumes whole sound clocks. */
enum {
    MYRAIDENII_CORE_SOUND_BOARD_TICKS_PER_SECOND = 16000000u,
    MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR = 28636363u,
    MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR =
        8u * MYRAIDENII_CORE_SOUND_BOARD_TICKS_PER_SECOND
};

typedef struct myraidenii_core_sound {
    lib_u64 z80_ticks;
    lib_u64 z80_phase;
} myraidenii_core_sound;

void myraidenii_core_sound_reset(myraidenii_core_sound *sound);
lib_status myraidenii_core_sound_advance(myraidenii_core_sound *sound,
    lib_u64 board_ticks);

#endif
