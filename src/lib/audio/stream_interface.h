#ifndef LIB_AUDIO_STREAM_INTERFACE_H
#define LIB_AUDIO_STREAM_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct lib_audio_stream lib_audio_stream;

typedef struct lib_audio_stream_options {
    lib_u32 sample_rate;
    lib_u32 channel_count;
} lib_audio_stream_options;

enum {
    LIB_AUDIO_STREAM_MAX_FRAMES_PER_SUBMISSION = 512u
};

/* PCM frames are interleaved signed 16-bit samples. A stream accepts mono or
 * stereo 22050, 44100 or 48000 Hz PCM without conversion. The library owns a
 * private bounded producer queue and delegates output to its platform backend.
 * The caller owns sample storage and serializes operations on a
 * stream, except that one other thread may call cancel_wait. */
lib_status lib_audio_stream_create(const lib_audio_stream_options *options,
    lib_audio_stream **out_stream);
/* Copies an ordered prefix without waiting. accepted_frames is authoritative:
 * OK may report a partial nonzero prefix; LIMIT_EXCEEDED reports zero. A zero
 * count is a successful no-op and permits a null samples pointer. */
lib_status lib_audio_stream_enqueue(lib_audio_stream *stream,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames);
lib_status lib_audio_stream_query(lib_audio_stream *stream,
    lib_u32 *out_queued_frames, lib_u32 *out_writable_frames);
/* Waits until the producer FIFO accepts at least one frame.  This is a
 * readiness primitive, not guest or device pacing. cancel_wait may interrupt
 * one concurrent wait and causes it to return INVALID_STATE. */
lib_status lib_audio_stream_wait_writable(lib_audio_stream *stream);
lib_status lib_audio_stream_cancel_wait(lib_audio_stream *stream);
/* Submits every accepted FIFO frame, including a private sub-batch tail, in
 * order. It returns only after the backend accepts that tail or the stream
 * reaches a terminal error. clear is the explicit discard operation. */
lib_status lib_audio_stream_flush(lib_audio_stream *stream);
/* Inactive streams reject nonempty enqueue. Disabling clears unsounded native
 * PCM; clear preserves active state. Neither operation retracts heard audio. */
lib_status lib_audio_stream_set_active(lib_audio_stream *stream, lib_bool active);
lib_status lib_audio_stream_clear(lib_audio_stream *stream);
/* Null is idempotent. Destruction submits a sole queued sub-batch tail before
 * teardown; once a full batch is awaiting a native backend, destruction
 * cancels that wait and discards remaining FIFO data. Use flush before destroy
 * when delivery of every accepted frame is required. A destroy failure retains
 * the stream for another cleanup attempt and never releases storage that the
 * platform may still own. */
lib_status lib_audio_stream_destroy(lib_audio_stream **stream);

#endif
