#include "lib/audio/midi/output.h"

lib_status audio_midi_short_pack(const lib_u8 *bytes, lib_u32 count,
    lib_u32 *out_packed)
{
    lib_u32 expected, packed;
    if (!bytes || !count || !out_packed) return LIB_STATUS_INVALID_ARGUMENT;
    if (bytes[0] >= 0x80 && bytes[0] < 0xf0)
        expected = (bytes[0] & 0xe0) == 0xc0 ? 2 : 3;
    else {
        switch (bytes[0]) {
        case 0xf1: case 0xf3: expected = 2; break;
        case 0xf2: expected = 3; break;
        case 0xf6: case 0xf8: case 0xfa: case 0xfb: case 0xfc:
        case 0xfe: case 0xff: expected = 1; break;
        default: return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    if (count != expected) return LIB_STATUS_INVALID_ARGUMENT;
    packed = bytes[0];
    for (lib_u32 i = 1; i < count; ++i) {
        if (bytes[i] >= 0x80) return LIB_STATUS_INVALID_ARGUMENT;
        packed |= (lib_u32)bytes[i] << (8 * i);
    }
    *out_packed = packed;
    return LIB_STATUS_OK;
}

lib_status audio_midi_sysex_validate(const lib_u8 *bytes, lib_u32 count)
{
    if (!bytes || count < 2) return LIB_STATUS_INVALID_ARGUMENT;
    if (count > LIB_AUDIO_MIDI_SYSEX_MAX_BYTES) return LIB_STATUS_UNSUPPORTED;
    if (bytes[0] != 0xf0 || bytes[count - 1] != 0xf7)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (lib_u32 i = 1; i + 1 < count; ++i)
        if (bytes[i] >= 0x80) return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_OK;
}
