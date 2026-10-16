#include "lib/types/test.h"
#include "lib/audio/midi/output.c"
#include "lib/audio/midi/linux/output.c"

int main(void)
{
    lib_audio_midi_output *output = (lib_audio_midi_output *)&output;
    lib_u32 count = 9;
    lib_u16 left, right;
    const lib_u8 note[] = {0x90, 60, 127}, sysex[] = {0xf0, 0xf7};
    lib_test_assert(lib_audio_midi_output_create(&output) == LIB_STATUS_UNSUPPORTED && !output);
    lib_test_assert(lib_audio_midi_output_create(LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_send_short(output, note, 3) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_send_sysex(output, sysex, 2) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_query_capacity(output, &count) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_wait_writable(output) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_cancel_wait(output) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_reset(output) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_get_volume(output, &left, &right) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_set_volume(output, 0, 0) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_destroy(&output) == LIB_STATUS_OK);
    return 0;
}
