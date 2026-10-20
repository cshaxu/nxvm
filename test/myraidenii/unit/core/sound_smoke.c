#include "myraidenii/core/sound.h"

static lib_i32 sound_rational_clock(void)
{
    myraidenii_core_sound sound = {0};
    lib_u64 slice = MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR / 8u;

    for (lib_u8 index = 0u; index < 8u; ++index) {
        if (myraidenii_core_sound_advance(&sound, slice) != LIB_STATUS_OK) return 1;
    }
    return sound.z80_ticks == MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR &&
        sound.z80_phase == 0u ? 0 : 1;
}

static lib_i32 sound_reset_and_bounds(void)
{
    myraidenii_core_sound sound = {0};

    if (myraidenii_core_sound_advance(LIB_NULL, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_core_sound_advance(&sound,
            MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR - 1u) != LIB_STATUS_OK ||
        sound.z80_ticks != MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR - 1u ||
        sound.z80_phase != MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR -
            MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR) {
        return 1;
    }
    myraidenii_core_sound_reset(&sound);
    return sound.z80_ticks == 0u && sound.z80_phase == 0u ? 0 : 1;
}

int main(void)
{
    return sound_rational_clock() || sound_reset_and_bounds() ? 1 : 0;
}
