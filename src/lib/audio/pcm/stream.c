#include "lib/audio/pcm/stream.h"

static lib_bool audio_pcm_stream_options_supported(const lib_audio_pcm_stream_options *options)
{
    return options->channel_count >= 1u && options->channel_count <= 2u &&
        options->sample_rate != 0u;
}

static void audio_pcm_stream_notify_locked(lib_audio_pcm_stream *stream)
{
    if (stream->notify != LIB_NULL) {
        lib_status status = stream->notify(stream->notify_context);
        if (stream->failure == LIB_STATUS_OK) stream->failure = status;
    }
    if (stream->failure != LIB_STATUS_OK) {
        (void)base_sync_task_request_cancel(stream->worker);
    }
}

static void audio_pcm_stream_fail_locked(lib_audio_pcm_stream *stream, lib_status status)
{
    if (stream->failure == LIB_STATUS_OK) {
        stream->failure = status;
        audio_pcm_stream_notify_locked(stream);
    }
}

static lib_status audio_pcm_stream_signal_locked(lib_audio_pcm_stream *stream,
    base_sync_event *event)
{
    lib_status status = base_sync_event_signal(event);
    if (status != LIB_STATUS_OK) audio_pcm_stream_fail_locked(stream, status);
    return status;
}

static void audio_pcm_stream_observe_progress(lib_audio_pcm_stream *stream, lib_u64 epoch)
{
    lib_u64 submitted, consumed;
    lib_status status = audio_pcm_stream_platform_progress(stream->platform,
        &submitted, &consumed);
    base_sync_mutex_lock(stream->lock);
    if (stream->progress.epoch == epoch && stream->failure == LIB_STATUS_OK) {
        if (status == LIB_STATUS_OK && (consumed > submitted ||
            submitted > stream->progress.accepted_frames ||
            submitted < stream->progress.submitted_frames ||
            consumed < stream->progress.consumed_frames)) status = LIB_STATUS_IO_ERROR;
        if (status != LIB_STATUS_OK) audio_pcm_stream_fail_locked(stream, status);
        else if (submitted != stream->progress.submitted_frames ||
            consumed != stream->progress.consumed_frames) {
            stream->progress.submitted_frames = submitted;
            stream->progress.consumed_frames = consumed;
            audio_pcm_stream_notify_locked(stream);
        }
    }
    base_sync_mutex_unlock(stream->lock);
}

static void audio_pcm_stream_copy_head(const lib_audio_pcm_stream *stream,
    lib_i16 *destination, lib_u32 frame_count)
{
    lib_u32 index;

    for (index = 0u; index < frame_count; ++index)
        lib_memory_copy(&destination[index * stream->channel_count],
            &stream->samples[((stream->head + index) %
                AUDIO_PCM_STREAM_QUEUE_CAPACITY) * stream->channel_count],
            (lib_size)stream->channel_count * sizeof(*destination));
}

/* The worker is the only live task that calls the platform stream.  It keeps
 * a batch in the producer FIFO until the native endpoint has accepted it. */
static void audio_pcm_stream_worker(void *context, const base_sync_task *task)
{
    lib_audio_pcm_stream *stream = context;
    lib_i16 batch[AUDIO_PCM_STREAM_PLAY_BATCH * 2u];
    lib_status attach_status;

    attach_status = audio_pcm_stream_platform_worker_attach(stream->platform);
    base_sync_mutex_lock(stream->lock);
    stream->failure = attach_status;
    if (base_sync_event_signal(stream->control_done) != LIB_STATUS_OK) {
        stream->failure = LIB_STATUS_IO_ERROR;
        (void)base_sync_task_request_cancel(stream->worker);
    }
    base_sync_mutex_unlock(stream->lock);
    if (attach_status != LIB_STATUS_OK) return;
    while (!base_sync_task_cancelled(task)) {
        audio_pcm_stream_control control = AUDIO_PCM_STREAM_CONTROL_NONE;
        lib_bool have_batch = LIB_FALSE;
        lib_bool pending_output;
        lib_u32 batch_count = 0u;
        lib_u64 generation;
        lib_u32 accepted = 0u;
        lib_status status;

        base_sync_mutex_lock(stream->lock);
        generation = stream->progress.epoch;
        pending_output = stream->failure == LIB_STATUS_OK &&
            stream->progress.submitted_frames > stream->progress.consumed_frames;
        if (stream->control == AUDIO_PCM_STREAM_CONTROL_CLEAR ||
            stream->control == AUDIO_PCM_STREAM_CONTROL_GAIN) {
            control = stream->control;
            stream->control = AUDIO_PCM_STREAM_CONTROL_NONE;
        } else if (stream->failure == LIB_STATUS_OK &&
            stream->active != LIB_FALSE &&
            stream->count != 0u) {
            batch_count = stream->count < AUDIO_PCM_STREAM_PLAY_BATCH ?
                stream->count : AUDIO_PCM_STREAM_PLAY_BATCH;
            audio_pcm_stream_copy_head(stream, batch, batch_count);
            have_batch = LIB_TRUE;
        }
        base_sync_mutex_unlock(stream->lock);

        if (control == AUDIO_PCM_STREAM_CONTROL_CLEAR ||
            control == AUDIO_PCM_STREAM_CONTROL_GAIN) {
            status = control == AUDIO_PCM_STREAM_CONTROL_CLEAR ?
                audio_pcm_stream_platform_clear(stream->platform) :
                audio_pcm_stream_platform_set_gain(stream->platform,
                    stream->gains, stream->channel_count);
            base_sync_mutex_lock(stream->lock);
            if (status != LIB_STATUS_OK) audio_pcm_stream_fail_locked(stream, status);
            else if (control == AUDIO_PCM_STREAM_CONTROL_CLEAR)
                audio_pcm_stream_notify_locked(stream);
            (void)audio_pcm_stream_signal_locked(stream, stream->control_done);
            base_sync_mutex_unlock(stream->lock);
            continue;
        }
        if (have_batch == LIB_FALSE) {
            if (pending_output != LIB_FALSE) {
                status = audio_pcm_stream_platform_wait_progress(stream->platform);
                if (base_sync_task_cancelled(task)) break;
                if (status == LIB_STATUS_OK)
                    audio_pcm_stream_observe_progress(stream, generation);
                else if (status != LIB_STATUS_LIMIT_EXCEEDED) {
                    base_sync_mutex_lock(stream->lock);
                    audio_pcm_stream_fail_locked(stream, status);
                    base_sync_mutex_unlock(stream->lock);
                }
                continue;
            }
            base_sync_wait_result result = base_sync_wait_any(&stream->wake, 1u, task,
                LIB_UINT32_MAX, LIB_NULL);
            if (result == BASE_SYNC_WAIT_SIGNALED) continue;
            if (result != BASE_SYNC_WAIT_CANCELLED) {
                base_sync_mutex_lock(stream->lock);
                audio_pcm_stream_fail_locked(stream, LIB_STATUS_IO_ERROR);
                base_sync_mutex_unlock(stream->lock);
            }
            break;
        }
        status = audio_pcm_stream_platform_wait_writable(stream->platform);
        if (base_sync_task_cancelled(task)) break;
        if (status == LIB_STATUS_LIMIT_EXCEEDED) continue;
        if (status == LIB_STATUS_OK) {
            status = audio_pcm_stream_platform_enqueue(stream->platform, batch,
                batch_count, &accepted);
            if (accepted > batch_count) {
                accepted = 0u;
                status = LIB_STATUS_IO_ERROR;
            } else if (status == LIB_STATUS_OK && accepted != batch_count)
                status = LIB_STATUS_IO_ERROR;
        }
        if (status == LIB_STATUS_OK || status == LIB_STATUS_LIMIT_EXCEEDED)
            audio_pcm_stream_observe_progress(stream, generation);
        base_sync_mutex_lock(stream->lock);
        if (stream->progress.epoch == generation && stream->count >= accepted) {
            stream->head = (stream->head + accepted) % AUDIO_PCM_STREAM_QUEUE_CAPACITY;
            stream->count -= accepted;
        }
        if (status != LIB_STATUS_OK && status != LIB_STATUS_LIMIT_EXCEEDED)
            audio_pcm_stream_fail_locked(stream, status);
        if (stream->control == AUDIO_PCM_STREAM_CONTROL_FLUSH &&
            (stream->failure != LIB_STATUS_OK || stream->count == 0u)) {
            stream->control = AUDIO_PCM_STREAM_CONTROL_NONE;
            (void)audio_pcm_stream_signal_locked(stream, stream->control_done);
        }
        if (accepted != 0u || stream->failure != LIB_STATUS_OK)
            (void)audio_pcm_stream_signal_locked(stream, stream->space);
        base_sync_mutex_unlock(stream->lock);
    }
    audio_pcm_stream_platform_worker_detach(stream->platform);
}

lib_status lib_audio_pcm_stream_create(const lib_audio_pcm_stream_options *options,
    lib_audio_pcm_stream **out_stream)
{
    lib_audio_pcm_stream *stream;
    lib_status status;

    if (out_stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_stream = LIB_NULL;
    if (options == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (audio_pcm_stream_options_supported(options) == LIB_FALSE)
        return LIB_STATUS_UNSUPPORTED;
    stream = lib_allocate_zero(1u, sizeof(*stream));
    if (stream == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = audio_pcm_stream_platform_create(options, &stream->platform);
    if (status == LIB_STATUS_OK) status = base_sync_mutex_create(&stream->lock);
    if (status == LIB_STATUS_OK)
        status = base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &stream->wake);
    if (status == LIB_STATUS_OK)
        status = base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &stream->space);
    if (status == LIB_STATUS_OK)
        status = base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &stream->control_done);
    if (status == LIB_STATUS_OK) {
        stream->channel_count = options->channel_count;
        stream->active = LIB_TRUE;
        stream->failure = LIB_STATUS_OK;
        lib_atomic_i32_initialize(&stream->wait_cancelled, 0);
        /* Publish the task handle before its startup result can cancel it. */
        base_sync_mutex_lock(stream->lock);
        status = base_sync_task_create(audio_pcm_stream_worker, stream, &stream->worker);
        base_sync_mutex_unlock(stream->lock);
    }
    if (status == LIB_STATUS_OK) {
        base_sync_wait_result result = base_sync_wait_any(&stream->control_done,
            1u, stream->worker, LIB_UINT32_MAX, LIB_NULL);
        base_sync_mutex_lock(stream->lock);
        status = result == BASE_SYNC_WAIT_SIGNALED ? stream->failure : LIB_STATUS_IO_ERROR;
        base_sync_mutex_unlock(stream->lock);
    }
    if (status != LIB_STATUS_OK) {
        /* No public owner exists on failure; never strand a live worker. */
        if (stream->worker != LIB_NULL &&
            base_sync_task_destroy(stream->worker) != LIB_STATUS_OK)
            lib_process_exit_immediate(LIB_STATUS_INTERNAL_ERROR);
        base_sync_event_destroy(stream->control_done);
        base_sync_event_destroy(stream->space);
        base_sync_event_destroy(stream->wake);
        base_sync_mutex_destroy(stream->lock);
        if (stream->platform != LIB_NULL)
            (void)audio_pcm_stream_platform_destroy(&stream->platform);
        lib_release(stream);
        return status;
    }
    *out_stream = stream;
    return LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_enqueue(lib_audio_pcm_stream *stream,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames)
{
    lib_u32 index;
    lib_status status;
    lib_bool pending_output;

    if (out_accepted_frames == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_accepted_frames = 0u;
    if (stream == LIB_NULL || frame_count > LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION ||
        (samples == LIB_NULL && frame_count != 0u)) return LIB_STATUS_INVALID_ARGUMENT;
    if (frame_count == 0u) return LIB_STATUS_OK;
    base_sync_mutex_lock(stream->lock);
    if (stream->failure != LIB_STATUS_OK) {
        lib_status status = stream->failure;
        base_sync_mutex_unlock(stream->lock);
        return status;
    }
    if (stream->active == LIB_FALSE) {
        base_sync_mutex_unlock(stream->lock);
        return LIB_STATUS_INVALID_STATE;
    }
    *out_accepted_frames = AUDIO_PCM_STREAM_QUEUE_CAPACITY - stream->count;
    if (*out_accepted_frames > frame_count) *out_accepted_frames = frame_count;
    if (*out_accepted_frames > LIB_UINT64_MAX - stream->progress.accepted_frames) {
        *out_accepted_frames = 0u;
        base_sync_mutex_unlock(stream->lock);
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    for (index = 0u; index < *out_accepted_frames; ++index)
        lib_memory_copy(&stream->samples[((stream->head + stream->count + index) %
                AUDIO_PCM_STREAM_QUEUE_CAPACITY) * stream->channel_count],
            &samples[index * stream->channel_count],
            (lib_size)stream->channel_count * sizeof(*samples));
    stream->count += *out_accepted_frames;
    stream->progress.accepted_frames += *out_accepted_frames;
    pending_output = stream->progress.submitted_frames > stream->progress.consumed_frames;
    base_sync_mutex_unlock(stream->lock);
    if (*out_accepted_frames == 0u) return LIB_STATUS_LIMIT_EXCEEDED;
    status = base_sync_event_signal(stream->wake);
    if (status == LIB_STATUS_OK && pending_output != LIB_FALSE)
        status = audio_pcm_stream_platform_cancel_wait(stream->platform);
    if (status == LIB_STATUS_OK) return LIB_STATUS_OK;
    /* The worker may already have consumed the signal, so accepted input
     * cannot be rolled back. Make the terminal state visible to later calls. */
    base_sync_mutex_lock(stream->lock);
    audio_pcm_stream_fail_locked(stream, status);
    base_sync_mutex_unlock(stream->lock);
    return status;
}

lib_status lib_audio_pcm_stream_query(lib_audio_pcm_stream *stream,
    lib_u32 *out_queued_frames, lib_u32 *out_writable_frames)
{
    lib_status status;

    if (out_queued_frames == LIB_NULL || out_writable_frames == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_queued_frames = 0u;
    *out_writable_frames = 0u;
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(stream->lock);
    status = stream->failure;
    if (status == LIB_STATUS_OK) {
        *out_queued_frames = stream->count;
        *out_writable_frames = AUDIO_PCM_STREAM_QUEUE_CAPACITY - stream->count;
    }
    base_sync_mutex_unlock(stream->lock);
    return status;
}

lib_status lib_audio_pcm_stream_progress(lib_audio_pcm_stream *stream,
    lib_audio_pcm_progress *out_progress)
{
    lib_status status;
    if (out_progress == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_progress, 0, sizeof(*out_progress));
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(stream->lock);
    status = stream->failure;
    if (status == LIB_STATUS_OK) *out_progress = stream->progress;
    base_sync_mutex_unlock(stream->lock);
    return status;
}

lib_status lib_audio_pcm_stream_set_notify(lib_audio_pcm_stream *stream,
    lib_audio_pcm_notify notify, void *context)
{
    lib_status status;
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(stream->lock);
    stream->notify = notify;
    stream->notify_context = context;
    if (notify != LIB_NULL) audio_pcm_stream_notify_locked(stream);
    status = notify == LIB_NULL ? LIB_STATUS_OK : stream->failure;
    base_sync_mutex_unlock(stream->lock);
    return status;
}

lib_status lib_audio_pcm_stream_wait_writable(lib_audio_pcm_stream *stream)
{
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    for (;;) {
        lib_status status;
        base_sync_wait_result result;

        if (lib_atomic_i32_exchange_explicit(&stream->wait_cancelled, 0,
                LIB_MEMORY_ORDER_ACQ_REL) != 0) return LIB_STATUS_INVALID_STATE;
        base_sync_mutex_lock(stream->lock);
        status = stream->failure;
        if (status == LIB_STATUS_OK && stream->active == LIB_FALSE)
            status = LIB_STATUS_INVALID_STATE;
        if (status == LIB_STATUS_OK &&
            stream->count < AUDIO_PCM_STREAM_QUEUE_CAPACITY) {
            base_sync_mutex_unlock(stream->lock);
            return LIB_STATUS_OK;
        }
        base_sync_mutex_unlock(stream->lock);
        if (status != LIB_STATUS_OK) return status;
        result = base_sync_wait_any(&stream->space, 1u, stream->worker,
            LIB_UINT32_MAX, LIB_NULL);
        if (result != BASE_SYNC_WAIT_SIGNALED) return LIB_STATUS_IO_ERROR;
    }
}

lib_status lib_audio_pcm_stream_cancel_wait(lib_audio_pcm_stream *stream)
{
    lib_status status;
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_atomic_i32_store_explicit(&stream->wait_cancelled, 1,
        LIB_MEMORY_ORDER_RELEASE);
    base_sync_mutex_lock(stream->lock);
    status = audio_pcm_stream_signal_locked(stream, stream->space);
    base_sync_mutex_unlock(stream->lock);
    return status;
}

static lib_status audio_pcm_stream_control_request(lib_audio_pcm_stream *stream,
    audio_pcm_stream_control control, const lib_u16 *gains)
{
    base_sync_wait_result result;
    lib_status status;

    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(stream->lock);
    if (stream->failure != LIB_STATUS_OK) {
        status = stream->failure;
        base_sync_mutex_unlock(stream->lock);
        return status;
    }
    if (control == AUDIO_PCM_STREAM_CONTROL_CLEAR && stream->progress.epoch == LIB_UINT64_MAX) {
        base_sync_mutex_unlock(stream->lock);
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    if (base_sync_event_reset(stream->control_done) != LIB_STATUS_OK) {
        base_sync_mutex_unlock(stream->lock);
        return LIB_STATUS_IO_ERROR;
    }
    if (audio_pcm_stream_platform_cancel_wait(stream->platform) != LIB_STATUS_OK) {
        base_sync_mutex_unlock(stream->lock);
        return LIB_STATUS_IO_ERROR;
    }
    if (control == AUDIO_PCM_STREAM_CONTROL_CLEAR) {
        stream->head = 0u;
        stream->count = 0u;
        ++stream->progress.epoch;
        stream->progress.accepted_frames = 0u;
        stream->progress.submitted_frames = 0u;
        stream->progress.consumed_frames = 0u;
    } else {
        lib_memory_copy(stream->gains, gains,
            stream->channel_count * sizeof(*gains));
    }
    stream->control = control;
    status = audio_pcm_stream_signal_locked(stream, stream->wake);
    base_sync_mutex_unlock(stream->lock);
    if (status != LIB_STATUS_OK) return status;
    result = base_sync_wait_any(&stream->control_done, 1u, stream->worker,
        LIB_UINT32_MAX, LIB_NULL);
    base_sync_mutex_lock(stream->lock);
    status = stream->failure;
    if (status == LIB_STATUS_OK && result != BASE_SYNC_WAIT_SIGNALED)
        status = LIB_STATUS_IO_ERROR;
    if (status == LIB_STATUS_OK && control == AUDIO_PCM_STREAM_CONTROL_CLEAR)
        status = audio_pcm_stream_signal_locked(stream, stream->space);
    base_sync_mutex_unlock(stream->lock);
    return status;
}

lib_status lib_audio_pcm_stream_clear(lib_audio_pcm_stream *stream)
{
    return audio_pcm_stream_control_request(stream,
        AUDIO_PCM_STREAM_CONTROL_CLEAR, LIB_NULL);
}

lib_status lib_audio_pcm_stream_set_gain(lib_audio_pcm_stream *stream,
    const lib_u16 *gains, lib_u32 channel_count)
{
    if (stream == LIB_NULL || gains == LIB_NULL ||
        channel_count != stream->channel_count) return LIB_STATUS_INVALID_ARGUMENT;
    return audio_pcm_stream_control_request(stream, AUDIO_PCM_STREAM_CONTROL_GAIN, gains);
}

lib_status lib_audio_pcm_stream_flush(lib_audio_pcm_stream *stream)
{
    base_sync_wait_result result;
    lib_status status;

    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(stream->lock);
    status = stream->failure;
    if (status != LIB_STATUS_OK || stream->active == LIB_FALSE || stream->count == 0u) {
        base_sync_mutex_unlock(stream->lock);
        return status == LIB_STATUS_OK && stream->active == LIB_FALSE ?
            LIB_STATUS_INVALID_STATE : status;
    }
    if (base_sync_event_reset(stream->control_done) != LIB_STATUS_OK) {
        base_sync_mutex_unlock(stream->lock);
        return LIB_STATUS_IO_ERROR;
    }
    stream->control = AUDIO_PCM_STREAM_CONTROL_FLUSH;
    status = audio_pcm_stream_signal_locked(stream, stream->wake);
    base_sync_mutex_unlock(stream->lock);
    if (status != LIB_STATUS_OK) return status;
    result = base_sync_wait_any(&stream->control_done, 1u, stream->worker,
        LIB_UINT32_MAX, LIB_NULL);
    base_sync_mutex_lock(stream->lock);
    status = stream->failure;
    if (status == LIB_STATUS_OK && result != BASE_SYNC_WAIT_SIGNALED)
        status = LIB_STATUS_IO_ERROR;
    base_sync_mutex_unlock(stream->lock);
    return status;
}

lib_status lib_audio_pcm_stream_set_active(lib_audio_pcm_stream *stream, lib_bool active)
{
    lib_status status;

    if (stream == LIB_NULL || (active != LIB_FALSE && active != LIB_TRUE))
        return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(stream->lock);
    status = stream->failure;
    if (status != LIB_STATUS_OK || active == stream->active) {
        base_sync_mutex_unlock(stream->lock);
        return status;
    }
    base_sync_mutex_unlock(stream->lock);
    if (active == LIB_FALSE) {
        status = lib_audio_pcm_stream_flush(stream);
        if (status != LIB_STATUS_OK) return status;
        status = lib_audio_pcm_stream_clear(stream);
        if (status != LIB_STATUS_OK) return status;
    }
    base_sync_mutex_lock(stream->lock);
    stream->active = active;
    base_sync_mutex_unlock(stream->lock);
    return LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_destroy(lib_audio_pcm_stream **stream)
{
    lib_status status;

    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (*stream == LIB_NULL) return LIB_STATUS_OK;
    (void)lib_audio_pcm_stream_cancel_wait(*stream);
    if ((*stream)->worker != LIB_NULL) {
        status = base_sync_task_request_cancel((*stream)->worker);
        if (status != LIB_STATUS_OK) return status;
        status = audio_pcm_stream_platform_cancel_wait((*stream)->platform);
        if (status != LIB_STATUS_OK) return status;
        (void)base_sync_event_signal((*stream)->wake);
        status = base_sync_task_destroy((*stream)->worker);
        if (status != LIB_STATUS_OK) return status;
        (*stream)->worker = LIB_NULL;
    }
    status = audio_pcm_stream_platform_destroy(&(*stream)->platform);
    if (status != LIB_STATUS_OK) return status;
    base_sync_event_destroy((*stream)->control_done);
    base_sync_event_destroy((*stream)->space);
    base_sync_event_destroy((*stream)->wake);
    base_sync_mutex_destroy((*stream)->lock);
    lib_release(*stream);
    *stream = LIB_NULL;
    return LIB_STATUS_OK;
}
