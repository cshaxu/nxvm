#include "app-mypc110/pc110_audio.h"

static lib_i16 received[32];
static lib_u32 received_count;
static lib_u32 clears;
static lib_bool partial = LIB_TRUE;
static lib_bool full;
static lib_bool interrupted;
static lib_bool close_failure;

lib_status lib_audio_pcm_stream_create(const lib_audio_pcm_stream_options *options,
    lib_audio_pcm_stream **out_stream)
{
    (void)options;
    *out_stream = (lib_audio_pcm_stream *)&received;
    return LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_enqueue(lib_audio_pcm_stream *stream,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted)
{
    (void)stream;
    if (full) {
        *out_accepted = 0u;
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    *out_accepted = partial && frame_count > 1u ? 1u : frame_count;
    partial = LIB_FALSE;
    for (lib_u32 i = 0u; i < *out_accepted; ++i)
        received[received_count++] = samples[i];
    return LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_wait_writable(lib_audio_pcm_stream *stream)
{ (void)stream; return interrupted ? LIB_STATUS_INVALID_STATE : LIB_STATUS_OK; }
lib_status lib_audio_pcm_stream_cancel_wait(lib_audio_pcm_stream *stream)
{ (void)stream; interrupted = LIB_TRUE; return LIB_STATUS_OK; }
lib_status lib_audio_pcm_stream_set_active(lib_audio_pcm_stream *stream, lib_bool enabled)
{ (void)stream; (void)enabled; return LIB_STATUS_OK; }
lib_status lib_audio_pcm_stream_clear(lib_audio_pcm_stream *stream)
{ (void)stream; ++clears; return LIB_STATUS_OK; }
lib_status lib_audio_pcm_stream_destroy(lib_audio_pcm_stream **stream)
{
    if (close_failure) return LIB_STATUS_IO_ERROR;
    *stream = LIB_NULL;
    return LIB_STATUS_OK;
}

int main(void)
{
    mypc110_es488 device = {0};
    mypc110_audio audio;
    device.samples[0] = 0u;
    device.samples[1] = 128u;
    device.samples[2] = 255u;
    device.sample_periods[0] = device.sample_periods[1] = device.sample_periods[2] = 3300u;
    device.sample_count = 3u;
    if (mypc110_audio_open(&audio, &device) != LIB_STATUS_OK ||
        mypc110_audio_pump(&audio) != LIB_STATUS_LIMIT_EXCEEDED ||
        received_count != 1u || mypc110_audio_pump(&audio) != LIB_STATUS_OK ||
        received_count != 14u || device.sample_count != 0u) return 1;
    for (lib_u32 i = 0u; i < received_count; ++i)
        if (received[i] != (i < 4u ? -32768 : i < 9u ? 0 : 32512)) return 2;
    device.reset_epoch++;
    if (mypc110_audio_pump(&audio) != LIB_STATUS_OK || clears != 1u) return 3;
    device.samples[device.sample_read] = 128u;
    device.sample_periods[device.sample_read] = 3300u;
    device.sample_count = 1u;
    full = LIB_TRUE;
    if (mypc110_audio_pump(&audio) != LIB_STATUS_LIMIT_EXCEEDED || audio.count != 4u)
        return 4;
    mypc110_audio_interrupt(&audio);
    if (mypc110_audio_wait(&audio) != LIB_STATUS_INVALID_STATE || audio.count != 4u)
        return 5;
    full = LIB_FALSE;
    if (mypc110_audio_pump(&audio) != LIB_STATUS_OK || received_count != 18u) return 6;
    if (mypc110_audio_active(&audio, LIB_FALSE) != LIB_STATUS_OK) return 7;
    close_failure = LIB_TRUE;
    if (mypc110_audio_close(&audio) != LIB_STATUS_IO_ERROR || audio.stream == LIB_NULL)
        return 8;
    close_failure = LIB_FALSE;
    if (mypc110_audio_close(&audio) != LIB_STATUS_OK || audio.stream != LIB_NULL) return 9;
    return 0;
}
