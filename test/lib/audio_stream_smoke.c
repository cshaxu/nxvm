#include "lib/audio/stream.h"

#include <assert.h>

struct audio_stream_platform {
    lib_i16 copied[AUDIO_STREAM_PLAY_BATCH * 2u];
    lib_u32 queued;
    lib_u32 writable;
    base_sync_event *enqueued;
};

static struct audio_stream_platform fake_platform;
static lib_status create_status = LIB_STATUS_OK;
static lib_status enqueue_status = LIB_STATUS_OK;
static lib_status wait_status = LIB_STATUS_OK;
static lib_status cancel_status = LIB_STATUS_OK;
static lib_status clear_status = LIB_STATUS_OK;
static lib_status destroy_status = LIB_STATUS_OK;
static lib_u32 accepted_limit = AUDIO_STREAM_PLAY_BATCH;
static unsigned create_calls, enqueue_calls, clear_calls;
static unsigned wait_calls, cancel_calls;
static lib_u32 last_enqueued_frames;
static base_sync_event *native_wait_entered;
static base_sync_event *native_wait_release;
static lib_bool native_wait_blocks;

lib_status audio_stream_platform_create(const lib_audio_stream_options *options,
    audio_stream_platform **out_platform)
{
    ++create_calls;
    assert(options != LIB_NULL && out_platform != LIB_NULL);
    *out_platform = create_status == LIB_STATUS_OK ? &fake_platform : LIB_NULL;
    return create_status;
}

lib_status audio_stream_platform_enqueue(audio_stream_platform *platform,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames)
{
    lib_u32 accepted = frame_count < accepted_limit ? frame_count : accepted_limit;
    ++enqueue_calls;
    last_enqueued_frames = accepted;
    assert(platform == &fake_platform);
    lib_memory_copy(platform->copied, samples,
        (lib_size)accepted * 2u * sizeof(*samples));
    if (platform->enqueued != LIB_NULL)
        (void)base_sync_event_signal(platform->enqueued);
    *out_accepted_frames = accepted;
    return accepted == 0u && frame_count != 0u ? LIB_STATUS_LIMIT_EXCEEDED : enqueue_status;
}

lib_status audio_stream_platform_wait_writable(audio_stream_platform *platform)
{
    ++wait_calls;
    assert(platform == &fake_platform);
    if (native_wait_blocks != LIB_FALSE) {
        assert(base_sync_event_signal(native_wait_entered) == LIB_STATUS_OK);
        assert(base_sync_event_wait(native_wait_release, LIB_UINT32_MAX) ==
            BASE_SYNC_WAIT_SIGNALED);
    }
    return wait_status;
}

lib_status audio_stream_platform_cancel_wait(audio_stream_platform *platform)
{
    ++cancel_calls;
    assert(platform == &fake_platform);
    if (native_wait_release != LIB_NULL)
        assert(base_sync_event_signal(native_wait_release) == LIB_STATUS_OK);
    return cancel_status;
}

lib_status audio_stream_platform_clear(audio_stream_platform *platform)
{
    ++clear_calls;
    assert(platform == &fake_platform);
    platform->queued = 0u;
    return clear_status;
}

lib_status audio_stream_platform_destroy(audio_stream_platform **platform)
{
    assert(platform != LIB_NULL && *platform == &fake_platform);
    if (destroy_status == LIB_STATUS_OK) *platform = LIB_NULL;
    return destroy_status;
}

#include "lib/audio/stream.c"

int main(void)
{
    lib_audio_stream_options options = { 48000u, 2u };
    lib_audio_stream *stream = LIB_NULL;
    base_sync_event *enqueued = LIB_NULL;
    base_sync_event *native_wait_entered_event = LIB_NULL;
    base_sync_event *native_wait_release_event = LIB_NULL;
    lib_i16 samples[6] = { -1, 1, -2, 2, -3, 3 };
    lib_i16 submission[LIB_AUDIO_STREAM_MAX_FRAMES_PER_SUBMISSION * 2u] = { 0 };
    lib_u32 accepted = 99u, queued = 99u, writable = 99u;
    unsigned before_destroy_enqueue_calls;

    assert(lib_audio_stream_create(LIB_NULL, &stream) == LIB_STATUS_INVALID_ARGUMENT);
    options.sample_rate = 8000u;
    assert(lib_audio_stream_create(&options, &stream) == LIB_STATUS_UNSUPPORTED);
    options.sample_rate = 48000u;
    options.channel_count = 3u;
    assert(lib_audio_stream_create(&options, &stream) == LIB_STATUS_UNSUPPORTED);
    options.channel_count = 2u;
    create_status = LIB_STATUS_IO_ERROR;
    assert(lib_audio_stream_create(&options, &stream) == LIB_STATUS_IO_ERROR && stream == LIB_NULL);
    create_status = LIB_STATUS_OK;
    assert(lib_audio_stream_create(&options, &stream) == LIB_STATUS_OK);
    assert(create_calls == 2u);

    assert(lib_audio_stream_query(stream, &queued, &writable) == LIB_STATUS_OK);
    assert(queued == 0u && writable == AUDIO_STREAM_QUEUE_CAPACITY);
    assert(lib_audio_stream_wait_writable(stream) == LIB_STATUS_OK && wait_calls == 0u);
    assert(lib_audio_stream_cancel_wait(stream) == LIB_STATUS_OK);
    assert(lib_audio_stream_wait_writable(stream) == LIB_STATUS_INVALID_STATE);
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &enqueued) == LIB_STATUS_OK);
    fake_platform.enqueued = enqueued;
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(base_sync_event_wait(enqueued, LIB_UINT32_MAX) == BASE_SYNC_WAIT_SIGNALED);
    assert(enqueue_calls == 1u && wait_calls == 1u);
    fake_platform.enqueued = LIB_NULL;
    base_sync_event_destroy(enqueued);
    assert(lib_audio_stream_enqueue(stream, LIB_NULL, 0u, &accepted) == LIB_STATUS_OK);
    assert(accepted == 0u && enqueue_calls == 1u);
    assert(lib_audio_stream_enqueue(stream, LIB_NULL, 1u, &accepted) == LIB_STATUS_INVALID_ARGUMENT);
    assert(lib_audio_stream_enqueue(stream, samples, 513u, &accepted) == LIB_STATUS_INVALID_ARGUMENT);
    assert(lib_audio_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
    assert(accepted == 3u && enqueue_calls == 1u);
    assert(lib_audio_stream_query(stream, &queued, &writable) == LIB_STATUS_OK);
    assert(queued == 3u && writable == AUDIO_STREAM_QUEUE_CAPACITY - 3u);
    assert(lib_audio_stream_flush(stream) == LIB_STATUS_OK);
    assert(enqueue_calls == 2u && last_enqueued_frames == 3u);
    assert(lib_audio_stream_query(stream, &queued, &writable) == LIB_STATUS_OK);
    assert(queued == 0u && writable == AUDIO_STREAM_QUEUE_CAPACITY);

    /* Deactivation is a delivery boundary: it flushes a short finite sound
     * before it clears the native backend and makes the producer inactive. */
    assert(lib_audio_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_set_active(stream, LIB_FALSE) == LIB_STATUS_OK);
    assert(enqueue_calls == 3u && last_enqueued_frames == 3u);
    assert(clear_calls == 1u && cancel_calls == 1u);
    assert(lib_audio_stream_enqueue(stream, samples, 1u, &accepted) == LIB_STATUS_INVALID_STATE);
    assert(lib_audio_stream_wait_writable(stream) == LIB_STATUS_INVALID_STATE);
    assert(lib_audio_stream_set_active(stream, LIB_FALSE) == LIB_STATUS_OK);
    assert(clear_calls == 1u);
    assert(lib_audio_stream_set_active(stream, LIB_TRUE) == LIB_STATUS_OK);
    clear_status = LIB_STATUS_OK;

    /* Clear alone intentionally discards a tail. */
    assert(lib_audio_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_clear(stream) == LIB_STATUS_OK);
    assert(enqueue_calls == 3u);
    assert(lib_audio_stream_query(stream, &queued, &writable) == LIB_STATUS_OK);
    assert(queued == 0u && writable == AUDIO_STREAM_QUEUE_CAPACITY);

    /* A sole queued tail is still completed while destruction is safe. */
    assert(lib_audio_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);

    destroy_status = LIB_STATUS_IO_ERROR;
    assert(lib_audio_stream_destroy(&stream) == LIB_STATUS_IO_ERROR && stream != LIB_NULL);
    assert(enqueue_calls == 4u && last_enqueued_frames == 3u);
    destroy_status = LIB_STATUS_OK;
    assert(lib_audio_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
    assert(lib_audio_stream_destroy(&stream) == LIB_STATUS_OK);

    /* Fill the producer FIFO while the native wait is held, then make the
     * write fail. A waiter must receive the terminal failure, never false
     * writable space or a silently discarded batch. */
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_entered_event) == LIB_STATUS_OK);
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_release_event) == LIB_STATUS_OK);
    native_wait_entered = native_wait_entered_event;
    native_wait_release = native_wait_release_event;
    native_wait_blocks = LIB_TRUE;
    wait_status = LIB_STATUS_OK;
    enqueue_status = LIB_STATUS_OK;
    assert(lib_audio_stream_create(&options, &stream) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(base_sync_event_wait(native_wait_entered_event, LIB_UINT32_MAX) ==
        BASE_SYNC_WAIT_SIGNALED);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    enqueue_status = LIB_STATUS_IO_ERROR;
    assert(base_sync_event_signal(native_wait_release_event) == LIB_STATUS_OK);
    assert(lib_audio_stream_wait_writable(stream) == LIB_STATUS_IO_ERROR);
    assert(lib_audio_stream_query(stream, &queued, &writable) == LIB_STATUS_IO_ERROR);
    native_wait_blocks = LIB_FALSE;
    assert(lib_audio_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
    native_wait_entered = LIB_NULL;
    native_wait_release = LIB_NULL;
    base_sync_event_destroy(native_wait_entered_event);
    base_sync_event_destroy(native_wait_release_event);

    /* Destroy must interrupt a worker waiting for the native backend before
     * releasing that backend; it may not submit the retained batch afterwards. */
    enqueue_status = LIB_STATUS_OK;
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_entered_event) == LIB_STATUS_OK);
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_release_event) == LIB_STATUS_OK);
    native_wait_entered = native_wait_entered_event;
    native_wait_release = native_wait_release_event;
    native_wait_blocks = LIB_TRUE;
    assert(lib_audio_stream_create(&options, &stream) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    assert(base_sync_event_wait(native_wait_entered_event, LIB_UINT32_MAX) ==
        BASE_SYNC_WAIT_SIGNALED);
    before_destroy_enqueue_calls = enqueue_calls;
    assert(lib_audio_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
    assert(enqueue_calls == before_destroy_enqueue_calls);
    native_wait_blocks = LIB_FALSE;
    native_wait_entered = LIB_NULL;
    native_wait_release = LIB_NULL;
    base_sync_event_destroy(native_wait_entered_event);
    base_sync_event_destroy(native_wait_release_event);
    return 0;
}
