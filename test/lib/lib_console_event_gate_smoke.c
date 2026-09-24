#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/console/console_interface.h"
#include "lib/console/binding_interface.h"


#ifdef _WIN32
#include "lib/types/win32/test.h"

typedef struct gate_probe {
    lib_console *console;
    lib_win32_handle entered;
    lib_win32_handle release;
    lib_win32_handle delivered;
    lib_win32_handle detached;
} gate_probe;

static void gate_sink(void *opaque, const lib_console_event *event)
{
    gate_probe *probe = (gate_probe *)opaque;
    lib_test_assert(probe != LIB_NULL && event != LIB_NULL);
    lib_win32_set_event(probe->entered);
    lib_test_assert(lib_win32_wait_for_single_object(probe->release, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
}

static lib_win32_dword LIB_WIN32_WINAPI gate_deliver(void *opaque)
{
    gate_probe *probe = (gate_probe *)opaque;
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 1u;
    event.value.raw_key.key = 'A';
    event.value.raw_key.pressed = LIB_TRUE;
    lib_test_assert(lib_console_deliver_event(probe->console, &event) == LIB_STATUS_OK);
    lib_win32_set_event(probe->delivered);
    return 0u;
}

static lib_win32_dword LIB_WIN32_WINAPI gate_detach(void *opaque)
{
    gate_probe *probe = (gate_probe *)opaque;
    lib_test_assert(lib_console_set_event_sink(probe->console, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    lib_win32_set_event(probe->detached);
    return 0u;
}

int main(void)
{
    gate_probe probe = { 0 };
    lib_win32_handle delivery_thread;
    lib_win32_handle detach_thread;
    lib_console_event event = { 0 };

    lib_test_assert(lib_console_create(&probe.console) == LIB_STATUS_OK);
    lib_test_assert(lib_console_bind_generation(probe.console, 1u) == LIB_STATUS_OK);
    probe.entered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    probe.release = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    probe.delivered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    probe.detached = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_test_assert(probe.entered && probe.release && probe.delivered && probe.detached);
    lib_test_assert(lib_console_set_event_sink(probe.console, gate_sink, &probe) ==
        LIB_STATUS_OK);
    delivery_thread = lib_win32_create_thread(LIB_NULL, 0u, gate_deliver, &probe, 0u, LIB_NULL);
    lib_test_assert(delivery_thread != LIB_NULL);
    lib_test_assert(lib_win32_wait_for_single_object(probe.entered, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    detach_thread = lib_win32_create_thread(LIB_NULL, 0u, gate_detach, &probe, 0u, LIB_NULL);
    lib_test_assert(detach_thread != LIB_NULL);
    /* Detach is a completion barrier, not a best-effort pointer swap. */
    lib_test_assert(lib_win32_wait_for_single_object(probe.detached, 0u) == LIB_WIN32_WAIT_TIMEOUT);
    lib_win32_set_event(probe.release);
    lib_test_assert(lib_win32_wait_for_single_object(probe.delivered, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_wait_for_single_object(probe.detached, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_wait_for_single_object(delivery_thread, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_wait_for_single_object(detach_thread, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 1u;
    lib_test_assert(lib_console_deliver_event(probe.console, &event) == LIB_STATUS_INVALID_STATE);
    lib_win32_close_handle(delivery_thread);
    lib_win32_close_handle(detach_thread);
    lib_win32_close_handle(probe.entered);
    lib_win32_close_handle(probe.release);
    lib_win32_close_handle(probe.delivered);
    lib_win32_close_handle(probe.detached);
    lib_console_release(probe.console);
    return 0;
}
#else
int main(void) { return 0; }
#endif
