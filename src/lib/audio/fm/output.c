#include "lib/audio/fm/output_interface.h"

lib_status lib_audio_fm_output_create(lib_audio_fm_output **out_output)
{
    if (!out_output) return LIB_STATUS_INVALID_ARGUMENT;
    *out_output = LIB_NULL;
    return LIB_STATUS_UNSUPPORTED;
}
