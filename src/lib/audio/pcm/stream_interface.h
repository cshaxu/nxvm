#ifndef LIB_AUDIO_PCM_STREAM_INTERFACE_H
#define LIB_AUDIO_PCM_STREAM_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct lib_audio_pcm_stream lib_audio_pcm_stream;

typedef struct lib_audio_pcm_progress {
    lib_u64 epoch;
    lib_u64 accepted_frames;
    lib_u64 submitted_frames;
    lib_u64 consumed_frames;
} lib_audio_pcm_progress;

typedef lib_status (*lib_audio_pcm_notify)(void *context);

typedef struct lib_audio_pcm_stream_options {
    lib_u32 sample_rate;
    lib_u32 channel_count;
} lib_audio_pcm_stream_options;

enum {
    LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION = 512u
};

/* PCM frames are interleaved signed 16-bit samples. A stream accepts mono or
 * stereo PCM at a nonzero sample rate; unsupported platform formats return
 * UNSUPPORTED. Create waits for native
 * initialization and returns no stream on failure. If creation rollback cannot
 * confirm worker exit, the process terminates without returning a half object.
 * The library owns a
 * private bounded producer queue and delegates output to its platform backend.
 * The caller owns sample storage and serializes operations on a
 * stream, except that one other thread may call cancel_wait. */
lib_status lib_audio_pcm_stream_create(const lib_audio_pcm_stream_options *options,
    lib_audio_pcm_stream **out_stream);
/* Copies an ordered prefix without waiting. accepted_frames is authoritative:
 * OK may report a partial nonzero prefix; LIMIT_EXCEEDED reports zero. A zero
 * count is a successful no-op and permits a null samples pointer. */
lib_status lib_audio_pcm_stream_enqueue(lib_audio_pcm_stream *stream,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames);
lib_status lib_audio_pcm_stream_query(lib_audio_pcm_stream *stream,
    lib_u32 *out_queued_frames, lib_u32 *out_writable_frames);
/* Copied counts in input frames for one epoch: accepted is copied into the
 * stream, submitted is accepted by the playback backend, consumed is reported
 * as processed by that backend, not proof of audible sound. Within an epoch,
 * 0 <= consumed <= submitted <= accepted; counts never decrease. Clear starts
 * a new epoch with zero counts, cancelling rather than completing old ranges. */
lib_status lib_audio_pcm_stream_progress(lib_audio_pcm_stream *stream,
    lib_audio_pcm_progress *out_progress);
/* Optional coalescing wake, not a callback carrying PCM or guest work. It runs
 * under the stream lock, must be brief and must not reenter stream operations.
 * Registering reports current readiness once; failure is terminal. Replacing
 * or removing the sink synchronously quiesces its previous borrowed context.
 * A null notify unregisters even on a failed stream. */
lib_status lib_audio_pcm_stream_set_notify(lib_audio_pcm_stream *stream,
    lib_audio_pcm_notify notify, void *context);
/* Waits until the producer FIFO accepts at least one frame.  This is a
 * readiness primitive, not guest or device pacing. cancel_wait may interrupt
 * one concurrent wait and causes it to return INVALID_STATE. */
lib_status lib_audio_pcm_stream_wait_writable(lib_audio_pcm_stream *stream);
lib_status lib_audio_pcm_stream_cancel_wait(lib_audio_pcm_stream *stream);
/* The worker submits even short prefixes without a subsequent producer call.
 * Flush waits for every accepted FIFO frame, including a sub-batch tail, in
 * order. It returns only after the backend accepts that tail or the stream
 * reaches a terminal error. It does not wait for consumption or audible sound.
 * clear is the explicit discard operation. */
lib_status lib_audio_pcm_stream_flush(lib_audio_pcm_stream *stream);
/* Inactive streams reject nonempty enqueue. Disabling flushes then clears
 * pending playback; clear preserves active state. Neither retracts heard audio. */
lib_status lib_audio_pcm_stream_set_active(lib_audio_pcm_stream *stream, lib_bool active);
lib_status lib_audio_pcm_stream_clear(lib_audio_pcm_stream *stream);
/* Applies one gain per input channel on the native worker, including while inactive.
 * Count must match creation; 0 is silent and 65535 is unity. Values are copied
 * for this synchronous request. No endpoint/session-wide volume is changed.
 * Gain applies to subsequent backend submissions, not already submitted frames.
 * Native failure makes the stream terminal, like submission failure. */
lib_status lib_audio_pcm_stream_set_gain(lib_audio_pcm_stream *stream,
    const lib_u16 *gains, lib_u32 channel_count);
/* Null is idempotent. Destruction cancels waits and discards pending playback,
 * independent of batch size. Flush first if all accepted frames must reach
 * the backend; neither flush nor destruction promises audible completion.
 * Failure retains ownership and never frees storage still in use. */
lib_status lib_audio_pcm_stream_destroy(lib_audio_pcm_stream **stream);

#endif
