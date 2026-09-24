#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/console/console_interface.h"
#include "lib/console/binding_interface.h"
#include "lib/console-broker/console_interface.h"
#include "lib/console-broker/console_backend.h"


#ifdef _WIN32
#include "lib/types/win32/test.h"
#endif

struct console_broker_backend {
    lib_console *active;
    console_broker_mode mode;
    lib_u32 generation;
    lib_bool cooked_request;
#ifdef _WIN32
    lib_win32_critical_section transaction;
#endif
};

static lib_i32 console_broker_fail_next_activation;
static const lib_console_text_frame output_frame = { .columns = 80u, .rows = 25u };
static lib_i32 console_broker_fail_next_prepare;
static lib_i32 console_broker_fail_next_retirement;
static lib_i32 console_broker_prepare_saw_active;
static lib_i32 console_broker_wait_for_callback;
static lib_u32 activations;
static lib_u32 deactivations, disposals;
static lib_u32 input_resets, activation_attempts;
static lib_console *reset_console;
static lib_u32 reset_generation;
static console_broker_backend *test_backend;
static void activated_sink(void *context, const lib_console_event *event)
{
    (void)context;
    if (event->kind == LIB_CONSOLE_EVENT_INPUT_RESET) return;
    lib_test_assert(event->kind == LIB_CONSOLE_EVENT_ACTIVATED && event->binding_generation);
    ++activations;
}
#ifdef _WIN32
static lib_win32_handle console_broker_callback_entered;
static lib_win32_handle console_broker_callback_release;
static lib_win32_handle console_broker_callback_finished;
#endif

lib_status console_broker_backend_create(console_broker_backend **out_native)
{
    static console_broker_backend native_console;
    native_console.active = LIB_NULL;
    test_backend = &native_console;
#ifdef _WIN32
    InitializeCriticalSection(&native_console.transaction);
#endif
    *out_native = &native_console;
    return LIB_STATUS_OK;
}

lib_status console_broker_backend_destroy(console_broker_backend *native_console)
{
    lib_test_assert(native_console->active==LIB_NULL);
    ++disposals;
#ifdef _WIN32
    lib_win32_delete_critical_section(&native_console->transaction);
#else
    (void)native_console;
#endif
    return LIB_STATUS_OK;
}

lib_status console_broker_backend_prepare(console_broker_backend *native_console,
    lib_console *console, console_broker_mode mode)
{
    if (native_console != LIB_NULL && native_console->active != LIB_NULL)
        console_broker_prepare_saw_active = 1;
    if (console_broker_fail_next_prepare) {
        console_broker_fail_next_prepare = 0;
        return LIB_STATUS_IO_ERROR;
    }
    return native_console == LIB_NULL || console == LIB_NULL ||
        (mode != CONSOLE_BROKER_RAW_EVENTS && mode != CONSOLE_BROKER_COOKED_LINES) ?
        LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

lib_status console_broker_backend_activate(console_broker_backend *native_console,
    lib_console *console, console_broker_mode mode, lib_u32 generation,
    lib_bool restore_cooked_request)
{
    lib_test_assert(reset_console == console && reset_generation == generation);
    lib_test_assert(input_resets == ++activation_attempts);
    if (console_broker_fail_next_activation > 0) {
        --console_broker_fail_next_activation;
        return LIB_STATUS_IO_ERROR;
    }
    native_console->active = console;
    native_console->mode = mode;
    native_console->generation = generation;
    native_console->cooked_request = restore_cooked_request;
    return LIB_STATUS_OK;
}

lib_status console_broker_backend_deactivate(console_broker_backend *native_console,
    lib_bool *out_cooked_request)
{
    ++deactivations;
    if (console_broker_fail_next_retirement) {
        console_broker_fail_next_retirement = 0;
        return LIB_STATUS_IO_ERROR;
    }
#ifdef _WIN32
    if (console_broker_wait_for_callback)
        lib_test_assert(lib_win32_wait_for_single_object(console_broker_callback_finished, LIB_WIN32_INFINITE) ==
            LIB_WIN32_WAIT_OBJECT_0);
#endif
    native_console->active = LIB_NULL;
    if (out_cooked_request) *out_cooked_request = native_console->cooked_request;
    native_console->cooked_request = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status console_broker_backend_request_cooked_line(
    console_broker_backend *native_console)
{
    if (native_console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    native_console->cooked_request = LIB_TRUE;
    return LIB_STATUS_OK;
}

void console_broker_backend_lock_output(console_broker_backend *native_console)
{ (void)native_console; }
lib_status console_broker_backend_cancel_cooked_line(console_broker_backend *native_console,
    lib_bool *out_completed)
{
    *out_completed = !native_console->cooked_request;
    native_console->cooked_request = LIB_FALSE;
    return LIB_STATUS_OK;
}
void console_broker_backend_unlock_output(console_broker_backend *native_console)
{ (void)native_console; }
lib_status console_broker_backend_write_bound(console_broker_backend *native_console,
    lib_console *expected_console, lib_u32 expected_generation, const char *text,
    lib_size length)
{
    (void)text;
    return native_console->active != expected_console ||
        native_console->generation != expected_generation || length == 0u ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

lib_status console_broker_backend_write_text_frame_bound(console_broker_backend *native_console,
    lib_console *expected_console, lib_u32 expected_generation,
    const lib_console_text_frame *frame)
{
    return native_console->active != expected_console ||
        native_console->generation != expected_generation || frame == LIB_NULL ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

#ifdef _WIN32
typedef struct console_broker_replace_probe {
    console_broker *broker;
    lib_console *old_console;
    lib_console *next_console;
    lib_win32_handle completed;
    lib_status status;
} console_broker_replace_probe;

static void console_broker_blocking_sink(void *opaque,
    const lib_console_event *event)
{
    (void)opaque;
    lib_test_assert(event != LIB_NULL);
    lib_win32_set_event(console_broker_callback_entered);
    lib_test_assert(lib_win32_wait_for_single_object(console_broker_callback_release, LIB_WIN32_INFINITE) ==
        LIB_WIN32_WAIT_OBJECT_0);
}

static lib_win32_dword LIB_WIN32_WINAPI console_broker_deliver_old(void *opaque)
{
    lib_console *console = (lib_console *)opaque;
    lib_console_event event = { 0 };
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 1u;
    event.value.raw_key.key = 'X';
    event.value.raw_key.pressed = LIB_TRUE;
    lib_test_assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    lib_win32_set_event(console_broker_callback_finished);
    return 0u;
}

static lib_win32_dword LIB_WIN32_WINAPI console_broker_replace_thread(void *opaque)
{
    console_broker_replace_probe *probe = (console_broker_replace_probe *)opaque;
    probe->status = console_broker_replace(probe->broker, probe->old_console,
        probe->next_console, CONSOLE_BROKER_RAW_EVENTS);
    lib_win32_set_event(probe->completed);
    return 0u;
}
#endif

#ifdef _WIN32
#include "lib/types/atomic.h"
static lib_win32_handle cleanup_entered, cleanup_release, replacement_attempted;
static lib_console *cleanup_console;
static lib_win32_long challenger;
static lib_win32_long observed_busy;
static lib_status tracked_output_binding(lib_console *console,
    const lib_console_output_binding *binding)
{
    if (console == cleanup_console && binding == LIB_NULL) {
        lib_win32_set_event(cleanup_entered);
        lib_test_assert(lib_win32_wait_for_single_object(cleanup_release, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
    }
    return lib_console_set_output_binding(console, binding);
}
void console_broker_backend_lock_transaction(console_broker_backend *backend)
{
    if (lib_win32_get_current_thread_id() == (lib_win32_dword)lib_win32_interlocked_compare_exchange(&challenger, 0, 0)) {
        lib_i32 entered = lib_win32_try_enter_critical_section(&backend->transaction);
        lib_win32_interlocked_exchange(&observed_busy, !entered);
        lib_win32_set_event(replacement_attempted);
        if (entered) return;
    }
    lib_win32_enter_critical_section(&backend->transaction);
}
void console_broker_backend_unlock_transaction(console_broker_backend *backend)
{ lib_win32_leave_critical_section(&backend->transaction); }
#define lib_console_set_output_binding tracked_output_binding
#endif
static lib_status tracked_delivery(lib_console *console, const lib_console_event *event)
{
    if (event->kind == LIB_CONSOLE_EVENT_INPUT_RESET) {
        lib_test_assert(test_backend->active == LIB_NULL); /* Old reader already quiesced. */
        reset_console = console;
        reset_generation = event->binding_generation;
        ++input_resets;
    }
    return lib_console_deliver_event(console, event);
}
#define lib_console_deliver_event tracked_delivery
#include "lib/console-broker/console.c"
#undef lib_console_deliver_event
#ifdef _WIN32
#undef lib_console_set_output_binding
static lib_win32_dword LIB_WIN32_WINAPI reverse_replace(void *opaque)
{
    lib_win32_interlocked_exchange(&challenger, (lib_win32_long)lib_win32_get_current_thread_id());
    return console_broker_replace_thread(opaque);
}
static void check_serial_cleanup(void)
{
    console_broker *broker;
    lib_console *a, *b;
    lib_test_assert(lib_console_create(&a) == LIB_STATUS_OK);
    lib_test_assert(lib_console_create(&b) == LIB_STATUS_OK);
    lib_test_assert(console_broker_create(&broker, a, CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_OK);
    cleanup_entered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    cleanup_release = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    replacement_attempted = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_test_assert(cleanup_entered && cleanup_release && replacement_attempted);
    cleanup_console = a;
    console_broker_replace_probe forward = { broker, a, b,
        lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL), LIB_STATUS_INVALID_STATE };
    console_broker_replace_probe reverse = { broker, b, a,
        lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL), LIB_STATUS_INVALID_STATE };
    lib_win32_handle first = lib_win32_create_thread(LIB_NULL, 0, console_broker_replace_thread, &forward, 0, LIB_NULL);
    lib_test_assert(first && lib_win32_wait_for_single_object(cleanup_entered, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_win32_handle second = lib_win32_create_thread(LIB_NULL, 0, reverse_replace, &reverse, 0, LIB_NULL);
    lib_test_assert(second && lib_win32_wait_for_single_object(replacement_attempted, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_interlocked_compare_exchange(&observed_busy, 0, 0) == 1);
    lib_win32_set_event(cleanup_release);
    lib_test_assert(lib_win32_wait_for_single_object(first, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(lib_win32_wait_for_single_object(second, 5000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(forward.status == LIB_STATUS_OK && reverse.status == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text(a, "a", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text_frame(a, &output_frame) == LIB_STATUS_OK);
    cleanup_console = LIB_NULL;
    lib_win32_interlocked_exchange(&challenger, 0);
    console_broker_destroy(broker);
    lib_console_release(a); lib_console_release(b);
    lib_win32_close_handle(first); lib_win32_close_handle(second);
    lib_win32_close_handle(forward.completed); lib_win32_close_handle(reverse.completed);
    lib_win32_close_handle(cleanup_entered); lib_win32_close_handle(cleanup_release); lib_win32_close_handle(replacement_attempted);
}
#endif

int main(void)
{
    lib_console *first = LIB_NULL;
    lib_console *second = LIB_NULL;
    console_broker *broker = LIB_NULL;
    console_broker *second_broker = LIB_NULL;
    lib_bool completed;

    lib_test_assert(lib_console_create(&first) == LIB_STATUS_OK);
    lib_test_assert(lib_console_create(&second) == LIB_STATUS_OK);
    console_broker_fail_next_activation=1;
    lib_test_assert(console_broker_create(&broker,first,CONSOLE_BROKER_RAW_EVENTS)==LIB_STATUS_IO_ERROR);
    lib_test_assert(!broker && deactivations==1 && disposals==1);
    lib_test_assert(lib_console_write_text(first,"x",1)==LIB_STATUS_OK);
    lib_test_assert(lib_console_set_event_sink(first, activated_sink, LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(console_broker_create(&broker, first,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_OK);
    lib_test_assert(activations == 1);
    lib_test_assert(console_broker_request_cooked_line(broker, second) ==
        LIB_STATUS_INVALID_STATE);
    lib_test_assert(console_broker_request_cooked_line(broker, first) ==
        LIB_STATUS_OK);
    lib_test_assert(console_broker_cancel_cooked_line(broker, second, &completed) ==
        LIB_STATUS_INVALID_STATE && broker->backend->cooked_request);
    lib_test_assert(console_broker_cancel_cooked_line(broker, first, &completed) ==
        LIB_STATUS_OK && !completed && !broker->backend->cooked_request);
    lib_test_assert(console_broker_request_cooked_line(broker, first) == LIB_STATUS_OK);
    console_broker_fail_next_activation = 1;
    lib_test_assert(console_broker_replace(broker, first, second,
        CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_IO_ERROR);
    lib_test_assert(broker->backend->active == first && broker->backend->cooked_request);
    lib_test_assert(activations == 2);
    broker->backend->cooked_request = LIB_FALSE; /* A delivered line is not restarted. */
    console_broker_fail_next_activation = 1;
    lib_test_assert(console_broker_replace(broker, first, second,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    lib_test_assert(broker->backend->active == first && !broker->backend->cooked_request);
    lib_test_assert(activations == 3);
    lib_test_assert(lib_console_set_event_sink(first, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    lib_test_assert(console_broker_create(&second_broker, second,
        CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(second_broker == LIB_NULL);
    lib_test_assert(lib_console_write_text(first, "a", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text_frame(first, &output_frame) == LIB_STATUS_OK);
    lib_test_assert(console_broker_replace(broker, first, second,
        CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_OK);
    lib_test_assert(console_broker_prepare_saw_active);
    lib_test_assert(lib_console_write_text(first, "a", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text_frame(first, &output_frame) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text_frame(second, &output_frame) == LIB_STATUS_OK);
    /* Reader retirement is an explicit transaction boundary.  After a failed
       retirement cancellation may already have disturbed the old reader, so
       the broker fails closed: neither old nor next is advertised Current. */
    console_broker_fail_next_retirement = 1;
    lib_u32 resets_before_failure = input_resets;
    lib_test_assert(console_broker_replace(broker, second, first,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text_frame(second, &output_frame) == LIB_STATUS_OK);
    lib_test_assert(input_resets == resets_before_failure);
    lib_test_assert(console_broker_replace(broker, second, first,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_INVALID_STATE);
    console_broker_destroy(broker);
    broker = LIB_NULL;

    /* The one native path is mode-agnostic: all four replacement pairs use
       the same retirement-before-activation contract. */
    lib_test_assert(console_broker_create(&broker, second,
        CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_OK);
    lib_test_assert(console_broker_replace(broker, second, first,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_OK);
    lib_test_assert(console_broker_replace(broker, first, second,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_OK);
    lib_test_assert(console_broker_replace(broker, second, first,
        CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_OK);
    lib_test_assert(console_broker_replace(broker, first, second,
        CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_OK);
    console_broker_fail_next_prepare = 1;
    resets_before_failure = input_resets;
    console_broker_prepare_saw_active = 0;
    lib_test_assert(console_broker_replace(broker, second, first,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    /* Preflight failure did not stop or detach the old current object. */
    lib_test_assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text_frame(second, &output_frame) == LIB_STATUS_OK);
    lib_test_assert(console_broker_prepare_saw_active);
    lib_test_assert(input_resets == resets_before_failure);
    console_broker_fail_next_activation = 1;
    lib_test_assert(console_broker_replace(broker, second, first,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text_frame(second, &output_frame) == LIB_STATUS_OK);
    /* If the next reader and the mandatory old-reader restoration both fail,
       the broker is terminally broken rather than falsely advertising old as
       Current. The application must stop; a later replacement cannot revive
       an indeterminate native Console transaction. */
    console_broker_fail_next_activation = 2;
    lib_test_assert(console_broker_replace(broker, second, first,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_IO_ERROR);
    lib_test_assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text_frame(second, &output_frame) == LIB_STATUS_OK);
    lib_test_assert(console_broker_replace(broker, second, first,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_INVALID_STATE);
    console_broker_destroy(broker);
    lib_test_assert(lib_console_write_text(second, "b", 1u) == LIB_STATUS_OK);
    lib_test_assert(lib_console_write_text_frame(second, &output_frame) == LIB_STATUS_OK);
    lib_test_assert(console_broker_create(&second_broker, first,
        CONSOLE_BROKER_COOKED_LINES) == LIB_STATUS_OK);
    console_broker_destroy(second_broker);
#ifdef _WIN32
    /* A replacement models a reader join: the new Current Console cannot be
       committed while the old reader's callback remains in flight. */
    lib_test_assert(console_broker_create(&broker, first,
        CONSOLE_BROKER_RAW_EVENTS) == LIB_STATUS_OK);
    lib_test_assert(lib_console_set_event_sink(first, console_broker_blocking_sink, LIB_NULL) ==
        LIB_STATUS_OK);
    console_broker_callback_entered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    console_broker_callback_release = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    console_broker_callback_finished = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    { console_broker_replace_probe probe = { broker, first, second, LIB_NULL,
            LIB_STATUS_INVALID_STATE };
      lib_win32_handle delivery;
      lib_win32_handle replacement;
      probe.completed = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
      lib_test_assert(probe.completed != LIB_NULL);
      delivery = lib_win32_create_thread(LIB_NULL, 0u, console_broker_deliver_old, first, 0u, LIB_NULL);
      lib_test_assert(delivery != LIB_NULL);
      lib_test_assert(lib_win32_wait_for_single_object(console_broker_callback_entered, LIB_WIN32_INFINITE) ==
          LIB_WIN32_WAIT_OBJECT_0);
      console_broker_wait_for_callback = 1;
      replacement = lib_win32_create_thread(LIB_NULL, 0u, console_broker_replace_thread, &probe,
          0u, LIB_NULL);
      lib_test_assert(replacement != LIB_NULL);
      lib_test_assert(lib_win32_wait_for_single_object(probe.completed, 0u) == LIB_WIN32_WAIT_TIMEOUT);
      lib_win32_set_event(console_broker_callback_release);
      lib_test_assert(lib_win32_wait_for_single_object(probe.completed, LIB_WIN32_INFINITE) == LIB_WIN32_WAIT_OBJECT_0);
      lib_test_assert(probe.status == LIB_STATUS_OK);
      console_broker_wait_for_callback = 0;
      lib_win32_close_handle(delivery);
      lib_win32_close_handle(replacement);
      lib_win32_close_handle(probe.completed); }
    lib_win32_close_handle(console_broker_callback_entered);
    lib_win32_close_handle(console_broker_callback_release);
    lib_win32_close_handle(console_broker_callback_finished);
    console_broker_destroy(broker);
#endif
    lib_console_release(first);
    lib_console_release(second);
#ifdef _WIN32
    check_serial_cleanup();
#endif
    return 0;
}
