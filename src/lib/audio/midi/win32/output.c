#include "lib/audio/midi/output.h"
#include "lib/types/win32/audio.h"
#include "lib/types/win32/sync.h"

typedef struct audio_midi_slot {
    lib_win32_midi_header header;
    lib_u8 bytes[LIB_AUDIO_MIDI_SYSEX_MAX_BYTES];
} audio_midi_slot;

struct lib_audio_midi_output {
    lib_win32_midi_output handle;
    lib_win32_handle completed;
    lib_win32_handle cancelled;
    audio_midi_slot slots[LIB_AUDIO_MIDI_SYSEX_CAPACITY];
    lib_status failure;
};

static lib_status audio_midi_result(lib_win32_mmresult result)
{
    if (result == LIB_WIN32_MM_OK) return LIB_STATUS_OK;
    if (result == LIB_WIN32_MM_NOT_SUPPORTED ||
        result == LIB_WIN32_MIDI_NO_DEVICE || result == LIB_WIN32_MM_BAD_DEVICE)
        return LIB_STATUS_UNSUPPORTED;
    if (result == LIB_WIN32_MM_NO_MEMORY) return LIB_STATUS_NO_MEMORY;
    return LIB_STATUS_IO_ERROR;
}

static lib_status audio_midi_failure(lib_audio_midi_output *output,
    lib_status status)
{
    if (status != LIB_STATUS_OK && output->failure == LIB_STATUS_OK)
        output->failure = status;
    return status;
}

/* Header flags are owned by WinMM until completion; storage never moves. */
static lib_status audio_midi_reap(lib_audio_midi_output *output,
    lib_u32 *out_free)
{
    *out_free = 0;
    for (lib_u32 i = 0; i < LIB_AUDIO_MIDI_SYSEX_CAPACITY; ++i) {
        lib_win32_midi_header *header = &output->slots[i].header;
        if ((header->dwFlags & LIB_WIN32_MIDI_PREPARED) &&
            (!(header->dwFlags & LIB_WIN32_MIDI_IN_QUEUE) ||
             (header->dwFlags & LIB_WIN32_MIDI_DONE))) {
            lib_status status = audio_midi_result(lib_win32_midi_out_unprepare(
                output->handle, header, sizeof(*header)));
            if (status != LIB_STATUS_OK) return audio_midi_failure(output, status);
            lib_memory_set(header, 0, sizeof(*header));
        }
        if (!(header->dwFlags & LIB_WIN32_MIDI_PREPARED)) ++*out_free;
    }
    return LIB_STATUS_OK;
}

static lib_status audio_midi_wait(lib_audio_midi_output *output, lib_u32 needed)
{
    lib_win32_handle events[2] = {output->cancelled, output->completed};
    for (;;) {
        lib_u32 available;
        lib_status status = audio_midi_reap(output, &available);
        if (status != LIB_STATUS_OK) return status;
        if (available >= needed) return LIB_STATUS_OK;
        lib_win32_dword result = lib_win32_wait_for_multiple_objects(
            2, events, LIB_WIN32_FALSE, LIB_WIN32_INFINITE);
        if (result == LIB_WIN32_WAIT_OBJECT_0) return LIB_STATUS_INVALID_STATE;
        if (result != LIB_WIN32_WAIT_OBJECT_0 + 1)
            return audio_midi_failure(output, LIB_STATUS_IO_ERROR);
        /* Auto-reset completion plus recheck covers synchronous completion and
         * multiple completions coalesced into one signal, without polling. */
    }
}

lib_status lib_audio_midi_output_create(lib_audio_midi_output **out_output)
{
    if (!out_output) return LIB_STATUS_INVALID_ARGUMENT;
    *out_output = LIB_NULL;
    lib_audio_midi_output *output = lib_allocate_zero(1, sizeof(*output));
    if (!output) return LIB_STATUS_NO_MEMORY;
    output->completed = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_FALSE,
        LIB_WIN32_FALSE, LIB_NULL);
    output->cancelled = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_FALSE,
        LIB_WIN32_FALSE, LIB_NULL);
    lib_status status = LIB_STATUS_IO_ERROR;
    if (output->completed && output->cancelled)
        status = audio_midi_result(lib_win32_midi_out_open(&output->handle,
            LIB_WIN32_MIDI_MAPPER, (lib_win32_dword_ptr)output->completed,
            0, LIB_WIN32_CALLBACK_EVENT));
    if (status != LIB_STATUS_OK) {
        if (output->completed) lib_win32_close_handle(output->completed);
        if (output->cancelled) lib_win32_close_handle(output->cancelled);
        lib_release(output);
        return status;
    }
    *out_output = output;
    return LIB_STATUS_OK;
}

lib_status lib_audio_midi_output_query_capacity(lib_audio_midi_output *output,
    lib_u32 *out_free_slots)
{
    if (!output || !out_free_slots) return LIB_STATUS_INVALID_ARGUMENT;
    *out_free_slots = 0;
    if (output->failure != LIB_STATUS_OK) return output->failure;
    return audio_midi_reap(output, out_free_slots);
}

lib_status lib_audio_midi_output_wait_writable(lib_audio_midi_output *output)
{
    if (!output) return LIB_STATUS_INVALID_ARGUMENT;
    if (output->failure != LIB_STATUS_OK) return output->failure;
    return audio_midi_wait(output, 1);
}

lib_status lib_audio_midi_output_cancel_wait(lib_audio_midi_output *output)
{
    if (!output) return LIB_STATUS_INVALID_ARGUMENT;
    return lib_win32_set_event(output->cancelled) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status lib_audio_midi_output_send_short(lib_audio_midi_output *output,
    const lib_u8 *bytes, lib_u32 count)
{
    lib_u32 packed;
    if (!output) return LIB_STATUS_INVALID_ARGUMENT;
    lib_status status = audio_midi_short_pack(bytes, count, &packed);
    if (status != LIB_STATUS_OK) return status;
    if (output->failure != LIB_STATUS_OK) return output->failure;
    if (bytes[0] < 0xf8) {
        status = audio_midi_wait(output, LIB_AUDIO_MIDI_SYSEX_CAPACITY);
        if (status != LIB_STATUS_OK) return status;
    }
    return audio_midi_failure(output, audio_midi_result(
        lib_win32_midi_out_short(output->handle, packed)));
}

static lib_status audio_midi_submit(lib_audio_midi_output *output,
    const lib_u8 *bytes, lib_u32 count)
{
    lib_u32 available;
    lib_status status = audio_midi_reap(output, &available);
    if (status != LIB_STATUS_OK) return status;
    if (!available) return LIB_STATUS_LIMIT_EXCEEDED;
    for (lib_u32 i = 0; i < LIB_AUDIO_MIDI_SYSEX_CAPACITY; ++i) {
        audio_midi_slot *slot = &output->slots[i];
        if (slot->header.dwFlags & LIB_WIN32_MIDI_PREPARED) continue;
        lib_memory_copy(slot->bytes, bytes, count);
        slot->header.lpData = (char *)slot->bytes;
        slot->header.dwBufferLength = count;
        status = audio_midi_result(lib_win32_midi_out_prepare(output->handle,
            &slot->header, sizeof(slot->header)));
        if (status == LIB_STATUS_OK)
            status = audio_midi_result(lib_win32_midi_out_long(output->handle,
                &slot->header, sizeof(slot->header)));
        /* Even on failure, a prepared header belongs to this output until
         * unprepare succeeds. Cleanup uses the same reap/reset path. */
        return audio_midi_failure(output, status);
    }
    return audio_midi_failure(output, LIB_STATUS_INTERNAL_ERROR);
}

lib_status lib_audio_midi_output_send_sysex(lib_audio_midi_output *output,
    const lib_u8 *bytes, lib_u32 count)
{
    if (!output) return LIB_STATUS_INVALID_ARGUMENT;
    lib_status status = audio_midi_sysex_validate(bytes, count);
    if (status != LIB_STATUS_OK) return status;
    if (output->failure != LIB_STATUS_OK) return output->failure;
    return audio_midi_submit(output, bytes, count);
}

static lib_status audio_midi_reset(lib_audio_midi_output *output)
{
    lib_u32 available;
    lib_status status = audio_midi_reap(output, &available);
    if (status != LIB_STATUS_OK) return status;
    lib_bool interrupted = available != LIB_AUDIO_MIDI_SYSEX_CAPACITY;
    status = audio_midi_result(lib_win32_midi_out_reset(output->handle));
    if (status != LIB_STATUS_OK) return audio_midi_failure(output, status);
    status = audio_midi_reap(output, &available);
    if (status != LIB_STATUS_OK) return status;
    if (interrupted) {
        const lib_u8 eox = 0xf7;
        /* Reset returns headers but does not send EOX. This is a native
         * transport continuation, not a second public message parser. */
        status = audio_midi_submit(output, &eox, 1);
        if (status != LIB_STATUS_OK) return status;
        status = audio_midi_wait(output, LIB_AUDIO_MIDI_SYSEX_CAPACITY);
    }
    return status;
}

lib_status lib_audio_midi_output_reset(lib_audio_midi_output *output)
{
    if (!output) return LIB_STATUS_INVALID_ARGUMENT;
    if (output->failure != LIB_STATUS_OK) return output->failure;
    return audio_midi_reset(output);
}

lib_status lib_audio_midi_output_get_volume(lib_audio_midi_output *output,
    lib_u16 *out_left, lib_u16 *out_right)
{
    lib_win32_dword value;
    if (!output || !out_left || !out_right) return LIB_STATUS_INVALID_ARGUMENT;
    if (output->failure != LIB_STATUS_OK) return output->failure;
    lib_status status = audio_midi_result(lib_win32_midi_out_get_volume(output->handle, &value));
    if (status == LIB_STATUS_OK) {
        *out_left = (lib_u16)value;
        *out_right = (lib_u16)(value >> 16);
    }
    return status == LIB_STATUS_UNSUPPORTED ? status : audio_midi_failure(output, status);
}

lib_status lib_audio_midi_output_set_volume(lib_audio_midi_output *output,
    lib_u16 left, lib_u16 right)
{
    if (!output) return LIB_STATUS_INVALID_ARGUMENT;
    if (output->failure != LIB_STATUS_OK) return output->failure;
    lib_status status = audio_midi_result(lib_win32_midi_out_set_volume(output->handle,
        (lib_win32_dword)left | ((lib_win32_dword)right << 16)));
    return status == LIB_STATUS_UNSUPPORTED ? status : audio_midi_failure(output, status);
}

lib_status lib_audio_midi_output_destroy(lib_audio_midi_output **output_pointer)
{
    if (!output_pointer || !*output_pointer) return LIB_STATUS_OK;
    lib_audio_midi_output *output = *output_pointer;
    lib_status status = audio_midi_reset(output);
    if (status != LIB_STATUS_OK) return status;
    status = audio_midi_result(lib_win32_midi_out_close(output->handle));
    if (status != LIB_STATUS_OK) return audio_midi_failure(output, status);
    lib_win32_close_handle(output->cancelled);
    lib_win32_close_handle(output->completed);
    lib_release(output);
    *output_pointer = LIB_NULL;
    return LIB_STATUS_OK;
}
