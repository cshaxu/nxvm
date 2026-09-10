#include "lib/base/internal/console.h"

struct lib_console {
    atomic_flag lock;
    /* Serializes sink replacement against an in-flight copied event callback.
       A retiring UX source waits here before publishing SOURCE_RETIRED. */
    atomic_flag event_gate;
    atomic_flag output_lock;
    atomic_uint references;
    lib_console_event_sink event_sink;
    void *event_context;
    lib_console_output_sink output_sink;
    void *output_context;
    lib_console_text_frame_sink text_frame_sink;
    void *text_frame_context;
    lib_u32 binding_generation;
    lib_bool binding_active;
};

static void lib_console_lock(lib_console *console)
{
    while (atomic_flag_test_and_set_explicit(&console->lock,
        memory_order_acquire)) { }
}

static void lib_console_unlock(lib_console *console)
{
    atomic_flag_clear_explicit(&console->lock, memory_order_release);
}

static lib_bool lib_console_event_valid(const lib_console_event *event)
{
    if (event == LIB_NULL) return LIB_FALSE;
    if (event->kind == LIB_CONSOLE_EVENT_COOKED_LINE ||
        event->kind == LIB_CONSOLE_EVENT_REJECTED_LINE)
        return event->value.line.length < LIB_CONSOLE_LINE_MAX;
    return event->kind == LIB_CONSOLE_EVENT_RAW_KEY ||
        event->kind == LIB_CONSOLE_EVENT_RAW_MOUSE;
}

lib_status lib_console_create(lib_console **out_console)
{
    lib_console *console;
    if (out_console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_console = LIB_NULL;
    console = calloc(1u, sizeof(*console));
    if (console == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    atomic_flag_clear(&console->lock);
    atomic_flag_clear(&console->event_gate);
    atomic_flag_clear(&console->output_lock);
    atomic_init(&console->references, 1u);
    *out_console = console;
    return LIB_STATUS_OK;
}

lib_console *lib_console_retain(lib_console *console)
{
    if (console == LIB_NULL) return LIB_NULL;
    (void)atomic_fetch_add_explicit(&console->references, 1u,
        memory_order_relaxed);
    return console;
}

void lib_console_release(lib_console *console)
{
    if (console == LIB_NULL) return;
    if (atomic_fetch_sub_explicit(&console->references, 1u,
            memory_order_acq_rel) == 1u)
        free(console);
}

void lib_console_destroy(lib_console *console)
{
    lib_console_release(console);
}

lib_status lib_console_set_event_sink(lib_console *console,
    lib_console_event_sink sink, void *context)
{
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    while (atomic_flag_test_and_set_explicit(&console->event_gate,
        memory_order_acquire)) { }
    lib_console_lock(console);
    console->event_sink = sink;
    console->event_context = context;
    lib_console_unlock(console);
    atomic_flag_clear_explicit(&console->event_gate, memory_order_release);
    return LIB_STATUS_OK;
}

lib_status lib_console_set_output_sink(lib_console *console,
    lib_console_output_sink sink, void *context)
{
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    while (atomic_flag_test_and_set_explicit(&console->output_lock,
        memory_order_acquire)) { }
    lib_console_lock(console);
    console->output_sink = sink;
    console->output_context = context;
    lib_console_unlock(console);
    atomic_flag_clear_explicit(&console->output_lock, memory_order_release);
    return LIB_STATUS_OK;
}

lib_status lib_console_set_text_frame_sink(lib_console *console,
    lib_console_text_frame_sink sink, void *context)
{
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    while (atomic_flag_test_and_set_explicit(&console->output_lock,
        memory_order_acquire)) { }
    lib_console_lock(console);
    console->text_frame_sink = sink;
    console->text_frame_context = context;
    lib_console_unlock(console);
    atomic_flag_clear_explicit(&console->output_lock, memory_order_release);
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
    while (atomic_flag_test_and_set_explicit(&console->event_gate,
        memory_order_acquire)) { }
    lib_console_lock(console);
    if (console->binding_active == LIB_FALSE ||
        copied.binding_generation == 0u ||
        copied.binding_generation != console->binding_generation) {
        lib_console_unlock(console);
        atomic_flag_clear_explicit(&console->event_gate, memory_order_release);
        return LIB_STATUS_NOT_CURRENT;
    }
    sink = console->event_sink;
    context = console->event_context;
    lib_console_unlock(console);
    if (sink == LIB_NULL) {
        atomic_flag_clear_explicit(&console->event_gate, memory_order_release);
        return LIB_STATUS_INVALID_STATE;
    }
    /* The sink is deliberately called while the event gate is held.  This
       makes detach a quiescence barrier: after it returns no old callback can
       enter a component before that component reports retirement. */
    sink(context, &copied);
    atomic_flag_clear_explicit(&console->event_gate, memory_order_release);
    return LIB_STATUS_OK;
}

lib_status lib_console_bind_generation(lib_console *console,
    lib_u32 generation)
{
    if (console == LIB_NULL || generation == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    lib_console_lock(console);
    console->binding_generation = generation;
    console->binding_active = LIB_TRUE;
    lib_console_unlock(console);
    return LIB_STATUS_OK;
}

void lib_console_invalidate_binding(lib_console *console)
{
    if (console == LIB_NULL) return;
    lib_console_lock(console);
    console->binding_generation = 0u;
    console->binding_active = LIB_FALSE;
    lib_console_unlock(console);
}

lib_status lib_console_write_text(lib_console *console,
    const char *text, lib_size length)
{
    lib_console_output_sink sink;
    void *context;
    lib_status status;
    if (console == LIB_NULL || (text == LIB_NULL && length != 0u))
        return LIB_STATUS_INVALID_ARGUMENT;
    while (atomic_flag_test_and_set_explicit(&console->output_lock,
        memory_order_acquire)) { }
    lib_console_lock(console);
    sink = console->output_sink;
    context = console->output_context;
    lib_console_unlock(console);
    status = sink == LIB_NULL ? LIB_STATUS_NOT_CURRENT : sink(context, text, length);
    atomic_flag_clear_explicit(&console->output_lock, memory_order_release);
    return status;
}

lib_status lib_console_write_text_frame(lib_console *console,
    const lib_console_text_frame *frame)
{
    lib_console_text_frame_sink sink;
    void *context;
    lib_status status;

    if (console == LIB_NULL || frame == LIB_NULL || frame->columns == 0u ||
        frame->columns > LIB_CONSOLE_TEXT_COLUMNS || frame->rows == 0u ||
        frame->rows > LIB_CONSOLE_TEXT_ROWS) return LIB_STATUS_INVALID_ARGUMENT;
    while (atomic_flag_test_and_set_explicit(&console->output_lock,
        memory_order_acquire)) { }
    lib_console_lock(console);
    sink = console->text_frame_sink;
    context = console->text_frame_context;
    lib_console_unlock(console);
    status = sink == LIB_NULL ? LIB_STATUS_NOT_CURRENT : sink(context, frame);
    atomic_flag_clear_explicit(&console->output_lock, memory_order_release);
    return status;
}
