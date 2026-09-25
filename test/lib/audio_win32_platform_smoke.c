#include "lib/types/test.h"
#include "lib/types/types_interface.h"
#include "lib/types/win32/audio.h"
#include "lib/types/win32/sync.h"

/* Exercise the real adapter, replacing only its OS calls. No device, mixer,
 * wall-clock deadline or audible signal participates in this unit test. */
static lib_u8 objects[6];
static lib_i16 native_buffer[8], delivered[32];
static lib_u32 channels, expected_rate, delivered_samples, pending_frames;
static lib_u32 acquired, released, initialized, uninitialized, closed;
static lib_u32 started, stopped, reset, waits, padding;
static lib_u32 events_created;
static lib_u32 fail_at, calls;
static lib_win32_dword wait_result;

static lib_win32_hresult result(void)
{ return ++calls == fail_at ? (lib_win32_hresult)-1 : 0; }

static lib_win32_hresult initialize(void *reserved, lib_win32_dword flags)
{
    lib_test_assert(reserved == LIB_NULL && flags == LIB_WIN32_COINIT_MULTITHREADED);
    lib_win32_hresult status = result();
    if (status >= 0) ++initialized;
    return status;
}

static lib_win32_hresult obtain(void **out, lib_size slot)
{
    lib_win32_hresult status = result();
    if (status >= 0) { *out = &objects[slot]; ++acquired; }
    return status;
}

static lib_win32_hresult format_check(const lib_win32_wave_format *format,
    lib_win32_dword flags)
{
    lib_test_assert(format->wFormatTag == LIB_WIN32_WAVE_FORMAT_PCM);
    lib_test_assert(format->nChannels == channels && format->nSamplesPerSec == expected_rate);
    lib_test_assert(format->wBitsPerSample == 16 && format->nBlockAlign == channels * 2);
    lib_test_assert(format->nAvgBytesPerSec == expected_rate * channels * 2 && format->cbSize == 0);
    lib_test_assert(flags == (LIB_WIN32_AUDIO_STREAM_FLAGS_EVENT_CALLBACK |
        LIB_WIN32_AUDIO_STREAM_FLAGS_AUTOCONVERTPCM | LIB_WIN32_AUDIO_STREAM_FLAGS_SRC_DEFAULT_QUALITY));
    return result();
}

static lib_win32_handle event_create(void)
{ return result() < 0 ? LIB_NULL : &objects[4 + events_created++]; }

static lib_win32_hresult buffer_size(lib_win32_uint32 *frames)
{ *frames = 4; return result(); }

static lib_win32_hresult get_padding(lib_win32_uint32 *frames)
{ *frames = padding; return result(); }

static lib_win32_hresult get_buffer(lib_win32_uint32 frames, lib_win32_byte **out)
{
    lib_test_assert(frames > 0 && frames <= 4);
    lib_win32_hresult status = result();
    if (status >= 0) {
        pending_frames = frames;
        lib_memory_set(native_buffer, 0x5a, sizeof(native_buffer));
        *out = (lib_win32_byte *)native_buffer;
    }
    return status;
}

static lib_win32_hresult release_buffer(lib_win32_uint32 frames, lib_win32_dword flags)
{
    lib_test_assert(frames == pending_frames);
    lib_win32_hresult status = result();
    if (status >= 0 && flags == 0) {
        lib_test_assert(delivered_samples + frames * channels <= 32);
        lib_memory_copy(delivered + delivered_samples, native_buffer,
            frames * channels * sizeof(*delivered));
        delivered_samples += frames * channels;
    } else lib_test_assert(flags == 0 || flags == LIB_WIN32_AUDIO_BUFFER_FLAGS_SILENT);
    pending_frames = 0;
    return status;
}

static lib_win32_dword wait_events(lib_win32_dword count,
    const lib_win32_handle *events, lib_win32_bool all, lib_win32_dword timeout)
{
    lib_test_assert(count == 2 && events[0] != LIB_NULL && events[1] != LIB_NULL);
    lib_test_assert(events[0] == &objects[5] && events[1] == &objects[4]);
    lib_test_assert(all == LIB_WIN32_FALSE && timeout == LIB_WIN32_INFINITE);
    ++waits;
    padding = 0;
    return wait_result;
}

#undef lib_win32_co_initialize_ex
#define lib_win32_co_initialize_ex initialize
#undef lib_win32_co_uninitialize
#define lib_win32_co_uninitialize() ((void)++uninitialized)
#undef lib_win32_co_create_instance
#define lib_win32_co_create_instance(c,o,f,i,p) ((void)(c),(void)(o),(void)(f),(void)(i),obtain(p,0))
#undef lib_win32_audio_device_enumerator_get_default_endpoint
#define lib_win32_audio_device_enumerator_get_default_endpoint(e,d,r,p) ((void)(e),(void)(d),(void)(r),obtain((void **)(p),1))
#undef lib_win32_audio_device_activate
#define lib_win32_audio_device_activate(d,i,f,p,o) ((void)(d),(void)(i),(void)(f),(void)(p),obtain(o,2))
#undef lib_win32_audio_client_initialize
#define lib_win32_audio_client_initialize(c,m,f,b,p,w,g) ((void)(c),(void)(m),(void)(b),(void)(p),(void)(g),format_check(w,f))
#undef lib_win32_audio_client_get_service
#define lib_win32_audio_client_get_service(c,i,p) ((void)(c),(void)(i),obtain(p,3))
#undef lib_win32_audio_client_get_buffer_size
#define lib_win32_audio_client_get_buffer_size(c,p) ((void)(c),buffer_size(p))
#undef lib_win32_audio_client_get_current_padding
#define lib_win32_audio_client_get_current_padding(c,p) ((void)(c),get_padding(p))
#undef lib_win32_audio_client_set_event_handle
#define lib_win32_audio_client_set_event_handle(c,e) ((void)(c),(void)(e),result())
#undef lib_win32_audio_render_client_get_buffer
#define lib_win32_audio_render_client_get_buffer(c,n,p) ((void)(c),get_buffer(n,p))
#undef lib_win32_audio_render_client_release_buffer
#define lib_win32_audio_render_client_release_buffer(c,n,f) ((void)(c),release_buffer(n,f))
#undef lib_win32_audio_client_start
#define lib_win32_audio_client_start(c) ((void)(c),++started,result())
#undef lib_win32_audio_client_stop
#define lib_win32_audio_client_stop(c) ((void)(c),++stopped,result())
#undef lib_win32_audio_client_reset
#define lib_win32_audio_client_reset(c) ((void)(c),++reset,result())
#undef lib_win32_audio_render_client_release
#define lib_win32_audio_render_client_release(c) ((void)(c),++released)
#undef lib_win32_audio_client_release
#define lib_win32_audio_client_release(c) ((void)(c),++released)
#undef lib_win32_audio_device_release
#define lib_win32_audio_device_release(c) ((void)(c),++released)
#undef lib_win32_audio_device_enumerator_release
#define lib_win32_audio_device_enumerator_release(c) ((void)(c),++released)
#undef lib_win32_create_event_a
#define lib_win32_create_event_a(a,m,s,n) ((void)(a),(void)(m),(void)(s),(void)(n),event_create())
#undef lib_win32_close_handle
#define lib_win32_close_handle(h) ((void)(h),++closed,LIB_WIN32_TRUE)
#undef lib_win32_set_event
#define lib_win32_set_event(h) ((void)(h),LIB_WIN32_TRUE)
#undef lib_win32_reset_event
#define lib_win32_reset_event(h) ((void)(h),LIB_WIN32_TRUE)
#undef lib_win32_wait_for_multiple_objects
#define lib_win32_wait_for_multiple_objects wait_events
#include "lib/audio/win32/stream.c"

static void clear_probe(void)
{
    calls = fail_at = acquired = released = initialized = uninitialized = closed = 0;
    started = stopped = reset = waits = padding = pending_frames = delivered_samples = 0;
    events_created = 0;
    wait_result = LIB_WIN32_WAIT_OBJECT_0;
}

int main(void)
{
    const lib_u32 rates[] = {22050, 44100, 48000};
    lib_i16 samples[18];
    for (lib_i32 i = 0; i < 18; ++i) samples[i] = (lib_i16)(i * 997 - 8000);
    for (lib_size rate = 0; rate < 3; ++rate) for (channels = 1; channels <= 2; ++channels) {
        audio_stream_platform *platform = LIB_NULL;
        lib_u32 accepted;
        expected_rate = rates[rate];
        lib_audio_stream_options options = {expected_rate, channels};
        clear_probe();
        lib_test_assert(audio_stream_platform_create(&options, &platform) == LIB_STATUS_OK);
        lib_test_assert(audio_stream_platform_worker_attach(platform) == LIB_STATUS_OK);
        lib_test_assert(delivered_samples == 0 && started == 1);
        padding = 4;
        lib_test_assert(audio_stream_platform_wait_writable(platform) == LIB_STATUS_OK && waits == 1);
        for (lib_u32 run = 0; run < 2; ++run) {
            delivered_samples = 0;
            padding = 4;
            lib_test_assert(audio_stream_platform_enqueue(platform, samples, 9, &accepted) == LIB_STATUS_OK);
            lib_test_assert(accepted == 9 && delivered_samples == 9 * channels);
            lib_test_assert(lib_memory_compare(samples, delivered, delivered_samples * sizeof(*samples)) == 0);
            lib_test_assert(audio_stream_platform_clear(platform) == LIB_STATUS_OK);
        }
        padding = 4; wait_result = LIB_WIN32_WAIT_OBJECT_0 + 1;
        lib_test_assert(audio_stream_platform_wait_writable(platform) == LIB_STATUS_LIMIT_EXCEEDED);
        padding = 4;
        lib_test_assert(audio_stream_platform_enqueue(platform, samples, 1, &accepted) == LIB_STATUS_LIMIT_EXCEEDED);
        lib_test_assert(accepted == 0);
        padding = 4; wait_result = LIB_WIN32_WAIT_FAILED;
        lib_test_assert(audio_stream_platform_wait_writable(platform) == LIB_STATUS_IO_ERROR);
        /* Padding, GetBuffer and ReleaseBuffer failures must not claim any
         * accepted PCM. Successful retries use the same production path. */
        for (lib_u32 failure = 1; failure <= 3; ++failure) {
            delivered_samples = 0; padding = 0; fail_at = calls + failure;
            lib_test_assert(audio_stream_platform_enqueue(platform, samples, 1, &accepted) == LIB_STATUS_IO_ERROR);
            lib_test_assert(accepted == 0 && delivered_samples == 0);
            fail_at = 0;
            lib_test_assert(audio_stream_platform_enqueue(platform, samples, 1, &accepted) == LIB_STATUS_OK);
            lib_test_assert(accepted == 1 && delivered_samples == channels);
            lib_test_assert(lib_memory_compare(samples, delivered, channels * sizeof(*samples)) == 0);
        }
        lib_test_assert(audio_stream_platform_cancel_wait(platform) == LIB_STATUS_OK);
        audio_stream_platform_worker_detach(platform);
        lib_test_assert(acquired == released && initialized == uninitialized);
        lib_test_assert(started == 3 && stopped == 3 && reset == 3);
        lib_test_assert(audio_stream_platform_destroy(&platform) == LIB_STATUS_OK && platform == LIB_NULL);
        lib_test_assert(closed == events_created && closed == 2);
    }
    /* Every initialization OS call may fail; all already acquired resources
     * must be released, and the platform remains safe to destroy. */
    channels = 1; expected_rate = 48000;
    for (lib_u32 failure = 1; failure <= 13; ++failure) {
        audio_stream_platform *platform = LIB_NULL;
        lib_audio_stream_options options = {expected_rate, channels};
        clear_probe(); fail_at = failure;
        lib_status status = audio_stream_platform_create(&options, &platform);
        if (status == LIB_STATUS_OK) {
            lib_test_assert(audio_stream_platform_worker_attach(platform) == LIB_STATUS_IO_ERROR);
            lib_test_assert(audio_stream_platform_destroy(&platform) == LIB_STATUS_OK);
        } else lib_test_assert(status == LIB_STATUS_IO_ERROR && platform == LIB_NULL);
        lib_test_assert(acquired == released && initialized == uninitialized);
        lib_test_assert(closed == events_created);
    }
    return 0;
}
