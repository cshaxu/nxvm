#include "lib/audio/stream.h"
#include "lib/types/win32/audio.h"
#include "lib/types/win32/sync.h"

enum { AUDIO_STREAM_SLOT_COUNT = 4u };

typedef struct audio_stream_slot {
    lib_win32_wave_header header;
    lib_i16 samples[AUDIO_STREAM_PLAY_BATCH * 2u];
    lib_u32 frame_count;
    lib_bool prepared;
    lib_bool submitted;
} audio_stream_slot;

struct audio_stream_platform {
    lib_win32_wave_output output;
    lib_win32_handle completion;
    lib_win32_handle interruption;
    lib_u32 channel_count;
    audio_stream_slot slots[AUDIO_STREAM_SLOT_COUNT];
};

/* waveOut invokes this on an audio-driver thread.  SetEvent is intentionally
 * the only work here; header reclamation remains serialized in caller context. */
static void LIB_WIN32_WINAPI audio_stream_completion_callback(
    lib_win32_wave_output output, lib_win32_uint message,
    lib_win32_dword_ptr instance, lib_win32_dword_ptr parameter_one,
    lib_win32_dword_ptr parameter_two)
{
    (void)output;
    (void)parameter_one;
    (void)parameter_two;
    if (message == LIB_WIN32_WOM_DONE && instance != 0u)
        (void)lib_win32_set_event((lib_win32_handle)(lib_iptr)instance);
}

static lib_status audio_stream_win32_status(lib_win32_mmresult result)
{ return result == LIB_WIN32_MMSYSERR_NOERROR ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }

static void audio_stream_reclaim(audio_stream_platform *platform)
{
    lib_u32 index;

    for (index = 0u; index < AUDIO_STREAM_SLOT_COUNT; ++index) {
        audio_stream_slot *slot = &platform->slots[index];
        if (slot->submitted != LIB_FALSE &&
            (slot->header.dwFlags & LIB_WIN32_WAVE_HEADER_DONE) != 0u) {
            slot->submitted = LIB_FALSE;
            slot->frame_count = 0u;
        }
    }
}

static lib_status audio_stream_prepare(audio_stream_platform *platform)
{
    lib_u32 index;

    for (index = 0u; index < AUDIO_STREAM_SLOT_COUNT; ++index) {
        audio_stream_slot *slot = &platform->slots[index];
        lib_status status;

        slot->header.lpData = (char *)slot->samples;
        slot->header.dwBufferLength = (lib_win32_dword)(
            AUDIO_STREAM_PLAY_BATCH * platform->channel_count *
            sizeof(*slot->samples));
        status = audio_stream_win32_status(lib_win32_wave_out_prepare_header(
            platform->output, &slot->header, (lib_win32_uint)sizeof(slot->header)));
        if (status != LIB_STATUS_OK) return status;
        slot->prepared = LIB_TRUE;
    }
    return LIB_STATUS_OK;
}

lib_status audio_stream_platform_create(const lib_audio_stream_options *options,
    audio_stream_platform **out_platform)
{
    audio_stream_platform *platform;
    lib_win32_wave_format format;
    lib_status status;

    if (options == LIB_NULL || out_platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_platform = LIB_NULL;
    platform = lib_allocate_zero(1u, sizeof(*platform));
    if (platform == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    platform->channel_count = options->channel_count;
    lib_memory_set(&format, 0, sizeof(format));
    format.wFormatTag = LIB_WIN32_WAVE_FORMAT_PCM;
    format.nChannels = (lib_win32_word)options->channel_count;
    format.nSamplesPerSec = options->sample_rate;
    format.wBitsPerSample = (lib_win32_word)(sizeof(lib_i16) * 8u);
    format.nBlockAlign = (lib_win32_word)(options->channel_count * sizeof(lib_i16));
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    platform->completion = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_FALSE,
        LIB_WIN32_FALSE, LIB_NULL);
    if (platform->completion == LIB_NULL) {
        lib_release(platform);
        return LIB_STATUS_IO_ERROR;
    }
    platform->interruption = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_FALSE,
        LIB_WIN32_FALSE, LIB_NULL);
    if (platform->interruption == LIB_NULL) {
        (void)lib_win32_close_handle(platform->completion);
        lib_release(platform);
        return LIB_STATUS_IO_ERROR;
    }
    status = audio_stream_win32_status(lib_win32_wave_out_open(&platform->output,
        LIB_WIN32_WAVE_MAPPER, &format,
        (lib_win32_dword_ptr)(lib_iptr)(lib_win32_wave_callback)
            audio_stream_completion_callback,
        (lib_win32_dword_ptr)(lib_iptr)platform->completion,
        LIB_WIN32_CALLBACK_FUNCTION));
    if (status != LIB_STATUS_OK) {
        (void)lib_win32_close_handle(platform->interruption);
        (void)lib_win32_close_handle(platform->completion);
        lib_release(platform);
        return status;
    }
    status = audio_stream_prepare(platform);
    if (status != LIB_STATUS_OK) {
        (void)audio_stream_platform_destroy(&platform);
        return status;
    }
    *out_platform = platform;
    return LIB_STATUS_OK;
}

lib_status audio_stream_platform_wait_writable(audio_stream_platform *platform)
{
    lib_win32_handle events[2];
    lib_u32 index;
    lib_win32_dword result;

    if (platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    events[0] = platform->completion;
    events[1] = platform->interruption;
    for (;;) {
        audio_stream_reclaim(platform);
        for (index = 0u; index < AUDIO_STREAM_SLOT_COUNT; ++index)
            if (platform->slots[index].submitted == LIB_FALSE) return LIB_STATUS_OK;
        result = lib_win32_wait_for_multiple_objects(2u, events, LIB_WIN32_FALSE,
            LIB_WIN32_INFINITE);
        if (result == LIB_WIN32_WAIT_OBJECT_0 + 1u) return LIB_STATUS_LIMIT_EXCEEDED;
        if (result != LIB_WIN32_WAIT_OBJECT_0) return LIB_STATUS_IO_ERROR;
    }
}

lib_status audio_stream_platform_cancel_wait(audio_stream_platform *platform)
{
    if (platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return lib_win32_set_event(platform->interruption) == LIB_WIN32_FALSE ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

lib_status audio_stream_platform_enqueue(audio_stream_platform *platform,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames)
{
    lib_u32 index;

    if (platform == LIB_NULL || samples == LIB_NULL || out_accepted_frames == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_accepted_frames = 0u;
    audio_stream_reclaim(platform);
    for (index = 0u; index < AUDIO_STREAM_SLOT_COUNT; ++index) {
        audio_stream_slot *slot = &platform->slots[index];
        lib_status status;

        if (slot->submitted != LIB_FALSE) continue;
        lib_memory_copy(slot->samples, samples,
            (lib_size)frame_count * platform->channel_count * sizeof(*samples));
        slot->header.dwBufferLength = (lib_win32_dword)(frame_count *
            platform->channel_count * sizeof(*samples));
        status = audio_stream_win32_status(lib_win32_wave_out_write(platform->output,
            &slot->header, (lib_win32_uint)sizeof(slot->header)));
        if (status != LIB_STATUS_OK) return status;
        slot->frame_count = frame_count;
        slot->submitted = LIB_TRUE;
        *out_accepted_frames = frame_count;
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_LIMIT_EXCEEDED;
}

lib_status audio_stream_platform_clear(audio_stream_platform *platform)
{
    lib_u32 index;
    lib_status status;

    if (platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = audio_stream_win32_status(lib_win32_wave_out_reset(platform->output));
    if (status != LIB_STATUS_OK) return status;
    for (index = 0u; index < AUDIO_STREAM_SLOT_COUNT; ++index) {
        platform->slots[index].submitted = LIB_FALSE;
        platform->slots[index].frame_count = 0u;
    }
    (void)lib_win32_reset_event(platform->completion);
    return LIB_STATUS_OK;
}

lib_status audio_stream_platform_destroy(audio_stream_platform **platform)
{
    lib_u32 index;
    lib_status status;

    if (platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (*platform == LIB_NULL) return LIB_STATUS_OK;
    status = audio_stream_platform_clear(*platform);
    if (status != LIB_STATUS_OK) return status;
    for (index = 0u; index < AUDIO_STREAM_SLOT_COUNT; ++index) {
        audio_stream_slot *slot = &(*platform)->slots[index];
        if (slot->prepared == LIB_FALSE) continue;
        status = audio_stream_win32_status(lib_win32_wave_out_unprepare_header(
            (*platform)->output, &slot->header, (lib_win32_uint)sizeof(slot->header)));
        if (status != LIB_STATUS_OK) return status;
        slot->prepared = LIB_FALSE;
    }
    status = audio_stream_win32_status(lib_win32_wave_out_close((*platform)->output));
    if (status != LIB_STATUS_OK) return status;
    (*platform)->output = LIB_NULL;
    if (lib_win32_close_handle((*platform)->interruption) == LIB_WIN32_FALSE)
        return LIB_STATUS_IO_ERROR;
    (*platform)->interruption = LIB_NULL;
    if (lib_win32_close_handle((*platform)->completion) == LIB_WIN32_FALSE)
        return LIB_STATUS_IO_ERROR;
    (*platform)->completion = LIB_NULL;
    lib_release(*platform);
    *platform = LIB_NULL;
    return LIB_STATUS_OK;
}
