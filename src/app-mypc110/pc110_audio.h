#ifndef MYPC110_AUDIO_H
#define MYPC110_AUDIO_H

#include "app-mypc110/pc110_es488.h"
#include "app-mypc110/pc110_opl2.h"
#include "lib/audio/pcm/stream_interface.h"

#define MYPC110_AUDIO_FRAME_CAPACITY 4096u

typedef struct mypc110_audio {
    lib_audio_pcm_stream *stream;
    mypc110_es488 *device;
    mypc110_opl2 *opl2;
    lib_i16 frames[MYPC110_AUDIO_FRAME_CAPACITY];
    lib_u32 read;
    lib_u32 count;
    lib_u64 next_tick;
    lib_u64 pcm_until;
    lib_u64 reset_epoch;
    lib_u32 tick_phase;
    lib_i16 pcm_sample;
    lib_bool clock_valid;
    lib_bool clear_pending;
    lib_status failure;
    lib_atomic_i32 interrupt_failure;
} mypc110_audio;

lib_status mypc110_audio_open(mypc110_audio *audio, mypc110_es488 *device,
    mypc110_opl2 *opl2);
lib_status mypc110_audio_advance(mypc110_audio *audio, lib_u64 tick);
lib_bool mypc110_audio_needs_deadline(const mypc110_audio *audio, lib_u64 tick);
lib_status mypc110_audio_pump(mypc110_audio *audio, lib_u64 tick);
lib_status mypc110_audio_wait(mypc110_audio *audio);
lib_status mypc110_audio_active(mypc110_audio *audio, lib_bool enabled);
void mypc110_audio_interrupt(mypc110_audio *audio);
lib_status mypc110_audio_close(mypc110_audio *audio);

#endif
