#include "myraidenii/core/sound.h"

void myraidenii_core_sound_reset(myraidenii_core_sound *sound)
{
    if (sound != LIB_NULL) {
        sound->z80_ticks = 0u;
        sound->z80_phase = 0u;
    }
}

lib_status myraidenii_core_sound_advance(myraidenii_core_sound *sound,
    lib_u64 board_ticks)
{
    lib_u64 source_whole;
    lib_u64 source_remainder;
    lib_u64 delivered;
    lib_u64 phase;

    if (sound == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    source_whole = board_ticks / MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    source_remainder = board_ticks % MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    if (source_whole > LIB_UINT64_MAX / MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR)
        return LIB_STATUS_LIMIT_EXCEEDED;
    delivered = source_whole * MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR;
    phase = source_remainder * MYRAIDENII_CORE_SOUND_Z80_CLOCK_NUMERATOR + sound->z80_phase;
    if (delivered > LIB_UINT64_MAX - phase / MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR)
        return LIB_STATUS_LIMIT_EXCEEDED;
    delivered += phase / MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    if (sound->z80_ticks > LIB_UINT64_MAX - delivered) return LIB_STATUS_LIMIT_EXCEEDED;
    sound->z80_ticks += delivered;
    sound->z80_phase = phase % MYRAIDENII_CORE_SOUND_Z80_CLOCK_DENOMINATOR;
    return LIB_STATUS_OK;
}
