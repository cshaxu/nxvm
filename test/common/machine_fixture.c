#include "machine_fixture.h"

#include <assert.h>
#include <string.h>

static lib_bool fake_reset(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    ResetEvent(fake->stopped);
    ResetEvent(fake->wake);
    ResetEvent(fake->run_waiting);
    InterlockedIncrement(&fake->resets);
    return LIB_TRUE;
}

static lib_bool fake_run(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    HANDLE events[2] = { fake->stopped, fake->wake };
    InterlockedIncrement(&fake->runs);
    fake->executor_thread = GetCurrentThreadId();
    if (fake->callback != NULL) fake->callback(fake->callback_context);
    for (;;) {
        SetEvent(fake->run_waiting);
        DWORD result = WaitForMultipleObjects(2u, events, FALSE, 5000u);
        if (result == WAIT_OBJECT_0) return LIB_TRUE;
        if (result != WAIT_OBJECT_0 + 1u) return LIB_FALSE;
        ResetEvent(fake->wake);
        if (InterlockedCompareExchange(&fake->fail_run, 0, 0)) return LIB_FALSE;
        if (fake->callback != NULL) fake->callback(fake->callback_context);
    }
}

static void fake_request_stop(void *opaque)
{ SetEvent(((machine_fake *)opaque)->stopped); }
static void fake_request_wake(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (InterlockedExchange(&fake->fail_on_read_wake, 0))
        InterlockedExchange(&fake->fail_frame, 1);
    if (InterlockedExchange(&fake->state_read_waiting, 0) != 0) {
        assert(fake->deferred_state_writer.write(
            fake->deferred_state_writer.context, &fake->state_byte, 1u) ==
            LIB_STATUS_OK);
        InterlockedIncrement(&fake->state_reads);
        InterlockedExchange(&fake->state_read_ready, 1);
    }
    SetEvent(fake->wake);
}
static lib_bool fake_set_media(void *opaque, const char *path,
    lib_storage_medium_mode mode)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (mode > LIB_STORAGE_MEDIUM_OVERLAY) return LIB_FALSE;
    ++fake->media_calls;
    fake->media_mode = mode;
    if (path == NULL) fake->media_path[0] = '\0';
    else {
        assert(strlen(path) < sizeof(fake->media_path));
        memcpy(fake->media_path, path, strlen(path) + 1u);
    }
    return LIB_TRUE;
}
static void fake_heartbeat(void *opaque, lib_bool enabled)
{ (void)opaque; (void)enabled; }
static void fake_set_callback(void *opaque, common_machine_executor_callback callback,
    void *context)
{
    machine_fake *fake = (machine_fake *)opaque;
    fake->callback = callback;
    fake->callback_context = context;
}
static void fake_deliver_input(void *opaque, const kvm_input_event *event)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)event;
    InterlockedIncrement(&fake->inputs);
    SetEvent(fake->input);
}
static lib_status fake_copy_frame(void *opaque, common_machine_frame *frame)
{
    machine_fake *fake = opaque;
    if (InterlockedCompareExchange(&fake->fail_frame, 0, 0))
        return LIB_STATUS_IO_ERROR;
    memset(frame, 0, sizeof(*frame));
    frame->window.valid = 1u;
    frame->window.text.base.text_columns = KVM_TEXT_COLUMNS;
    frame->window.text.base.text_rows = KVM_TEXT_ROWS;
    return LIB_STATUS_OK;
}

/* State transfer is deliberately exercised through the injected driver, on
 * the executor thread.  The Common test never needs to know what the byte
 * represents. */
static lib_status fake_begin_state_read(void *opaque,
    const common_machine_state_writer *writer)
{
    machine_fake *fake = (machine_fake *)opaque;
    assert(GetCurrentThreadId() == fake->executor_thread);
    if (writer == NULL || writer->write == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (InterlockedExchange(&fake->fail_after_read_arm, 0)) {
        InterlockedExchange(&fake->fail_run, 1);
        SetEvent(fake->wake);
        return LIB_STATUS_OK;
    }
    if (InterlockedExchange(&fake->defer_state_read, 0) != 0) {
        fake->deferred_state_writer = *writer;
        InterlockedExchange(&fake->state_read_waiting, 1);
        return LIB_STATUS_OK;
    }
    if (writer->write(writer->context, &fake->state_byte, 1u) != LIB_STATUS_OK)
        return LIB_STATUS_IO_ERROR;
    InterlockedIncrement(&fake->state_reads);
    InterlockedExchange(&fake->state_read_ready, 1);
    return LIB_STATUS_OK;
}

static lib_bool fake_take_state_read_result(void *opaque, lib_status *status)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (status == NULL) return LIB_FALSE;
    if (InterlockedExchange(&fake->state_read_ready, 0) == 0) return LIB_FALSE;
    *status = LIB_STATUS_OK;
    return LIB_TRUE;
}

static lib_status fake_write_state(void *opaque,
    const common_machine_state_reader *reader)
{
    machine_fake *fake = (machine_fake *)opaque;
    lib_u8 byte = 0u;
    if (reader == NULL || reader->read == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (reader->read(reader->context, &byte, 1u) != LIB_STATUS_OK)
        return LIB_STATUS_IO_ERROR;
    fake->state_byte = byte;
    InterlockedIncrement(&fake->state_writes);
    return LIB_STATUS_OK;
}

static lib_status fake_execute_debug(void *opaque, const void *bytes, lib_size size,
    void *response, lib_size capacity, lib_size *response_size)
{
    machine_fake *fake = opaque;
    assert(GetCurrentThreadId() == fake->executor_thread);
    InterlockedIncrement(&fake->debug_calls);
    return fake->debug(fake->debug_context, bytes, size, response, capacity,
        response_size);
}

void machine_fake_note_state(void *opaque, common_machine_state state,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)generation;
    InterlockedIncrement(&fake->notifications);
    if (state == COMMON_MACHINE_RUNNING) {
        InterlockedIncrement(&fake->running_notifications);
        SetEvent(fake->running);
    }
    if (state == COMMON_MACHINE_PAUSED)
        InterlockedIncrement(&fake->paused_notifications);
    if (state == COMMON_MACHINE_STOPPED && fake->callback_entered != NULL) {
        SetEvent(fake->callback_entered);
        assert(WaitForSingleObject(fake->callback_release, 5000u) == WAIT_OBJECT_0);
    }
    if (state == COMMON_MACHINE_STOPPED) SetEvent(fake->state_stopped);
    if (state == COMMON_MACHINE_RESET_COMPLETED) SetEvent(fake->reset_completed);
}
void machine_fake_note_frame(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)sequence; (void)graphics; (void)generation;
    InterlockedIncrement(&fake->notifications);
    SetEvent(fake->frame);
}

void machine_fake_initialize(machine_fake *fake, common_machine_driver *driver)
{
    fake->stopped = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->state_stopped = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->reset_completed = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->wake = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->running = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->run_waiting = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->frame = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->input = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(fake->stopped != NULL && fake->state_stopped != NULL &&
        fake->reset_completed != NULL && fake->wake != NULL && fake->running != NULL &&
        fake->frame != NULL && fake->input != NULL && fake->run_waiting != NULL);
    driver->context = fake;
    driver->reset = fake_reset;
    driver->run = fake_run;
    driver->request_stop = fake_request_stop;
    driver->request_wake = fake_request_wake;
    driver->set_heartbeat = fake_heartbeat;
    driver->set_executor_callback = fake_set_callback;
    driver->deliver_input = fake_deliver_input;
    driver->copy_frame = fake_copy_frame;
    driver->set_removable_media = fake_set_media;
    driver->begin_state_read = fake_begin_state_read;
    driver->take_state_read_result = fake_take_state_read_result;
    driver->write_state = fake_write_state;
    driver->execute_debug = fake_execute_debug;
}

void machine_fake_dispose(machine_fake *fake)
{
    CloseHandle(fake->input); CloseHandle(fake->frame); CloseHandle(fake->running);
    CloseHandle(fake->run_waiting);
    CloseHandle(fake->wake); CloseHandle(fake->reset_completed);
    CloseHandle(fake->state_stopped); CloseHandle(fake->stopped);
}
