#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/types/win32/test.h"
#include "lib/base/win32/sync.c"

static lib_win32_handle entered, release_gate, blocked;
static lib_win32_long contender;
static lib_u32 mutex_creates, fail_mutex, live_mutexes;
static lib_status tracked_create(base_sync_mutex **out)
{
    if (++mutex_creates == fail_mutex) {
        *out = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    lib_status status = base_sync_mutex_create(out);
    if (status == LIB_STATUS_OK) ++live_mutexes;
    return status;
}
static void tracked_destroy(base_sync_mutex *mutex)
{
    if (mutex != LIB_NULL) { lib_test_assert(live_mutexes); --live_mutexes; }
    base_sync_mutex_destroy(mutex);
}
static void tracked_enter(base_sync_mutex *mutex)
{
    if (lib_win32_get_current_thread_id() == (lib_win32_dword)lib_win32_interlocked_compare_exchange(&contender, 0, 0)) {
        if (lib_win32_try_enter_critical_section(&mutex->gate)) return;
        lib_win32_set_event(blocked); /* proven contention, not a scheduling guess */
    }
    base_sync_mutex_lock(mutex);
}
#define base_sync_mutex_lock tracked_enter
#define base_sync_mutex_create tracked_create
#define base_sync_mutex_destroy tracked_destroy
#include "lib/console/console.c"
#include "lib/console-broker/win32/console.c"
#undef base_sync_mutex_lock
#undef base_sync_mutex_create
#undef base_sync_mutex_destroy

static lib_console *object;
static console_broker_backend backend;
static lib_i32 mode;
static lib_i32 replacement_calls;
static lib_console_text_frame frame = { .columns = 80u, .rows = 25u };
static lib_status output(void *opaque, const char *text, lib_size count)
{
    (void)opaque; (void)text; (void)count;
    lib_win32_set_event(entered);
    lib_test_assert(lib_win32_wait_for_single_object(release_gate, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    return LIB_STATUS_OK;
}
static void input(void *opaque, const lib_console_event *event)
{ (void)event; lib_test_assert(output(opaque, LIB_NULL, 0) == LIB_STATUS_OK); }
static lib_status frame_output(void *opaque, const lib_console_text_frame *value)
{ (void)value; return output(opaque, LIB_NULL, 0); }
static lib_status replacement_text(void *opaque, const char *text, lib_size count)
{
    (void)text; (void)count;
    lib_test_assert(opaque == &replacement_calls);
    ++replacement_calls;
    return LIB_STATUS_OK;
}
static lib_status replacement_frame(void *opaque, const lib_console_text_frame *value)
{ (void)value; return replacement_text(opaque, LIB_NULL, 0); }
static lib_win32_dword LIB_WIN32_WINAPI writer(void *unused)
{
    (void)unused;
    if (mode == 0 || mode == 3) lib_test_assert(lib_console_write_text(object, "x", 1) == LIB_STATUS_OK);
    else if (mode == 1 || mode == 4) lib_test_assert(lib_console_write_text_frame(object, &frame) == LIB_STATUS_OK);
    else {
        lib_console_event event = { 0 };
        event.kind = LIB_CONSOLE_EVENT_RAW_KEY; event.binding_generation = 1;
        lib_test_assert(lib_console_deliver_event(object, &event) == LIB_STATUS_OK);
    }
    return 0;
}
static lib_win32_dword LIB_WIN32_WINAPI detach(void *unused)
{
    (void)unused;
    lib_win32_interlocked_exchange(&contender, (lib_win32_long)lib_win32_get_current_thread_id());
    if (mode < 2) lib_test_assert(lib_console_set_output_binding(object, LIB_NULL) == LIB_STATUS_OK);
    else if (mode > 2) {
        const lib_console_output_binding binding = {
            replacement_text, replacement_frame, &replacement_calls
        };
        lib_test_assert(lib_console_set_output_binding(object, &binding) == LIB_STATUS_OK);
    }
    else lib_test_assert(lib_console_set_event_sink(object, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    return 0;
}
static lib_win32_dword LIB_WIN32_WINAPI lock_consumer(void *unused)
{
    (void)unused;
    lib_win32_interlocked_exchange(&contender, (lib_win32_long)lib_win32_get_current_thread_id());
    if (mode == 0) lib_test_assert(lib_console_bind_generation(object, 2) == LIB_STATUS_OK);
    else if (mode == 1) {
        console_broker_backend_lock_output(&backend);
        console_broker_backend_unlock_output(&backend);
    } else {
        console_broker_backend_lock_transaction(&backend);
        console_broker_backend_unlock_transaction(&backend);
    }
    return 0;
}
int main(void)
{
    for (fail_mutex = 1; fail_mutex <= 3; ++fail_mutex) {
        mutex_creates = 0;
        lib_test_assert(lib_console_create(&object) == LIB_STATUS_NO_MEMORY);
        lib_test_assert(object == LIB_NULL && live_mutexes == 0);
    }
    fail_mutex = 0;
    for (mode = 0; mode != 5; ++mode) {
        lib_win32_handle a, b;
        const lib_console_output_binding binding = { output, frame_output, LIB_NULL };
        lib_test_assert(lib_console_create(&object) == LIB_STATUS_OK);
        lib_test_assert(lib_console_bind_generation(object, 1) == LIB_STATUS_OK);
        lib_test_assert(lib_console_set_output_binding(object, &binding) == LIB_STATUS_OK);
        lib_test_assert(lib_console_set_event_sink(object, input, LIB_NULL) == LIB_STATUS_OK);
        entered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        release_gate = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        blocked = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        lib_test_assert(entered && release_gate && blocked);
        a = lib_win32_create_thread(LIB_NULL, 0, writer, LIB_NULL, 0, LIB_NULL);
        lib_test_assert(a && lib_win32_wait_for_single_object(entered, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        b = lib_win32_create_thread(LIB_NULL, 0, detach, LIB_NULL, 0, LIB_NULL);
        lib_test_assert(b && lib_win32_wait_for_single_object(blocked, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(lib_win32_wait_for_single_object(b, 0) == LIB_WIN32_WAIT_TIMEOUT);
        lib_win32_set_event(release_gate);
        lib_test_assert(lib_win32_wait_for_single_object(a, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(lib_win32_wait_for_single_object(b, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        if (mode < 2) {
            lib_test_assert(lib_console_write_text(object, "x", 1u) == LIB_STATUS_OK);
            lib_test_assert(lib_console_write_text_frame(object, &frame) == LIB_STATUS_OK);
        } else if (mode > 2) {
            replacement_calls = 0;
            lib_test_assert(lib_console_write_text(object, "x", 1u) == LIB_STATUS_OK);
            lib_test_assert(lib_console_write_text_frame(object, &frame) == LIB_STATUS_OK);
            lib_test_assert(replacement_calls == 2);
        }
        lib_win32_interlocked_exchange(&contender, 0);
        lib_console_release(object);
        lib_test_assert(live_mutexes == 0);
        lib_win32_close_handle(a); lib_win32_close_handle(b);
        lib_win32_close_handle(entered); lib_win32_close_handle(release_gate); lib_win32_close_handle(blocked);
    }
    lib_test_assert(lib_console_create(&object) == LIB_STATUS_OK);
    lib_test_assert(base_sync_mutex_create(&backend.output_lock) == LIB_STATUS_OK);
    lib_test_assert(base_sync_mutex_create(&backend.transaction_lock) == LIB_STATUS_OK);
    for (mode = 0; mode < 3; ++mode) {
        base_sync_mutex *held = mode == 0 ? object->lock :
            mode == 1 ? backend.output_lock : backend.transaction_lock;
        blocked = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
        lib_test_assert(blocked);
        base_sync_mutex_lock(held);
        lib_win32_handle thread = lib_win32_create_thread(LIB_NULL, 0, lock_consumer, LIB_NULL, 0, LIB_NULL);
        lib_test_assert(thread && lib_win32_wait_for_single_object(blocked, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(lib_win32_wait_for_single_object(thread, 0) == LIB_WIN32_WAIT_TIMEOUT);
        base_sync_mutex_unlock(held);
        lib_test_assert(lib_win32_wait_for_single_object(thread, 5000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_win32_interlocked_exchange(&contender, 0);
        lib_win32_close_handle(thread); lib_win32_close_handle(blocked);
    }
    base_sync_mutex_destroy(backend.output_lock);
    base_sync_mutex_destroy(backend.transaction_lock);
    lib_console_release(object);
    lib_test_assert(live_mutexes == 0);
    return 0;
}
