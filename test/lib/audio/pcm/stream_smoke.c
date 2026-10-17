#include "lib/types/test.h"
#include "lib/audio/pcm/stream.h"


struct audio_pcm_stream_platform {
    lib_i16 copied[AUDIO_PCM_STREAM_PLAY_BATCH * 2u];
    lib_u32 queued;
    lib_u32 writable;
    lib_u64 submitted;
    lib_u64 consumed;
};

static struct audio_pcm_stream_platform fake_platform;
static lib_status create_status = LIB_STATUS_OK;
static lib_status attach_status = LIB_STATUS_OK;
static lib_u32 attach_calls;
static lib_u32 detach_calls;
static lib_status enqueue_status = LIB_STATUS_OK;
static lib_status wait_status = LIB_STATUS_OK;
static lib_status cancel_status = LIB_STATUS_OK;
static lib_status clear_status = LIB_STATUS_OK;
static lib_status gain_status = LIB_STATUS_OK;
static lib_u16 observed_gains[2];
static lib_u32 observed_gain_channels;
static lib_status destroy_status = LIB_STATUS_OK;
static lib_u32 accepted_limit = AUDIO_PCM_STREAM_PLAY_BATCH;
static unsigned create_calls, enqueue_calls, clear_calls;
static unsigned wait_calls, cancel_calls;
static lib_u32 last_enqueued_frames;
static lib_u32 total_enqueued_frames;
static base_sync_event *tail_delivered;
static base_sync_event *native_wait_entered;
static base_sync_event *native_wait_release;
static lib_bool native_wait_blocks;
static lib_bool interrupt_prefix;
static lib_status signal_status = LIB_STATUS_OK;
static base_sync_event *failed_signal;
static lib_bool release_native_on_control_wait;
static lib_i16 delivered_prefix[6];
static lib_u32 delivered_count;
static base_sync_event *idle_wait_entered;
static base_sync_event *worker_wake;
static unsigned idle_wait_calls;
static lib_bool hold_consumption;
static lib_bool finish_consumption;
static base_sync_event *progress_entered;
static base_sync_event *progress_release;
static lib_status notify_status = LIB_STATUS_OK;

static lib_status audio_test_notify(void *context)
{
    return notify_status == LIB_STATUS_OK ?
        base_sync_event_signal(context) : notify_status;
}

static base_sync_wait_result audio_test_wait_any(base_sync_event *const *events,
    lib_u32 count, const base_sync_task *task, lib_u32 timeout, lib_u32 *index)
{
    if (release_native_on_control_wait && events[0] == failed_signal)
        lib_test_assert(base_sync_event_signal(native_wait_release) == LIB_STATUS_OK);
    if (idle_wait_entered != LIB_NULL && task != LIB_NULL &&
        events[0] == worker_wake) {
        ++idle_wait_calls;
        lib_test_assert(base_sync_event_signal(idle_wait_entered) == LIB_STATUS_OK);
        if (base_sync_wait_any(events, count, task, timeout, index) ==
            BASE_SYNC_WAIT_SIGNALED) return BASE_SYNC_WAIT_FAULT;
        return BASE_SYNC_WAIT_CANCELLED;
    }
    return base_sync_wait_any(events, count, task, timeout, index);
}

static lib_status audio_test_event_signal(base_sync_event *event)
{
    lib_status status = signal_status;

    if (event == failed_signal) return LIB_STATUS_IO_ERROR;
    if (status != LIB_STATUS_OK) {
        signal_status = LIB_STATUS_OK;
        return status;
    }
    return base_sync_event_signal(event);
}

lib_status audio_pcm_stream_platform_create(const lib_audio_pcm_stream_options *options,
    audio_pcm_stream_platform **out_platform)
{
    ++create_calls;
    fake_platform.submitted = fake_platform.consumed = 0u;
    lib_test_assert(options != LIB_NULL && out_platform != LIB_NULL);
    *out_platform = create_status == LIB_STATUS_OK ? &fake_platform : LIB_NULL;
    return create_status;
}

lib_status audio_pcm_stream_platform_worker_attach(audio_pcm_stream_platform *platform)
{
    lib_test_assert(platform == &fake_platform);
    ++attach_calls;
    return attach_status;
}

void audio_pcm_stream_platform_worker_detach(audio_pcm_stream_platform *platform)
{ lib_test_assert(platform == &fake_platform); ++detach_calls; }

lib_status audio_pcm_stream_platform_enqueue(audio_pcm_stream_platform *platform,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames)
{
    lib_u32 accepted = frame_count < accepted_limit ? frame_count : accepted_limit;
    lib_status status = enqueue_status;
    if (interrupt_prefix) {
        accepted = 1u;
        interrupt_prefix = LIB_FALSE;
        status = LIB_STATUS_LIMIT_EXCEEDED;
    }
    if (delivered_count < 3u) {
        lib_u32 copied = accepted < 3u - delivered_count ? accepted : 3u - delivered_count;
        lib_memory_copy(delivered_prefix + delivered_count * 2u, samples,
            (lib_size)copied * 2u * sizeof(*samples));
        delivered_count += copied;
    }
    ++enqueue_calls;
    last_enqueued_frames = accepted;
    total_enqueued_frames += accepted;
    platform->submitted += accepted;
    lib_test_assert(platform == &fake_platform);
    lib_memory_copy(platform->copied, samples,
        (lib_size)accepted * 2u * sizeof(*samples));
    *out_accepted_frames = accepted;
    if (tail_delivered != LIB_NULL)
        lib_test_assert(base_sync_event_signal(tail_delivered) == LIB_STATUS_OK);
    return accepted == 0u && frame_count != 0u ? LIB_STATUS_LIMIT_EXCEEDED : status;
}

lib_status audio_pcm_stream_platform_wait_writable(audio_pcm_stream_platform *platform)
{
    ++wait_calls;
    lib_test_assert(platform == &fake_platform);
    if (native_wait_blocks != LIB_FALSE) {
        lib_test_assert(base_sync_event_signal(native_wait_entered) == LIB_STATUS_OK);
        lib_test_assert(base_sync_event_wait(native_wait_release, LIB_UINT32_MAX) ==
            BASE_SYNC_WAIT_SIGNALED);
    }
    return wait_status;
}

lib_status audio_pcm_stream_platform_cancel_wait(audio_pcm_stream_platform *platform)
{
    ++cancel_calls;
    lib_test_assert(platform == &fake_platform);
    if (cancel_status == LIB_STATUS_OK && native_wait_release != LIB_NULL)
        lib_test_assert(base_sync_event_signal(native_wait_release) == LIB_STATUS_OK);
    if (cancel_status == LIB_STATUS_OK && progress_release != LIB_NULL)
        lib_test_assert(base_sync_event_signal(progress_release) == LIB_STATUS_OK);
    return cancel_status;
}

lib_status audio_pcm_stream_platform_clear(audio_pcm_stream_platform *platform)
{
    ++clear_calls;
    lib_test_assert(platform == &fake_platform);
    platform->queued = 0u;
    platform->submitted = platform->consumed = 0u;
    return clear_status;
}

lib_status audio_pcm_stream_platform_progress(audio_pcm_stream_platform *platform,
    lib_u64 *submitted, lib_u64 *consumed)
{
    lib_test_assert(platform == &fake_platform);
    *submitted = platform->submitted;
    *consumed = hold_consumption ? platform->consumed : platform->submitted;
    return LIB_STATUS_OK;
}

lib_status audio_pcm_stream_platform_wait_progress(audio_pcm_stream_platform *platform)
{
    lib_test_assert(platform == &fake_platform && progress_entered != LIB_NULL);
    lib_test_assert(base_sync_event_signal(progress_entered) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(progress_release, 1000u) == BASE_SYNC_WAIT_SIGNALED);
    if (finish_consumption) {
        platform->consumed = platform->submitted;
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_LIMIT_EXCEEDED;
}

lib_status audio_pcm_stream_platform_set_gain(audio_pcm_stream_platform *platform,
    const lib_u16 *gains, lib_u32 channel_count)
{
    lib_test_assert(platform == &fake_platform && channel_count >= 1u && channel_count <= 2u);
    lib_memory_copy(observed_gains, gains, channel_count * sizeof(*gains));
    observed_gain_channels = channel_count;
    return gain_status;
}

lib_status audio_pcm_stream_platform_destroy(audio_pcm_stream_platform **platform)
{
    lib_test_assert(platform != LIB_NULL && *platform == &fake_platform);
    if (destroy_status == LIB_STATUS_OK) *platform = LIB_NULL;
    return destroy_status;
}

static lib_status audio_test_task_create(base_sync_task_entry entry,
    void *context, base_sync_task **out_task)
{
    worker_wake = ((lib_audio_pcm_stream *)context)->wake;
    return base_sync_task_create(entry, context, out_task);
}

#define base_sync_task_create audio_test_task_create
#define base_sync_wait_any audio_test_wait_any
#define base_sync_event_signal audio_test_event_signal
#include "lib/audio/pcm/stream.c"
#undef base_sync_event_signal
#undef base_sync_wait_any
#undef base_sync_task_create

static void control_completion_failure(void)
{
    const lib_audio_pcm_stream_options options = {48000u, 2u};
    const lib_u16 gains[2] = {65535u, 65535u};
    lib_audio_pcm_stream *stream = LIB_NULL;
    lib_u32 operation;

    gain_status = LIB_STATUS_OK;
    for (operation = 0u; operation < 2u; ++operation) {
        lib_u32 queued, writable;
        lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
        failed_signal = stream->control_done;
        lib_test_assert((operation == 0u ? lib_audio_pcm_stream_clear(stream) :
            lib_audio_pcm_stream_set_gain(stream, gains, 2u)) == LIB_STATUS_IO_ERROR);
        lib_test_assert(base_sync_task_cancelled(stream->worker));
        lib_test_assert(lib_audio_pcm_stream_query(stream, &queued, &writable) == LIB_STATUS_IO_ERROR);
        lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);
        failed_signal = LIB_NULL;
    }
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
    failed_signal = stream->wake;
    lib_test_assert(lib_audio_pcm_stream_clear(stream) == LIB_STATUS_IO_ERROR);
    lib_test_assert(base_sync_task_cancelled(stream->worker));
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);
    failed_signal = LIB_NULL;
    {
        const lib_i16 samples[2] = {1, -1};
        lib_u32 accepted;
        lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
            &native_wait_entered) == LIB_STATUS_OK);
        lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
            &native_wait_release) == LIB_STATUS_OK);
        native_wait_blocks = LIB_TRUE;
        lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 1u, &accepted) == LIB_STATUS_OK);
        lib_test_assert(accepted == 1u);
        lib_test_assert(base_sync_event_wait(native_wait_entered, 1000u) == BASE_SYNC_WAIT_SIGNALED);
        failed_signal = stream->control_done;
        release_native_on_control_wait = LIB_TRUE;
        lib_test_assert(lib_audio_pcm_stream_flush(stream) == LIB_STATUS_IO_ERROR);
        lib_test_assert(base_sync_task_join(stream->worker) == LIB_STATUS_OK);
        lib_test_assert(stream->count == 0u && stream->progress.accepted_frames == 1u);
        lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);
        failed_signal = LIB_NULL;
        release_native_on_control_wait = LIB_FALSE;
        native_wait_blocks = LIB_FALSE;
        base_sync_event_destroy(native_wait_entered);
        base_sync_event_destroy(native_wait_release);
        native_wait_entered = native_wait_release = LIB_NULL;
    }
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
    failed_signal = stream->space;
    lib_test_assert(lib_audio_pcm_stream_cancel_wait(stream) == LIB_STATUS_IO_ERROR);
    lib_test_assert(base_sync_task_cancelled(stream->worker));
    lib_test_assert(lib_audio_pcm_stream_clear(stream) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);
    failed_signal = LIB_NULL;
}

int main(void)
{
    lib_audio_pcm_stream_options options = { 48000u, 2u };
    lib_audio_pcm_stream *stream = LIB_NULL;
    base_sync_event *native_wait_entered_event = LIB_NULL;
    base_sync_event *native_wait_release_event = LIB_NULL;
    lib_i16 samples[6] = { -1, 1, -2, 2, -3, 3 };
    lib_i16 submission[LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION * 2u] = { 0 };
    lib_u32 accepted = 99u, queued = 99u, writable = 99u;
    unsigned before_destroy_enqueue_calls;

    lib_test_assert(lib_audio_pcm_stream_create(LIB_NULL, &stream) == LIB_STATUS_INVALID_ARGUMENT);
    options.sample_rate = 0u;
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_UNSUPPORTED);
    options.sample_rate = 48000u;
    options.channel_count = 3u;
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_UNSUPPORTED);
    options.channel_count = 2u;
    create_status = LIB_STATUS_IO_ERROR;
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_IO_ERROR && stream == LIB_NULL);
    create_status = LIB_STATUS_OK;
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
    lib_test_assert(create_calls == 2u);

    lib_test_assert(lib_audio_pcm_stream_query(stream, &queued, &writable) == LIB_STATUS_OK);
    lib_test_assert(queued == 0u && writable == AUDIO_PCM_STREAM_QUEUE_CAPACITY);
    lib_test_assert(lib_audio_pcm_stream_wait_writable(stream) == LIB_STATUS_OK && wait_calls == 0u);
    lib_test_assert(lib_audio_pcm_stream_cancel_wait(stream) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_wait_writable(stream) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_flush(stream) == LIB_STATUS_OK);
    lib_test_assert(total_enqueued_frames == 1024u && enqueue_calls == wait_calls);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, LIB_NULL, 0u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(accepted == 0u && total_enqueued_frames == 1024u);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, LIB_NULL, 1u, &accepted) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 513u, &accepted) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &tail_delivered) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(accepted == 3u);
    /* No flush or second enqueue is allowed to cause this delivery. */
    lib_test_assert(base_sync_event_wait(tail_delivered, 1000u) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(lib_audio_pcm_stream_flush(stream) == LIB_STATUS_OK);
    lib_test_assert(total_enqueued_frames == 1027u && last_enqueued_frames == 3u);
    lib_test_assert(lib_memory_compare(fake_platform.copied, samples, sizeof(samples)) == 0);
    base_sync_event_destroy(tail_delivered);
    tail_delivered = LIB_NULL;
    lib_test_assert(lib_audio_pcm_stream_query(stream, &queued, &writable) == LIB_STATUS_OK);
    lib_test_assert(queued == 0u && writable == AUDIO_PCM_STREAM_QUEUE_CAPACITY);

    /* Deactivation is a delivery boundary: it flushes a short finite sound
     * before it clears the native backend and makes the producer inactive. */
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_set_active(stream, LIB_FALSE) == LIB_STATUS_OK);
    lib_test_assert(total_enqueued_frames == 1030u && last_enqueued_frames == 3u);
    lib_test_assert(clear_calls == 1u && cancel_calls == 1u);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 1u, &accepted) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(lib_audio_pcm_stream_wait_writable(stream) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(lib_audio_pcm_stream_set_active(stream, LIB_FALSE) == LIB_STATUS_OK);
    lib_test_assert(clear_calls == 1u);
    lib_test_assert(lib_audio_pcm_stream_set_active(stream, LIB_TRUE) == LIB_STATUS_OK);
    clear_status = LIB_STATUS_OK;

    /* Hold the native writer so clear must discard an unsubmitted tail. */
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_entered_event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_release_event) == LIB_STATUS_OK);
    native_wait_entered = native_wait_entered_event;
    native_wait_release = native_wait_release_event;
    native_wait_blocks = LIB_TRUE;
    wait_status = LIB_STATUS_LIMIT_EXCEEDED;
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(native_wait_entered_event, 1000u) == BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(lib_audio_pcm_stream_clear(stream) == LIB_STATUS_OK);
    lib_test_assert(total_enqueued_frames == 1030u);
    native_wait_blocks = LIB_FALSE;
    wait_status = LIB_STATUS_OK;
    native_wait_entered = LIB_NULL;
    native_wait_release = LIB_NULL;
    base_sync_event_destroy(native_wait_entered_event);
    base_sync_event_destroy(native_wait_release_event);
    lib_test_assert(lib_audio_pcm_stream_query(stream, &queued, &writable) == LIB_STATUS_OK);
    lib_test_assert(queued == 0u && writable == AUDIO_PCM_STREAM_QUEUE_CAPACITY);

    /* Explicit flush, not destruction, guarantees backend submission. */
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_flush(stream) == LIB_STATUS_OK);

    destroy_status = LIB_STATUS_IO_ERROR;
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_IO_ERROR && stream != LIB_NULL);
    lib_test_assert(total_enqueued_frames == 1033u && last_enqueued_frames == 3u);
    destroy_status = LIB_STATUS_OK;
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);

    /* Queue ownership transfers before wake-up. A failed wake makes that
     * accepted prefix terminal without inviting the producer to resend it. */
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
    signal_status = LIB_STATUS_IO_ERROR;
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) ==
        LIB_STATUS_IO_ERROR && accepted == 3u);
    lib_test_assert(lib_audio_pcm_stream_query(stream, &queued, &writable) ==
        LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 1u, &accepted) ==
        LIB_STATUS_IO_ERROR && accepted == 0u);
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);

    /* Fill the producer FIFO while the native wait is held, then make the
     * write fail. A waiter must receive the terminal failure, never false
     * writable space or a silently discarded batch. */
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_entered_event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_release_event) == LIB_STATUS_OK);
    native_wait_entered = native_wait_entered_event;
    native_wait_release = native_wait_release_event;
    native_wait_blocks = LIB_TRUE;
    wait_status = LIB_STATUS_OK;
    enqueue_status = LIB_STATUS_OK;
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(native_wait_entered_event, LIB_UINT32_MAX) ==
        BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, 512u, &accepted) == LIB_STATUS_OK);
    enqueue_status = LIB_STATUS_IO_ERROR;
    lib_test_assert(base_sync_event_signal(native_wait_release_event) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_wait_writable(stream) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_audio_pcm_stream_query(stream, &queued, &writable) == LIB_STATUS_IO_ERROR);
    native_wait_blocks = LIB_FALSE;
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
    native_wait_entered = LIB_NULL;
    native_wait_release = LIB_NULL;
    base_sync_event_destroy(native_wait_entered_event);
    base_sync_event_destroy(native_wait_release_event);

    /* Destroy must interrupt a worker waiting for the native backend before
     * releasing that backend; it may not submit the retained batch afterwards. */
    enqueue_status = LIB_STATUS_OK;
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_entered_event) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_release_event) == LIB_STATUS_OK);
    native_wait_entered = native_wait_entered_event;
    native_wait_release = native_wait_release_event;
    native_wait_blocks = LIB_TRUE;
    const lib_u32 destroy_counts[] = {1u, 512u, 1024u, 1025u, AUDIO_PCM_STREAM_QUEUE_CAPACITY};
    for (lib_size n = 0u; n < sizeof(destroy_counts) / sizeof(destroy_counts[0]); ++n) {
        lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
        for (lib_u32 remaining = destroy_counts[n]; remaining != 0u;) {
            lib_u32 count = remaining > 512u ? 512u : remaining;
            lib_test_assert(lib_audio_pcm_stream_enqueue(stream, submission, count, &accepted) == LIB_STATUS_OK);
            lib_test_assert(accepted == count);
            remaining -= accepted;
        }
        lib_test_assert(base_sync_event_wait(native_wait_entered_event, LIB_UINT32_MAX) ==
            BASE_SYNC_WAIT_SIGNALED);
        before_destroy_enqueue_calls = enqueue_calls;
        cancel_status = LIB_STATUS_IO_ERROR;
        lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_IO_ERROR);
        lib_test_assert(stream != LIB_NULL && stream->worker != LIB_NULL &&
            stream->platform == &fake_platform);
        lib_test_assert(base_sync_event_wait(native_wait_release_event, 0u) ==
            BASE_SYNC_WAIT_TIMED_OUT);
        cancel_status = LIB_STATUS_OK;
        lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
        lib_test_assert(enqueue_calls == before_destroy_enqueue_calls);
    }
    native_wait_blocks = LIB_FALSE;
    native_wait_entered = LIB_NULL;
    native_wait_release = LIB_NULL;
    base_sync_event_destroy(native_wait_entered_event);
    base_sync_event_destroy(native_wait_release_event);
    /* A backend cancellation after one accepted frame must retry only the
     * suffix, through the same worker and FIFO completion boundary. */
    delivered_count = 0u;
    interrupt_prefix = LIB_TRUE;
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_flush(stream) == LIB_STATUS_OK);
    lib_test_assert(delivered_count == 3u &&
        lib_memory_compare(delivered_prefix, samples, sizeof(samples)) == 0);
    lib_test_assert(lib_audio_pcm_stream_query(stream, &queued, &writable) == LIB_STATUS_OK && queued == 0u);
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);
    /* Fail the idle wait when flush wakes it: the caller must receive failure
     * and the worker must detach once, rather than retrying the failed wait. */
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
        &native_wait_entered_event) == LIB_STATUS_OK);
    idle_wait_entered = native_wait_entered_event;
    before_destroy_enqueue_calls = enqueue_calls;
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_wait(idle_wait_entered, LIB_UINT32_MAX) ==
        BASE_SYNC_WAIT_SIGNALED);
    lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_pcm_stream_flush(stream) == LIB_STATUS_IO_ERROR);
    lib_test_assert(base_sync_task_join(stream->worker) == LIB_STATUS_OK);
    lib_test_assert(idle_wait_calls == 1u && enqueue_calls == before_destroy_enqueue_calls);
    lib_test_assert(lib_audio_pcm_stream_query(stream, &queued, &writable) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
    idle_wait_entered = LIB_NULL;
    base_sync_event_destroy(native_wait_entered_event);
    /* Rates are admitted by representation, not a fixed playback whitelist.
     * This fixture proves forwarding; the native fixture proves negotiation. */
    {
        const lib_u32 rates[] = { 3921u, 8000u, 11025u, 12345u,
            22050u, 44100u, 48000u, LIB_UINT32_MAX };
        lib_size index;
        for (options.channel_count = 1u; options.channel_count <= 2u;
            ++options.channel_count) {
            for (index = 0u; index < sizeof(rates) / sizeof(rates[0]); ++index) {
                unsigned before_create = create_calls;
                options.sample_rate = rates[index];
                lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
                lib_test_assert(create_calls == before_create + 1u);
                lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);
            }
        }
    }
    options.channel_count = 2u;
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
    {
        const lib_u16 gains[2] = { 0u, 65535u };
        lib_test_assert(lib_audio_pcm_stream_set_gain(stream, LIB_NULL, 2u) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(lib_audio_pcm_stream_set_gain(stream, gains, 1u) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(lib_audio_pcm_stream_set_gain(stream, gains, 2u) == LIB_STATUS_OK);
        lib_test_assert(observed_gain_channels == 2u && observed_gains[0] == 0u && observed_gains[1] == 65535u);
        lib_test_assert(lib_audio_pcm_stream_set_active(stream, LIB_FALSE) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_set_gain(stream, gains, 2u) == LIB_STATUS_OK);
        gain_status = LIB_STATUS_IO_ERROR;
        lib_test_assert(lib_audio_pcm_stream_set_gain(stream, gains, 2u) == LIB_STATUS_IO_ERROR);
        lib_test_assert(lib_audio_pcm_stream_query(stream, &queued, &writable) == LIB_STATUS_IO_ERROR);
    }
    lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);
    {
        lib_u32 before_attach = attach_calls;
        lib_u32 before_detach = detach_calls;
        attach_status = LIB_STATUS_UNSUPPORTED;
        lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_UNSUPPORTED);
        lib_test_assert(stream == LIB_NULL && attach_calls == before_attach + 1u);
        lib_test_assert(detach_calls == before_detach);
        attach_status = LIB_STATUS_OK;
        signal_status = LIB_STATUS_IO_ERROR;
        lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_IO_ERROR);
        lib_test_assert(stream == LIB_NULL && detach_calls == before_detach + 1u);
        lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
        lib_test_assert(attach_calls == before_attach + 3u);
        lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);
    }
    {
        base_sync_event *notification = LIB_NULL;
        lib_audio_pcm_progress progress;
        hold_consumption = LIB_TRUE;
        finish_consumption = LIB_FALSE;
        lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &progress_entered) == LIB_STATUS_OK);
        lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &progress_release) == LIB_STATUS_OK);
        lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &notification) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_set_notify(stream, audio_test_notify, notification) == LIB_STATUS_OK);
        lib_test_assert(base_sync_event_wait(notification, 1000u) == BASE_SYNC_WAIT_SIGNALED);
        lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
        lib_test_assert(base_sync_event_wait(progress_entered, 1000u) == BASE_SYNC_WAIT_SIGNALED);
        lib_test_assert(lib_audio_pcm_stream_flush(stream) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_progress(stream, &progress) == LIB_STATUS_OK);
        lib_test_assert(progress.epoch == 0u && progress.accepted_frames == 3u &&
            progress.submitted_frames == 3u && progress.consumed_frames == 0u);
        lib_test_assert(base_sync_event_reset(notification) == LIB_STATUS_OK);
        finish_consumption = LIB_TRUE;
        lib_test_assert(base_sync_event_signal(progress_release) == LIB_STATUS_OK);
        /* Native completion alone must notify: no producer or flush call. */
        lib_test_assert(base_sync_event_wait(notification, 1000u) == BASE_SYNC_WAIT_SIGNALED);
        lib_test_assert(lib_audio_pcm_stream_progress(stream, &progress) == LIB_STATUS_OK);
        lib_test_assert(progress.consumed_frames == 3u);
        lib_test_assert(lib_audio_pcm_stream_clear(stream) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_progress(stream, &progress) == LIB_STATUS_OK);
        lib_test_assert(progress.epoch == 1u && progress.accepted_frames == 0u &&
            progress.submitted_frames == 0u && progress.consumed_frames == 0u);
        /* A new prefix interrupts the native tail wait without discarding
         * the earlier submitted range; clear then cancels both ranges. */
        finish_consumption = LIB_FALSE;
        lib_test_assert(base_sync_event_reset(progress_release) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
        lib_test_assert(base_sync_event_wait(progress_entered, 1000u) == BASE_SYNC_WAIT_SIGNALED);
        lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 3u, &accepted) == LIB_STATUS_OK);
        lib_test_assert(base_sync_event_wait(progress_entered, 1000u) == BASE_SYNC_WAIT_SIGNALED);
        lib_test_assert(lib_audio_pcm_stream_progress(stream, &progress) == LIB_STATUS_OK);
        lib_test_assert(progress.accepted_frames == 6u && progress.submitted_frames == 6u &&
            progress.consumed_frames == 0u);
        lib_test_assert(lib_audio_pcm_stream_clear(stream) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_progress(stream, &progress) == LIB_STATUS_OK);
        lib_test_assert(progress.epoch == 2u && progress.accepted_frames == 0u &&
            progress.submitted_frames == 0u && progress.consumed_frames == 0u);
        /* Seed only the boundary counters, not an alternate processing path. */
        base_sync_mutex_lock(stream->lock);
        stream->progress.accepted_frames = LIB_UINT64_MAX;
        base_sync_mutex_unlock(stream->lock);
        lib_test_assert(lib_audio_pcm_stream_enqueue(stream, samples, 1u, &accepted) == LIB_STATUS_LIMIT_EXCEEDED);
        lib_test_assert(accepted == 0u);
        lib_test_assert(lib_audio_pcm_stream_clear(stream) == LIB_STATUS_OK);
        base_sync_mutex_lock(stream->lock);
        stream->progress.epoch = LIB_UINT64_MAX;
        base_sync_mutex_unlock(stream->lock);
        lib_test_assert(lib_audio_pcm_stream_clear(stream) == LIB_STATUS_LIMIT_EXCEEDED);
        notify_status = LIB_STATUS_IO_ERROR;
        lib_test_assert(lib_audio_pcm_stream_set_notify(stream, audio_test_notify, notification) == LIB_STATUS_IO_ERROR);
        lib_test_assert(lib_audio_pcm_stream_progress(stream, &progress) == LIB_STATUS_IO_ERROR);
        lib_test_assert(progress.epoch == 0u && progress.consumed_frames == 0u);
        lib_test_assert(lib_audio_pcm_stream_set_notify(stream, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
        lib_test_assert(lib_audio_pcm_stream_destroy(&stream) == LIB_STATUS_OK);
        base_sync_event_destroy(notification);
        base_sync_event_destroy(progress_entered);
        base_sync_event_destroy(progress_release);
        progress_entered = progress_release = LIB_NULL;
    }
    control_completion_failure();
    return 0;
}
