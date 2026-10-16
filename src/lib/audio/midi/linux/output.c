#include "lib/audio/midi/output.h"

/* No host MIDI transport is selected on this platform. No successful create
 * can produce a handle; the remaining entries still validate null arguments. */
lib_status lib_audio_midi_output_create(lib_audio_midi_output **out_output)
{
    if (!out_output) return LIB_STATUS_INVALID_ARGUMENT;
    *out_output = LIB_NULL;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status lib_audio_midi_output_send_short(lib_audio_midi_output *output,
    const lib_u8 *bytes, lib_u32 count)
{
    lib_u32 packed;
    if (!output) return LIB_STATUS_INVALID_ARGUMENT;
    lib_status status = audio_midi_short_pack(bytes, count, &packed);
    return status == LIB_STATUS_OK ? LIB_STATUS_UNSUPPORTED : status;
}

lib_status lib_audio_midi_output_send_sysex(lib_audio_midi_output *output,
    const lib_u8 *bytes, lib_u32 count)
{
    if (!output) return LIB_STATUS_INVALID_ARGUMENT;
    lib_status status = audio_midi_sysex_validate(bytes, count);
    return status == LIB_STATUS_OK ? LIB_STATUS_UNSUPPORTED : status;
}

lib_status lib_audio_midi_output_query_capacity(lib_audio_midi_output *output,
    lib_u32 *out_free_slots)
{
    if (!output || !out_free_slots) return LIB_STATUS_INVALID_ARGUMENT;
    *out_free_slots = 0;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status lib_audio_midi_output_wait_writable(lib_audio_midi_output *output)
{ return output ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_INVALID_ARGUMENT; }

lib_status lib_audio_midi_output_cancel_wait(lib_audio_midi_output *output)
{ return output ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_INVALID_ARGUMENT; }

lib_status lib_audio_midi_output_reset(lib_audio_midi_output *output)
{ return output ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_INVALID_ARGUMENT; }

lib_status lib_audio_midi_output_get_volume(lib_audio_midi_output *output,
    lib_u16 *out_left, lib_u16 *out_right)
{
    if (!output || !out_left || !out_right) return LIB_STATUS_INVALID_ARGUMENT;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status lib_audio_midi_output_set_volume(lib_audio_midi_output *output,
    lib_u16 left, lib_u16 right)
{
    (void)left;
    (void)right;
    return output ? LIB_STATUS_UNSUPPORTED : LIB_STATUS_INVALID_ARGUMENT;
}

lib_status lib_audio_midi_output_destroy(lib_audio_midi_output **output)
{
    if (!output || !*output) return LIB_STATUS_OK;
    return LIB_STATUS_INVALID_ARGUMENT;
}
