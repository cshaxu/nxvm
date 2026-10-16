#include "lib/types/test.h"
#include "lib/types/win32/audio.h"
#include "lib/types/win32/sync.h"
#include "lib/audio/midi/output.c"

static lib_u8 handles[3];
static lib_bool signals[2], synchronous, fail_wait, fail_signal;
static lib_u32 event_count, closed_events, waits, short_count, long_count, resets, closes;
static lib_u32 eox_count, prepared, unprepared, pending_count;
static lib_u32 fail_event;
static lib_win32_midi_header *pending[LIB_AUDIO_MIDI_SYSEX_CAPACITY];
static lib_win32_mmresult open_error, prepare_error, long_error, short_error;
static lib_win32_mmresult reset_error, unprepare_error, close_error, volume_error;
static lib_win32_dword volume, last_short;

static lib_win32_handle event_create(void)
{
    ++event_count;
    return event_count == fail_event ? LIB_NULL : &handles[event_count - 1];
}

static lib_win32_bool event_close(lib_win32_handle handle)
{
    lib_test_assert(handle == &handles[0] || handle == &handles[1]);
    ++closed_events;
    return LIB_WIN32_TRUE;
}

static lib_win32_bool event_signal(lib_win32_handle handle)
{
    if (fail_signal) return LIB_WIN32_FALSE;
    lib_test_assert(handle == &handles[1]);
    signals[1] = LIB_TRUE;
    return LIB_WIN32_TRUE;
}

static void complete(void)
{
    for (lib_u32 i = 0; i < pending_count; ++i) {
        pending[i]->dwFlags &= ~LIB_WIN32_MIDI_IN_QUEUE;
        pending[i]->dwFlags |= LIB_WIN32_MIDI_DONE;
    }
    pending_count = 0;
    signals[0] = LIB_TRUE;
}

static lib_win32_dword wait_events(lib_win32_dword count,
    const lib_win32_handle *events, lib_win32_bool all, lib_win32_dword timeout)
{
    lib_test_assert(count == 2 && events[0] == &handles[1] && events[1] == &handles[0]);
    lib_test_assert(!all && timeout == LIB_WIN32_INFINITE);
    ++waits;
    if (fail_wait) return LIB_WIN32_WAIT_FAILED;
    if (signals[1]) { signals[1] = LIB_FALSE; return LIB_WIN32_WAIT_OBJECT_0; }
    /* A host completion event, not a producer call, drains the accepted tail. */
    complete();
    signals[0] = LIB_FALSE;
    return LIB_WIN32_WAIT_OBJECT_0 + 1;
}

static lib_win32_mmresult open_output(lib_win32_midi_output *handle,
    lib_win32_uint device, lib_win32_dword_ptr event, lib_win32_dword_ptr instance,
    lib_win32_dword flags)
{
    lib_test_assert(device == LIB_WIN32_MIDI_MAPPER && !instance);
    lib_test_assert(event == (lib_win32_dword_ptr)&handles[0] && flags == LIB_WIN32_CALLBACK_EVENT);
    if (!open_error) *handle = (lib_win32_midi_output)&handles[2];
    return open_error;
}

static lib_win32_mmresult prepare_header(lib_win32_midi_output handle,
    lib_win32_midi_header *header, lib_win32_uint size)
{
    lib_test_assert(handle == (lib_win32_midi_output)&handles[2] && size == sizeof(*header));
    if (prepare_error) return prepare_error;
    lib_test_assert(header->lpData && header->dwBufferLength && !header->dwFlags);
    header->dwFlags = LIB_WIN32_MIDI_PREPARED;
    ++prepared;
    return 0;
}

static lib_win32_mmresult unprepare_header(lib_win32_midi_output handle,
    lib_win32_midi_header *header, lib_win32_uint size)
{
    (void)handle;
    lib_test_assert(size == sizeof(*header) && !(header->dwFlags & LIB_WIN32_MIDI_IN_QUEUE));
    if (unprepare_error) return unprepare_error;
    header->dwFlags &= ~LIB_WIN32_MIDI_PREPARED;
    ++unprepared;
    return 0;
}

static lib_win32_mmresult send_long(lib_win32_midi_output handle,
    lib_win32_midi_header *header, lib_win32_uint size)
{
    (void)handle;
    lib_test_assert(size == sizeof(*header) && header->dwFlags == LIB_WIN32_MIDI_PREPARED);
    if (long_error) return long_error;
    ++long_count;
    if (header->dwBufferLength == 1) {
        lib_test_assert((lib_u8)header->lpData[0] == 0xf7);
        ++eox_count;
    }
    header->dwFlags |= LIB_WIN32_MIDI_IN_QUEUE;
    lib_test_assert(pending_count < LIB_AUDIO_MIDI_SYSEX_CAPACITY);
    pending[pending_count++] = header;
    if (synchronous) complete();
    return 0;
}

static lib_win32_mmresult send_short(lib_win32_midi_output handle, lib_win32_dword message)
{
    (void)handle;
    if ((message & 0xff) < 0xf8) lib_test_assert(pending_count == 0);
    last_short = message;
    ++short_count;
    return short_error;
}

static lib_win32_mmresult reset_output(lib_win32_midi_output handle)
{
    (void)handle;
    ++resets;
    if (!reset_error) complete();
    return reset_error;
}

static lib_win32_mmresult close_output(lib_win32_midi_output handle)
{
    (void)handle;
    lib_test_assert(!pending_count && prepared == unprepared);
    if (!close_error) ++closes;
    return close_error;
}

static lib_win32_mmresult get_volume(lib_win32_midi_output handle, lib_win32_dword *out)
{ (void)handle; if (!volume_error) *out = volume; return volume_error; }
static lib_win32_mmresult set_volume(lib_win32_midi_output handle, lib_win32_dword value)
{ (void)handle; if (!volume_error) volume = value; return volume_error; }

#undef lib_win32_create_event_a
#undef lib_win32_close_handle
#undef lib_win32_set_event
#undef lib_win32_wait_for_multiple_objects
#undef lib_win32_midi_out_open
#undef lib_win32_midi_out_prepare
#undef lib_win32_midi_out_unprepare
#undef lib_win32_midi_out_long
#undef lib_win32_midi_out_short
#undef lib_win32_midi_out_reset
#undef lib_win32_midi_out_close
#undef lib_win32_midi_out_get_volume
#undef lib_win32_midi_out_set_volume
#define lib_win32_create_event_a(a,b,c,d) event_create()
#define lib_win32_close_handle event_close
#define lib_win32_set_event event_signal
#define lib_win32_wait_for_multiple_objects wait_events
#define lib_win32_midi_out_open open_output
#define lib_win32_midi_out_prepare prepare_header
#define lib_win32_midi_out_unprepare unprepare_header
#define lib_win32_midi_out_long send_long
#define lib_win32_midi_out_short send_short
#define lib_win32_midi_out_reset reset_output
#define lib_win32_midi_out_close close_output
#define lib_win32_midi_out_get_volume get_volume
#define lib_win32_midi_out_set_volume set_volume
#include "lib/audio/midi/win32/output.c"

static lib_audio_midi_output *create(void)
{
    event_count = closed_events = prepared = unprepared = 0;
    signals[0] = signals[1] = LIB_FALSE;
    lib_audio_midi_output *output = LIB_NULL;
    lib_test_assert(lib_audio_midi_output_create(&output) == LIB_STATUS_OK && output);
    return output;
}

static void destroy(lib_audio_midi_output **output)
{
    lib_test_assert(lib_audio_midi_output_destroy(output) == LIB_STATUS_OK && !*output);
    lib_test_assert(closed_events == 2 && prepared == unprepared);
}

int main(void)
{
    lib_audio_midi_output *output = (lib_audio_midi_output *)handles;
    open_error = LIB_WIN32_MIDI_NO_DEVICE;
    lib_test_assert(lib_audio_midi_output_create(&output) == LIB_STATUS_UNSUPPORTED && !output);
    lib_test_assert(closed_events == 2);
    open_error = 0;
    for (fail_event = 1; fail_event <= 2; ++fail_event) {
        event_count = closed_events = 0;
        lib_test_assert(lib_audio_midi_output_create(&output) == LIB_STATUS_IO_ERROR && !output);
        lib_test_assert(closed_events == 1);
    }
    fail_event = 0;
    output = create();
    lib_u8 sysex[] = {0xf0, 0x7d, 1, 0xf7};
    const lib_u8 note[] = {0x90, 60, 127}, clock[] = {0xf8};
    lib_u32 capacity;
    fail_signal = LIB_TRUE;
    lib_test_assert(lib_audio_midi_output_cancel_wait(output) == LIB_STATUS_IO_ERROR);
    fail_signal = LIB_FALSE;
    lib_test_assert(lib_audio_midi_output_send_short(output, note, 2) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(lib_audio_midi_output_send_sysex(output, sysex, 3) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(!short_count && !long_count);
    for (lib_u32 i = 0; i < 4; ++i)
        lib_test_assert(lib_audio_midi_output_send_sysex(output, sysex, sizeof(sysex)) == LIB_STATUS_OK);
    sysex[2] = 9;
    for (lib_u32 i = 0; i < pending_count; ++i) lib_test_assert(pending[i]->lpData[2] == 1);
    lib_test_assert(lib_audio_midi_output_send_sysex(output, sysex, sizeof(sysex)) == LIB_STATUS_LIMIT_EXCEEDED);
    lib_test_assert(lib_audio_midi_output_query_capacity(output, &capacity) == LIB_STATUS_OK && !capacity);
    lib_test_assert(lib_audio_midi_output_send_short(output, clock, 1) == LIB_STATUS_OK && pending_count == 4);
    lib_test_assert(last_short == 0xf8 && !waits);
    lib_test_assert(lib_audio_midi_output_cancel_wait(output) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_midi_output_wait_writable(output) == LIB_STATUS_INVALID_STATE && pending_count == 4);
    lib_test_assert(lib_audio_midi_output_send_short(output, note, 3) == LIB_STATUS_OK);
    lib_test_assert(last_short == 0x7f3c90 && pending_count == 0 && waits == 2);
    lib_test_assert(lib_audio_midi_output_query_capacity(output, &capacity) == LIB_STATUS_OK && capacity == 4);
    lib_test_assert(lib_audio_midi_output_send_sysex(output, sysex, sizeof(sysex)) == LIB_STATUS_OK);
    lib_test_assert(lib_audio_midi_output_reset(output) == LIB_STATUS_OK && eox_count == 1);
    lib_test_assert(!pending_count && prepared == unprepared);
    lib_test_assert(lib_audio_midi_output_set_volume(output, 123, 456) == LIB_STATUS_OK);
    lib_u16 left = 0, right = 0;
    lib_test_assert(lib_audio_midi_output_get_volume(output, &left, &right) == LIB_STATUS_OK && left == 123 && right == 456);
    volume_error = LIB_WIN32_MM_NOT_SUPPORTED;
    lib_test_assert(lib_audio_midi_output_set_volume(output, 0, 0) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(lib_audio_midi_output_get_volume(output, &left, &right) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(lib_audio_midi_output_send_short(output, clock, 1) == LIB_STATUS_OK);
    volume_error = 0;
    synchronous = LIB_TRUE;
    for (lib_u32 i = 0; i < 10; ++i)
        lib_test_assert(lib_audio_midi_output_send_sysex(output, sysex, sizeof(sysex)) == LIB_STATUS_OK);
    destroy(&output);
    synchronous = LIB_FALSE;

    /* Every native failure leaves still-owned headers alive for cleanup. */
    lib_win32_mmresult *failures[] = {&prepare_error, &long_error, &short_error,
        &reset_error, &unprepare_error, &close_error, &volume_error};
    for (lib_u32 i = 0; i < sizeof(failures) / sizeof(failures[0]); ++i) {
        output = create();
        lib_status status;
        if (i == 4) {
            lib_test_assert(lib_audio_midi_output_send_sysex(output, sysex, sizeof(sysex)) == LIB_STATUS_OK);
            complete();
        }
        *failures[i] = 1;
        if (i < 2) status = lib_audio_midi_output_send_sysex(output, sysex, sizeof(sysex));
        else if (i == 2) status = lib_audio_midi_output_send_short(output, note, 3);
        else if (i == 3) status = lib_audio_midi_output_reset(output);
        else if (i == 4) status = lib_audio_midi_output_query_capacity(output, &capacity);
        else if (i == 5) status = lib_audio_midi_output_destroy(&output);
        else status = lib_audio_midi_output_set_volume(output, 0, 0);
        lib_test_assert(status == LIB_STATUS_IO_ERROR && output);
        lib_test_assert(lib_audio_midi_output_send_short(output, note, 3) == LIB_STATUS_IO_ERROR);
        *failures[i] = 0;
        destroy(&output);
    }
    output = create();
    for (lib_u32 i = 0; i < 4; ++i)
        lib_test_assert(lib_audio_midi_output_send_sysex(output, sysex, sizeof(sysex)) == LIB_STATUS_OK);
    fail_wait = LIB_TRUE;
    lib_test_assert(lib_audio_midi_output_wait_writable(output) == LIB_STATUS_IO_ERROR);
    fail_wait = LIB_FALSE;
    destroy(&output);
    lib_test_assert(lib_audio_midi_output_destroy(&output) == LIB_STATUS_OK);
    return 0;
}
