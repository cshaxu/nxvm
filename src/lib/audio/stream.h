#ifndef LIB_AUDIO_STREAM_H
#define LIB_AUDIO_STREAM_H

#include "lib/audio/stream_interface.h"
#include "lib/base/sync_interface.h"

typedef struct audio_stream_platform audio_stream_platform;

enum {
    AUDIO_STREAM_PLAY_BATCH = 1024u,
    AUDIO_STREAM_QUEUE_CAPACITY = 4096u
};

typedef enum audio_stream_control {
    AUDIO_STREAM_CONTROL_NONE,
    AUDIO_STREAM_CONTROL_CLEAR,
    AUDIO_STREAM_CONTROL_FLUSH
} audio_stream_control;

struct lib_audio_stream {
    audio_stream_platform *platform;
    base_sync_mutex *lock;
    base_sync_event *wake;
    base_sync_event *space;
    base_sync_event *control_done;
    base_sync_task *worker;
    lib_i16 samples[AUDIO_STREAM_QUEUE_CAPACITY * 2u];
    lib_u32 head;
    lib_u32 count;
    lib_u32 generation;
    lib_u32 channel_count;
    audio_stream_control control;
    lib_atomic_i32 wait_cancelled;
    lib_bool active;
    lib_status failure;
};

lib_status audio_stream_platform_create(const lib_audio_stream_options *options,
    audio_stream_platform **out_platform);
lib_status audio_stream_platform_enqueue(audio_stream_platform *platform,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames);
lib_status audio_stream_platform_wait_writable(audio_stream_platform *platform);
lib_status audio_stream_platform_cancel_wait(audio_stream_platform *platform);
lib_status audio_stream_platform_clear(audio_stream_platform *platform);
lib_status audio_stream_platform_destroy(audio_stream_platform **platform);

#endif
