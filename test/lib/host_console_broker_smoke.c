#include "lib/console/console_interface.h"
#include "lib/console/binding_interface.h"
#include "lib/host/console_interface.h"
#include "lib/host/console_backend.h"

#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#endif

struct host_console_backend {
    lib_console *active;
    host_console_mode mode;
    lib_u32 generation;
    lib_bool cooked_request;
#ifdef _WIN32
    CRITICAL_SECTION transaction;
#endif
};

static int host_console_fail_next_activation;
static int host_console_fail_next_prepare;
static int host_console_fail_next_retirement;
static int host_console_prepare_saw_active;
static int host_console_wait_for_callback;
static unsigned activations;
static unsigned deactivations, disposals;
static unsigned input_resets, activation_attempts;
static lib_console *reset_console;
static lib_u32 reset_generation;
static host_console_backend *test_backend;
static void activated_sink(void *context, const lib_console_event *event)
{
    (void)context;
    if (event->kind == LIB_CONSOLE_EVENT_INPUT_RESET) return;
    assert(event->kind == LIB_CONSOLE_EVENT_ACTIVATED && event->binding_generation);
    ++activations;
}
#ifdef _WIN32
static HANDLE host_console_callback_entered;
static HANDLE host_console_callback_release;
static HANDLE host_console_callback_finished;
#endif

lib_status host_console_backend_create(host_console_backend **out_native)
{
    static host_console_backend native_console;
    native_console.active = LIB_NULL;
    test_backend = &native_console;
#ifdef _WIN32
    InitializeCriticalSection(&native_console.transaction);
#endif
    *out_native = &native_console;
    return LIB_STATUS_OK;
}

lib_status host_console_backend_destroy(host_console_backend *native_console)
{
    assert(native_console->active==NULL);
    ++disposals;
#ifdef _WIN32
    DeleteCriticalSection(&native_console->transaction);
#else
    (void)native_console;
#endif
    return LIB_STATUS_OK;
}

lib_status host_console_backend_prepare(host_console_backend *native_console,
    lib_console *console, host_console_mode mode)
{
    if (native_console != NULL && native_console->active != LIB_NULL)
        host_console_prepare_saw_active = 1;
    if (host_console_fail_next_prepare) {
        host_console_fail_next_prepare = 0;
        return LIB_STATUS_IO_ERROR;
    }
    return native_console == NULL || console == NULL ||
        (mode != HOST_CONSOLE_RAW_EVENTS && mode != HOST_CONSOLE_COOKED_LINES) ?
        LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

lib_status host_console_backend_activate(host_console_backend *native_console,
    lib_console *console, host_console_mode mode, lib_u32 generation,
    lib_bool restore_cooked_request)
{
    assert(reset_console == console && reset_generation == generation);
    assert(input_resets == ++activation_attempts);
    if (host_console_fail_next_activation > 0) {
        --host_console_fail_next_activation;
        return LIB_STATUS_IO_ERROR;
    }
    native_console->active = console;
    native_console->mode = mode;
    native_console->generation = generation;
    native_console->cooked_request = restore_cooked_request;
    return LIB_STATUS_OK;
}

lib_status host_console_backend_deactivate(host_console_backend *native_console,
    lib_bool *out_cooked_request)
{
    ++deactivations;
    if (host_console_fail_next_retirement) {
        host_console_fail_next_retirement = 0;
        return LIB_STATUS_IO_ERROR;
    }
#ifdef _WIN32
    if (host_console_wait_for_callback)
        assert(WaitForSingleObject(host_console_callback_finished, INFINITE) ==
            WAIT_OBJECT_0);
#endif
    native_console->active = LIB_NULL;
    if (out_cooked_request) *out_cooked_request = native_console->cooked_request;
    native_console->cooked_request = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status host_console_backend_request_cooked_line(
    host_console_backend *native_console)
{
    if (native_console == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    native_console->cooked_request = LIB_TRUE;
    return LIB_STATUS_OK;
}

void host_console_backend_lock_output(host_console_backend *native_console)
{ (void)native_console; }
void host_console_backend_unlock_output(host_console_backend *native_console)
{ (void)native_console; }
lib_status host_console_backend_write_bound(host_console_backend *native_console,
    lib_console *expected_console, lib_u32 expected_generation, const char *text,
    lib_size length)
{
    (void)text;
    return native_console->active != expected_console ||
        native_console->generation != expected_generation || length == 0u ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

lib_status host_console_backend_write_text_frame_bound(host_console_backend *native_console,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame)
{
    return native_console->active != expected_console ||
        native_console->generation != expected_generation || frame == LIB_NULL ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

#ifdef _WIN32
typedef struct host_console_replace_probe {
    host_console_broker *broker;
    lib_console *old_console;
    lib_console *next_console;
    HANDLE completed;
    lib_status status;
} host_console_replace_probe;

static void host_console_blocking_sink(void *opaque,
    const lib_console_event *event)
{
    (void)opaque;
    assert(event != LIB_NULL);
    SetEvent(host_console_callback_entered);
    assert(WaitForSingleObject(host_console_callback_release, INFINITE) ==
        WAIT_OBJECT_0);
}

static DWORD WINAPI host_console_deliver_old(void *opaque)
{
    lib_console *console = (lib_console *)opaque;
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 1u;
    event.value.raw_key.key = 'X';
    event.value.raw_key.pressed = LIB_TRUE;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    SetEvent(host_console_callback_finished);
    return 0u;
}

static DWORD WINAPI host_console_replace_thread(void *opaque)
{
    host_console_replace_probe *probe = (host_console_replace_probe *)opaque;
    probe->status = host_console_broker_replace(probe->broker, probe->old_console,
        probe->next_console, HOST_CONSOLE_RAW_EVENTS);
    SetEvent(probe->completed);
    return 0u;
}
#endif

#ifdef _WIN32
#include "lib/types/atomic.h"
static HANDLE cleanup_entered, cleanup_release, replacement_attempted;
static lib_console *cleanup_console;
static LONG challenger;
static LONG observed_busy;
static lib_status tracked_output_sink(lib_console *console,
    lib_console_output_sink sink, void *context)
{
    if (console == cleanup_console && sink == NULL) {
        SetEvent(cleanup_entered);
        assert(WaitForSingleObject(cleanup_release, INFINITE) == WAIT_OBJECT_0);
    }
    return lib_console_set_output_sink(console, sink, context);
}
void host_console_backend_lock_transaction(host_console_backend *backend)
{
    if (GetCurrentThreadId() == (DWORD)InterlockedCompareExchange(&challenger, 0, 0)) {
        int entered = TryEnterCriticalSection(&backend->transaction);
        InterlockedExchange(&observed_busy, !entered);
        SetEvent(replacement_attempted);
        if (entered) return;
    }
    EnterCriticalSection(&backend->transaction);
}
void host_console_backend_unlock_transaction(host_console_backend *backend)
{ LeaveCriticalSection(&backend->transaction); }
#define lib_console_set_output_sink tracked_output_sink
#endif
static lib_status tracked_delivery(lib_console *console, const lib_console_event *event)
{
    if (event->kind == LIB_CONSOLE_EVENT_INPUT_RESET) {
        assert(test_backend->active == NULL); /* Old reader already quiesced. */
        reset_console = console;
        reset_generation = event->binding_generation;
        ++input_resets;
    }
    return lib_console_deliver_event(console, event);
}
#define lib_console_deliver_event tracked_delivery
#include "lib/host/console.c"
#undef lib_console_deliver_event
#ifdef _WIN32
#undef lib_console_set_output_sink
static DWORD WINAPI reverse_replace(void *opaque)
{
    InterlockedExchange(&challenger, (LONG)GetCurrentThreadId());
    return host_console_replace_thread(opaque);
}
static void check_serial_cleanup(void)
{
    host_console_broker *broker;
    lib_console *a, *b;
    assert(lib_console_create(&a) == LIB_STATUS_OK);
    assert(lib_console_create(&b) == LIB_STATUS_OK);
    assert(host_console_broker_create(&broker, a, HOST_CONSOLE_RAW_EVENTS) == LIB_STATUS_OK);
    cleanup_entered = CreateEventA(NULL, TRUE, FALSE, NULL);
    cleanup_release = CreateEventA(NULL, TRUE, FALSE, NULL);
    replacement_attempted = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(cleanup_entered && cleanup_release && replacement_attempted);
    cleanup_console = a;
    host_console_replace_probe forward = { broker, a, b,
        CreateEventA(NULL, TRUE, FALSE, NULL), LIB_STATUS_INVALID_STATE };
    host_console_replace_probe reverse = { broker, b, a,
        CreateEventA(NULL, TRUE, FALSE, NULL), LIB_STATUS_INVALID_STATE };
    HANDLE first = CreateThread(NULL, 0, host_console_replace_thread, &forward, 0, NULL);
    assert(first && WaitForSingleObject(cleanup_entered, 5000) == WAIT_OBJECT_0);
    HANDLE second = CreateThread(NULL, 0, reverse_replace, &reverse, 0, NULL);
    assert(second && WaitForSingleObject(replacement_attempted, 5000) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&observed_busy, 0, 0) == 1);
    SetEvent(cleanup_release);
    assert(WaitForSingleObject(first, 5000) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(second, 5000) == WAIT_OBJECT_0);
    assert(forward.status == LIB_STATUS_OK && reverse.status == LIB_STATUS_OK);
    assert(lib_console_write_text(a, "a", 1u) == LIB_STATUS_OK);
    cleanup_console = NULL;
    InterlockedExchange(&challenger, 0);
    host_console_broker_destroy(broker);
    lib_console_release(a); lib_console_release(b);
    CloseHandle(first); CloseHandle(second);
    CloseHandle(forward.completed); CloseHandle(reverse.completed);
    CloseHandle(cleanup_entered); CloseHandle(cleanup_release); CloseHandle(replacement_attempted);
}
#endif

int main(void)
{
    lib_console *first = LIB_NULL;
    lib_console *second = LIB_NULL;
    host_console_broker *broker = LIB_NULL;
    host_console_broker *second_broker = LIB_NULL;

    assert(lib_console_create(&first) == LIB_STATUS_OK);
    assert(lib_console_create(&second) == LIB_STATUS_OK);
    host_console_fail_next_activation=1;
    assert(host_console_broker_create(&broker,first,HOST_CONSOLE_RAW_EVENTS)==LIB_STATUS_IO_ERROR);
    assert(!broker && deactivations==1 && disposals==1);
    assert(lib_console_write_text(first,"x",1)==LIB_STATUS_NOT_CURRENT);
    assert(lib_console_set_event_sink(first, activated_sink, NULL) == LIB_STATUS_OK);
    assert(host_console_broker_create(&broker, first,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_OK);
    assert(activations == 1);
    assert(host_console_broker_request_cooked_line(broker, second) ==
        LIB_STATUS_NOT_CURRENT);
    assert(host_console_broker_request_cooked_line(broker, first) ==
        LIB_STATUS_OK);
    host_console_fail_next_activation = 1;
    assert(host_console_broker_replace(broker, first, second,
        HOST_CONSOLE_RAW_EVENTS) == LIB_STATUS_IO_ERROR);
    assert(broker->backend->active == first && broker->backend->cooked_request);
    assert(activations == 2);
    broker->backend->cooked_request = LIB_FALSE; /* A delivered line is not restarted. */
    host_console_fail_next_activation = 1;
    assert(host_console_broker_replace(broker, first, second,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    assert(broker->backend->active == first && !broker->backend->cooked_request);
    assert(activations == 3);
    assert(lib_console_set_event_sink(first, NULL, NULL) == LIB_STATUS_OK);
    assert(host_console_broker_create(&second_broker, second,
        HOST_CONSOLE_RAW_EVENTS) == LIB_STATUS_INVALID_STATE);
    assert(second_broker == LIB_NULL);
    assert(lib_console_write_text(first, "a", 1u) == LIB_STATUS_OK);
    assert(host_console_broker_replace(broker, first, second,
        HOST_CONSOLE_RAW_EVENTS) == LIB_STATUS_OK);
    assert(host_console_prepare_saw_active);
    assert(lib_console_write_text(first, "a", 1u) == LIB_STATUS_NOT_CURRENT);
    assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_OK);
    /* Reader retirement is an explicit transaction boundary.  After a failed
       retirement cancellation may already have disturbed the old reader, so
       the broker fails closed: neither old nor next is advertised Current. */
    host_console_fail_next_retirement = 1;
    unsigned resets_before_failure = input_resets;
    assert(host_console_broker_replace(broker, second, first,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_NOT_CURRENT);
    assert(input_resets == resets_before_failure);
    assert(host_console_broker_replace(broker, second, first,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_INVALID_STATE);
    host_console_broker_destroy(broker);
    broker = NULL;

    /* The one native path is mode-agnostic: all four replacement pairs use
       the same retirement-before-activation contract. */
    assert(host_console_broker_create(&broker, second,
        HOST_CONSOLE_RAW_EVENTS) == LIB_STATUS_OK);
    assert(host_console_broker_replace(broker, second, first,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_OK);
    assert(host_console_broker_replace(broker, first, second,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_OK);
    assert(host_console_broker_replace(broker, second, first,
        HOST_CONSOLE_RAW_EVENTS) == LIB_STATUS_OK);
    assert(host_console_broker_replace(broker, first, second,
        HOST_CONSOLE_RAW_EVENTS) == LIB_STATUS_OK);
    host_console_fail_next_prepare = 1;
    resets_before_failure = input_resets;
    host_console_prepare_saw_active = 0;
    assert(host_console_broker_replace(broker, second, first,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    /* Preflight failure did not stop or detach the old current object. */
    assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_OK);
    assert(host_console_prepare_saw_active);
    assert(input_resets == resets_before_failure);
    host_console_fail_next_activation = 1;
    assert(host_console_broker_replace(broker, second, first,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_OK);
    /* If the next reader and the mandatory old-reader restoration both fail,
       the broker is terminally broken rather than falsely advertising old as
       Current. The application must stop; a later replacement cannot revive
       an indeterminate native Console transaction. */
    host_console_fail_next_activation = 2;
    assert(host_console_broker_replace(broker, second, first,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_NOT_CURRENT);
    assert(host_console_broker_replace(broker, second, first,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_INVALID_STATE);
    host_console_broker_destroy(broker);
    assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_NOT_CURRENT);
    assert(host_console_broker_create(&second_broker, first,
        HOST_CONSOLE_COOKED_LINES) == LIB_STATUS_OK);
    host_console_broker_destroy(second_broker);
#ifdef _WIN32
    /* A replacement models a reader join: the new Current Console cannot be
       committed while the old reader's callback remains in flight. */
    assert(host_console_broker_create(&broker, first,
        HOST_CONSOLE_RAW_EVENTS) == LIB_STATUS_OK);
    assert(lib_console_set_event_sink(first, host_console_blocking_sink, NULL) ==
        LIB_STATUS_OK);
    host_console_callback_entered = CreateEventA(NULL, TRUE, FALSE, NULL);
    host_console_callback_release = CreateEventA(NULL, TRUE, FALSE, NULL);
    host_console_callback_finished = CreateEventA(NULL, TRUE, FALSE, NULL);
    { host_console_replace_probe probe = { broker, first, second, NULL,
            LIB_STATUS_INVALID_STATE };
      HANDLE delivery;
      HANDLE replacement;
      probe.completed = CreateEventA(NULL, TRUE, FALSE, NULL);
      assert(probe.completed != NULL);
      delivery = CreateThread(NULL, 0u, host_console_deliver_old, first, 0u, NULL);
      assert(delivery != NULL);
      assert(WaitForSingleObject(host_console_callback_entered, INFINITE) ==
          WAIT_OBJECT_0);
      host_console_wait_for_callback = 1;
      replacement = CreateThread(NULL, 0u, host_console_replace_thread, &probe,
          0u, NULL);
      assert(replacement != NULL);
      assert(WaitForSingleObject(probe.completed, 0u) == WAIT_TIMEOUT);
      SetEvent(host_console_callback_release);
      assert(WaitForSingleObject(probe.completed, INFINITE) == WAIT_OBJECT_0);
      assert(probe.status == LIB_STATUS_OK);
      host_console_wait_for_callback = 0;
      CloseHandle(delivery);
      CloseHandle(replacement);
      CloseHandle(probe.completed); }
    CloseHandle(host_console_callback_entered);
    CloseHandle(host_console_callback_release);
    CloseHandle(host_console_callback_finished);
    host_console_broker_destroy(broker);
#endif
    lib_console_destroy(first);
    lib_console_destroy(second);
#ifdef _WIN32
    check_serial_cleanup();
#endif
    return 0;
}
