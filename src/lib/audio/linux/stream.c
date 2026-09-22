#include "lib/audio/stream.h"

struct audio_stream_platform { lib_u32 unused; };

lib_status audio_stream_platform_create(const lib_audio_stream_options *options,
    audio_stream_platform **out_platform)
{
    if (options == LIB_NULL || out_platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_platform = LIB_NULL;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status audio_stream_platform_enqueue(audio_stream_platform *platform,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames)
{
    (void)platform;
    (void)samples;
    (void)frame_count;
    if (out_accepted_frames == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_accepted_frames = 0u;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status audio_stream_platform_wait_writable(audio_stream_platform *platform)
{
    return platform == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_UNSUPPORTED;
}

lib_status audio_stream_platform_cancel_wait(audio_stream_platform *platform)
{ return platform == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_UNSUPPORTED; }

lib_status audio_stream_platform_clear(audio_stream_platform *platform)
{ return platform == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_UNSUPPORTED; }

lib_status audio_stream_platform_destroy(audio_stream_platform **platform)
{
    if (platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *platform = LIB_NULL;
    return LIB_STATUS_OK;
}
