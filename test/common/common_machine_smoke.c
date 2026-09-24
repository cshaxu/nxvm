#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "machine_fixture.h"


typedef struct state_transfer {
    lib_u8 byte;
    lib_win32_long calls;
} state_transfer;

static lib_status state_write(void *opaque, const lib_u8 *bytes,
    lib_size byte_count)
{
    state_transfer *transfer = (state_transfer *)opaque;
    if (transfer == LIB_NULL || bytes == LIB_NULL || byte_count != 1u)
        return LIB_STATUS_INVALID_ARGUMENT;
    transfer->byte = bytes[0];
    lib_win32_interlocked_increment(&transfer->calls);
    return LIB_STATUS_OK;
}

static lib_status state_read(void *opaque, lib_u8 *bytes, lib_size byte_count)
{
    state_transfer *transfer = (state_transfer *)opaque;
    if (transfer == LIB_NULL || bytes == LIB_NULL || byte_count != 1u)
        return LIB_STATUS_INVALID_ARGUMENT;
    bytes[0] = transfer->byte;
    lib_win32_interlocked_increment(&transfer->calls);
    return LIB_STATUS_OK;
}
/* Deliberately not an architecture protocol: the executor returns token + 1. */
static lib_status execute_token(void *context, const void *bytes, lib_size size,
    void *response, lib_size capacity, lib_size *response_size)
{
    lib_u32 token;
    (void)context;
    lib_test_assert(size == sizeof(token) && capacity == sizeof(token));
    lib_memory_copy(&token, bytes, sizeof(token));
    ++token;
    lib_memory_copy(response, &token, sizeof(token));
    *response_size = sizeof(token);
    return LIB_STATUS_OK;
}

static lib_win32_dword LIB_WIN32_WINAPI shutdown_machine(void *opaque)
{
    common_machine_shutdown(opaque);
    return 0;
}

/* A blocked final callback must prevent shutdown returning. A thread handle
 * is the completion barrier; no timing sleep guesses at worker quiescence. */
static void shutdown_active(common_machine *machine, machine_fake *fake)
{
    lib_win32_handle thread;
    lib_win32_long notifications;
    fake->callback_entered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->callback_release = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_test_assert(fake->callback_entered && fake->callback_release);
    thread = lib_win32_create_thread(LIB_NULL, 0u, shutdown_machine, machine, 0u, LIB_NULL);
    lib_test_assert(thread != LIB_NULL);
    lib_test_assert(lib_win32_wait_for_single_object(fake->callback_entered, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_wait_for_single_object(thread, 0u) == LIB_WIN32_WAIT_TIMEOUT);
    lib_win32_set_event(fake->callback_release);
    lib_test_assert(lib_win32_wait_for_single_object(thread, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(fake->callback == LIB_NULL && fake->callback_context == LIB_NULL);
    notifications = fake->notifications;
    lib_test_assert(!common_machine_start(machine));
    lib_test_assert(!common_machine_reset(machine));
    lib_test_assert(!common_machine_set_removable_media(machine, LIB_NULL,
        LIB_STORAGE_MEDIUM_OVERLAY));
    common_machine_shutdown(machine);
    common_machine_destroy(machine);
    lib_test_assert(fake->notifications == notifications);
    lib_win32_close_handle(thread);
    lib_win32_close_handle(fake->callback_entered); lib_win32_close_handle(fake->callback_release);
    fake->callback_entered = fake->callback_release = LIB_NULL;
}

int main(void)
{
    machine_fake fake = { 0 };
    common_machine_driver driver = { 0 };
    common_machine *machine = LIB_NULL;
    kvm_input_event input = { 0 };
    common_machine_frame frame = { 0 };
    lib_u32 generation = 0u;
    common_machine_debug_lease lease = { 0 };
    lib_u32 token = 0x1234u, debug_result = 0u;
    lib_size response_size = 0u;
    state_transfer transfer = { 0 };

    machine_fake_initialize(&fake, &driver);
    fake.debug = execute_token;
    lib_test_assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
    frame.window.valid = 1u;
    frame.sequence = 77u;
    lib_test_assert(!common_machine_copy_published_frame(machine, &frame,
        common_machine_run_generation(machine)));
    lib_test_assert(frame.window.valid == 1u && frame.sequence == 77u);
    common_machine_set_state_sink(machine, machine_fake_note_state, &fake);
    common_machine_set_frame_sink(machine, machine_fake_note_frame, &fake);
    lib_test_assert(common_machine_start(machine));
    lib_test_assert(lib_win32_wait_for_single_object(fake.running, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_wait_for_single_object(fake.frame, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_interlocked_compare_exchange(&fake.resets, 0, 0) == 1);
    generation = common_machine_run_generation(machine);
    lib_memory_set(&frame.window, 0xa5, sizeof(frame.window));
    lib_test_assert(common_machine_copy_published_frame(machine, &frame, generation));
    lib_test_assert(frame.window.valid == 1u && generation == common_machine_run_generation(machine));
    lib_test_assert(!frame.window.graphics);
    for (lib_size i = kvm_window_frame_size_bytes(&frame.window);
            i < sizeof(frame.window); ++i)
        lib_test_assert(((const lib_u8 *)&frame.window)[i] == 0xa5);
    {
        lib_u32 sequence = frame.sequence;
        lib_test_assert(!common_machine_copy_published_frame(machine, &frame, generation + 1u));
        lib_test_assert(frame.window.valid == 1u && frame.sequence == sequence);
    }
    input.type = KVM_EVENT_KEY;
    input.data.key.pressed = 1u;
    lib_test_assert(common_machine_enqueue_input(machine, &input));
    lib_test_assert(lib_win32_wait_for_single_object(fake.input, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_interlocked_compare_exchange(&fake.inputs, 0, 0) == 1);
    fake.state_byte = 0x5au;
    /* A pending ordinary pause must not complete a save before its result. */
    fake.defer_state_read = 1;
    lib_test_assert(common_machine_pause(machine));
    lib_test_assert(common_machine_read_state(machine,
        &(common_machine_state_writer) { state_write, &transfer }) == LIB_STATUS_OK);
    lib_test_assert(transfer.byte == 0x5au && lib_win32_interlocked_compare_exchange(
        &transfer.calls, 0, 0) == 1 && lib_win32_interlocked_compare_exchange(
        &fake.state_reads, 0, 0) == 1);
    lib_test_assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    lib_test_assert(common_machine_set_removable_media(machine, "Mixed-Case.img",
        LIB_STORAGE_MEDIUM_DIRECT));
    lib_test_assert(fake.media_calls == 1 && fake.media_mode == LIB_STORAGE_MEDIUM_DIRECT &&
        !lib_text_compare(fake.media_path, "Mixed-Case.img"));
    lib_test_assert(common_machine_set_removable_media(machine, "ReadOnly.img",
        LIB_STORAGE_MEDIUM_READONLY));
    lib_test_assert(fake.media_calls == 2 && fake.media_mode == LIB_STORAGE_MEDIUM_READONLY &&
        !lib_text_compare(fake.media_path, "ReadOnly.img"));
    lib_test_assert(common_machine_set_removable_media(machine, "Overlay.img",
        LIB_STORAGE_MEDIUM_OVERLAY));
    lib_test_assert(fake.media_calls == 3 && fake.media_mode == LIB_STORAGE_MEDIUM_OVERLAY &&
        !lib_text_compare(fake.media_path, "Overlay.img"));
    lib_test_assert(common_machine_set_removable_media(machine, LIB_NULL,
        LIB_STORAGE_MEDIUM_OVERLAY));
    lib_test_assert(fake.media_calls == 4 && fake.media_mode == LIB_STORAGE_MEDIUM_OVERLAY &&
        fake.media_path[0] == '\0');
    fake.defer_state_read = 1;
    {
        lib_win32_long running = fake.running_notifications;
        lib_win32_long paused = fake.paused_notifications;
        lib_test_assert(common_machine_read_state(machine,
            &(common_machine_state_writer) { state_write, &transfer }) ==
            LIB_STATUS_OK);
        lib_test_assert(transfer.byte == 0x5au && lib_win32_interlocked_compare_exchange(
            &transfer.calls, 0, 0) == 2 && lib_win32_interlocked_compare_exchange(
            &fake.state_reads, 0, 0) == 2);
        lib_test_assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
        lib_test_assert(fake.running_notifications == running);
        lib_test_assert(fake.paused_notifications == paused);
    }
    lib_win32_reset_event(fake.running);
    lib_test_assert(common_machine_resume(machine));
    lib_test_assert(lib_win32_wait_for_single_object(fake.running, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(common_machine_state_get(machine) == COMMON_MACHINE_RUNNING);
    lib_win32_reset_event(fake.running);
    lib_test_assert(common_machine_reset(machine));
    lib_test_assert(lib_win32_wait_for_single_object(fake.reset_completed, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_interlocked_compare_exchange(&fake.resets, 0, 0) == 2);
    lib_test_assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    lib_test_assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    lib_test_assert(common_machine_debug_execute_with_lease(machine, &lease,
        &token, sizeof(token), &debug_result, sizeof(debug_result),
        &response_size) == LIB_STATUS_OK);
    lib_test_assert(debug_result == token + 1u && response_size == sizeof(debug_result) &&
        lib_win32_interlocked_compare_exchange(&fake.debug_calls, 0, 0) == 1);
    lib_test_assert(common_machine_resume(machine));
    lib_test_assert(common_machine_debug_execute_with_lease(machine, &lease,
        &token, sizeof(token), &debug_result, sizeof(debug_result),
        &response_size) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(response_size == 0u);
    lib_test_assert(lib_win32_wait_for_single_object(fake.running, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(common_machine_stop(machine));
    lib_test_assert(lib_win32_wait_for_single_object(fake.state_stopped, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(common_machine_state_get(machine) == COMMON_MACHINE_STOPPED);
    transfer.byte = 0xa5u;
    transfer.calls = 0;
    lib_test_assert(common_machine_write_state(machine,
        &(common_machine_state_reader) { state_read, &transfer }) == LIB_STATUS_OK);
    lib_test_assert(transfer.calls == 1 && fake.state_byte == 0xa5u &&
        lib_win32_interlocked_compare_exchange(&fake.state_writes, 0, 0) == 1);
    lib_test_assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    lib_test_assert(common_machine_write_state(machine,
        &(common_machine_state_reader) { state_read, &transfer }) ==
        LIB_STATUS_INVALID_STATE);
    {
        lib_u32 sequence = frame.sequence;
        lib_test_assert(!common_machine_copy_published_frame(machine, &frame, generation));
        lib_test_assert(frame.window.valid == 1u && frame.sequence == sequence);
    }
    common_machine_shutdown(machine);
    common_machine_shutdown(machine);
    common_machine_destroy(machine);
    /* A never-started worker uses the same terminal path. */
    lib_test_assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
    common_machine_shutdown(machine);
    lib_test_assert(!common_machine_start(machine));
    common_machine_destroy(machine);
    for (lib_i32 paused = 0; paused != 2; ++paused) {
        lib_win32_reset_event(fake.running); lib_win32_reset_event(fake.reset_completed);
        lib_test_assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
        common_machine_set_state_sink(machine, machine_fake_note_state, &fake);
        common_machine_set_frame_sink(machine, machine_fake_note_frame, &fake);
        lib_test_assert(paused ? common_machine_reset(machine) : common_machine_start(machine));
        lib_test_assert(lib_win32_wait_for_single_object(paused ? fake.reset_completed : fake.running,
            5000u) == LIB_WIN32_WAIT_OBJECT_0);
        shutdown_active(machine, &fake);
    }
    common_machine_shutdown(LIB_NULL);
    /* Public synchronous calls must return after frame failure (before save
     * admission or restored pause) and after an armed save loses its executor. */
    for (lib_u32 scenario = 0; scenario < 3u; ++scenario) {
        lib_win32_reset_event(fake.running); lib_win32_reset_event(fake.frame);
        fake.fail_frame = fake.fail_run = fake.state_read_ready = 0;
        lib_test_assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
        common_machine_set_state_sink(machine, machine_fake_note_state, &fake);
        common_machine_set_frame_sink(machine, machine_fake_note_frame, &fake);
        lib_win32_long paused = fake.paused_notifications;
        if (scenario < 2u) {
            lib_test_assert(common_machine_start(machine));
            lib_test_assert(lib_win32_wait_for_single_object(fake.running, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
            lib_test_assert(lib_win32_wait_for_single_object(fake.frame, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
            /* Frame notification precedes state servicing in the callback.
             * Finish that callback before arming a read-wake failure. */
            lib_test_assert(lib_win32_wait_for_single_object(fake.run_waiting, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
            if (scenario == 0u) fake.fail_on_read_wake = 1;
            else fake.fail_after_read_arm = 1;
            lib_test_assert(common_machine_read_state(machine,
                &(common_machine_state_writer) { state_write, &transfer }) ==
                LIB_STATUS_IO_ERROR);
        } else {
            fake.fail_frame = 1;
            lib_test_assert(common_machine_write_state(machine,
                &(common_machine_state_reader) { state_read, &transfer }) ==
                LIB_STATUS_IO_ERROR);
        }
        lib_test_assert(common_machine_state_get(machine) == COMMON_MACHINE_ERROR);
        lib_test_assert(fake.callback == LIB_NULL && fake.paused_notifications == paused);
        lib_test_assert(common_machine_destroy(machine) == LIB_STATUS_OK);
    }
    machine_fake_dispose(&fake);
    return 0;
}
