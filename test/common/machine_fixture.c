#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "machine_fixture.h"

static lib_bool fake_reset(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    lib_win32_reset_event(fake->stopped);
    lib_win32_reset_event(fake->wake);
    lib_win32_reset_event(fake->run_waiting);
    lib_win32_interlocked_increment(&fake->resets);
    return LIB_TRUE;
}

static lib_bool fake_run(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    lib_win32_handle events[2] = { fake->stopped, fake->wake };
    lib_win32_interlocked_increment(&fake->runs);
    fake->executor_thread = lib_win32_get_current_thread_id();
    if (fake->callback != LIB_NULL) fake->callback(fake->callback_context);
    for (;;) {
        lib_win32_set_event(fake->run_waiting);
        lib_win32_dword result = lib_win32_wait_for_multiple_objects(2u, events, LIB_WIN32_FALSE, 5000u);
        if (result == LIB_WIN32_WAIT_OBJECT_0) return LIB_TRUE;
        if (result != LIB_WIN32_WAIT_OBJECT_0 + 1u) return LIB_FALSE;
        lib_win32_reset_event(fake->wake);
        if (lib_win32_interlocked_compare_exchange(&fake->fail_run, 0, 0)) return LIB_FALSE;
        if (fake->callback != LIB_NULL) fake->callback(fake->callback_context);
    }
}

static void fake_request_stop(void *opaque)
{ lib_win32_set_event(((machine_fake *)opaque)->stopped); }
static void fake_request_wake(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (lib_win32_interlocked_exchange(&fake->fail_on_read_wake, 0))
        lib_win32_interlocked_exchange(&fake->fail_frame, 1);
    if (lib_win32_interlocked_exchange(&fake->state_read_waiting, 0) != 0) {
        lib_test_assert(fake->deferred_state_writer.write(
            fake->deferred_state_writer.context, &fake->state_byte, 1u) ==
            LIB_STATUS_OK);
        lib_win32_interlocked_increment(&fake->state_reads);
        lib_win32_interlocked_exchange(&fake->state_read_ready, 1);
    }
    lib_win32_set_event(fake->wake);
}
static lib_bool fake_set_media(void *opaque, const char *path,
    lib_storage_medium_mode mode)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (mode > LIB_STORAGE_MEDIUM_OVERLAY) return LIB_FALSE;
    ++fake->media_calls;
    fake->media_mode = mode;
    if (path == LIB_NULL) fake->media_path[0] = '\0';
    else {
        lib_test_assert(lib_text_length(path) < sizeof(fake->media_path));
        lib_memory_copy(fake->media_path, path, lib_text_length(path) + 1u);
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
    lib_win32_interlocked_increment(&fake->inputs);
    lib_win32_set_event(fake->input);
}
static lib_status fake_copy_frame(void *opaque, common_machine_frame *frame)
{
    machine_fake *fake = opaque;
    if (lib_win32_interlocked_compare_exchange(&fake->fail_frame, 0, 0))
        return LIB_STATUS_IO_ERROR;
    lib_memory_set(frame, 0, sizeof(*frame));
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
    lib_test_assert(lib_win32_get_current_thread_id() == fake->executor_thread);
    if (writer == LIB_NULL || writer->write == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (lib_win32_interlocked_exchange(&fake->fail_after_read_arm, 0)) {
        lib_win32_interlocked_exchange(&fake->fail_run, 1);
        lib_win32_set_event(fake->wake);
        return LIB_STATUS_OK;
    }
    if (lib_win32_interlocked_exchange(&fake->defer_state_read, 0) != 0) {
        fake->deferred_state_writer = *writer;
        lib_win32_interlocked_exchange(&fake->state_read_waiting, 1);
        return LIB_STATUS_OK;
    }
    if (writer->write(writer->context, &fake->state_byte, 1u) != LIB_STATUS_OK)
        return LIB_STATUS_IO_ERROR;
    lib_win32_interlocked_increment(&fake->state_reads);
    lib_win32_interlocked_exchange(&fake->state_read_ready, 1);
    return LIB_STATUS_OK;
}

static lib_bool fake_take_state_read_result(void *opaque, lib_status *status)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (status == LIB_NULL) return LIB_FALSE;
    if (lib_win32_interlocked_exchange(&fake->state_read_ready, 0) == 0) return LIB_FALSE;
    *status = LIB_STATUS_OK;
    return LIB_TRUE;
}

static lib_status fake_write_state(void *opaque,
    const common_machine_state_reader *reader)
{
    machine_fake *fake = (machine_fake *)opaque;
    lib_u8 byte = 0u;
    if (reader == LIB_NULL || reader->read == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (reader->read(reader->context, &byte, 1u) != LIB_STATUS_OK)
        return LIB_STATUS_IO_ERROR;
    fake->state_byte = byte;
    lib_win32_interlocked_increment(&fake->state_writes);
    return LIB_STATUS_OK;
}

static lib_status fake_execute_debug(void *opaque, const void *bytes, lib_size size,
    void *response, lib_size capacity, lib_size *response_size)
{
    machine_fake *fake = opaque;
    lib_test_assert(lib_win32_get_current_thread_id() == fake->executor_thread);
    lib_win32_interlocked_increment(&fake->debug_calls);
    return fake->debug(fake->debug_context, bytes, size, response, capacity,
        response_size);
}

void machine_fake_note_state(void *opaque, common_machine_state state,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)generation;
    lib_win32_interlocked_increment(&fake->notifications);
    if (state == COMMON_MACHINE_RUNNING) {
        lib_win32_interlocked_increment(&fake->running_notifications);
        lib_win32_set_event(fake->running);
    }
    if (state == COMMON_MACHINE_PAUSED)
        lib_win32_interlocked_increment(&fake->paused_notifications);
    if (state == COMMON_MACHINE_STOPPED && fake->callback_entered != LIB_NULL) {
        lib_win32_set_event(fake->callback_entered);
        lib_test_assert(lib_win32_wait_for_single_object(fake->callback_release, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    }
    if (state == COMMON_MACHINE_STOPPED) lib_win32_set_event(fake->state_stopped);
    if (state == COMMON_MACHINE_RESET_COMPLETED) lib_win32_set_event(fake->reset_completed);
}
void machine_fake_note_frame(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)sequence; (void)graphics; (void)generation;
    lib_win32_interlocked_increment(&fake->notifications);
    lib_win32_set_event(fake->frame);
}

void machine_fake_initialize(machine_fake *fake, common_machine_driver *driver)
{
    fake->stopped = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->state_stopped = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->reset_completed = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->wake = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->running = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->run_waiting = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->frame = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->input = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_test_assert(fake->stopped != LIB_NULL && fake->state_stopped != LIB_NULL &&
        fake->reset_completed != LIB_NULL && fake->wake != LIB_NULL && fake->running != LIB_NULL &&
        fake->frame != LIB_NULL && fake->input != LIB_NULL && fake->run_waiting != LIB_NULL);
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
    lib_win32_close_handle(fake->input); lib_win32_close_handle(fake->frame); lib_win32_close_handle(fake->running);
    lib_win32_close_handle(fake->run_waiting);
    lib_win32_close_handle(fake->wake); lib_win32_close_handle(fake->reset_completed);
    lib_win32_close_handle(fake->state_stopped); lib_win32_close_handle(fake->stopped);
}
