#include "lib/types/test.h"
#include "lib/audio/pcm/stream_interface.h"
#include "myraidenii/machine/driver_interface.h"

static lib_i32 reset_count;
static lib_i32 run_count;
static lib_i32 stop_count;
static lib_i32 stream_clear_count;
static lib_i32 stream_active_count;
static lib_i32 stream_cancel_count;
static lib_i32 stream_destroy_count;
static lib_i32 frame_copy_count;
static lib_status frame_status = LIB_STATUS_OK;
static lib_i32 callback_count;
static lib_status destroy_status = LIB_STATUS_OK;
static lib_u32 audio_frames;
static lib_u32 audio_accepted;
static lib_i32 control_count;
static myraidenii_core_board_control control_last;
static lib_bool control_pressed;
static lib_u32 queued_count;
static lib_u32 queue_fail_at = LIB_UINT32_MAX;
static base_key queued_keys[9];

lib_bool emulator_machine_enqueue_input(emulator_machine *machine, const kvm_input_event *event)
{
    if (machine == LIB_NULL || event == LIB_NULL || event->type != KVM_EVENT_KEY ||
        event->data.key.pressed || queued_count == queue_fail_at) return LIB_FALSE;
    if (queued_count >= sizeof(queued_keys) / sizeof(queued_keys[0])) return LIB_FALSE;
    queued_keys[queued_count++] = event->data.key.key;
    return LIB_TRUE;
}

lib_status myraidenii_core_board_set_control(myraidenii_core_board *board,
    myraidenii_core_board_control control, lib_bool pressed)
{
    if (board == LIB_NULL || control >= MYRAIDENII_CORE_BOARD_CONTROL_COUNT)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++control_count;
    control_last = control;
    control_pressed = pressed;
    return LIB_STATUS_OK;
}

x86_core_machine *myraidenii_core_board_machine_get(myraidenii_core_board *board)
{
    return board == LIB_NULL ? LIB_NULL : (x86_core_machine *)(lib_uptr)2u;
}

lib_status myraidenii_core_board_pcm_copy(myraidenii_core_board *board,
    lib_i16 *out_samples, lib_u32 capacity, lib_u32 *out_frames, lib_u64 *out_dropped)
{
    if (board == LIB_NULL || out_samples == LIB_NULL || capacity == 0u ||
        out_frames == LIB_NULL || out_dropped == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (audio_frames > capacity) audio_frames = capacity;
    if (audio_frames != 0u) out_samples[0u] = 7;
    *out_frames = audio_frames;
    *out_dropped = 0u;
    audio_frames = 0u;
    return LIB_STATUS_OK;
}

lib_status myraidenii_core_board_frame_copy(const myraidenii_core_board *board,
    lib_u32 *pixels, lib_size pixel_count)
{
    if (board == LIB_NULL || pixels == LIB_NULL ||
        pixel_count != MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS) return LIB_STATUS_INVALID_ARGUMENT;
    if (frame_status != LIB_STATUS_OK) return frame_status;
    lib_memory_set(pixels, 0, pixel_count * sizeof(*pixels));
    pixels[0u] = 0x00112233u;
    pixels[MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH - 1u] = 0x00445566u;
    pixels[(MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT - 1u) *
        MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH] = 0x00778899u;
    pixels[MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS - 1u] = 0x00aabbccu;
    pixels[17u * MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH + 29u] = 0x00010203u;
    ++frame_copy_count;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_reset(x86_core_machine *machine)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++reset_count;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_run(x86_core_machine *machine,
    x86_core_machine_run_budget budget, x86_core_machine_run_result *result)
{
    if (machine == LIB_NULL || result == LIB_NULL || budget.instructions != 256u || budget.ticks != 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    ++run_count;
    *result = (x86_core_machine_run_result) { .reason = X86_CORE_MACHINE_STOP_BUDGET };
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_request_stop(x86_core_machine *machine)
{
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++stop_count;
    return LIB_STATUS_OK;
}

lib_status x86_core_machine_advance_to_next_deadline(x86_core_machine *machine,
    lib_u8 *out_advanced)
{
    if (machine == LIB_NULL || out_advanced == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_advanced = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_enqueue(lib_audio_pcm_stream *stream,
    const lib_i16 *samples, lib_u32 frame_count, lib_u32 *out_accepted)
{
    if (stream == LIB_NULL || out_accepted == LIB_NULL ||
        (samples == LIB_NULL && frame_count != 0u)) return LIB_STATUS_INVALID_ARGUMENT;
    *out_accepted = audio_accepted > frame_count ? frame_count : audio_accepted;
    return *out_accepted == frame_count ? LIB_STATUS_OK : LIB_STATUS_LIMIT_EXCEEDED;
}

lib_status lib_audio_pcm_stream_clear(lib_audio_pcm_stream *stream)
{
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++stream_clear_count;
    return LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_set_active(lib_audio_pcm_stream *stream, lib_bool active)
{
    if (stream == LIB_NULL || (active != LIB_FALSE && active != LIB_TRUE))
        return LIB_STATUS_INVALID_ARGUMENT;
    ++stream_active_count;
    return LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_wait_writable(lib_audio_pcm_stream *stream)
{
    return stream == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_cancel_wait(lib_audio_pcm_stream *stream)
{
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    ++stream_cancel_count;
    return LIB_STATUS_OK;
}

lib_status lib_audio_pcm_stream_destroy(lib_audio_pcm_stream **stream)
{
    if (stream == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (destroy_status != LIB_STATUS_OK) return destroy_status;
    if (*stream != LIB_NULL) {
        ++stream_destroy_count;
        *stream = LIB_NULL;
    }
    return LIB_STATUS_OK;
}

void base_sync_yield(void)
{
}

static void executor_callback(void *context)
{
    if (context != (void *)(lib_uptr)4u) return;
    ++callback_count;
}

int main(void)
{
    myraidenii_machine *machine = LIB_NULL;
    emulator_machine_driver driver = {0};
    static kvm_window_frame_storage frame;
    lib_audio_pcm_stream *stream = (lib_audio_pcm_stream *)(lib_uptr)3u;
    lib_bool produced = LIB_FALSE;
    kvm_input_event event = {0};
    const struct {
        base_key key;
        myraidenii_core_board_control control;
    } mappings[] = {
        {BASE_KEY_UP, MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_UP},
        {BASE_KEY_DOWN, MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_DOWN},
        {BASE_KEY_LEFT, MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_LEFT},
        {BASE_KEY_RIGHT, MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_RIGHT},
        {'Z', MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_BUTTON_1},
        {'X', MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_BUTTON_2},
        {'1', MYRAIDENII_CORE_BOARD_CONTROL_START_1},
        {'5', MYRAIDENII_CORE_BOARD_CONTROL_COIN_1},
        {BASE_KEY_F2, MYRAIDENII_CORE_BOARD_CONTROL_SERVICE}
    };
    const myraidenii_core_board *board = (const myraidenii_core_board *)(lib_uptr)1u;

    if (myraidenii_machine_release_inputs(LIB_NULL) ||
        !myraidenii_machine_release_inputs((emulator_machine *)(lib_uptr)5u) ||
        queued_count != sizeof(mappings) / sizeof(mappings[0])) return 1;
    for (lib_u32 i = 0u; i < sizeof(mappings) / sizeof(mappings[0]); ++i)
        if (queued_keys[i] != mappings[i].key) return 1;
    for (lib_u32 i = 0u; i < sizeof(mappings) / sizeof(mappings[0]); ++i) {
        queued_count = 0u;
        queue_fail_at = i;
        if (myraidenii_machine_release_inputs((emulator_machine *)(lib_uptr)5u) ||
            queued_count != i) return 1;
    }
    queue_fail_at = LIB_UINT32_MAX;

    if (myraidenii_machine_create(LIB_NULL, stream, &machine) != LIB_STATUS_INVALID_ARGUMENT ||
        myraidenii_machine_create((myraidenii_core_board *)board, LIB_NULL, &machine) !=
            LIB_STATUS_OK || myraidenii_machine_destroy(&machine) != LIB_STATUS_OK ||
        myraidenii_machine_create((myraidenii_core_board *)board, stream, &machine) != LIB_STATUS_OK ||
        myraidenii_machine_driver_get(machine, &driver) != LIB_STATUS_OK ||
        driver.context != machine || driver.reset == LIB_NULL || driver.run == LIB_NULL ||
        driver.request_stop == LIB_NULL || driver.request_wake == LIB_NULL ||
        driver.set_heartbeat == LIB_NULL || driver.set_executor_callback == LIB_NULL ||
        driver.deliver_input == LIB_NULL || driver.copy_frame == LIB_NULL) return 1;
    driver.set_executor_callback(driver.context, executor_callback, (void *)(lib_uptr)4u);
    if (driver.reset(driver.context) == LIB_FALSE || reset_count != 1 || stream_clear_count != 1 ||
        driver.run(driver.context) == LIB_FALSE || run_count != 1 || callback_count != 2 ||
        driver.copy_frame(driver.context, &frame, LIB_NULL, &produced) != LIB_STATUS_OK ||
        produced == LIB_FALSE || frame.frame.graphics == LIB_NULL ||
        frame.frame.graphics->width != MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT ||
        frame.frame.graphics->height != MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH ||
        frame.frame.graphics->stride != MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT ||
        frame.frame.graphics->pixels[0u] != 0x00445566u ||
        frame.frame.graphics->pixels[MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT - 1u] != 0x00aabbccu ||
        frame.frame.graphics->pixels[(MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH - 1u) *
            MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT] != 0x00112233u ||
        frame.frame.graphics->pixels[MYRAIDENII_CORE_GRAPHICS_FRAME_PIXELS - 1u] != 0x00778899u ||
        frame.frame.graphics->pixels[(MYRAIDENII_CORE_GRAPHICS_FRAME_WIDTH - 1u - 29u) *
            MYRAIDENII_CORE_GRAPHICS_FRAME_HEIGHT + 17u] != 0x00010203u ||
        frame_copy_count != 1 || frame.frame.sequence != 1u) return 1;
    frame_status = LIB_STATUS_IO_ERROR;
    if (driver.copy_frame(driver.context, &frame, LIB_NULL, &produced) != LIB_STATUS_IO_ERROR ||
        produced != LIB_FALSE || frame.frame.sequence != 1u ||
        frame.frame.graphics->pixels[0u] != 0x00445566u) return 1;
    frame_status = LIB_STATUS_OK;
    event.type = KVM_EVENT_KEY;
    for (lib_size index = 0u; index < sizeof(mappings) / sizeof(mappings[0]); ++index) {
        event.data.key.key = mappings[index].key;
        event.data.key.pressed = LIB_TRUE;
        driver.deliver_input(driver.context, &event);
        if (control_count != (lib_i32)(index + 1u) || control_last != mappings[index].control ||
            control_pressed != LIB_TRUE) return 1;
    }
    event.data.key.key = 'Z';
    event.data.key.pressed = LIB_FALSE;
    driver.deliver_input(driver.context, &event);
    if (control_count != 10 || control_last != MYRAIDENII_CORE_BOARD_CONTROL_PLAYER_1_BUTTON_1 ||
        control_pressed != LIB_FALSE) return 1;
    event.data.key.key = 'Q';
    driver.deliver_input(driver.context, &event);
    event.type = KVM_EVENT_INPUT_RESET;
    driver.deliver_input(driver.context, &event);
    event.type = KVM_EVENT_SOURCE_RETIRED;
    driver.deliver_input(driver.context, &event);
    if (control_count != 10) return 1;
    driver.set_heartbeat(driver.context, LIB_FALSE);
    driver.set_heartbeat(driver.context, LIB_TRUE);
    audio_frames = 1u;
    audio_accepted = 0u;
    driver.request_wake(driver.context);
    if (driver.run(driver.context) == LIB_FALSE || run_count != 2 || callback_count != 3)
        return 1;
    audio_accepted = 1u;
    if (driver.run(driver.context) == LIB_FALSE || run_count != 3 || callback_count != 5)
        return 1;
    driver.request_stop(driver.context);
    if (stream_clear_count != 2 || stream_active_count != 2 || stop_count != 1 ||
        stream_cancel_count != 2) return 1;
    destroy_status = LIB_STATUS_IO_ERROR;
    if (myraidenii_machine_destroy(&machine) != LIB_STATUS_IO_ERROR || machine == LIB_NULL)
        return 1;
    destroy_status = LIB_STATUS_OK;
    if (myraidenii_machine_destroy(&machine) != LIB_STATUS_OK ||
        machine != LIB_NULL || stream_destroy_count != 1) return 1;
    return 0;
}
