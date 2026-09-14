#include "lib/console/console_interface.h"
#include "lib/console/binding_interface.h"

#include <assert.h>

#ifdef _WIN32
#include <windows.h>

typedef struct gate_probe {
    lib_console *console;
    HANDLE entered;
    HANDLE release;
    HANDLE delivered;
    HANDLE detached;
} gate_probe;

static void gate_sink(void *opaque, const lib_console_event *event)
{
    gate_probe *probe = (gate_probe *)opaque;
    assert(probe != NULL && event != NULL);
    SetEvent(probe->entered);
    assert(WaitForSingleObject(probe->release, INFINITE) == WAIT_OBJECT_0);
}

static DWORD WINAPI gate_deliver(void *opaque)
{
    gate_probe *probe = (gate_probe *)opaque;
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 1u;
    event.value.raw_key.key = 'A';
    event.value.raw_key.pressed = LIB_TRUE;
    assert(lib_console_deliver_event(probe->console, &event) == LIB_STATUS_OK);
    SetEvent(probe->delivered);
    return 0u;
}

static DWORD WINAPI gate_detach(void *opaque)
{
    gate_probe *probe = (gate_probe *)opaque;
    assert(lib_console_set_event_sink(probe->console, NULL, NULL) == LIB_STATUS_OK);
    SetEvent(probe->detached);
    return 0u;
}

int main(void)
{
    gate_probe probe = { 0 };
    HANDLE delivery_thread;
    HANDLE detach_thread;
    lib_console_event event = { 0 };

    assert(lib_console_create(&probe.console) == LIB_STATUS_OK);
    assert(lib_console_bind_generation(probe.console, 1u) == LIB_STATUS_OK);
    probe.entered = CreateEventA(NULL, TRUE, FALSE, NULL);
    probe.release = CreateEventA(NULL, TRUE, FALSE, NULL);
    probe.delivered = CreateEventA(NULL, TRUE, FALSE, NULL);
    probe.detached = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(probe.entered && probe.release && probe.delivered && probe.detached);
    assert(lib_console_set_event_sink(probe.console, gate_sink, &probe) ==
        LIB_STATUS_OK);
    delivery_thread = CreateThread(NULL, 0u, gate_deliver, &probe, 0u, NULL);
    assert(delivery_thread != NULL);
    assert(WaitForSingleObject(probe.entered, INFINITE) == WAIT_OBJECT_0);
    detach_thread = CreateThread(NULL, 0u, gate_detach, &probe, 0u, NULL);
    assert(detach_thread != NULL);
    /* Detach is a completion barrier, not a best-effort pointer swap. */
    assert(WaitForSingleObject(probe.detached, 0u) == WAIT_TIMEOUT);
    SetEvent(probe.release);
    assert(WaitForSingleObject(probe.delivered, INFINITE) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(probe.detached, INFINITE) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(delivery_thread, INFINITE) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(detach_thread, INFINITE) == WAIT_OBJECT_0);
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 1u;
    assert(lib_console_deliver_event(probe.console, &event) == LIB_STATUS_INVALID_STATE);
    CloseHandle(delivery_thread);
    CloseHandle(detach_thread);
    CloseHandle(probe.entered);
    CloseHandle(probe.release);
    CloseHandle(probe.delivered);
    CloseHandle(probe.detached);
    lib_console_destroy(probe.console);
    return 0;
}
#else
int main(void) { return 0; }
#endif
