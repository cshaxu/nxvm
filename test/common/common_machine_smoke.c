#include "machine_fixture.h"

#include <assert.h>
#include <string.h>

typedef struct state_transfer {
    lib_u8 byte;
    LONG calls;
} state_transfer;

static lib_status state_write(void *opaque, const lib_u8 *bytes,
    lib_size byte_count)
{
    state_transfer *transfer = (state_transfer *)opaque;
    if (transfer == NULL || bytes == NULL || byte_count != 1u)
        return LIB_STATUS_INVALID_ARGUMENT;
    transfer->byte = bytes[0];
    InterlockedIncrement(&transfer->calls);
    return LIB_STATUS_OK;
}

static lib_status state_read(void *opaque, lib_u8 *bytes, lib_size byte_count)
{
    state_transfer *transfer = (state_transfer *)opaque;
    if (transfer == NULL || bytes == NULL || byte_count != 1u)
        return LIB_STATUS_INVALID_ARGUMENT;
    bytes[0] = transfer->byte;
    InterlockedIncrement(&transfer->calls);
    return LIB_STATUS_OK;
}
/* Deliberately not an architecture protocol: the executor returns token + 1. */
static lib_status execute_token(void *context, const void *bytes, lib_size size,
    void *response, lib_size capacity, lib_size *response_size)
{
    lib_u32 token;
    (void)context;
    assert(size == sizeof(token) && capacity == sizeof(token));
    memcpy(&token, bytes, sizeof(token));
    ++token;
    memcpy(response, &token, sizeof(token));
    *response_size = sizeof(token);
    return LIB_STATUS_OK;
}

static DWORD WINAPI shutdown_machine(void *opaque)
{
    common_machine_shutdown(opaque);
    return 0;
}

/* A blocked final callback must prevent shutdown returning. A thread handle
 * is the completion barrier; no timing sleep guesses at worker quiescence. */
static void shutdown_active(common_machine *machine, machine_fake *fake)
{
    HANDLE thread;
    LONG notifications;
    fake->callback_entered = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->callback_release = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(fake->callback_entered && fake->callback_release);
    thread = CreateThread(NULL, 0u, shutdown_machine, machine, 0u, NULL);
    assert(thread != NULL);
    assert(WaitForSingleObject(fake->callback_entered, 5000u) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(thread, 0u) == WAIT_TIMEOUT);
    SetEvent(fake->callback_release);
    assert(WaitForSingleObject(thread, 5000u) == WAIT_OBJECT_0);
    assert(fake->callback == NULL && fake->callback_context == NULL);
    notifications = fake->notifications;
    assert(!common_machine_start(machine));
    assert(!common_machine_reset(machine));
    assert(!common_machine_set_removable_media(machine, NULL,
        LIB_STORAGE_MEDIUM_OVERLAY));
    common_machine_shutdown(machine);
    common_machine_destroy(machine);
    assert(fake->notifications == notifications);
    CloseHandle(thread);
    CloseHandle(fake->callback_entered); CloseHandle(fake->callback_release);
    fake->callback_entered = fake->callback_release = NULL;
}

int main(void)
{
    machine_fake fake = { 0 };
    common_machine_driver driver = { 0 };
    common_machine *machine = NULL;
    kvm_input_event input = { 0 };
    common_machine_frame frame = { 0 };
    lib_u32 generation = 0u;
    common_machine_debug_lease lease = { 0 };
    lib_u32 token = 0x1234u, debug_result = 0u;
    lib_size response_size = 0u;
    state_transfer transfer = { 0 };

    machine_fake_initialize(&fake, &driver);
    fake.debug = execute_token;
    assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
    frame.window.valid = 1u;
    frame.sequence = 77u;
    assert(!common_machine_copy_published_frame(machine, &frame,
        common_machine_run_generation(machine)));
    assert(frame.window.valid == 1u && frame.sequence == 77u);
    common_machine_set_state_sink(machine, machine_fake_note_state, &fake);
    common_machine_set_frame_sink(machine, machine_fake_note_frame, &fake);
    assert(common_machine_start(machine));
    assert(WaitForSingleObject(fake.running, 5000u) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(fake.frame, 5000u) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&fake.resets, 0, 0) == 1);
    generation = common_machine_run_generation(machine);
    memset(&frame.window, 0xa5, sizeof(frame.window));
    assert(common_machine_copy_published_frame(machine, &frame, generation));
    assert(frame.window.valid == 1u && generation == common_machine_run_generation(machine));
    assert(!frame.window.graphics);
    for (lib_size i = kvm_window_frame_size_bytes(&frame.window);
            i < sizeof(frame.window); ++i)
        assert(((const lib_u8 *)&frame.window)[i] == 0xa5);
    {
        lib_u32 sequence = frame.sequence;
        assert(!common_machine_copy_published_frame(machine, &frame, generation + 1u));
        assert(frame.window.valid == 1u && frame.sequence == sequence);
    }
    input.type = KVM_EVENT_KEY;
    input.data.key.pressed = 1u;
    assert(common_machine_enqueue_input(machine, &input));
    assert(WaitForSingleObject(fake.input, 5000u) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&fake.inputs, 0, 0) == 1);
    fake.state_byte = 0x5au;
    /* A pending ordinary pause must not complete a save before its result. */
    fake.defer_state_read = 1;
    assert(common_machine_pause(machine));
    assert(common_machine_read_state(machine,
        &(common_machine_state_writer) { state_write, &transfer }) == LIB_STATUS_OK);
    assert(transfer.byte == 0x5au && InterlockedCompareExchange(
        &transfer.calls, 0, 0) == 1 && InterlockedCompareExchange(
        &fake.state_reads, 0, 0) == 1);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    assert(common_machine_set_removable_media(machine, "Mixed-Case.img",
        LIB_STORAGE_MEDIUM_DIRECT));
    assert(fake.media_calls == 1 && fake.media_mode == LIB_STORAGE_MEDIUM_DIRECT &&
        !strcmp(fake.media_path, "Mixed-Case.img"));
    assert(common_machine_set_removable_media(machine, "ReadOnly.img",
        LIB_STORAGE_MEDIUM_READONLY));
    assert(fake.media_calls == 2 && fake.media_mode == LIB_STORAGE_MEDIUM_READONLY &&
        !strcmp(fake.media_path, "ReadOnly.img"));
    assert(common_machine_set_removable_media(machine, "Overlay.img",
        LIB_STORAGE_MEDIUM_OVERLAY));
    assert(fake.media_calls == 3 && fake.media_mode == LIB_STORAGE_MEDIUM_OVERLAY &&
        !strcmp(fake.media_path, "Overlay.img"));
    assert(common_machine_set_removable_media(machine, NULL,
        LIB_STORAGE_MEDIUM_OVERLAY));
    assert(fake.media_calls == 4 && fake.media_mode == LIB_STORAGE_MEDIUM_OVERLAY &&
        fake.media_path[0] == '\0');
    fake.defer_state_read = 1;
    {
        LONG running = fake.running_notifications;
        LONG paused = fake.paused_notifications;
        assert(common_machine_read_state(machine,
            &(common_machine_state_writer) { state_write, &transfer }) ==
            LIB_STATUS_OK);
        assert(transfer.byte == 0x5au && InterlockedCompareExchange(
            &transfer.calls, 0, 0) == 2 && InterlockedCompareExchange(
            &fake.state_reads, 0, 0) == 2);
        assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
        assert(fake.running_notifications == running);
        assert(fake.paused_notifications == paused);
    }
    ResetEvent(fake.running);
    assert(common_machine_resume(machine));
    assert(WaitForSingleObject(fake.running, 5000u) == WAIT_OBJECT_0);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_RUNNING);
    ResetEvent(fake.running);
    assert(common_machine_reset(machine));
    assert(WaitForSingleObject(fake.reset_completed, 5000u) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&fake.resets, 0, 0) == 2);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease,
        &token, sizeof(token), &debug_result, sizeof(debug_result),
        &response_size) == LIB_STATUS_OK);
    assert(debug_result == token + 1u && response_size == sizeof(debug_result) &&
        InterlockedCompareExchange(&fake.debug_calls, 0, 0) == 1);
    assert(common_machine_resume(machine));
    assert(common_machine_debug_execute_with_lease(machine, &lease,
        &token, sizeof(token), &debug_result, sizeof(debug_result),
        &response_size) == LIB_STATUS_INVALID_STATE);
    assert(response_size == 0u);
    assert(WaitForSingleObject(fake.running, 5000u) == WAIT_OBJECT_0);
    assert(common_machine_stop(machine));
    assert(WaitForSingleObject(fake.state_stopped, 5000u) == WAIT_OBJECT_0);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_STOPPED);
    transfer.byte = 0xa5u;
    transfer.calls = 0;
    assert(common_machine_write_state(machine,
        &(common_machine_state_reader) { state_read, &transfer }) == LIB_STATUS_OK);
    assert(transfer.calls == 1 && fake.state_byte == 0xa5u &&
        InterlockedCompareExchange(&fake.state_writes, 0, 0) == 1);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    assert(common_machine_write_state(machine,
        &(common_machine_state_reader) { state_read, &transfer }) ==
        LIB_STATUS_INVALID_STATE);
    {
        lib_u32 sequence = frame.sequence;
        assert(!common_machine_copy_published_frame(machine, &frame, generation));
        assert(frame.window.valid == 1u && frame.sequence == sequence);
    }
    common_machine_shutdown(machine);
    common_machine_shutdown(machine);
    common_machine_destroy(machine);
    /* A never-started worker uses the same terminal path. */
    assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
    common_machine_shutdown(machine);
    assert(!common_machine_start(machine));
    common_machine_destroy(machine);
    for (lib_i32 paused = 0; paused != 2; ++paused) {
        ResetEvent(fake.running); ResetEvent(fake.reset_completed);
        assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
        common_machine_set_state_sink(machine, machine_fake_note_state, &fake);
        common_machine_set_frame_sink(machine, machine_fake_note_frame, &fake);
        assert(paused ? common_machine_reset(machine) : common_machine_start(machine));
        assert(WaitForSingleObject(paused ? fake.reset_completed : fake.running,
            5000u) == WAIT_OBJECT_0);
        shutdown_active(machine, &fake);
    }
    common_machine_shutdown(NULL);
    /* Public synchronous calls must return after frame failure (before save
     * admission or restored pause) and after an armed save loses its executor. */
    for (lib_u32 scenario = 0; scenario < 3u; ++scenario) {
        ResetEvent(fake.running); ResetEvent(fake.frame);
        fake.fail_frame = fake.fail_run = fake.state_read_ready = 0;
        assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
        common_machine_set_state_sink(machine, machine_fake_note_state, &fake);
        common_machine_set_frame_sink(machine, machine_fake_note_frame, &fake);
        LONG paused = fake.paused_notifications;
        if (scenario < 2u) {
            assert(common_machine_start(machine));
            assert(WaitForSingleObject(fake.running, 5000u) == WAIT_OBJECT_0);
            assert(WaitForSingleObject(fake.frame, 5000u) == WAIT_OBJECT_0);
            /* Frame notification precedes state servicing in the callback.
             * Finish that callback before arming a read-wake failure. */
            assert(WaitForSingleObject(fake.run_waiting, 5000u) == WAIT_OBJECT_0);
            if (scenario == 0u) fake.fail_on_read_wake = 1;
            else fake.fail_after_read_arm = 1;
            assert(common_machine_read_state(machine,
                &(common_machine_state_writer) { state_write, &transfer }) ==
                LIB_STATUS_IO_ERROR);
        } else {
            fake.fail_frame = 1;
            assert(common_machine_write_state(machine,
                &(common_machine_state_reader) { state_read, &transfer }) ==
                LIB_STATUS_IO_ERROR);
        }
        assert(common_machine_state_get(machine) == COMMON_MACHINE_ERROR);
        assert(fake.callback == NULL && fake.paused_notifications == paused);
        assert(common_machine_destroy(machine) == LIB_STATUS_OK);
    }
    machine_fake_dispose(&fake);
    return 0;
}
