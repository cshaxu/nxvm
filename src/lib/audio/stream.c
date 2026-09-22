#include "lib/audio/stream.h"

static lib_bool audio_stream_options_supported(const lib_audio_stream_options *options)
{
    return options->channel_count >= 1u && options->channel_count <= 2u &&
        (options->sample_rate == 22050u || options->sample_rate == 44100u ||
         options->sample_rate == 48000u);
}

static void audio_stream_fail_locked(lib_audio_stream *stream, lib_status status)
{
    if (stream->failure == LIB_STATUS_OK) stream->failure = status;
}

static void audio_stream_copy_head(const lib_audio_stream *stream,
    lib_i16 *destination, lib_u32 frame_count)
{
    lib_u32 index;

    for (index = 0u; index < frame_count; ++index)
        lib_memory_copy(&destination[index * stream->channel_count],
            &stream->samples[((stream->head + index) %
                AUDIO_STREAM_QUEUE_CAPACITY) * stream->channel_count],
            (lib_size)stream->channel_count * sizeof(*destination));
}

/* The worker is the only live task that calls the platform stream.  It keeps
 * a batch in the producer FIFO until waveOut has accepted the whole batch. */
static void audio_stream_worker(void *context, const base_sync_task *task)
{
    lib_audio_stream *stream = context;
    lib_i16 batch[AUDIO_STREAM_PLAY_BATCH * 2u];

    while (!base_sync_task_cancelled(task)) {
        audio_stream_control control = AUDIO_STREAM_CONTROL_NONE;
        lib_bool have_batch = LIB_FALSE;
        lib_u32 batch_count = 0u;
        lib_u32 generation = 0u;
        lib_status status;

        base_sync_mutex_lock(stream->lock);
        if (stream->control == AUDIO_STREAM_CONTROL_CLEAR) {
            stream->control = AUDIO_STREAM_CONTROL_NONE;
            control = AUDIO_STREAM_CONTROL_CLEAR;
        } else if (stream->failure == LIB_STATUS_OK &&
            stream->active != LIB_FALSE &&
            stream->count != 0u && (stream->count >= AUDIO_STREAM_PLAY_BATCH ||
                stream->control == AUDIO_STREAM_CONTROL_FLUSH)) {
            batch_count = stream->count < AUDIO_STREAM_PLAY_BATCH ?
                stream->count : AUDIO_STREAM_PLAY_BATCH;
            audio_stream_copy_head(stream, batch, batch_count);
            generation = stream->generation;
            have_batch = LIB_TRUE;
        }
        base_sync_mutex_unlock(stream->lock);

        if (control == AUDIO_STREAM_CONTROL_CLEAR) {
            status = audio_stream_platform_clear(stream->platform);
            base_sync_mutex_lock(stream->lock);
            if (status != LIB_STATUS_OK) audio_stream_fail_locked(stream, status);
            base_sync_mutex_unlock(stream->lock);
            (void)base_sync_event_signal(stream->control_done);
            continue;
        }
        if (have_batch == LIB_FALSE) {
            (void)base_sync_wait_any(&stream->wake, 1u, task,
                LIB_UINT32_MAX, LIB_NULL);
            continue;
        }
        status = audio_stream_platform_wait_writable(stream->platform);
        if (base_sync_task_cancelled(task)) break;
        if (status == LIB_STATUS_LIMIT_EXCEEDED) continue;
        if (status == LIB_STATUS_OK) {
            lib_u32 accepted = 0u;
            status = audio_stream_platform_enqueue(stream->platform, batch,
                batch_count, &accepted);
            if (status == LIB_STATUS_OK && accepted != batch_count)
                status = LIB_STATUS_IO_ERROR;
        }
        if (status == LIB_STATUS_LIMIT_EXCEEDED) continue;
        if (status != LIB_STATUS_OK) {
            base_sync_mutex_lock(stream->lock);
            audio_stream_fail_locked(stream, status);
            if (stream->control == AUDIO_STREAM_CONTROL_FLUSH) {
                stream->control = AUDIO_STREAM_CONTROL_NONE;
                (void)base_sync_event_signal(stream->control_done);
            }
            base_sync_mutex_unlock(stream->lock);
            (void)base_sync_event_signal(stream->space);
            continue;
        }
        base_sync_mutex_lock(stream->lock);
        if (stream->generation == generation &&
            stream->count >= batch_count) {
            stream->head = (stream->head + batch_count) %
                AUDIO_STREAM_QUEUE_CAPACITY;
            stream->count -= batch_count;
            (void)base_sync_event_signal(stream->space);
            if (stream->control == AUDIO_STREAM_CONTROL_FLUSH && stream->count == 0u) {
                stream->control = AUDIO_STREAM_CONTROL_NONE;
                (void)base_sync_event_signal(stream->control_done);
            }
        }
        base_sync_mutex_unlock(stream->lock);
    }
}

lib_status lib_audio_stream_create(const lib_audio_stream_options *options,
    lib_audio_stream **out_stream)
{
    lib_audio_stream *stream;
    lib_status status;

    if (out_stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_stream = LIB_NULL;
    if (options == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (audio_stream_options_supported(options) == LIB_FALSE)
        return LIB_STATUS_UNSUPPORTED;
    stream = lib_allocate_zero(1u, sizeof(*stream));
    if (stream == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = audio_stream_platform_create(options, &stream->platform);
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
        status = base_sync_task_create(audio_stream_worker, stream, &stream->worker);
    }
    if (status != LIB_STATUS_OK) {
        base_sync_event_destroy(stream->control_done);
        base_sync_event_destroy(stream->space);
        base_sync_event_destroy(stream->wake);
        base_sync_mutex_destroy(stream->lock);
        if (stream->platform != LIB_NULL)
            (void)audio_stream_platform_destroy(&stream->platform);
        lib_release(stream);
        return status;
    }
    *out_stream = stream;
    return LIB_STATUS_OK;
}

lib_status lib_audio_stream_enqueue(lib_audio_stream *stream,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames)
{
    lib_u32 index;

    if (out_accepted_frames == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_accepted_frames = 0u;
    if (stream == LIB_NULL || frame_count > LIB_AUDIO_STREAM_MAX_FRAMES_PER_SUBMISSION ||
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
    *out_accepted_frames = AUDIO_STREAM_QUEUE_CAPACITY - stream->count;
    if (*out_accepted_frames > frame_count) *out_accepted_frames = frame_count;
    for (index = 0u; index < *out_accepted_frames; ++index)
        lib_memory_copy(&stream->samples[((stream->head + stream->count + index) %
                AUDIO_STREAM_QUEUE_CAPACITY) * stream->channel_count],
            &samples[index * stream->channel_count],
            (lib_size)stream->channel_count * sizeof(*samples));
    stream->count += *out_accepted_frames;
    base_sync_mutex_unlock(stream->lock);
    if (*out_accepted_frames != 0u) (void)base_sync_event_signal(stream->wake);
    return *out_accepted_frames == 0u ? LIB_STATUS_LIMIT_EXCEEDED : LIB_STATUS_OK;
}

lib_status lib_audio_stream_query(lib_audio_stream *stream,
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
        *out_writable_frames = AUDIO_STREAM_QUEUE_CAPACITY - stream->count;
    }
    base_sync_mutex_unlock(stream->lock);
    return status;
}

lib_status lib_audio_stream_wait_writable(lib_audio_stream *stream)
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
            stream->count < AUDIO_STREAM_QUEUE_CAPACITY) {
            base_sync_mutex_unlock(stream->lock);
            return LIB_STATUS_OK;
        }
        base_sync_mutex_unlock(stream->lock);
        if (status != LIB_STATUS_OK) return status;
        result = base_sync_event_wait(stream->space, LIB_UINT32_MAX);
        if (result != BASE_SYNC_WAIT_SIGNALED) return LIB_STATUS_IO_ERROR;
    }
}

lib_status lib_audio_stream_cancel_wait(lib_audio_stream *stream)
{
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_atomic_i32_store_explicit(&stream->wait_cancelled, 1,
        LIB_MEMORY_ORDER_RELEASE);
    return base_sync_event_signal(stream->space);
}

lib_status lib_audio_stream_clear(lib_audio_stream *stream)
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
    if (base_sync_event_reset(stream->control_done) != LIB_STATUS_OK) {
        base_sync_mutex_unlock(stream->lock);
        return LIB_STATUS_IO_ERROR;
    }
    stream->head = 0u;
    stream->count = 0u;
    ++stream->generation;
    stream->control = AUDIO_STREAM_CONTROL_CLEAR;
    base_sync_mutex_unlock(stream->lock);
    if (audio_stream_platform_cancel_wait(stream->platform) != LIB_STATUS_OK)
        return LIB_STATUS_IO_ERROR;
    if (base_sync_event_signal(stream->wake) != LIB_STATUS_OK) return LIB_STATUS_IO_ERROR;
    result = base_sync_event_wait(stream->control_done, LIB_UINT32_MAX);
    if (result != BASE_SYNC_WAIT_SIGNALED) return LIB_STATUS_IO_ERROR;
    base_sync_mutex_lock(stream->lock);
    status = stream->failure;
    base_sync_mutex_unlock(stream->lock);
    (void)base_sync_event_signal(stream->space);
    return status;
}

lib_status lib_audio_stream_flush(lib_audio_stream *stream)
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
    stream->control = AUDIO_STREAM_CONTROL_FLUSH;
    base_sync_mutex_unlock(stream->lock);
    if (base_sync_event_signal(stream->wake) != LIB_STATUS_OK) return LIB_STATUS_IO_ERROR;
    result = base_sync_event_wait(stream->control_done, LIB_UINT32_MAX);
    if (result != BASE_SYNC_WAIT_SIGNALED) return LIB_STATUS_IO_ERROR;
    base_sync_mutex_lock(stream->lock);
    status = stream->failure;
    base_sync_mutex_unlock(stream->lock);
    return status;
}

lib_status lib_audio_stream_set_active(lib_audio_stream *stream, lib_bool active)
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
        status = lib_audio_stream_flush(stream);
        if (status != LIB_STATUS_OK) return status;
        status = lib_audio_stream_clear(stream);
        if (status != LIB_STATUS_OK) return status;
    }
    base_sync_mutex_lock(stream->lock);
    stream->active = active;
    base_sync_mutex_unlock(stream->lock);
    return LIB_STATUS_OK;
}

lib_status lib_audio_stream_destroy(lib_audio_stream **stream)
{
    lib_status status;

    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (*stream == LIB_NULL) return LIB_STATUS_OK;
    base_sync_mutex_lock((*stream)->lock);
    status = (*stream)->failure;
    {
        lib_bool tail_only = (*stream)->count != 0u &&
            (*stream)->count < AUDIO_STREAM_PLAY_BATCH;
        base_sync_mutex_unlock((*stream)->lock);
        if (status == LIB_STATUS_OK && tail_only) {
            status = lib_audio_stream_flush(*stream);
            if (status != LIB_STATUS_OK && status != LIB_STATUS_INVALID_STATE) return status;
        }
    }
    (void)lib_audio_stream_cancel_wait(*stream);
    (void)audio_stream_platform_cancel_wait((*stream)->platform);
    (void)base_sync_event_signal((*stream)->wake);
    status = base_sync_task_destroy((*stream)->worker);
    if (status != LIB_STATUS_OK) return status;
    (*stream)->worker = LIB_NULL;
    status = audio_stream_platform_destroy(&(*stream)->platform);
    if (status != LIB_STATUS_OK) return status;
    base_sync_event_destroy((*stream)->control_done);
    base_sync_event_destroy((*stream)->space);
    base_sync_event_destroy((*stream)->wake);
    base_sync_mutex_destroy((*stream)->lock);
    lib_release(*stream);
    *stream = LIB_NULL;
    return LIB_STATUS_OK;
}
