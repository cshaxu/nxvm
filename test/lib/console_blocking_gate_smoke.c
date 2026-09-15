#include <assert.h>
#include <windows.h>
#include "lib/console/win32/mutex.c"

static HANDLE entered, release_gate, blocked;
static LONG contender;
static void tracked_enter(console_mutex *mutex)
{
    if (GetCurrentThreadId() == (DWORD)InterlockedCompareExchange(&contender, 0, 0)) {
        if (TryEnterCriticalSection(&mutex->gate)) return;
        SetEvent(blocked); /* proven contention, not a scheduling guess */
    }
    console_mutex_enter(mutex);
}
#define console_mutex_enter tracked_enter
#include "lib/console/console.c"
#undef console_mutex_enter

static lib_console *object;
static int mode;
static int replacement_calls;
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
int main(void)
{
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
            assert(lib_console_write_text(object, "x", 1u) == LIB_STATUS_NOT_CURRENT);
            assert(lib_console_write_text_frame(object, &frame) == LIB_STATUS_NOT_CURRENT);
        } else if (mode > 2) {
            replacement_calls = 0;
            assert(lib_console_write_text(object, "x", 1u) == LIB_STATUS_OK);
            assert(lib_console_write_text_frame(object, &frame) == LIB_STATUS_OK);
            assert(replacement_calls == 2);
        }
        InterlockedExchange(&contender, 0);
        lib_console_destroy(object);
        CloseHandle(a); CloseHandle(b);
        CloseHandle(entered); CloseHandle(release_gate); CloseHandle(blocked);
    }
    return 0;
}
