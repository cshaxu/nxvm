#ifndef LIB_AUDIO_MIDI_OUTPUT_H
#define LIB_AUDIO_MIDI_OUTPUT_H

#include "lib/audio/midi/output_interface.h"

lib_status audio_midi_short_pack(const lib_u8 *bytes, lib_u32 count,
    lib_u32 *out_packed);
lib_status audio_midi_sysex_validate(const lib_u8 *bytes, lib_u32 count);

#endif
