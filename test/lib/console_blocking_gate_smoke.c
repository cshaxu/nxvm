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
static lib_status output(void *opaque, const char *text, lib_size count)
{
    (void)opaque; (void)text; (void)count;
    SetEvent(entered);
    assert(WaitForSingleObject(release_gate, 5000) == WAIT_OBJECT_0);
    return LIB_STATUS_OK;
}
static void input(void *opaque, const lib_console_event *event)
{ (void)event; assert(output(opaque, NULL, 0) == LIB_STATUS_OK); }
static DWORD WINAPI writer(void *unused)
{
    (void)unused;
    if (mode == 0) assert(lib_console_write_text(object, "x", 1) == LIB_STATUS_OK);
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
    if (mode == 0) assert(lib_console_set_output_sink(object, NULL, NULL) == LIB_STATUS_OK);
    else assert(lib_console_set_event_sink(object, NULL, NULL) == LIB_STATUS_OK);
    return 0;
}
int main(void)
{
    for (mode = 0; mode != 2; ++mode) {
        HANDLE a, b;
        assert(lib_console_create(&object) == LIB_STATUS_OK);
        assert(lib_console_bind_generation(object, 1) == LIB_STATUS_OK);
        assert(lib_console_set_output_sink(object, output, NULL) == LIB_STATUS_OK);
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
        InterlockedExchange(&contender, 0);
        lib_console_destroy(object);
        CloseHandle(a); CloseHandle(b);
        CloseHandle(entered); CloseHandle(release_gate); CloseHandle(blocked);
    }
    return 0;
}
