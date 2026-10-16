#ifndef LIB_AUDIO_FM_OUTPUT_INTERFACE_H
#define LIB_AUDIO_FM_OUTPUT_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct lib_audio_fm_output lib_audio_fm_output;

/* Acquire a native raw FM register-output capability, not a MIDI instrument
 * or software synthesizer. No current backend supplies this facility, so a
 * valid request returns UNSUPPORTED and null. No register transport is exposed
 * until there is a genuine backend and an admitted transport contract. */
lib_status lib_audio_fm_output_create(lib_audio_fm_output **out_output);

#endif
