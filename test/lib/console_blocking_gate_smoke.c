#include <assert.h>
#include <windows.h>
#include "lib/base/win32/sync.c"

static HANDLE entered, release_gate, blocked;
static LONG contender;
static lib_u32 mutex_creates, fail_mutex, live_mutexes;
static lib_status tracked_create(base_sync_mutex **out)
{
    if (++mutex_creates == fail_mutex) {
        *out = NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    lib_status status = base_sync_mutex_create(out);
    if (status == LIB_STATUS_OK) ++live_mutexes;
    return status;
}
static void tracked_destroy(base_sync_mutex *mutex)
{
    if (mutex != NULL) { assert(live_mutexes); --live_mutexes; }
    base_sync_mutex_destroy(mutex);
}
static void tracked_enter(base_sync_mutex *mutex)
{
    if (GetCurrentThreadId() == (DWORD)InterlockedCompareExchange(&contender, 0, 0)) {
        if (TryEnterCriticalSection(&mutex->gate)) return;
        SetEvent(blocked); /* proven contention, not a scheduling guess */
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
    SetEvent(entered);
    assert(WaitForSingleObject(release_gate, 5000) == WAIT_OBJECT_0);
    return LIB_STATUS_OK;
}
static void input(void *opaque, const lib_console_event *event)
{ (void)event; assert(output(opaque, NULL, 0) == LIB_STATUS_OK); }
static lib_status frame_output(void *opaque, const lib_console_text_frame *value)
{ (void)value; return output(opaque, NULL, 0); }
static lib_status replacement_text(void *opaque, const char *text, lib_size count)
{
    (void)text; (void)count;
    assert(opaque == &replacement_calls);
    ++replacement_calls;
    return LIB_STATUS_OK;
}
static lib_status replacement_frame(void *opaque, const lib_console_text_frame *value)
{ (void)value; return replacement_text(opaque, NULL, 0); }
static DWORD WINAPI writer(void *unused)
{
    (void)unused;
    if (mode == 0 || mode == 3) assert(lib_console_write_text(object, "x", 1) == LIB_STATUS_OK);
    else if (mode == 1 || mode == 4) assert(lib_console_write_text_frame(object, &frame) == LIB_STATUS_OK);
    else {
        lib_console_event event = { 0 };
        event.kind = LIB_CONSOLE_EVENT_RAW_KEY; event.binding_generation = 1;
        assert(lib_console_deliver_event(object, &event) == LIB_STATUS_OK);
    }
    return 0;
}
static DWORD WINAPI detach(void *unused)
{
    (void)unused;
    InterlockedExchange(&contender, (LONG)GetCurrentThreadId());
    if (mode < 2) assert(lib_console_set_output_binding(object, NULL) == LIB_STATUS_OK);
    else if (mode > 2) {
        const lib_console_output_binding binding = {
            replacement_text, replacement_frame, &replacement_calls
        };
        assert(lib_console_set_output_binding(object, &binding) == LIB_STATUS_OK);
    }
    else assert(lib_console_set_event_sink(object, NULL, NULL) == LIB_STATUS_OK);
    return 0;
}
static DWORD WINAPI lock_consumer(void *unused)
{
    (void)unused;
    InterlockedExchange(&contender, (LONG)GetCurrentThreadId());
    if (mode == 0) assert(lib_console_bind_generation(object, 2) == LIB_STATUS_OK);
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
        assert(lib_console_create(&object) == LIB_STATUS_NO_MEMORY);
        assert(object == NULL && live_mutexes == 0);
    }
    fail_mutex = 0;
    for (mode = 0; mode != 5; ++mode) {
        HANDLE a, b;
        const lib_console_output_binding binding = { output, frame_output, NULL };
        assert(lib_console_create(&object) == LIB_STATUS_OK);
        assert(lib_console_bind_generation(object, 1) == LIB_STATUS_OK);
        assert(lib_console_set_output_binding(object, &binding) == LIB_STATUS_OK);
        assert(lib_console_set_event_sink(object, input, NULL) == LIB_STATUS_OK);
        entered = CreateEventA(NULL, TRUE, FALSE, NULL);
        release_gate = CreateEventA(NULL, TRUE, FALSE, NULL);
        blocked = CreateEventA(NULL, TRUE, FALSE, NULL);
        assert(entered && release_gate && blocked);
        a = CreateThread(NULL, 0, writer, NULL, 0, NULL);
        assert(a && WaitForSingleObject(entered, 5000) == WAIT_OBJECT_0);
        b = CreateThread(NULL, 0, detach, NULL, 0, NULL);
        assert(b && WaitForSingleObject(blocked, 5000) == WAIT_OBJECT_0);
        assert(WaitForSingleObject(b, 0) == WAIT_TIMEOUT);
        SetEvent(release_gate);
        assert(WaitForSingleObject(a, 5000) == WAIT_OBJECT_0);
        assert(WaitForSingleObject(b, 5000) == WAIT_OBJECT_0);
        if (mode < 2) {
            assert(lib_console_write_text(object, "x", 1u) == LIB_STATUS_OK);
            assert(lib_console_write_text_frame(object, &frame) == LIB_STATUS_OK);
        } else if (mode > 2) {
            replacement_calls = 0;
            assert(lib_console_write_text(object, "x", 1u) == LIB_STATUS_OK);
            assert(lib_console_write_text_frame(object, &frame) == LIB_STATUS_OK);
            assert(replacement_calls == 2);
        }
        InterlockedExchange(&contender, 0);
        lib_console_release(object);
        assert(live_mutexes == 0);
        CloseHandle(a); CloseHandle(b);
        CloseHandle(entered); CloseHandle(release_gate); CloseHandle(blocked);
    }
    assert(lib_console_create(&object) == LIB_STATUS_OK);
    assert(base_sync_mutex_create(&backend.output_lock) == LIB_STATUS_OK);
    assert(base_sync_mutex_create(&backend.transaction_lock) == LIB_STATUS_OK);
    for (mode = 0; mode < 3; ++mode) {
        base_sync_mutex *held = mode == 0 ? object->lock :
            mode == 1 ? backend.output_lock : backend.transaction_lock;
        blocked = CreateEventA(NULL, TRUE, FALSE, NULL);
        assert(blocked);
        base_sync_mutex_lock(held);
        HANDLE thread = CreateThread(NULL, 0, lock_consumer, NULL, 0, NULL);
        assert(thread && WaitForSingleObject(blocked, 5000) == WAIT_OBJECT_0);
        assert(WaitForSingleObject(thread, 0) == WAIT_TIMEOUT);
        base_sync_mutex_unlock(held);
        assert(WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0);
        InterlockedExchange(&contender, 0);
        CloseHandle(thread); CloseHandle(blocked);
    }
    base_sync_mutex_destroy(backend.output_lock);
    base_sync_mutex_destroy(backend.transaction_lock);
    lib_console_release(object);
    assert(live_mutexes == 0);
    return 0;
}
