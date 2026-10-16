#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/audio/midi/output_interface.h"

int main(void)
{
    lib_audio_midi_output *output = LIB_NULL;
    lib_status status = lib_audio_midi_output_create(&output);
    if (status == LIB_STATUS_UNSUPPORTED) {
        lib_test_assert(!output);
        lib_c_printf("LIB native MIDI: capability unavailable (no playback claim)\n");
        return 0;
    }
    lib_test_assert(status == LIB_STATUS_OK && output);
    lib_u32 capacity;
    lib_test_assert(lib_audio_midi_output_query_capacity(output, &capacity) == LIB_STATUS_OK);
    lib_test_assert(capacity == LIB_AUDIO_MIDI_SYSEX_CAPACITY);
    lib_test_assert(lib_audio_midi_output_destroy(&output) == LIB_STATUS_OK && !output);
    lib_c_printf("LIB native MIDI: default output opened and released (no playback claim)\n");
    return 0;
}
