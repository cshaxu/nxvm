#ifndef LIB_AUDIO_MIDI_OUTPUT_INTERFACE_H
#define LIB_AUDIO_MIDI_OUTPUT_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct lib_audio_midi_output lib_audio_midi_output;

enum {
    LIB_AUDIO_MIDI_SYSEX_CAPACITY = 4,
    LIB_AUDIO_MIDI_SYSEX_MAX_BYTES = 4096
};

/* Default host MIDI output, not a synthesizer. All calls are serialized by the
 * owner except cancel_wait, which may interrupt a concurrent waiting call.
 * Create returns null on failure; absent native capability is UNSUPPORTED.
 * Host instruments and optional device volume are not stream-local PCM gain. */
lib_status lib_audio_midi_output_create(lib_audio_midi_output **out_output);
/* Complete status plus data, never running status. Ordinary messages wait for
 * preceding SysEx completion; realtime messages may interleave. Native short
 * output itself may block and cannot be interrupted by cancel_wait. */
lib_status lib_audio_midi_output_send_short(lib_audio_midi_output *output,
    const lib_u8 *bytes, lib_u32 count);
/* Copies one complete F0...F7 message. Exhaustion returns LIMIT_EXCEEDED without
 * accepting bytes. Oversize returns UNSUPPORTED, never a truncated message.
 * Accepted data is submitted immediately; no future call is needed to play it. */
lib_status lib_audio_midi_output_send_sysex(lib_audio_midi_output *output,
    const lib_u8 *bytes, lib_u32 count);
lib_status lib_audio_midi_output_query_capacity(lib_audio_midi_output *output,
    lib_u32 *out_free_slots);
/* Event-driven readiness, not playback timing. Cancellation returns
 * INVALID_STATE and does not reclaim buffers still owned by the host. */
lib_status lib_audio_midi_output_wait_writable(lib_audio_midi_output *output);
lib_status lib_audio_midi_output_cancel_wait(lib_audio_midi_output *output);
/* Stops notes, cancels pending messages and terminates interrupted SysEx.
 * Native output errors are terminal; reset is not an automatic reopen. */
lib_status lib_audio_midi_output_reset(lib_audio_midi_output *output);
/* Optional host device volume: 0..65535 per left/right channel. May affect
 * other clients of the same device. UNSUPPORTED does not fault the output. */
lib_status lib_audio_midi_output_get_volume(lib_audio_midi_output *output,
    lib_u16 *out_left, lib_u16 *out_right);
lib_status lib_audio_midi_output_set_volume(lib_audio_midi_output *output,
    lib_u16 left, lib_u16 right);
/* Stops output rather than draining all notes. Null is harmless. Failure
 * retains ownership until native buffers and handle can be safely released. */
lib_status lib_audio_midi_output_destroy(lib_audio_midi_output **output);

#endif
