#include "lib/types/test.h"
#include "lib/audio/fm/output_interface.h"
int main(void)
{
    lib_audio_fm_output *fm = (lib_audio_fm_output *)1;
    lib_test_assert(lib_audio_fm_output_create(&fm) == LIB_STATUS_UNSUPPORTED && !fm);
    lib_test_assert(lib_audio_fm_output_create(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    return 0;
}
