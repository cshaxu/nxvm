#include "lib/console/binding_interface.h"

#include "lib/types/atomic.h"
#include "lib/base/sync_interface.h"

struct lib_console {
    base_sync_mutex *lock;
    /* Serializes sink replacement against an in-flight copied event callback.
       A retiring KVM source waits here before publishing SOURCE_RETIRED. */
    base_sync_mutex *event_gate;
    base_sync_mutex *output_lock;
    lib_atomic_u32 references;
    lib_console_event_sink event_sink;
    void *event_context;
    lib_console_output_binding output;
    lib_u32 binding_generation;
};

static lib_bool lib_console_event_valid(const lib_console_event *event)
{
    if (event == LIB_NULL) return LIB_FALSE;
    if (event->kind == LIB_CONSOLE_EVENT_COOKED_LINE ||
        event->kind == LIB_CONSOLE_EVENT_REJECTED_LINE)
        return event->value.line.length < LIB_CONSOLE_LINE_MAX;
    return event->kind == LIB_CONSOLE_EVENT_RAW_KEY ||
        event->kind == LIB_CONSOLE_EVENT_RAW_MOUSE ||
        event->kind == LIB_CONSOLE_EVENT_IO_FAILURE ||
        event->kind == LIB_CONSOLE_EVENT_ACTIVATED ||
        event->kind == LIB_CONSOLE_EVENT_INPUT_RESET;
}

lib_status lib_console_create(lib_console **out_console)
{
    lib_console *console;
    lib_status status;
    if (out_console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_console = LIB_NULL;
    console = lib_allocate_zero(1u, sizeof(*console));
    if (console == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = base_sync_mutex_create(&console->lock);
    if (status == LIB_STATUS_OK) status = base_sync_mutex_create(&console->event_gate);
    if (status == LIB_STATUS_OK) status = base_sync_mutex_create(&console->output_lock);
    if (status != LIB_STATUS_OK) {
        base_sync_mutex_destroy(console->lock);
        base_sync_mutex_destroy(console->event_gate);
        lib_release(console);
        return status;
    }
    lib_atomic_u32_initialize(&console->references, 1u);
    *out_console = console;
    return LIB_STATUS_OK;
}

lib_console *lib_console_retain(lib_console *console)
{
    if (console == LIB_NULL) return LIB_NULL;
    (void)lib_atomic_u32_fetch_add_explicit(&console->references, 1u,
        LIB_MEMORY_ORDER_RELAXED);
    return console;
}

void lib_console_release(lib_console *console)
{
    if (console == LIB_NULL) return;
    if (lib_atomic_u32_fetch_sub_explicit(&console->references, 1u,
            LIB_MEMORY_ORDER_ACQ_REL) == 1u) {
        base_sync_mutex_destroy(console->lock);
        base_sync_mutex_destroy(console->event_gate);
        base_sync_mutex_destroy(console->output_lock);
        lib_release(console);
    }
}

lib_status lib_console_set_event_sink(lib_console *console,
    lib_console_event_sink sink, void *context)
{
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(console->event_gate);
    base_sync_mutex_lock(console->lock);
    console->event_sink = sink;
    console->event_context = context;
    base_sync_mutex_unlock(console->lock);
    base_sync_mutex_unlock(console->event_gate);
    return LIB_STATUS_OK;
}

lib_status lib_console_set_output_binding(lib_console *console,
    const lib_console_output_binding *binding)
{
    const lib_console_output_binding empty = { 0 };
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(console->output_lock);
    console->output = binding != LIB_NULL ? *binding : empty;
    base_sync_mutex_unlock(console->output_lock);
    return LIB_STATUS_OK;
}

lib_status lib_console_deliver_event(lib_console *console,
    const lib_console_event *event)
{
    lib_console_event_sink sink;
    void *context;
    lib_console_event copied;

    if (console == LIB_NULL || !lib_console_event_valid(event))
        return LIB_STATUS_INVALID_ARGUMENT;
    copied = *event;
    base_sync_mutex_lock(console->event_gate);
    base_sync_mutex_lock(console->lock);
    if (copied.binding_generation == 0u ||
        copied.binding_generation != console->binding_generation) {
        base_sync_mutex_unlock(console->lock);
        base_sync_mutex_unlock(console->event_gate);
        return LIB_STATUS_OK;
    }
    sink = console->event_sink;
    context = console->event_context;
    base_sync_mutex_unlock(console->lock);
    if (sink == LIB_NULL) {
        base_sync_mutex_unlock(console->event_gate);
        return LIB_STATUS_INVALID_STATE;
    }
    /* The sink is deliberately called while the event gate is held.  This
       makes detach a quiescence barrier: after it returns no old callback can
       enter a component before that component reports retirement. */
    sink(context, &copied);
    base_sync_mutex_unlock(console->event_gate);
    return LIB_STATUS_OK;
}

lib_status lib_console_bind_generation(lib_console *console,
    lib_u32 generation)
{
    if (console == LIB_NULL || generation == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(console->lock);
    console->binding_generation = generation;
    base_sync_mutex_unlock(console->lock);
    return LIB_STATUS_OK;
}

void lib_console_invalidate_binding(lib_console *console)
{
    if (console == LIB_NULL) return;
    base_sync_mutex_lock(console->lock);
    console->binding_generation = 0u;
    base_sync_mutex_unlock(console->lock);
}

lib_status lib_console_write_text(lib_console *console,
    const char *text, lib_size length)
{
    lib_console_output_sink sink;
    void *context;
    lib_status status;
    if (console == LIB_NULL || (text == LIB_NULL && length != 0u))
        return LIB_STATUS_INVALID_ARGUMENT;
    base_sync_mutex_lock(console->output_lock);
    sink = console->output.text;
    context = console->output.context;
    status = sink == LIB_NULL ? LIB_STATUS_OK : sink(context, text, length);
    base_sync_mutex_unlock(console->output_lock);
    return status;
}

lib_status lib_console_write_text_frame(lib_console *console,
    const lib_console_text_frame *frame)
{
    lib_console_text_frame_sink sink;
    void *context;
    lib_status status;

    if (console == LIB_NULL || frame == LIB_NULL || frame->columns == 0u || frame->rows == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (frame->columns > LIB_CONSOLE_TEXT_COLUMNS || frame->rows > LIB_CONSOLE_TEXT_ROWS)
        return LIB_STATUS_UNSUPPORTED;
    for (lib_size row = 0u; row < frame->rows; ++row) {
        for (lib_size column = 0u; column < frame->columns; ++column) {
            lib_size index = row * LIB_CONSOLE_TEXT_COLUMNS + column;
            lib_u16 character = frame->text[index];
            if ((character >= 0xd800u && character <= 0xdfffu) ||
                frame->foreground[index] > 15u || frame->background[index] > 15u)
                return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    base_sync_mutex_lock(console->output_lock);
    sink = console->output.frame;
    context = console->output.context;
    status = sink == LIB_NULL ? LIB_STATUS_OK : sink(context, frame);
    base_sync_mutex_unlock(console->output_lock);
    return status;
}
