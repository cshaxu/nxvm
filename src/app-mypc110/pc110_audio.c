#include "app-mypc110/pc110_audio.h"
#include "app-mypc110/pc110_clock.h"

#define MYPC110_AUDIO_RATE 48000u

lib_status mypc110_audio_open(mypc110_audio *audio, mypc110_es488 *device)
{
    lib_status status;
    if (audio == LIB_NULL || device == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *audio = (mypc110_audio){.device = device};
    lib_atomic_i32_initialize(&audio->interrupt_failure, LIB_STATUS_OK);
    status = lib_audio_pcm_stream_create(&(lib_audio_pcm_stream_options){
        .sample_rate = MYPC110_AUDIO_RATE, .channel_count = 1u}, &audio->stream);
    /* Host audio is optional, including on a machine without an endpoint. */
    return status == LIB_STATUS_UNSUPPORTED || status == LIB_STATUS_IO_ERROR ?
        LIB_STATUS_OK : status;
}

lib_status mypc110_audio_pump(mypc110_audio *audio)
{
    mypc110_es488 *device;
    if (audio == LIB_NULL || audio->device == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (lib_atomic_i32_load_explicit(&audio->interrupt_failure, LIB_MEMORY_ORDER_ACQUIRE) !=
        LIB_STATUS_OK) return LIB_STATUS_IO_ERROR;
    if (audio->failure != LIB_STATUS_OK) return audio->failure;
    device = audio->device;
    if (audio->reset_epoch != device->reset_epoch) {
        lib_status status = audio->stream == LIB_NULL ? LIB_STATUS_OK :
            lib_audio_pcm_stream_clear(audio->stream);
        if (status != LIB_STATUS_OK) return status;
        audio->read = audio->count = audio->repeats = audio->phase = 0u;
        audio->reset_epoch = device->reset_epoch;
    }
    if (audio->stream == LIB_NULL) {
        device->sample_read = 0u;
        device->sample_count = 0u;
        return LIB_STATUS_OK;
    }
    for (;;) {
        if (audio->count != 0u) {
            lib_u32 accepted = 0u;
            lib_status status = lib_audio_pcm_stream_enqueue(audio->stream,
                &audio->staging[audio->read], audio->count, &accepted);
            audio->read += accepted;
            audio->count -= accepted;
            if (status != LIB_STATUS_OK) return status;
            if (audio->count != 0u) return LIB_STATUS_LIMIT_EXCEEDED;
        }
        audio->read = 0u;
        while (audio->count < LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION) {
            if (audio->repeats == 0u) {
                lib_u32 numerator;
                if (device->sample_count == 0u) break;
                const lib_u16 index = device->sample_read;
                numerator = audio->phase + (lib_u32)device->sample_periods[index] *
                    MYPC110_AUDIO_RATE;
                audio->repeats = numerator / MYPC110_CPU_CLOCK_HZ;
                audio->phase = numerator % MYPC110_CPU_CLOCK_HZ;
                audio->sample = (lib_i16)(((lib_i32)device->samples[index] - 128) * 256);
                device->sample_read = (index + 1u) % MYPC110_ES488_SAMPLE_CAPACITY;
                --device->sample_count;
                if (audio->repeats == 0u) continue;
            }
            audio->staging[audio->count++] = audio->sample;
            --audio->repeats;
        }
        if (audio->count == 0u) return LIB_STATUS_OK;
    }
}

lib_status mypc110_audio_wait(mypc110_audio *audio)
{
    return audio != LIB_NULL && audio->stream != LIB_NULL ?
        lib_audio_pcm_stream_wait_writable(audio->stream) : LIB_STATUS_OK;
}

lib_status mypc110_audio_active(mypc110_audio *audio, lib_bool enabled)
{
    lib_status status;
    if (audio == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!enabled) {
        audio->count = audio->read = audio->repeats = audio->phase = 0u;
        if (audio->device != LIB_NULL)
            audio->device->sample_count = audio->device->sample_read = 0u;
    }
    if (audio->stream == LIB_NULL) return LIB_STATUS_OK;
    status = lib_audio_pcm_stream_set_active(audio->stream, enabled);
    if (status != LIB_STATUS_OK) audio->failure = status;
    return status;
}

void mypc110_audio_interrupt(mypc110_audio *audio)
{
    if (audio != LIB_NULL && audio->stream != LIB_NULL &&
        lib_audio_pcm_stream_cancel_wait(audio->stream) != LIB_STATUS_OK)
        lib_atomic_i32_store_explicit(&audio->interrupt_failure, LIB_STATUS_IO_ERROR,
            LIB_MEMORY_ORDER_RELEASE);
}

lib_status mypc110_audio_close(mypc110_audio *audio)
{
    if (audio == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return lib_audio_pcm_stream_destroy(&audio->stream);
}
