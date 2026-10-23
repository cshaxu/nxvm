#include "lib/types/test.h"
#include "app-myraidenii/machine/audio_interface.h"

static lib_i16 source[3u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS] = {
    1, -1, 2, -2, 3, -3
};
static lib_u32 source_read;
static lib_u32 source_count;
static lib_u64 source_dropped;
static lib_u32 accepted_script[4u];
static lib_u32 accepted_index;
static lib_i16 submitted[3u * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS];
static lib_u32 submitted_count;
static lib_status clear_status = LIB_STATUS_OK;

lib_status myraidenii_core_board_pcm_copy(myraidenii_core_board *board,
    lib_i16 *out_samples, lib_u32 capacity, lib_u32 *out_frames, lib_u64 *out_dropped)
{
    lib_u32 frames;

    if (board == LIB_NULL || out_samples == LIB_NULL || out_frames == LIB_NULL ||
        out_dropped == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    frames = source_count - source_read;
    if (frames > capacity) frames = capacity;
    for (lib_u32 index = 0u; index < frames; ++index) {
        for (lib_u32 channel = 0u; channel < MYRAIDENII_CORE_AUDIO_PCM_CHANNELS; ++channel)
            out_samples[index * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS + channel] =
                source_read + index < 3u ?
                    source[(source_read + index) * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS + channel] : 0;
    }
    source_read += frames;
    *out_frames = frames;
    *out_dropped = source_dropped;
    source_dropped = 0u;
    return LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_enqueue(lib_audio_pcm_stream *stream,
    const lib_i16 *samples, lib_u32 frames, lib_u32 *out_accepted)
{
    lib_u32 accepted;

    if (stream == LIB_NULL || samples == LIB_NULL || out_accepted == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    accepted = accepted_script[accepted_index++];
    if (accepted > frames) accepted = frames;
    lib_memory_copy(submitted + submitted_count * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS,
        samples, (lib_size)accepted * MYRAIDENII_CORE_AUDIO_PCM_CHANNELS * sizeof(*samples));
    submitted_count += accepted;
    *out_accepted = accepted;
    return accepted == frames ? LIB_STATUS_OK : LIB_STATUS_LIMIT_EXCEEDED;
}

lib_status lib_audio_pcm_stream_clear(lib_audio_pcm_stream *stream)
{
    return stream == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : clear_status;
}

static void reset_fixture(void)
{
    source_read = 0u;
    source_count = 3u;
    source_dropped = 0u;
    accepted_index = 0u;
    submitted_count = 0u;
    clear_status = LIB_STATUS_OK;
    lib_memory_set(accepted_script, 0u, sizeof(accepted_script));
    lib_memory_set(submitted, 0u, sizeof(submitted));
}

static lib_i32 audio_retains_partial_prefix_suffix(void)
{
    myraidenii_machine_audio *audio = LIB_NULL;
    const myraidenii_core_board *board = (const myraidenii_core_board *)(lib_uptr)1u;
    const lib_audio_pcm_stream *stream = (const lib_audio_pcm_stream *)(lib_uptr)1u;
    lib_i32 failed = 1;

    reset_fixture();
    accepted_script[0u] = 1u;
    accepted_script[1u] = 2u;
    if (myraidenii_machine_audio_create((myraidenii_core_board *)board,
            (lib_audio_pcm_stream *)stream, &audio) != LIB_STATUS_OK ||
        myraidenii_machine_audio_drain(audio) != LIB_STATUS_LIMIT_EXCEEDED)
        goto done;
    if (source_read != 3u || submitted_count != 1u || submitted[0u] != 1 ||
        submitted[1u] != -1 || myraidenii_machine_audio_drain(audio) != LIB_STATUS_OK ||
        submitted_count != 3u || lib_memory_compare(submitted, source, sizeof(source)) != 0)
        goto done;
    failed = 0;
done:
    myraidenii_machine_audio_destroy(audio);
    return failed;
}

static lib_i32 audio_retains_staging_when_clear_fails(void)
{
    myraidenii_machine_audio *audio = LIB_NULL;
    const myraidenii_core_board *board = (const myraidenii_core_board *)(lib_uptr)1u;
    const lib_audio_pcm_stream *stream = (const lib_audio_pcm_stream *)(lib_uptr)1u;
    lib_i32 failed = 1;

    reset_fixture();
    accepted_script[0u] = 0u;
    accepted_script[1u] = 3u;
    if (myraidenii_machine_audio_create((myraidenii_core_board *)board,
            (lib_audio_pcm_stream *)stream, &audio) != LIB_STATUS_OK ||
        myraidenii_machine_audio_drain(audio) != LIB_STATUS_LIMIT_EXCEEDED)
        goto done;
    clear_status = LIB_STATUS_IO_ERROR;
    if (myraidenii_machine_audio_clear(audio) != LIB_STATUS_IO_ERROR ||
        myraidenii_machine_audio_drain(audio) != LIB_STATUS_OK || submitted_count != 3u ||
        lib_memory_compare(submitted, source, sizeof(source)) != 0)
        goto done;
    failed = 0;
done:
    myraidenii_machine_audio_destroy(audio);
    return failed;
}

static lib_i32 audio_discards_without_host_stream(void)
{
    myraidenii_machine_audio *audio = LIB_NULL;
    const myraidenii_core_board *board = (const myraidenii_core_board *)(lib_uptr)1u;
    lib_i32 failed = 1;

    reset_fixture();
    source_dropped = 1u;
    if (myraidenii_machine_audio_create((myraidenii_core_board *)board, LIB_NULL,
            &audio) != LIB_STATUS_OK || myraidenii_machine_audio_drain(audio) != LIB_STATUS_OK ||
        source_read != 3u || submitted_count != 0u ||
        myraidenii_machine_audio_clear(audio) != LIB_STATUS_OK)
        goto done;
    failed = 0;
done:
    myraidenii_machine_audio_destroy(audio);
    return failed;
}

static lib_i32 audio_reports_irrecoverable_board_loss(void)
{
    myraidenii_machine_audio *audio = LIB_NULL;
    lib_i32 failed = 1;

    reset_fixture();
    source_dropped = 1u;
    if (myraidenii_machine_audio_create((myraidenii_core_board *)(lib_uptr)1u,
            (lib_audio_pcm_stream *)(lib_uptr)1u, &audio) != LIB_STATUS_OK ||
        myraidenii_machine_audio_drain(audio) != LIB_STATUS_INTERNAL_ERROR ||
        submitted_count != 0u) goto done;
    failed = 0;
done:
    myraidenii_machine_audio_destroy(audio);
    return failed;
}

static lib_i32 audio_clear_discards_board_pcm(void)
{
    myraidenii_machine_audio *audio = LIB_NULL;
    const myraidenii_core_board *board = (const myraidenii_core_board *)(lib_uptr)1u;
    const lib_audio_pcm_stream *stream = (const lib_audio_pcm_stream *)(lib_uptr)1u;
    lib_i32 failed = 1;

    reset_fixture();
    source_count = LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION + 1u;
    if (myraidenii_machine_audio_create((myraidenii_core_board *)board,
            (lib_audio_pcm_stream *)stream, &audio) != LIB_STATUS_OK)
        goto done;
    clear_status = LIB_STATUS_IO_ERROR;
    if (myraidenii_machine_audio_clear(audio) != LIB_STATUS_IO_ERROR || source_read != 0u)
        goto done;
    clear_status = LIB_STATUS_OK;
    if (myraidenii_machine_audio_clear(audio) != LIB_STATUS_OK || source_read != source_count ||
        myraidenii_machine_audio_drain(audio) != LIB_STATUS_OK || submitted_count != 0u)
        goto done;
    failed = 0;
done:
    myraidenii_machine_audio_destroy(audio);
    return failed;
}

int main(void)
{
    lib_i32 result;

    if (myraidenii_machine_audio_create(LIB_NULL, LIB_NULL, LIB_NULL) !=
        LIB_STATUS_INVALID_ARGUMENT) return 1;
    result = audio_retains_partial_prefix_suffix();
    if (result != 0) return result;
    if (audio_retains_staging_when_clear_fails() != 0) return 1;
    return audio_reports_irrecoverable_board_loss() != 0 ||
        audio_discards_without_host_stream() != 0 ||
        audio_clear_discards_board_pcm() != 0 ? 1 : 0;
}
