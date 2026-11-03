#include "app-mypc110/pc110_audio.h"
#include "app-mypc110/pc110_clock.h"

static void mypc110_audio_step(mypc110_audio *audio)
{
    audio->next_tick += MYPC110_CPU_CLOCK_HZ / MYPC110_AUDIO_SAMPLE_RATE;
    audio->tick_phase += MYPC110_CPU_CLOCK_HZ % MYPC110_AUDIO_SAMPLE_RATE;
    if (audio->tick_phase >= MYPC110_AUDIO_SAMPLE_RATE) {
        ++audio->next_tick;
        audio->tick_phase -= MYPC110_AUDIO_SAMPLE_RATE;
    }
}

static lib_bool mypc110_audio_fm_active(const mypc110_opl2 *opl2)
{
    if (opl2 == LIB_NULL) return LIB_FALSE;
    for (lib_u8 channel = 0u; channel < 9u; ++channel)
        if (opl2->envelope[channel][0] != 0u || opl2->envelope[channel][1] != 0u)
            return LIB_TRUE;
    return LIB_FALSE;
}

lib_bool mypc110_audio_needs_deadline(const mypc110_audio *audio, lib_u64 tick)
{
    return audio != LIB_NULL && audio->stream != LIB_NULL && audio->device != LIB_NULL &&
        (audio->device->playing || audio->device->sample_count != 0u ||
         audio->pcm_until > tick || mypc110_audio_fm_active(audio->opl2));
}

static void mypc110_audio_reset_pending(mypc110_audio *audio)
{
    if (audio->reset_epoch == audio->device->reset_epoch) return;
    audio->read = audio->count = audio->tick_phase = 0u;
    audio->clock_valid = LIB_FALSE;
    audio->pcm_until = 0u;
    audio->reset_epoch = audio->device->reset_epoch;
    audio->clear_pending = LIB_TRUE;
}

lib_status mypc110_audio_open(mypc110_audio *audio, mypc110_es488 *device,
    mypc110_opl2 *opl2)
{
    lib_status status;
    if (audio == LIB_NULL || device == LIB_NULL || opl2 == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *audio = (mypc110_audio){.device = device, .opl2 = opl2};
    lib_atomic_i32_initialize(&audio->interrupt_failure, LIB_STATUS_OK);
    status = lib_audio_pcm_stream_create(&(lib_audio_pcm_stream_options){
        .sample_rate = MYPC110_AUDIO_SAMPLE_RATE, .channel_count = 1u}, &audio->stream);
    return status == LIB_STATUS_UNSUPPORTED || status == LIB_STATUS_IO_ERROR ?
        LIB_STATUS_OK : status;
}

lib_status mypc110_audio_advance(mypc110_audio *audio, lib_u64 tick)
{
    mypc110_es488 *device;
    if (audio == LIB_NULL || audio->device == LIB_NULL || audio->opl2 == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (lib_atomic_i32_load_explicit(&audio->interrupt_failure, LIB_MEMORY_ORDER_ACQUIRE) !=
        LIB_STATUS_OK) return LIB_STATUS_IO_ERROR;
    if (audio->failure != LIB_STATUS_OK) return audio->failure;
    device = audio->device;
    mypc110_audio_reset_pending(audio);
    if (audio->stream == LIB_NULL) {
        device->sample_read = device->sample_count = 0u;
        audio->clock_valid = LIB_FALSE;
        return LIB_STATUS_OK;
    }
    if (!audio->clock_valid) {
        audio->next_tick = device->sample_count != 0u &&
            device->sample_ticks[device->sample_read] < tick ?
            device->sample_ticks[device->sample_read] : tick;
        audio->tick_phase = 0u;
        mypc110_audio_step(audio);
        audio->clock_valid = LIB_TRUE;
    }
    while (audio->next_tick < tick) {
        while (device->sample_count != 0u &&
            device->sample_ticks[device->sample_read] <= audio->next_tick) {
            const lib_u16 index = device->sample_read;
            audio->pcm_sample = (lib_i16)(((lib_i32)device->samples[index] - 128) * 256);
            audio->pcm_until = device->sample_ticks[index] + device->sample_periods[index];
            device->sample_read = (index + 1u) % MYPC110_ES488_SAMPLE_CAPACITY;
            --device->sample_count;
        }
        if (audio->next_tick >= audio->pcm_until && !mypc110_audio_fm_active(audio->opl2)) {
            if (device->sample_count != 0u && device->sample_ticks[device->sample_read] < tick)
                audio->next_tick = device->sample_ticks[device->sample_read];
            else audio->next_tick = tick;
            audio->tick_phase = 0u;
            mypc110_audio_step(audio);
            continue;
        }
        if (audio->count == MYPC110_AUDIO_FRAME_CAPACITY) return LIB_STATUS_LIMIT_EXCEEDED;
        {
            lib_i16 fm = 0;
            lib_i32 mixed;
            if (mypc110_audio_fm_active(audio->opl2))
                mypc110_opl2_render(audio->opl2, &fm, 1u);
            mixed = (audio->next_tick < audio->pcm_until ? audio->pcm_sample : 0) +
                (lib_i32)fm * 45 / 100;
            if (mixed > LIB_INT16_MAX) mixed = LIB_INT16_MAX;
            if (mixed < LIB_INT16_MIN) mixed = LIB_INT16_MIN;
            audio->frames[(audio->read + audio->count) % MYPC110_AUDIO_FRAME_CAPACITY] =
                (lib_i16)mixed;
            ++audio->count;
        }
        mypc110_audio_step(audio);
    }
    return LIB_STATUS_OK;
}

static lib_status mypc110_audio_submit(mypc110_audio *audio)
{
    while (audio->count != 0u) {
        lib_u32 accepted = 0u;
        lib_u32 available = MYPC110_AUDIO_FRAME_CAPACITY - audio->read;
        lib_status status;
        if (available > audio->count) available = audio->count;
        if (available > LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION)
            available = LIB_AUDIO_PCM_STREAM_MAX_FRAMES_PER_SUBMISSION;
        status = lib_audio_pcm_stream_enqueue(audio->stream, &audio->frames[audio->read],
            available, &accepted);
        audio->read = (audio->read + accepted) % MYPC110_AUDIO_FRAME_CAPACITY;
        audio->count -= accepted;
        if (status != LIB_STATUS_OK) return status;
        if (accepted != available) return LIB_STATUS_LIMIT_EXCEEDED;
    }
    return LIB_STATUS_OK;
}

lib_status mypc110_audio_pump(mypc110_audio *audio, lib_u64 tick)
{
    lib_status status;
    if (audio == LIB_NULL || audio->device == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    mypc110_audio_reset_pending(audio);
    if (audio->stream == LIB_NULL) return mypc110_audio_advance(audio, tick);
    if (audio->clear_pending) {
        status = lib_audio_pcm_stream_clear(audio->stream);
        if (status != LIB_STATUS_OK) return status;
        audio->clear_pending = LIB_FALSE;
    }
    for (;;) {
        status = mypc110_audio_submit(audio);
        if (status != LIB_STATUS_OK) return status;
        status = mypc110_audio_advance(audio, tick);
        if (status != LIB_STATUS_OK && status != LIB_STATUS_LIMIT_EXCEEDED) return status;
        if (status == LIB_STATUS_OK) return mypc110_audio_submit(audio);
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
        audio->count = audio->read = audio->tick_phase = 0u;
        audio->clock_valid = LIB_FALSE;
        audio->pcm_until = 0u;
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
