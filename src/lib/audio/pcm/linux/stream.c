#include "lib/audio/pcm/stream.h"

struct audio_pcm_stream_platform { lib_u32 unused; };

lib_status audio_pcm_stream_platform_create(const lib_audio_pcm_stream_options *options,
    audio_pcm_stream_platform **out_platform)
{
    if (options == LIB_NULL || out_platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_platform = LIB_NULL;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status audio_pcm_stream_platform_worker_attach(audio_pcm_stream_platform *platform)
{ return platform == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_UNSUPPORTED; }

void audio_pcm_stream_platform_worker_detach(audio_pcm_stream_platform *platform)
{ (void)platform; }

lib_status audio_pcm_stream_platform_enqueue(audio_pcm_stream_platform *platform,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames)
{
    (void)platform;
    (void)samples;
    (void)frame_count;
    if (out_accepted_frames == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_accepted_frames = 0u;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status audio_pcm_stream_platform_wait_writable(audio_pcm_stream_platform *platform)
{
    return platform == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_UNSUPPORTED;
}

lib_status audio_pcm_stream_platform_cancel_wait(audio_pcm_stream_platform *platform)
{ return platform == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_UNSUPPORTED; }

lib_status audio_pcm_stream_platform_wait_progress(audio_pcm_stream_platform *platform)
{ return platform == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_UNSUPPORTED; }

lib_status audio_pcm_stream_platform_progress(audio_pcm_stream_platform *platform,
    lib_u64 *out_submitted, lib_u64 *out_consumed)
{
    if (out_submitted == LIB_NULL || out_consumed == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_submitted = *out_consumed = 0u;
    return platform == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_UNSUPPORTED;
}

lib_status audio_pcm_stream_platform_set_gain(audio_pcm_stream_platform *platform,
    const lib_u16 *gains, lib_u32 channel_count)
{
    if (platform == LIB_NULL || gains == LIB_NULL || channel_count < 1u || channel_count > 2u)
        return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status audio_pcm_stream_platform_clear(audio_pcm_stream_platform *platform)
{ return platform == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_UNSUPPORTED; }

lib_status audio_pcm_stream_platform_destroy(audio_pcm_stream_platform **platform)
{
    if (platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *platform = LIB_NULL;
    return LIB_STATUS_OK;
}
