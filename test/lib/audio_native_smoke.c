#include "lib/audio/stream_interface.h"

#include <assert.h>

int main(void)
{
    lib_audio_stream_options options = { 48000u, 1u };
    lib_audio_stream *stream = LIB_NULL;
    lib_i16 silence[32] = { 0 };
    lib_u32 accepted = 0u;
    lib_u32 queued = 0u;
    lib_u32 writable = 0u;
    lib_status status;

    status = lib_audio_stream_create(&options, &stream);
    /* A Remote Desktop session can expose no 32-bit waveOut endpoint.  The
     * platform-independent stream contract is covered by audio_stream; this
     * native probe only exercises an endpoint when Windows supplies one. */
    if (status == LIB_STATUS_IO_ERROR) return 0;
    assert(status == LIB_STATUS_OK);
    assert(lib_audio_stream_query(stream, &queued, &writable) == LIB_STATUS_OK);
    assert(queued == 0u && writable == 4096u);
    assert(lib_audio_stream_wait_writable(stream) == LIB_STATUS_OK);
    assert(lib_audio_stream_enqueue(stream, silence, 32u, &accepted) == LIB_STATUS_OK);
    assert(accepted == 32u);
    assert(lib_audio_stream_clear(stream) == LIB_STATUS_OK);
    assert(lib_audio_stream_query(stream, &queued, &writable) == LIB_STATUS_OK);
    assert(queued == 0u && writable == 4096u);
    assert(lib_audio_stream_destroy(&stream) == LIB_STATUS_OK && stream == LIB_NULL);
    return 0;
}
