#include "myraidenii/machine/audio_interface.h"

enum {
    MYRAIDENII_MACHINE_AUDIO_STAGING_FRAME_CAPACITY =
        2u * LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION
};

struct myraidenii_machine_audio {
    myraidenii_core_board *board;
    lib_audio_pcm_stream *stream;
    lib_i16 staging[MYRAIDENII_MACHINE_AUDIO_STAGING_FRAME_CAPACITY *
        MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    lib_u32 read;
    lib_u32 count;
};

static void myraidenii_machine_audio_pop(myraidenii_machine_audio *audio,
    lib_u32 frames)
{
    audio->read = (audio->read + frames) % MYRAIDENII_MACHINE_AUDIO_STAGING_FRAME_CAPACITY;
    audio->count -= frames;
}

static lib_status myraidenii_machine_audio_copy_board(myraidenii_machine_audio *audio)
{
    lib_i16 copied[LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION *
        MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
    lib_u32 capacity = MYRAIDENII_MACHINE_AUDIO_STAGING_FRAME_CAPACITY - audio->count;
    lib_u32 frames = 0u;
    lib_u64 dropped = 0u;
    lib_u32 first;
    lib_status status;

    if (capacity == 0u) return LIB_STATUS_OK;
    if (capacity > LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION)
        capacity = LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION;
    status = myraidenii_core_board_pcm_copy(audio->board, copied, capacity, &frames, &dropped);
    if (status != LIB_STATUS_OK) return status;
    if (audio->stream == LIB_NULL) return LIB_STATUS_OK;
    if (dropped != 0u) return LIB_STATUS_LIMIT_EXCEEDED;
    if (frames == 0u) return LIB_STATUS_OK;
    if (frames > capacity) return LIB_STATUS_INTERNAL_ERROR;
    first = MYRAIDENII_MACHINE_AUDIO_STAGING_FRAME_CAPACITY -
        ((audio->read + audio->count) % MYRAIDENII_MACHINE_AUDIO_STAGING_FRAME_CAPACITY);
    if (first > frames) first = frames;
    lib_memory_copy(audio->staging + ((audio->read + audio->count) %
        MYRAIDENII_MACHINE_AUDIO_STAGING_FRAME_CAPACITY) * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS,
        copied, (lib_size)first * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS * sizeof(*copied));
    if (first != frames) lib_memory_copy(audio->staging,
        copied + first * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS,
        (lib_size)(frames - first) * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS * sizeof(*copied));
    audio->count += frames;
    return LIB_STATUS_OK;
}

static lib_status myraidenii_machine_audio_submit(myraidenii_machine_audio *audio)
{
    lib_u32 frames;
    lib_u32 accepted;
    lib_status status;

    if (audio->stream == LIB_NULL) return audio->count == 0u ? LIB_STATUS_OK :
        LIB_STATUS_INTERNAL_ERROR;
    while (audio->count != 0u) {
        frames = MYRAIDENII_MACHINE_AUDIO_STAGING_FRAME_CAPACITY - audio->read;
        if (frames > audio->count) frames = audio->count;
        if (frames > LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION)
            frames = LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION;
        accepted = 0u;
        status = lib_audio_pcm_stream_enqueue(audio->stream,
            audio->staging + audio->read * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS,
            frames, &accepted);
        if (accepted > frames) return LIB_STATUS_INTERNAL_ERROR;
        if (accepted != 0u) myraidenii_machine_audio_pop(audio, accepted);
        if (status != LIB_STATUS_OK || accepted != frames) return status == LIB_STATUS_OK ?
            LIB_STATUS_LIMIT_EXCEEDED : status;
    }
    return LIB_STATUS_OK;
}

lib_status myraidenii_machine_audio_create(myraidenii_core_board *board,
    lib_audio_pcm_stream *stream, myraidenii_machine_audio **out_audio)
{
    myraidenii_machine_audio *audio;

    if (out_audio == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_audio = LIB_NULL;
    if (board == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    audio = lib_allocate_zero(1u, sizeof(*audio));
    if (audio == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    audio->board = board;
    audio->stream = stream;
    *out_audio = audio;
    return LIB_STATUS_OK;
}

lib_status myraidenii_machine_audio_drain(myraidenii_machine_audio *audio)
{
    lib_status status;

    if (audio == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = myraidenii_machine_audio_submit(audio);
    if (status != LIB_STATUS_OK) return status;
    status = myraidenii_machine_audio_copy_board(audio);
    return status == LIB_STATUS_OK ? myraidenii_machine_audio_submit(audio) : status;
}

lib_status myraidenii_machine_audio_clear(myraidenii_machine_audio *audio)
{
    lib_status status;

    if (audio == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = audio->stream == LIB_NULL ? LIB_STATUS_OK :
        lib_audio_pcm_stream_clear(audio->stream);
    if (status == LIB_STATUS_OK) {
        audio->read = 0u;
        audio->count = 0u;
    }
    return status;
}

void myraidenii_machine_audio_destroy(myraidenii_machine_audio *audio)
{
    lib_release(audio);
}
