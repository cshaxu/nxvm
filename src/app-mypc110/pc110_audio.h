#ifndef MYPC110_AUDIO_H
#define MYPC110_AUDIO_H

#include "app-mypc110/pc110_es488.h"
#include "lib/audio/pcm/stream_interface.h"

typedef struct mypc110_audio {
    lib_audio_pcm_stream *stream;
    mypc110_es488 *device;
    lib_i16 staging[LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION];
    lib_u32 read;
    lib_u32 count;
    lib_u32 repeats;
    lib_u32 phase;
    lib_i16 sample;
    lib_u64 reset_epoch;
    lib_status failure;
    lib_atomic_i32 interrupt_failure;
} mypc110_audio;

lib_status mypc110_audio_open(mypc110_audio *audio, mypc110_es488 *device);
lib_status mypc110_audio_pump(mypc110_audio *audio);
lib_status mypc110_audio_wait(mypc110_audio *audio);
lib_status mypc110_audio_active(mypc110_audio *audio, lib_bool enabled);
void mypc110_audio_interrupt(mypc110_audio *audio);
lib_status mypc110_audio_close(mypc110_audio *audio);

#endif
