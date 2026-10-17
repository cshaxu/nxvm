#include "lib/types/test.h"
#include "lib/audio/pcm/stream.c"
#include "lib/audio/pcm/linux/stream.c"

int main(void)
{
    lib_audio_pcm_stream_options options = { 44100u, 2u };
    lib_audio_pcm_stream *stream = (void *)&stream;
    audio_pcm_stream_platform *platform = (void *)&platform;
    lib_u32 accepted = 7u;
    lib_u64 submitted = 8u, consumed = 9u;
    lib_test_assert(lib_audio_pcm_stream_create(&options, &stream) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(stream == LIB_NULL);
    lib_test_assert(audio_pcm_stream_platform_create(&options, &platform) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(platform == LIB_NULL);
    lib_test_assert(audio_pcm_stream_platform_create(LIB_NULL, &platform) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_pcm_stream_platform_create(&options, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_pcm_stream_platform_worker_attach(platform) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_pcm_stream_platform_enqueue(platform, LIB_NULL, 1u, &accepted) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(accepted == 0u);
    lib_test_assert(audio_pcm_stream_platform_enqueue(platform, LIB_NULL, 1u, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_pcm_stream_platform_progress(platform, &submitted, &consumed) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(submitted == 0u && consumed == 0u);
    lib_test_assert(audio_pcm_stream_platform_wait_writable(platform) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_pcm_stream_platform_wait_progress(platform) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_pcm_stream_platform_cancel_wait(platform) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_pcm_stream_platform_clear(platform) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(audio_pcm_stream_platform_set_gain(platform, LIB_NULL, 0u) == LIB_STATUS_INVALID_ARGUMENT);
    audio_pcm_stream_platform_worker_detach(platform);
    lib_test_assert(audio_pcm_stream_platform_destroy(&platform) == LIB_STATUS_OK);
    lib_test_assert(platform == LIB_NULL);
    lib_test_assert(audio_pcm_stream_platform_destroy(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    return 0;
}
