#include "lib/audio/stream.h"
#include "lib/types/win32/audio.h"
#include "lib/types/win32/sync.h"

struct audio_stream_platform {
    lib_win32_audio_client *client;
    lib_win32_audio_render_client *renderer;
    lib_win32_handle render_ready;
    lib_win32_handle interruption;
    lib_u32 sample_rate;
    lib_u32 channel_count;
    lib_win32_uint32 buffer_frames;
    lib_bool com_initialized;
    lib_bool attached;
};

static const lib_win32_guid audio_device_enumerator_class = {
    0xbcde0395u, 0xe52fu, 0x467cu, { 0x8eu, 0x3du, 0xc4u, 0x57u,
    0x92u, 0x91u, 0x69u, 0x2eu } };
static const lib_win32_guid audio_device_enumerator_interface = {
    0xa95664d2u, 0x9614u, 0x4f35u, { 0xa7u, 0x46u, 0xdeu, 0x8du,
    0xb6u, 0x36u, 0x17u, 0xe6u } };
static const lib_win32_guid audio_client_interface = {
    0x1cb9ad4cu, 0xdbfau, 0x4c32u, { 0xb1u, 0x78u, 0xc2u, 0xf5u,
    0x68u, 0xa7u, 0x03u, 0xb2u } };
static const lib_win32_guid audio_render_client_interface = {
    0xf294acfcu, 0x3146u, 0x4483u, { 0xa7u, 0xbfu, 0xadu, 0xdcu,
    0xa7u, 0xc2u, 0x60u, 0xe2u } };

static lib_status audio_stream_win32_status(lib_win32_hresult result)
{ return result >= 0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }

static void audio_stream_release_endpoint(audio_stream_platform *platform)
{
    if (platform->renderer != LIB_NULL) {
        (void)lib_win32_audio_render_client_release(platform->renderer);
        platform->renderer = LIB_NULL;
    }
    if (platform->client != LIB_NULL) {
        (void)lib_win32_audio_client_release(platform->client);
        platform->client = LIB_NULL;
    }
    if (platform->render_ready != LIB_NULL) {
        (void)lib_win32_close_handle(platform->render_ready);
        platform->render_ready = LIB_NULL;
    }
    platform->buffer_frames = 0u;
}

static lib_status audio_stream_wait_event(audio_stream_platform *platform)
{
    lib_win32_handle events[2];
    lib_win32_dword result;

    events[0] = platform->render_ready;
    events[1] = platform->interruption;
    result = lib_win32_wait_for_multiple_objects(2u, events, LIB_WIN32_FALSE,
        LIB_WIN32_INFINITE);
    if (result == LIB_WIN32_WAIT_OBJECT_0 + 1u) return LIB_STATUS_LIMIT_EXCEEDED;
    return result == LIB_WIN32_WAIT_OBJECT_0 ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status audio_stream_platform_create(const lib_audio_stream_options *options,
    audio_stream_platform **out_platform)
{
    audio_stream_platform *platform;

    if (options == LIB_NULL || out_platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_platform = LIB_NULL;
    platform = lib_allocate_zero(1u, sizeof(*platform));
    if (platform == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    platform->sample_rate = options->sample_rate;
    platform->channel_count = options->channel_count;
    platform->interruption = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_FALSE,
        LIB_WIN32_FALSE, LIB_NULL);
    if (platform->interruption == LIB_NULL) {
        lib_release(platform);
        return LIB_STATUS_IO_ERROR;
    }
    *out_platform = platform;
    return LIB_STATUS_OK;
}

lib_status audio_stream_platform_worker_attach(audio_stream_platform *platform)
{
    lib_win32_audio_device_enumerator *enumerator = LIB_NULL;
    lib_win32_audio_device *device = LIB_NULL;
    lib_win32_wave_format format;
    lib_status status;

    if (platform == LIB_NULL || platform->attached != LIB_FALSE)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = audio_stream_win32_status(lib_win32_co_initialize_ex(LIB_NULL,
        LIB_WIN32_COINIT_MULTITHREADED));
    if (status != LIB_STATUS_OK) return status;
    platform->com_initialized = LIB_TRUE;
    status = audio_stream_win32_status(lib_win32_co_create_instance(
        &audio_device_enumerator_class, LIB_NULL, LIB_WIN32_CLSCTX_ALL,
        &audio_device_enumerator_interface, (void **)&enumerator));
    if (status != LIB_STATUS_OK) goto fail;
    status = audio_stream_win32_status(
        lib_win32_audio_device_enumerator_get_default_endpoint(enumerator,
            LIB_WIN32_E_RENDER, LIB_WIN32_E_CONSOLE, &device));
    if (status != LIB_STATUS_OK) goto fail;
    status = audio_stream_win32_status(lib_win32_audio_device_activate(device,
        &audio_client_interface, LIB_WIN32_CLSCTX_ALL, LIB_NULL,
        (void **)&platform->client));
    if (status != LIB_STATUS_OK) goto fail;
    lib_memory_set(&format, 0, sizeof(format));
    format.wFormatTag = LIB_WIN32_WAVE_FORMAT_PCM;
    format.nChannels = (lib_win32_word)platform->channel_count;
    format.nSamplesPerSec = platform->sample_rate;
    format.wBitsPerSample = (lib_win32_word)(sizeof(lib_i16) * 8u);
    format.nBlockAlign = (lib_win32_word)(platform->channel_count * sizeof(lib_i16));
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    status = audio_stream_win32_status(lib_win32_audio_client_initialize(platform->client,
        LIB_WIN32_AUDIO_SHARE_MODE_SHARED,
        LIB_WIN32_AUDIO_STREAM_FLAGS_EVENT_CALLBACK |
        LIB_WIN32_AUDIO_STREAM_FLAGS_AUTOCONVERTPCM |
        LIB_WIN32_AUDIO_STREAM_FLAGS_SRC_DEFAULT_QUALITY,
        0, 0, &format, LIB_NULL));
    if (status != LIB_STATUS_OK) goto fail;
    platform->render_ready = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_FALSE,
        LIB_WIN32_FALSE, LIB_NULL);
    if (platform->render_ready == LIB_NULL) { status = LIB_STATUS_IO_ERROR; goto fail; }
    status = audio_stream_win32_status(lib_win32_audio_client_set_event_handle(
        platform->client, platform->render_ready));
    if (status != LIB_STATUS_OK) goto fail;
    status = audio_stream_win32_status(lib_win32_audio_client_get_buffer_size(
        platform->client, &platform->buffer_frames));
    if (status != LIB_STATUS_OK || platform->buffer_frames == 0u) {
        if (status == LIB_STATUS_OK) status = LIB_STATUS_IO_ERROR;
        goto fail;
    }
    status = audio_stream_win32_status(lib_win32_audio_client_get_service(platform->client,
        &audio_render_client_interface, (void **)&platform->renderer));
    if (status != LIB_STATUS_OK) goto fail;
    {
        lib_win32_byte *silence = LIB_NULL;

        status = audio_stream_win32_status(lib_win32_audio_render_client_get_buffer(
            platform->renderer, platform->buffer_frames, &silence));
        if (status != LIB_STATUS_OK) goto fail;
        status = audio_stream_win32_status(
            lib_win32_audio_render_client_release_buffer(platform->renderer,
                platform->buffer_frames, LIB_WIN32_AUDIO_BUFFER_FLAGS_SILENT));
        if (status != LIB_STATUS_OK) goto fail;
    }
    status = audio_stream_win32_status(lib_win32_audio_client_start(platform->client));
    if (status != LIB_STATUS_OK) goto fail;
    (void)lib_win32_audio_device_release(device);
    (void)lib_win32_audio_device_enumerator_release(enumerator);
    platform->attached = LIB_TRUE;
    return LIB_STATUS_OK;

fail:
    if (device != LIB_NULL) (void)lib_win32_audio_device_release(device);
    if (enumerator != LIB_NULL) (void)lib_win32_audio_device_enumerator_release(enumerator);
    audio_stream_release_endpoint(platform);
    lib_win32_co_uninitialize();
    platform->com_initialized = LIB_FALSE;
    return status;
}

void audio_stream_platform_worker_detach(audio_stream_platform *platform)
{
    if (platform == LIB_NULL || platform->com_initialized == LIB_FALSE) return;
    if (platform->client != LIB_NULL) {
        (void)lib_win32_audio_client_stop(platform->client);
        (void)lib_win32_audio_client_reset(platform->client);
    }
    audio_stream_release_endpoint(platform);
    lib_win32_co_uninitialize();
    platform->com_initialized = LIB_FALSE;
    platform->attached = LIB_FALSE;
}

lib_status audio_stream_platform_wait_writable(audio_stream_platform *platform)
{
    lib_win32_uint32 padding;
    lib_status status;

    if (platform == LIB_NULL || platform->attached == LIB_FALSE)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (;;) {
        status = audio_stream_win32_status(lib_win32_audio_client_get_current_padding(
            platform->client, &padding));
        if (status != LIB_STATUS_OK) return status;
        if (padding < platform->buffer_frames) return LIB_STATUS_OK;
        status = audio_stream_wait_event(platform);
        if (status != LIB_STATUS_OK) return status;
    }
}

lib_status audio_stream_platform_cancel_wait(audio_stream_platform *platform)
{
    if (platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return lib_win32_set_event(platform->interruption) == LIB_WIN32_FALSE ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

lib_status audio_stream_platform_enqueue(audio_stream_platform *platform,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted_frames)
{
    lib_u32 submitted = 0u;

    if (platform == LIB_NULL || platform->attached == LIB_FALSE || samples == LIB_NULL ||
        out_accepted_frames == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_accepted_frames = 0u;
    while (submitted < frame_count) {
        lib_win32_uint32 padding;
        lib_win32_uint32 writable;
        lib_win32_byte *destination = LIB_NULL;
        lib_status status = audio_stream_win32_status(
            lib_win32_audio_client_get_current_padding(platform->client, &padding));
        if (status != LIB_STATUS_OK) return status;
        if (padding >= platform->buffer_frames) {
            status = audio_stream_wait_event(platform);
            if (status != LIB_STATUS_OK) return status;
            continue;
        }
        writable = platform->buffer_frames - padding;
        if (writable > frame_count - submitted) writable = frame_count - submitted;
        status = audio_stream_win32_status(lib_win32_audio_render_client_get_buffer(
            platform->renderer, writable, &destination));
        if (status != LIB_STATUS_OK) return status;
        lib_memory_copy(destination, &samples[(lib_size)submitted * platform->channel_count],
            (lib_size)writable * platform->channel_count * sizeof(*samples));
        status = audio_stream_win32_status(lib_win32_audio_render_client_release_buffer(
            platform->renderer, writable, 0u));
        if (status != LIB_STATUS_OK) return status;
        submitted += writable;
        *out_accepted_frames = submitted;
    }
    return LIB_STATUS_OK;
}

lib_status audio_stream_platform_clear(audio_stream_platform *platform)
{
    lib_status status;

    if (platform == LIB_NULL || platform->attached == LIB_FALSE)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (lib_win32_reset_event(platform->interruption) == LIB_WIN32_FALSE)
        return LIB_STATUS_IO_ERROR;
    status = audio_stream_win32_status(lib_win32_audio_client_stop(platform->client));
    if (status != LIB_STATUS_OK) return status;
    status = audio_stream_win32_status(lib_win32_audio_client_reset(platform->client));
    if (status != LIB_STATUS_OK) return status;
    (void)lib_win32_reset_event(platform->render_ready);
    return audio_stream_win32_status(lib_win32_audio_client_start(platform->client));
}

lib_status audio_stream_platform_destroy(audio_stream_platform **platform)
{
    if (platform == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (*platform == LIB_NULL) return LIB_STATUS_OK;
    if ((*platform)->attached != LIB_FALSE) return LIB_STATUS_INVALID_STATE;
    if (lib_win32_close_handle((*platform)->interruption) == LIB_WIN32_FALSE)
        return LIB_STATUS_IO_ERROR;
    (*platform)->interruption = LIB_NULL;
    lib_release(*platform);
    *platform = LIB_NULL;
    return LIB_STATUS_OK;
}
