#include "lib/types/test.h"
#include "lib/types/atomic.h"
#include "machine_fixture.h"

/* C11 thread-local identity verifies executor affinity without an OS thread ID. */
static _Thread_local machine_fake *executor_fixture;

static lib_bool fake_reset(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    base_sync_event_reset(fake->stopped);
    base_sync_event_reset(fake->wake);
    base_sync_event_reset(fake->run_waiting);
    lib_atomic_i32_fetch_add_explicit(&fake->resets, 1, LIB_MEMORY_ORDER_SEQ_CST);
    return LIB_TRUE;
}

static lib_bool fake_run(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    base_sync_event *events[2] = { fake->stopped, fake->wake };
    lib_atomic_i32_fetch_add_explicit(&fake->runs, 1, LIB_MEMORY_ORDER_SEQ_CST);
    executor_fixture = fake;
    if (fake->callback != LIB_NULL) fake->callback(fake->callback_context);
    for (;;) {
        base_sync_event_signal(fake->run_waiting);
        lib_u32 index;
        if (base_sync_wait_any(events, 2u, LIB_NULL, 5000u, &index) !=
            BASE_SYNC_WAIT_SIGNALED) return LIB_FALSE;
        if (index == 0u) return LIB_TRUE;
        base_sync_event_reset(fake->wake);
        if (lib_atomic_i32_load_explicit(&fake->fail_run, LIB_MEMORY_ORDER_SEQ_CST)) return LIB_FALSE;
        if (fake->callback != LIB_NULL) fake->callback(fake->callback_context);
    }
}

static void fake_request_stop(void *opaque)
{ base_sync_event_signal(((machine_fake *)opaque)->stopped); }
static void fake_request_wake(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (lib_atomic_i32_exchange_explicit(&fake->fail_on_read_wake, 0, LIB_MEMORY_ORDER_SEQ_CST))
        lib_atomic_i32_exchange_explicit(&fake->fail_frame, 1, LIB_MEMORY_ORDER_SEQ_CST);
    if (lib_atomic_i32_exchange_explicit(&fake->state_read_waiting, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0) {
        lib_test_assert(fake->deferred_state_writer.write(
            fake->deferred_state_writer.context, &fake->state_byte, 1u) ==
            LIB_STATUS_OK);
        lib_atomic_i32_fetch_add_explicit(&fake->state_reads, 1, LIB_MEMORY_ORDER_SEQ_CST);
        lib_atomic_i32_exchange_explicit(&fake->state_read_ready, 1, LIB_MEMORY_ORDER_SEQ_CST);
    }
    base_sync_event_signal(fake->wake);
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
static void fake_set_callback(void *opaque, emulator_machine_executor_callback callback,
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
    lib_atomic_i32_fetch_add_explicit(&fake->inputs, 1, LIB_MEMORY_ORDER_SEQ_CST);
    base_sync_event_signal(fake->input);
}
static lib_status fake_copy_frame(void *opaque, kvm_window_frame_storage *frame,
    kvm_console_text_frame *console, lib_bool *produced)
{
    machine_fake *fake = opaque;
    if (lib_atomic_i32_load_explicit(&fake->fail_frame, LIB_MEMORY_ORDER_SEQ_CST))
        return LIB_STATUS_IO_ERROR;
    lib_test_assert(kvm_window_frame_prepare_text(frame, 80u, 50u, 80u) == LIB_STATUS_OK);
    *produced = LIB_TRUE;
    frame->data.text->base.text_columns = 80u;
    frame->data.text->base.text_rows = 50u;
    return kvm_console_frame_prepare(console, 80u, 50u, 80u);
}

/* State transfer is deliberately exercised through the injected driver, on
 * the executor thread.  The Emulator test never needs to know what the byte
 * represents. */
static lib_status fake_begin_state_read(void *opaque,
    const emulator_machine_state_writer *writer)
{
    machine_fake *fake = (machine_fake *)opaque;
    lib_test_assert(executor_fixture == fake);
    if (writer == LIB_NULL || writer->write == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (lib_atomic_i32_exchange_explicit(&fake->fail_after_read_arm, 0, LIB_MEMORY_ORDER_SEQ_CST)) {
        lib_atomic_i32_exchange_explicit(&fake->fail_run, 1, LIB_MEMORY_ORDER_SEQ_CST);
        base_sync_event_signal(fake->wake);
        return LIB_STATUS_OK;
    }
    if (lib_atomic_i32_exchange_explicit(&fake->defer_state_read, 0, LIB_MEMORY_ORDER_SEQ_CST) != 0) {
        fake->deferred_state_writer = *writer;
        lib_atomic_i32_exchange_explicit(&fake->state_read_waiting, 1, LIB_MEMORY_ORDER_SEQ_CST);
        return LIB_STATUS_OK;
    }
    if (writer->write(writer->context, &fake->state_byte, 1u) != LIB_STATUS_OK)
        return LIB_STATUS_IO_ERROR;
    lib_atomic_i32_fetch_add_explicit(&fake->state_reads, 1, LIB_MEMORY_ORDER_SEQ_CST);
    lib_atomic_i32_exchange_explicit(&fake->state_read_ready, 1, LIB_MEMORY_ORDER_SEQ_CST);
    return LIB_STATUS_OK;
}

static lib_bool fake_take_state_read_result(void *opaque, lib_status *status)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (status == LIB_NULL) return LIB_FALSE;
    if (lib_atomic_i32_exchange_explicit(&fake->state_read_ready, 0, LIB_MEMORY_ORDER_SEQ_CST) == 0) return LIB_FALSE;
    *status = LIB_STATUS_OK;
    return LIB_TRUE;
}

static lib_status fake_write_state(void *opaque,
    const emulator_machine_state_reader *reader)
{
    machine_fake *fake = (machine_fake *)opaque;
    lib_u8 byte = 0u;
    if (reader == LIB_NULL || reader->read == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (reader->read(reader->context, &byte, 1u) != LIB_STATUS_OK)
        return LIB_STATUS_IO_ERROR;
    fake->state_byte = byte;
    lib_atomic_i32_fetch_add_explicit(&fake->state_writes, 1, LIB_MEMORY_ORDER_SEQ_CST);
    return LIB_STATUS_OK;
}

static lib_status fake_execute_debug(void *opaque, const void *bytes, lib_size size,
    void *response, lib_size capacity, lib_size *response_size)
{
    machine_fake *fake = opaque;
    lib_test_assert(executor_fixture == fake);
    lib_atomic_i32_fetch_add_explicit(&fake->debug_calls, 1, LIB_MEMORY_ORDER_SEQ_CST);
    return fake->debug(fake->debug_context, bytes, size, response, capacity,
        response_size);
}

void machine_fake_note_state(void *opaque, emulator_machine_state state,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)generation;
    lib_atomic_i32_fetch_add_explicit(&fake->notifications, 1, LIB_MEMORY_ORDER_SEQ_CST);
    if (state == EMULATOR_MACHINE_RUNNING) {
        lib_atomic_i32_fetch_add_explicit(&fake->running_notifications, 1, LIB_MEMORY_ORDER_SEQ_CST);
        base_sync_event_signal(fake->running);
    }
    if (state == EMULATOR_MACHINE_PAUSED)
        lib_atomic_i32_fetch_add_explicit(&fake->paused_notifications, 1, LIB_MEMORY_ORDER_SEQ_CST);
    if (state == EMULATOR_MACHINE_STOPPED && fake->callback_entered != LIB_NULL) {
        base_sync_event_signal(fake->callback_entered);
        lib_test_assert(base_sync_event_wait(fake->callback_release, 5000u) == BASE_SYNC_WAIT_SIGNALED);
    }
    if (state == EMULATOR_MACHINE_STOPPED) base_sync_event_signal(fake->state_stopped);
    if (state == EMULATOR_MACHINE_RESET_COMPLETED) base_sync_event_signal(fake->reset_completed);
}
void machine_fake_note_frame(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)sequence; (void)graphics; (void)generation;
    lib_atomic_i32_fetch_add_explicit(&fake->notifications, 1, LIB_MEMORY_ORDER_SEQ_CST);
    base_sync_event_signal(fake->frame);
}

void machine_fake_initialize(machine_fake *fake, emulator_machine_driver *driver)
{
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &fake->stopped) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &fake->state_stopped) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &fake->reset_completed) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &fake->wake) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &fake->running) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &fake->run_waiting) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &fake->frame) == LIB_STATUS_OK);
    lib_test_assert(base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &fake->input) == LIB_STATUS_OK);
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
    base_sync_event_destroy(fake->input); base_sync_event_destroy(fake->frame); base_sync_event_destroy(fake->running);
    base_sync_event_destroy(fake->run_waiting);
    base_sync_event_destroy(fake->wake); base_sync_event_destroy(fake->reset_completed);
    base_sync_event_destroy(fake->state_stopped); base_sync_event_destroy(fake->stopped);
}
