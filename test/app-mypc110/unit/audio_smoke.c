#include "app-mypc110/pc110_audio.h"

static lib_i16 received[8192];
static lib_u32 received_count;
static lib_u32 clears;
static lib_bool partial = LIB_TRUE;
static lib_bool full;
static lib_bool interrupted;
static lib_bool close_failure;
static lib_bool no_endpoint;

lib_status lib_audio_pcm_stream_create(const lib_audio_pcm_stream_options *options,
    lib_audio_pcm_stream **out_stream)
{
    (void)options;
    if (no_endpoint) { *out_stream = LIB_NULL; return LIB_STATUS_IO_ERROR; }
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
    mypc110_opl2 opl2 = {0};
    device.samples[0] = 0u;
    device.samples[1] = 128u;
    device.samples[2] = 255u;
    device.sample_ticks[0] = 0u;
    device.sample_ticks[1] = 3300u;
    device.sample_ticks[2] = 6600u;
    device.sample_periods[0] = device.sample_periods[1] = device.sample_periods[2] = 3300u;
    device.sample_count = 3u;
    if (mypc110_audio_open(&audio, &device, &opl2) != LIB_STATUS_OK ||
        mypc110_audio_pump(&audio, 9900u) != LIB_STATUS_LIMIT_EXCEEDED ||
        received_count != 1u || mypc110_audio_pump(&audio, 9900u) != LIB_STATUS_OK ||
        received_count != 14u || device.sample_count != 0u) return 1;
    for (lib_u32 i = 0u; i < received_count; ++i)
        if (received[i] != (i < 4u ? -32768 : i < 9u ? 0 : 32512)) return 2;
    device.reset_epoch++;
    if (mypc110_audio_pump(&audio, 9900u) != LIB_STATUS_OK || clears != 1u) return 3;
    device.samples[device.sample_read] = 128u;
    device.sample_periods[device.sample_read] = 3300u;
    device.sample_ticks[device.sample_read] = 9900u;
    device.sample_count = 1u;
    full = LIB_TRUE;
    if (mypc110_audio_pump(&audio, 13200u) != LIB_STATUS_LIMIT_EXCEEDED || audio.count != 4u)
        return 4;
    mypc110_audio_interrupt(&audio);
    if (mypc110_audio_wait(&audio) != LIB_STATUS_INVALID_STATE || audio.count != 4u)
        return 5;
    full = LIB_FALSE;
    if (mypc110_audio_pump(&audio, 13200u) != LIB_STATUS_OK || received_count != 18u) return 6;
    /* FM and DAC share one time axis and one output, rather than serial streams. */
    opl2.registers[0x20u] = opl2.registers[0x23u] = 1u;
    opl2.registers[0x60u] = opl2.registers[0x63u] = 0xf0u;
    opl2.registers[0x80u] = opl2.registers[0x83u] = 0x0fu;
    opl2.registers[0xa0u] = 0x98u;
    opl2.registers[0xb0u] = 0x31u;
    opl2.envelope[0][0] = opl2.envelope[0][1] = 1u;
    if (mypc110_audio_pump(&audio, 66000u) != LIB_STATUS_OK) return 10;
    {
        lib_bool nonzero = LIB_FALSE;
        for (lib_u32 i = 18u; i < received_count; ++i)
            if (received[i] != 0) nonzero = LIB_TRUE;
        if (!nonzero) return 11;
    }
    {
        const lib_u32 before = received_count;
        mypc110_opl2 reference = opl2;
        lib_i16 fm = 0;
        mypc110_opl2_render(&reference, &fm, 1u);
        device.samples[device.sample_read] = 192u;
        device.sample_ticks[device.sample_read] = 66000u;
        device.sample_periods[device.sample_read] = 33000u;
        device.sample_count = 1u;
        if (mypc110_audio_pump(&audio, 99000u) != LIB_STATUS_OK ||
            received_count <= before ||
            received[before] != 16384 + (lib_i32)fm * 45 / 100) return 12;
    }
    {
        const lib_u32 before = received_count;
        mypc110_opl2 reference = opl2;
        lib_i16 old_fm = 0;
        mypc110_opl2_render(&reference, &old_fm, 1u);
        if (mypc110_audio_advance(&audio, 101000u) != LIB_STATUS_OK ||
            audio.count == 0u) return 15;
        opl2.registers[0xa0u] = 0x20u;
        if (mypc110_audio_pump(&audio, 101000u) != LIB_STATUS_OK ||
            received_count <= before || received[before] != (lib_i32)old_fm * 45 / 100)
            return 16;
    }
    {
        const lib_u32 before = received_count;
        const lib_u64 target = audio.next_tick +
            (lib_u64)(MYPC110_AUDIO_FRAME_CAPACITY + 32u) * 688u;
        if (mypc110_audio_advance(&audio, target) != LIB_STATUS_LIMIT_EXCEEDED ||
            audio.count != MYPC110_AUDIO_FRAME_CAPACITY ||
            mypc110_audio_pump(&audio, target) != LIB_STATUS_OK ||
            audio.count != 0u || received_count <= before + MYPC110_AUDIO_FRAME_CAPACITY)
            return 13;
    }
    if (mypc110_audio_active(&audio, LIB_FALSE) != LIB_STATUS_OK) return 7;
    {
        const lib_u32 before = received_count;
        if (mypc110_audio_active(&audio, LIB_TRUE) != LIB_STATUS_OK ||
            mypc110_audio_pump(&audio, 10000000u) != LIB_STATUS_OK ||
            received_count != before ||
            mypc110_audio_pump(&audio, 10033000u) != LIB_STATUS_OK ||
            received_count <= before) return 14;
    }
    close_failure = LIB_TRUE;
    if (mypc110_audio_close(&audio) != LIB_STATUS_IO_ERROR || audio.stream == LIB_NULL)
        return 8;
    close_failure = LIB_FALSE;
    if (mypc110_audio_close(&audio) != LIB_STATUS_OK || audio.stream != LIB_NULL) return 9;
    no_endpoint = LIB_TRUE;
    device.sample_count = 1u;
    if (mypc110_audio_open(&audio, &device, &opl2) != LIB_STATUS_OK ||
        audio.stream != LIB_NULL || mypc110_audio_needs_deadline(&audio, 0u) ||
        mypc110_audio_advance(&audio, 100u) != LIB_STATUS_OK || device.sample_count != 0u ||
        mypc110_audio_close(&audio) != LIB_STATUS_OK) return 17;
    return 0;
}
