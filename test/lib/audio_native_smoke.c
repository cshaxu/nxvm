#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "lib/audio/stream_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/types/win32/audio.h"
#include "lib/types/win32/sync.h"


static lib_status wait_until_empty(lib_audio_stream *stream, base_sync_event *delay)
{
    lib_u32 attempt;

    for (attempt = 0u; attempt < 40u; ++attempt) {
        lib_u32 queued = 0u;
        lib_u32 writable = 0u;
        lib_status status = lib_audio_stream_query(stream, &queued, &writable);

        if (status != LIB_STATUS_OK) return status;
        if (queued == 0u && writable == 4096u) return LIB_STATUS_OK;
        lib_test_assert(base_sync_event_wait(delay, 50u) == BASE_SYNC_WAIT_TIMED_OUT);
    }
    return LIB_STATUS_LIMIT_EXCEEDED;
}

typedef struct audio_loopback {
    lib_win32_audio_client *client;
    lib_win32_audio_capture_client *capture;
    lib_win32_wave_format *format;
    lib_win32_handle ready;
    lib_bool initialized;
} audio_loopback;

static const lib_win32_guid audio_device_enumerator_class = {
    0xbcde0395u, 0xe52fu, 0x467cu, { 0x8eu, 0x3du, 0xc4u, 0x57u,
    0x92u, 0x91u, 0x69u, 0x2eu } };
static const lib_win32_guid audio_device_enumerator_interface = {
    0xa95664d2u, 0x9614u, 0x4f35u, { 0xa7u, 0x46u, 0xdeu, 0x8du,
    0xb6u, 0x36u, 0x17u, 0xe6u } };
static const lib_win32_guid audio_client_interface = {
    0x1cb9ad4cu, 0xdbfau, 0x4c32u, { 0xb1u, 0x78u, 0xc2u, 0xf5u,
    0x68u, 0xa7u, 0x03u, 0xb2u } };
static const lib_win32_guid audio_capture_client_interface = {
    0xc8adbd64u, 0xe71eu, 0x48a0u, { 0xa4u, 0xdeu, 0x18u, 0x5cu,
    0x39u, 0x5cu, 0xd3u, 0x17u } };

static lib_bool audio_ok(lib_win32_hresult result)
{ return result >= 0 ? LIB_TRUE : LIB_FALSE; }

static void audio_loopback_close(audio_loopback *loopback)
{
    if (loopback->client != LIB_NULL) {
        (void)lib_win32_audio_client_stop(loopback->client);
        (void)lib_win32_audio_client_reset(loopback->client);
    }
    if (loopback->capture != LIB_NULL)
        (void)lib_win32_audio_capture_client_release(loopback->capture);
    if (loopback->client != LIB_NULL)
        (void)lib_win32_audio_client_release(loopback->client);
    if (loopback->format != LIB_NULL) lib_win32_co_task_mem_free(loopback->format);
    if (loopback->ready != LIB_NULL) (void)lib_win32_close_handle(loopback->ready);
    if (loopback->initialized != LIB_FALSE) lib_win32_co_uninitialize();
    lib_memory_set(loopback, 0, sizeof(*loopback));
}

static lib_bool audio_loopback_open(audio_loopback *loopback)
{
    lib_win32_audio_device_enumerator *enumerator = LIB_NULL;
    lib_win32_audio_device *device = LIB_NULL;

    lib_memory_set(loopback, 0, sizeof(*loopback));
    if (audio_ok(lib_win32_co_initialize_ex(LIB_NULL,
        LIB_WIN32_COINIT_MULTITHREADED)) == LIB_FALSE) return LIB_FALSE;
    loopback->initialized = LIB_TRUE;
    if (audio_ok(lib_win32_co_create_instance(&audio_device_enumerator_class,
        LIB_NULL, LIB_WIN32_CLSCTX_ALL, &audio_device_enumerator_interface,
        (void **)&enumerator)) == LIB_FALSE) goto fail;
    if (audio_ok(lib_win32_audio_device_enumerator_get_default_endpoint(enumerator,
        LIB_WIN32_E_RENDER, LIB_WIN32_E_CONSOLE, &device)) == LIB_FALSE) goto fail;
    if (audio_ok(lib_win32_audio_device_activate(device, &audio_client_interface,
        LIB_WIN32_CLSCTX_ALL, LIB_NULL, (void **)&loopback->client)) == LIB_FALSE)
        goto fail;
    if (audio_ok(lib_win32_audio_client_get_mix_format(loopback->client,
        &loopback->format)) == LIB_FALSE) goto fail;
    if (audio_ok(lib_win32_audio_client_initialize(loopback->client,
        LIB_WIN32_AUDIO_SHARE_MODE_SHARED,
        LIB_WIN32_AUDIO_STREAM_FLAGS_EVENT_CALLBACK |
        LIB_WIN32_AUDIO_STREAM_FLAGS_LOOPBACK, 0, 0, loopback->format,
        LIB_NULL)) == LIB_FALSE) goto fail;
    loopback->ready = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_FALSE,
        LIB_WIN32_FALSE, LIB_NULL);
    if (loopback->ready == LIB_NULL || audio_ok(lib_win32_audio_client_set_event_handle(
        loopback->client, loopback->ready)) == LIB_FALSE) goto fail;
    if (audio_ok(lib_win32_audio_client_get_service(loopback->client,
        &audio_capture_client_interface, (void **)&loopback->capture)) == LIB_FALSE)
        goto fail;
    if (audio_ok(lib_win32_audio_client_start(loopback->client)) == LIB_FALSE) goto fail;
    (void)lib_win32_audio_device_release(device);
    (void)lib_win32_audio_device_enumerator_release(enumerator);
    return LIB_TRUE;

fail:
    if (device != LIB_NULL) (void)lib_win32_audio_device_release(device);
    if (enumerator != LIB_NULL) (void)lib_win32_audio_device_enumerator_release(enumerator);
    audio_loopback_close(loopback);
    return LIB_FALSE;
}

static lib_bool audio_loopback_non_silent(audio_loopback *loopback)
{
    lib_u32 attempt;

    for (attempt = 0u; attempt < 40u; ++attempt) {
        lib_win32_uint32 packet = 0u;

        /* Windows versions before 10 1703 do not signal an event callback
           for a loopback capture client.  This is test-only, bounded packet
           inspection; the production render worker remains event-driven. */
        (void)lib_win32_wait_for_single_object(loopback->ready, 50u);
        do {
            lib_win32_byte *samples = LIB_NULL;
            lib_win32_uint32 frames = 0u;
            lib_win32_dword flags = 0u;
            lib_size bytes;
            lib_size index;

            if (audio_ok(lib_win32_audio_capture_client_get_next_packet_size(
                loopback->capture, &packet)) == LIB_FALSE) return LIB_FALSE;
            if (packet == 0u) break;
            if (audio_ok(lib_win32_audio_capture_client_get_buffer(loopback->capture,
                &samples, &frames, &flags, LIB_NULL, LIB_NULL)) == LIB_FALSE)
                return LIB_FALSE;
            bytes = (lib_size)frames * loopback->format->nBlockAlign;
            if ((flags & LIB_WIN32_AUDIO_BUFFER_FLAGS_SILENT) == 0u) {
                for (index = 0u; index < bytes; ++index) {
                    if (samples[index] != 0u) {
                        (void)lib_win32_audio_capture_client_release_buffer(
                            loopback->capture, frames);
                        return LIB_TRUE;
                    }
                }
            }
            if (audio_ok(lib_win32_audio_capture_client_release_buffer(
                loopback->capture, frames)) == LIB_FALSE) return LIB_FALSE;
        } while (packet != 0u);
    }
    return LIB_FALSE;
}

int main(void)
{
    lib_audio_stream_options options = { 48000u, 1u };
    lib_audio_stream *stream = LIB_NULL;
    lib_i16 tone[LIB_AUDIO_STREAM_MAX_FRAMES_PER_SUBMISSION];
    base_sync_event *delay = LIB_NULL;
    audio_loopback loopback;
    lib_u32 accepted = 0u;
    lib_u32 queued = 0u;
    lib_u32 writable = 0u;
    lib_u32 index;
    lib_u32 phase = 0u;
    lib_status status;

    if (audio_loopback_open(&loopback) == LIB_FALSE) return 0;
    status = lib_audio_stream_create(&options, &stream);
    /* A Remote Desktop session can expose no default render endpoint.  The
     * platform-independent stream contract is covered by audio_stream; this
     * native probe only exercises an endpoint when Windows supplies one. */
    if (status == LIB_STATUS_IO_ERROR) {
        audio_loopback_close(&loopback);
        return 0;
    }
    lib_test_assert(status == LIB_STATUS_OK);
    status = lib_audio_stream_query(stream, &queued, &writable);
    if (status == LIB_STATUS_IO_ERROR) {
        lib_test_assert(lib_audio_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
        audio_loopback_close(&loopback);
        return 0;
    }
    lib_test_assert(status == LIB_STATUS_OK);
    lib_test_assert(queued == 0u && writable == 4096u);
    lib_test_assert(lib_audio_stream_wait_writable(stream) == LIB_STATUS_OK);
    for (index = 0u; index < LIB_AUDIO_STREAM_MAX_FRAMES_PER_SUBMISSION; ++index) {
        tone[index] = phase < 24000u ? 12000 : -12000;
        phase += 439u;
        if (phase >= 48000u) phase -= 48000u;
    }
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET, &delay) == LIB_STATUS_OK);

    /* Exercise two real endpoint runs.  Each run is four native batches: the
       first tests cold creation, the second tests reuse after completion. */
    for (index = 0u; index < 2u; ++index) {
        lib_u32 batch;

        for (batch = 0u; batch < 4u; ++batch) {
            lib_test_assert(lib_audio_stream_enqueue(stream, tone,
                LIB_AUDIO_STREAM_MAX_FRAMES_PER_SUBMISSION, &accepted) == LIB_STATUS_OK);
            lib_test_assert(accepted == LIB_AUDIO_STREAM_MAX_FRAMES_PER_SUBMISSION);
        }
        lib_test_assert(wait_until_empty(stream, delay) == LIB_STATUS_OK);
        lib_test_assert(audio_loopback_non_silent(&loopback) != LIB_FALSE);
    }
    lib_test_assert(wait_until_empty(stream, delay) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
    base_sync_event_destroy(delay);
    audio_loopback_close(&loopback);
    return 0;
}
