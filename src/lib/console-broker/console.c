#include "lib/console/binding_interface.h"
#include "lib/console-broker/console_interface.h"
#include "lib/console-broker/console_backend.h"

#include "lib/types/atomic.h"

struct console_broker {
    console_broker_backend *backend;
    lib_console *current;
    console_broker_mode current_mode;
    lib_u32 generation;
    struct console_broker_output_binding *current_output;
    /* A next-reader failure followed by a failed old-reader restore leaves
       native Console I/O unusable.  Do not masquerade that as a current
       logical Console; all later replacements fail and the app terminates. */
    lib_bool broken;
};

typedef struct console_broker_output_binding {
    console_broker_backend *backend;
    lib_console *console;
    lib_u32 generation;
} console_broker_output_binding;

static lib_atomic_flag console_broker_process_claimed = LIB_ATOMIC_FLAG_INITIALIZER;

static void console_broker_lock(console_broker *broker)
{
    console_broker_backend_lock_transaction(broker->backend);
}

static void console_broker_unlock(console_broker *broker)
{
    console_broker_backend_unlock_transaction(broker->backend);
}

static lib_status console_broker_write_bound(void *context, const char *text,
    lib_size length)
{
    console_broker_output_binding *binding = (console_broker_output_binding *)context;
    if (binding == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return console_broker_backend_write_bound(binding->backend,
        binding->console, binding->generation, text, length);
}

static lib_status console_broker_write_text_frame_bound(void *context,
    const lib_console_text_frame *frame)
{
    console_broker_output_binding *binding = (console_broker_output_binding *)context;
    if (binding == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return console_broker_backend_write_text_frame_bound(binding->backend,
        binding->console, binding->generation, frame);
}

static console_broker_output_binding *console_broker_output_binding_create(
    console_broker *broker, lib_console *console, lib_u32 generation)
{
    console_broker_output_binding *binding;
    if (broker == LIB_NULL || console == LIB_NULL || generation == 0u)
        return LIB_NULL;
    binding = lib_allocate_zero(1u, sizeof(*binding));
    if (binding != LIB_NULL) {
        binding->backend = broker->backend;
        binding->console = console;
        binding->generation = generation;
    }
    return binding;
}

static void console_broker_install_output_binding(lib_console *console,
    console_broker_output_binding *binding)
{
    const lib_console_output_binding output = {
        console_broker_write_bound, console_broker_write_text_frame_bound, binding
    };
    (void)lib_console_set_output_binding(console, &output);
}

static void console_broker_remove_output_binding(lib_console *console,
    console_broker_output_binding *binding)
{
    if (console != LIB_NULL)
        (void)lib_console_set_output_binding(console, LIB_NULL);
    lib_release(binding);
}

static lib_status console_broker_activate_bound(console_broker *broker,
    lib_console *console, console_broker_mode mode, lib_u32 generation,
    lib_bool restore_cooked_request)
{
    lib_status status;
    lib_console_event reset = { .kind = LIB_CONSOLE_EVENT_INPUT_RESET };
    status = lib_console_bind_generation(console, generation);
    if (status != LIB_STATUS_OK) return status;
    /* Old reader is quiescent. Reset local input before any next reader can
     * deliver; a logical Console without an input sink needs no reset work. */
    reset.binding_generation = generation;
    status = lib_console_deliver_event(console, &reset);
    if (status != LIB_STATUS_OK && status != LIB_STATUS_INVALID_STATE) return status;
    status = console_broker_backend_activate(broker->backend, console, mode,
        generation, restore_cooked_request);
    if (status != LIB_STATUS_OK) lib_console_invalidate_binding(console);
    return status;
}

static lib_status console_broker_notify_activation(console_broker *broker)
{
    lib_console_event event = { 0 };
    lib_status status;
    event.kind = LIB_CONSOLE_EVENT_ACTIVATED;
    event.binding_generation = broker->generation;
    status = lib_console_deliver_event(broker->current, &event);
    /* A logical Console may deliberately have no input sink. */
    return status == LIB_STATUS_INVALID_STATE ? LIB_STATUS_OK : status;
}

lib_status console_broker_create(console_broker **out_broker,
    lib_console *initial_console, console_broker_mode initial_mode)
{
    console_broker *broker;
    lib_status status;
    if (out_broker == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_broker = LIB_NULL;
    if (initial_console == LIB_NULL ||
        (initial_mode != CONSOLE_BROKER_RAW_EVENTS &&
         initial_mode != CONSOLE_BROKER_COOKED_LINES)) return LIB_STATUS_INVALID_ARGUMENT;
    if (lib_atomic_flag_test_and_set_explicit(&console_broker_process_claimed,
            LIB_MEMORY_ORDER_ACQ_REL)) return LIB_STATUS_INVALID_STATE;
    broker = lib_allocate_zero(1u, sizeof(*broker));
    if (broker == LIB_NULL) {
        lib_atomic_flag_clear_explicit(&console_broker_process_claimed,
            LIB_MEMORY_ORDER_RELEASE);
        return LIB_STATUS_NO_MEMORY;
    }
    status = console_broker_backend_create(&broker->backend);
    if (status == LIB_STATUS_OK) {
        broker->current = lib_console_retain(initial_console);
        broker->current_mode = initial_mode;
        broker->generation = 1u;
        broker->current_output = console_broker_output_binding_create(broker,
            broker->current, broker->generation);
        if (broker->current_output == LIB_NULL) status = LIB_STATUS_NO_MEMORY;
        if (status == LIB_STATUS_OK) {
            console_broker_install_output_binding(broker->current, broker->current_output);
            console_broker_backend_lock_output(broker->backend);
            status = console_broker_activate_bound(broker, broker->current,
                initial_mode, broker->generation, LIB_FALSE);
            console_broker_backend_unlock_output(broker->backend);
        }
    }
    if (status != LIB_STATUS_OK) {
        /* Activation can change native mode before failing. Broker owns the
         * same quiesce-before-dispose sequence here as in normal destruction. */
        if (broker->backend != LIB_NULL &&
            console_broker_backend_deactivate(broker->backend, LIB_NULL) != LIB_STATUS_OK) {
            /* No public broker exists yet.  The application treats this
             * impossible-to-quiesce native reader as terminal; do not invent
             * a half-broker ownership protocol for ordinary callers. */
            return status;
        }
        console_broker_remove_output_binding(broker->current, broker->current_output);
        broker->current_output = LIB_NULL;
        lib_console_release(broker->current);
        broker->current = LIB_NULL;
        (void)console_broker_backend_destroy(broker->backend);
        lib_release(broker);
        lib_atomic_flag_clear_explicit(&console_broker_process_claimed,
            LIB_MEMORY_ORDER_RELEASE);
        return status;
    }
    *out_broker = broker;
    return console_broker_notify_activation(broker);
}

lib_status console_broker_replace(console_broker *broker,
    lib_console *expected_current, lib_console *next_console,
    console_broker_mode next_mode)
{
    lib_status status;
    lib_console *old;
    lib_console *next;
    console_broker_output_binding *next_output;
    console_broker_output_binding *old_output;
    lib_u32 next_generation;
    lib_bool old_cooked_request = LIB_FALSE;
    if (broker == LIB_NULL || expected_current == LIB_NULL || next_console == LIB_NULL ||
        expected_current == next_console ||
        (next_mode != CONSOLE_BROKER_RAW_EVENTS &&
         next_mode != CONSOLE_BROKER_COOKED_LINES)) return LIB_STATUS_INVALID_ARGUMENT;
    console_broker_lock(broker);
    if (broker->broken || broker->current != expected_current) {
        console_broker_unlock(broker);
        return LIB_STATUS_INVALID_STATE;
    }
    old = broker->current;
    next_generation = broker->generation + 1u;
    if (next_generation == 0u) {
        console_broker_unlock(broker);
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    next = lib_console_retain(next_console);
    /* A failed next binding must leave the old native reader wholly active.
     * This is the non-disruptive phase of the replacement transaction. */
    status = console_broker_backend_prepare(broker->backend, next, next_mode);
    if (status != LIB_STATUS_OK) {
        lib_console_release(next);
        console_broker_unlock(broker);
        return status;
    }
    next_output = console_broker_output_binding_create(broker, next, next_generation);
    if (next_output == LIB_NULL) {
        lib_console_release(next);
        console_broker_unlock(broker);
        return LIB_STATUS_NO_MEMORY;
    }
    console_broker_install_output_binding(next, next_output);
    /* Every bound writer blocks behind this native gate. Once it opens,
       validation makes an old write NOT_CURRENT and a next write current. */
    console_broker_backend_lock_output(broker->backend);
    status = console_broker_backend_deactivate(broker->backend, &old_cooked_request);
    if (status != LIB_STATUS_OK) {
        /* A cancellation request whose reader never completed has no
           trustworthy Current Console.  Do not start next, and do not claim
           old remains usable: it may still consume input after this point.
           Fail closed so the product can terminate/report host I/O failure
           rather than operating two indeterminate input paths. */
        broker->broken = LIB_TRUE;
        lib_console_invalidate_binding(old);
        old_output = broker->current_output;
        broker->current_output = LIB_NULL;
        console_broker_backend_unlock_output(broker->backend);
        console_broker_remove_output_binding(next, next_output);
        lib_console_release(next); /* Caller still owns next. */
        console_broker_remove_output_binding(old, old_output);
        console_broker_unlock(broker);
        return status;
    }
    lib_console_invalidate_binding(old);
    status = console_broker_activate_bound(broker, next, next_mode, next_generation,
        LIB_FALSE);
    if (status != LIB_STATUS_OK) {
        lib_status restore_status;
        /* A failed next object never becomes visible.  Restore the old reader
         * before returning. If restoration itself fails, native Console I/O is
         * terminally broken and callers must not keep operating on a fiction
         * that old remains active. */
        restore_status = console_broker_activate_bound(broker, old, broker->current_mode,
            broker->generation, old_cooked_request);
        console_broker_backend_unlock_output(broker->backend);
        console_broker_remove_output_binding(next, next_output);
        lib_console_release(next); /* Caller still owns next. */
        if (restore_status != LIB_STATUS_OK) {
            broker->broken = LIB_TRUE;
            old_output = broker->current_output;
            broker->current_output = LIB_NULL;
            console_broker_remove_output_binding(old, old_output);
            console_broker_unlock(broker);
            return restore_status;
        }
        restore_status = console_broker_notify_activation(broker);
        console_broker_unlock(broker);
        return restore_status == LIB_STATUS_OK ? status : restore_status;
    }
    broker->current = next;
    broker->current_mode = next_mode;
    broker->generation = next_generation;
    old_output = broker->current_output;
    broker->current_output = next_output;
    console_broker_backend_unlock_output(broker->backend);
    /* The old binding is still safe until this setter has waited out any
       already-entered base write. Its native validation now rejects it. */
    console_broker_remove_output_binding(old, old_output);
    lib_console_release(old);
    status = console_broker_notify_activation(broker);
    if (status != LIB_STATUS_OK) broker->broken = LIB_TRUE;
    console_broker_unlock(broker);
    return status;
}

lib_status console_broker_request_cooked_line(console_broker *broker,
    lib_console *expected_current)
{
    lib_status status;

    if (broker == LIB_NULL || expected_current == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    console_broker_lock(broker);
    if (broker->broken || broker->current != expected_current ||
        broker->current_mode != CONSOLE_BROKER_COOKED_LINES) {
        console_broker_unlock(broker);
        return LIB_STATUS_NOT_CURRENT;
    }
    status = console_broker_backend_request_cooked_line(broker->backend);
    console_broker_unlock(broker);
    return status;
}

lib_status console_broker_cancel_cooked_line(console_broker *broker,
    lib_console *expected_current, lib_bool *out_completed)
{
    lib_status status;
    if (out_completed != LIB_NULL) *out_completed = LIB_FALSE;
    if (broker == LIB_NULL || expected_current == LIB_NULL || out_completed == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    console_broker_lock(broker);
    if (broker->broken || broker->current != expected_current ||
        broker->current_mode != CONSOLE_BROKER_COOKED_LINES) {
        console_broker_unlock(broker);
        return LIB_STATUS_NOT_CURRENT;
    }
    status = console_broker_backend_cancel_cooked_line(broker->backend, out_completed);
    if (status != LIB_STATUS_OK) broker->broken = LIB_TRUE;
    console_broker_unlock(broker);
    return status;
}

lib_status console_broker_destroy(console_broker *broker)
{
    lib_console *current;
    console_broker_output_binding *output;
    lib_status status;
    if (broker == LIB_NULL) return LIB_STATUS_OK;
    console_broker_lock(broker);
    current = broker->current;
    output = broker->current_output;
    console_broker_backend_lock_output(broker->backend);
    status = console_broker_backend_deactivate(broker->backend, LIB_NULL);
    if (status != LIB_STATUS_OK) {
        /* A live reader still references backend and current.  This is
           already a terminal broker failure; retain its process-lifetime
           state rather than releasing either object underneath that worker. */
        console_broker_backend_unlock_output(broker->backend);
        console_broker_unlock(broker);
        return status;
    }
    broker->current = LIB_NULL;
    broker->current_output = LIB_NULL;
    if (current != LIB_NULL) lib_console_invalidate_binding(current);
    console_broker_backend_unlock_output(broker->backend);
    console_broker_unlock(broker);
    console_broker_remove_output_binding(current, output);
    lib_console_release(current);
    status = console_broker_backend_destroy(broker->backend);
    if (status != LIB_STATUS_OK) return status;
    lib_release(broker);
    lib_atomic_flag_clear_explicit(&console_broker_process_claimed, LIB_MEMORY_ORDER_RELEASE);
    return LIB_STATUS_OK;
}
