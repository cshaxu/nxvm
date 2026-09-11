#include "lib/console/console.h"
#include "lib/host/console_interface.h"
#include "lib/host/console_backend.h"

#include "lib/types/atomic.h"
#include <stdlib.h>

struct host_console_broker {
    lib_atomic_flag lock;
    host_console_backend *backend;
    lib_console *current;
    host_console_mode current_mode;
    lib_u32 generation;
    struct host_console_output_binding *current_output;
    /* A next-reader failure followed by a failed old-reader restore leaves
       native Console I/O unusable.  Do not masquerade that as a current
       logical Console; all later replacements fail and the app terminates. */
    lib_bool broken;
};

typedef struct host_console_output_binding {
    host_console_backend *backend;
    lib_console *console;
    lib_u32 generation;
} host_console_output_binding;

static lib_atomic_flag host_console_process_claimed = LIB_ATOMIC_FLAG_INITIALIZER;

static void host_console_lock(host_console_broker *broker)
{
    while (lib_atomic_flag_test_and_set_explicit(&broker->lock,
        LIB_MEMORY_ORDER_ACQUIRE)) { }
}

static void host_console_unlock(host_console_broker *broker)
{
    lib_atomic_flag_clear_explicit(&broker->lock, LIB_MEMORY_ORDER_RELEASE);
}

static lib_status host_console_write_bound(void *context, const char *text,
    lib_size length)
{
    host_console_output_binding *binding = (host_console_output_binding *)context;
    if (binding == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return host_console_backend_write_bound(binding->backend,
        binding->console, binding->generation, text, length);
}

static lib_status host_console_write_text_frame_bound(void *context,
    const lib_console_text_frame *frame)
{
    host_console_output_binding *binding = (host_console_output_binding *)context;
    if (binding == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return host_console_backend_write_text_frame_bound(binding->backend,
        binding->console, binding->generation, frame);
}

static host_console_output_binding *host_console_output_binding_create(
    host_console_broker *broker, lib_console *console, lib_u32 generation)
{
    host_console_output_binding *binding;
    if (broker == LIB_NULL || console == LIB_NULL || generation == 0u)
        return LIB_NULL;
    binding = calloc(1u, sizeof(*binding));
    if (binding != LIB_NULL) {
        binding->backend = broker->backend;
        binding->console = console;
        binding->generation = generation;
    }
    return binding;
}

static lib_status host_console_install_output_binding(lib_console *console,
    host_console_output_binding *binding)
{
    lib_status status;
    if (console == LIB_NULL || binding == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = lib_console_set_output_sink(console, host_console_write_bound, binding);
    if (status == LIB_STATUS_OK)
        status = lib_console_set_text_frame_sink(console,
            host_console_write_text_frame_bound, binding);
    if (status != LIB_STATUS_OK)
        (void)lib_console_set_output_sink(console, LIB_NULL, LIB_NULL);
    return status;
}

static void host_console_remove_output_binding(lib_console *console,
    host_console_output_binding *binding)
{
    if (console == LIB_NULL) return;
    (void)lib_console_set_output_sink(console, LIB_NULL, LIB_NULL);
    (void)lib_console_set_text_frame_sink(console, LIB_NULL, LIB_NULL);
    free(binding);
}

static lib_status host_console_activate_bound(host_console_broker *broker,
    lib_console *console, host_console_mode mode, lib_u32 generation)
{
    lib_status status;
    status = lib_console_bind_generation(console, generation);
    if (status != LIB_STATUS_OK) return status;
    status = host_console_backend_activate(broker->backend, console, mode,
        generation);
    if (status != LIB_STATUS_OK) lib_console_invalidate_binding(console);
    return status;
}

lib_status host_console_broker_create(host_console_broker **out_broker,
    lib_console *initial_console, host_console_mode initial_mode)
{
    host_console_broker *broker;
    lib_status status;
    if (out_broker == LIB_NULL || initial_console == LIB_NULL ||
        (initial_mode != HOST_CONSOLE_RAW_EVENTS &&
         initial_mode != HOST_CONSOLE_COOKED_LINES)) return LIB_STATUS_INVALID_ARGUMENT;
    *out_broker = LIB_NULL;
    if (lib_atomic_flag_test_and_set_explicit(&host_console_process_claimed,
            LIB_MEMORY_ORDER_ACQ_REL)) return LIB_STATUS_INVALID_STATE;
    broker = calloc(1u, sizeof(*broker));
    if (broker == LIB_NULL) {
        lib_atomic_flag_clear_explicit(&host_console_process_claimed,
            LIB_MEMORY_ORDER_RELEASE);
        return LIB_STATUS_NO_MEMORY;
    }
    lib_atomic_flag_clear(&broker->lock);
    status = host_console_backend_create(&broker->backend);
    if (status == LIB_STATUS_OK) {
        broker->current = lib_console_retain(initial_console);
        broker->current_mode = initial_mode;
        broker->generation = 1u;
        broker->current_output = host_console_output_binding_create(broker,
            broker->current, broker->generation);
        if (broker->current_output == LIB_NULL) status = LIB_STATUS_NO_MEMORY;
        if (status == LIB_STATUS_OK)
            status = host_console_install_output_binding(broker->current,
                broker->current_output);
        if (status == LIB_STATUS_OK) {
            host_console_backend_lock_output(broker->backend);
            status = host_console_activate_bound(broker, broker->current,
                initial_mode, broker->generation);
            host_console_backend_unlock_output(broker->backend);
        }
    }
    if (status != LIB_STATUS_OK) {
        host_console_remove_output_binding(broker->current, broker->current_output);
        if (broker->current != LIB_NULL) lib_console_release(broker->current);
        host_console_backend_destroy(broker->backend);
        free(broker);
        lib_atomic_flag_clear_explicit(&host_console_process_claimed,
            LIB_MEMORY_ORDER_RELEASE);
        return status;
    }
    *out_broker = broker;
    return LIB_STATUS_OK;
}

lib_status host_console_broker_replace(host_console_broker *broker,
    lib_console *expected_current, lib_console *next_console,
    host_console_mode next_mode)
{
    lib_status status;
    lib_console *old;
    lib_console *next;
    host_console_output_binding *next_output;
    host_console_output_binding *old_output;
    lib_u32 next_generation;
    if (broker == LIB_NULL || expected_current == LIB_NULL || next_console == LIB_NULL ||
        expected_current == next_console ||
        (next_mode != HOST_CONSOLE_RAW_EVENTS &&
         next_mode != HOST_CONSOLE_COOKED_LINES)) return LIB_STATUS_INVALID_ARGUMENT;
    host_console_lock(broker);
    if (broker->broken || broker->current != expected_current) {
        host_console_unlock(broker);
        return LIB_STATUS_INVALID_STATE;
    }
    old = broker->current;
    next_generation = broker->generation + 1u;
    if (next_generation == 0u) {
        host_console_unlock(broker);
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    next = lib_console_retain(next_console);
    /* A failed next binding must leave the old native reader wholly active.
     * This is the non-disruptive phase of the replacement transaction. */
    status = host_console_backend_prepare(broker->backend, next, next_mode);
    if (status != LIB_STATUS_OK) {
        lib_console_release(next);
        host_console_unlock(broker);
        return status;
    }
    next_output = host_console_output_binding_create(broker, next, next_generation);
    if (next_output == LIB_NULL) {
        host_console_backend_discard_prepare(broker->backend);
        lib_console_release(next);
        host_console_unlock(broker);
        return LIB_STATUS_NO_MEMORY;
    }
    status = host_console_install_output_binding(next, next_output);
    if (status != LIB_STATUS_OK) {
        host_console_backend_discard_prepare(broker->backend);
        free(next_output);
        lib_console_release(next);
        host_console_unlock(broker);
        return status;
    }
    /* Every bound writer blocks behind this native gate. Once it opens,
       validation makes an old write NOT_CURRENT and a next write current. */
    host_console_backend_lock_output(broker->backend);
    status = host_console_backend_deactivate(broker->backend);
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
        host_console_backend_unlock_output(broker->backend);
        host_console_backend_discard_prepare(broker->backend);
        host_console_remove_output_binding(next, next_output);
        lib_console_release(next);
        host_console_unlock(broker);
        host_console_remove_output_binding(old, old_output);
        return status;
    }
    lib_console_invalidate_binding(old);
    status = host_console_activate_bound(broker, next, next_mode, next_generation);
    if (status != LIB_STATUS_OK) {
        lib_status restore_status;
        /* A failed next object never becomes visible.  Restore the old reader
         * before returning. If restoration itself fails, native Console I/O is
         * terminally broken and callers must not keep operating on a fiction
         * that old remains active. */
        restore_status = host_console_activate_bound(broker, old, broker->current_mode,
            broker->generation);
        host_console_backend_discard_prepare(broker->backend);
        host_console_backend_unlock_output(broker->backend);
        host_console_remove_output_binding(next, next_output);
        lib_console_release(next);
        if (restore_status != LIB_STATUS_OK) {
            broker->broken = LIB_TRUE;
            old_output = broker->current_output;
            broker->current_output = LIB_NULL;
            host_console_unlock(broker);
            host_console_remove_output_binding(old, old_output);
            return restore_status;
        }
        host_console_unlock(broker);
        return LIB_STATUS_IO_ERROR;
    }
    broker->current = next;
    broker->current_mode = next_mode;
    broker->generation = next_generation;
    old_output = broker->current_output;
    broker->current_output = next_output;
    host_console_backend_unlock_output(broker->backend);
    host_console_unlock(broker);
    /* The old binding is still safe until this setter has waited out any
       already-entered base write. Its native validation now rejects it. */
    host_console_remove_output_binding(old, old_output);
    lib_console_release(old);
    return LIB_STATUS_OK;
}

lib_status host_console_broker_request_cooked_line(host_console_broker *broker,
    lib_console *expected_current)
{
    lib_status status;

    if (broker == LIB_NULL || expected_current == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    host_console_lock(broker);
    if (broker->broken || broker->current != expected_current ||
        broker->current_mode != HOST_CONSOLE_COOKED_LINES) {
        host_console_unlock(broker);
        return LIB_STATUS_NOT_CURRENT;
    }
    status = host_console_backend_request_cooked_line(broker->backend);
    host_console_unlock(broker);
    return status;
}

void host_console_broker_destroy(host_console_broker *broker)
{
    lib_console *current;
    host_console_output_binding *output;
    lib_status status;
    if (broker == LIB_NULL) return;
    host_console_lock(broker);
    current = broker->current;
    output = broker->current_output;
    host_console_backend_lock_output(broker->backend);
    status = host_console_backend_deactivate(broker->backend);
    if (status != LIB_STATUS_OK) {
        /* A live reader still references backend and current.  This is
           already a terminal broker failure; retain its process-lifetime
           state rather than releasing either object underneath that worker. */
        host_console_backend_unlock_output(broker->backend);
        host_console_unlock(broker);
        return;
    }
    broker->current = LIB_NULL;
    broker->current_output = LIB_NULL;
    if (current != LIB_NULL) lib_console_invalidate_binding(current);
    host_console_backend_unlock_output(broker->backend);
    host_console_unlock(broker);
    host_console_remove_output_binding(current, output);
    if (current != LIB_NULL) lib_console_release(current);
    host_console_backend_destroy(broker->backend);
    free(broker);
    lib_atomic_flag_clear_explicit(&host_console_process_claimed, LIB_MEMORY_ORDER_RELEASE);
}
