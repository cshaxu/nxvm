#include "lib/types/test.h"
#include "lib/audio/midi/output.c"

int main(void)
{
    lib_u32 packed = 0;
    lib_u8 bytes[LIB_AUDIO_MIDI_SYSEX_MAX_BYTES + 1] = {0};
    for (lib_u32 status = 0x80; status < 0xf0; ++status) {
        lib_u32 size = (status >> 4 == 0xc || status >> 4 == 0xd) ? 2 : 3;
        bytes[0] = (lib_u8)status;
        bytes[1] = 0x12;
        bytes[2] = 0x34;
        lib_test_assert(audio_midi_short_pack(bytes, size, &packed) == LIB_STATUS_OK);
        lib_test_assert(packed == (status | 0x1200u | (size == 3 ? 0x340000u : 0)));
        lib_test_assert(audio_midi_short_pack(bytes, size - 1, &packed) == LIB_STATUS_INVALID_ARGUMENT);
        bytes[1] = 0x80;
        lib_test_assert(audio_midi_short_pack(bytes, size, &packed) == LIB_STATUS_INVALID_ARGUMENT);
    }
    const lib_u8 lengths[16] = {0, 2, 3, 2, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1};
    for (lib_u32 i = 0; i < 16; ++i) {
        bytes[0] = (lib_u8)(0xf0 + i);
        bytes[1] = bytes[2] = 0;
        lib_test_assert(audio_midi_short_pack(bytes, lengths[i] ? lengths[i] : 1,
            &packed) == (lengths[i] ? LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT));
    }
    lib_test_assert(audio_midi_short_pack(LIB_NULL, 1, &packed) == LIB_STATUS_INVALID_ARGUMENT);
    bytes[0] = 0x40;
    lib_test_assert(audio_midi_short_pack(bytes, 1, &packed) == LIB_STATUS_INVALID_ARGUMENT);
    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 0xf0;
    bytes[LIB_AUDIO_MIDI_SYSEX_MAX_BYTES - 1] = 0xf7;
    lib_test_assert(audio_midi_sysex_validate(bytes, LIB_AUDIO_MIDI_SYSEX_MAX_BYTES) == LIB_STATUS_OK);
    lib_test_assert(audio_midi_sysex_validate(bytes, sizeof(bytes)) == LIB_STATUS_UNSUPPORTED);
    bytes[1] = 0xf7;
    lib_test_assert(audio_midi_sysex_validate(bytes, 2) == LIB_STATUS_OK);
    lib_test_assert(audio_midi_sysex_validate(bytes, LIB_AUDIO_MIDI_SYSEX_MAX_BYTES) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_midi_sysex_validate(bytes, 1) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_midi_sysex_validate(LIB_NULL, 2) == LIB_STATUS_INVALID_ARGUMENT);
    return 0;
}
