#include "lib/console/binding_interface.h"

#include "lib/types/atomic.h"
#include "lib/console/mutex.h"

struct lib_console {
    lib_atomic_flag lock;
    /* Serializes sink replacement against an in-flight copied event callback.
       A retiring UI source waits here before publishing SOURCE_RETIRED. */
    console_mutex *event_gate;
    console_mutex *output_lock;
    lib_atomic_u32 references;
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
    while (lib_atomic_flag_test_and_set_explicit(&console->lock,
        LIB_MEMORY_ORDER_ACQUIRE)) { }
}

static void lib_console_unlock(lib_console *console)
{
    lib_atomic_flag_clear_explicit(&console->lock, LIB_MEMORY_ORDER_RELEASE);
}

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
    lib_atomic_flag_clear(&console->lock);
    status = console_mutex_create(&console->event_gate);
    if (status == LIB_STATUS_OK) status = console_mutex_create(&console->output_lock);
    if (status != LIB_STATUS_OK) {
        console_mutex_destroy(console->event_gate);
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
        console_mutex_destroy(console->event_gate);
        console_mutex_destroy(console->output_lock);
        lib_release(console);
    }
}

void lib_console_destroy(lib_console *console)
{
    lib_console_release(console);
}

lib_status lib_console_set_event_sink(lib_console *console,
    lib_console_event_sink sink, void *context)
{
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    console_mutex_enter(console->event_gate);
    lib_console_lock(console);
    console->event_sink = sink;
    console->event_context = context;
    lib_console_unlock(console);
    console_mutex_leave(console->event_gate);
    return LIB_STATUS_OK;
}

lib_status lib_console_set_output_sink(lib_console *console,
    lib_console_output_sink sink, void *context)
{
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    console_mutex_enter(console->output_lock);
    lib_console_lock(console);
    console->output_sink = sink;
    console->output_context = context;
    lib_console_unlock(console);
    console_mutex_leave(console->output_lock);
    return LIB_STATUS_OK;
}

lib_status lib_console_set_text_frame_sink(lib_console *console,
    lib_console_text_frame_sink sink, void *context)
{
    if (console == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    console_mutex_enter(console->output_lock);
    lib_console_lock(console);
    console->text_frame_sink = sink;
    console->text_frame_context = context;
    lib_console_unlock(console);
    console_mutex_leave(console->output_lock);
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
    console_mutex_enter(console->event_gate);
    lib_console_lock(console);
    if (console->binding_active == LIB_FALSE ||
        copied.binding_generation == 0u ||
        copied.binding_generation != console->binding_generation) {
        lib_console_unlock(console);
        console_mutex_leave(console->event_gate);
        return LIB_STATUS_NOT_CURRENT;
    }
    sink = console->event_sink;
    context = console->event_context;
    lib_console_unlock(console);
    if (sink == LIB_NULL) {
        console_mutex_leave(console->event_gate);
        return LIB_STATUS_INVALID_STATE;
    }
    /* The sink is deliberately called while the event gate is held.  This
       makes detach a quiescence barrier: after it returns no old callback can
       enter a component before that component reports retirement. */
    sink(context, &copied);
    console_mutex_leave(console->event_gate);
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
    console_mutex_enter(console->output_lock);
    lib_console_lock(console);
    sink = console->output_sink;
    context = console->output_context;
    lib_console_unlock(console);
    status = sink == LIB_NULL ? LIB_STATUS_NOT_CURRENT : sink(context, text, length);
    console_mutex_leave(console->output_lock);
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
    console_mutex_enter(console->output_lock);
    lib_console_lock(console);
    sink = console->text_frame_sink;
    context = console->text_frame_context;
    lib_console_unlock(console);
    status = sink == LIB_NULL ? LIB_STATUS_NOT_CURRENT : sink(context, frame);
    console_mutex_leave(console->output_lock);
    return status;
}


lib_u16 lib_console_pc_glyph(lib_u8 value)
{
    /* Fixed PC display glyphs: low bytes are symbols, not terminal controls.
     * This is Unicode approximation, not a raster font or code-page detection. */
    static const lib_u16 glyphs[256] = {
        0x0020u, 0x263au, 0x263bu, 0x2665u, 0x2666u, 0x2663u, 0x2660u, 0x2022u,
        0x25d8u, 0x25cbu, 0x25d9u, 0x2642u, 0x2640u, 0x266au, 0x266bu, 0x263cu,
        0x25bau, 0x25c4u, 0x2195u, 0x203cu, 0x00b6u, 0x00a7u, 0x25acu, 0x21a8u,
        0x2191u, 0x2193u, 0x2192u, 0x2190u, 0x221fu, 0x2194u, 0x25b2u, 0x25bcu,
        0x0020u, 0x0021u, 0x0022u, 0x0023u, 0x0024u, 0x0025u, 0x0026u, 0x0027u,
        0x0028u, 0x0029u, 0x002au, 0x002bu, 0x002cu, 0x002du, 0x002eu, 0x002fu,
        0x0030u, 0x0031u, 0x0032u, 0x0033u, 0x0034u, 0x0035u, 0x0036u, 0x0037u,
        0x0038u, 0x0039u, 0x003au, 0x003bu, 0x003cu, 0x003du, 0x003eu, 0x003fu,
        0x0040u, 0x0041u, 0x0042u, 0x0043u, 0x0044u, 0x0045u, 0x0046u, 0x0047u,
        0x0048u, 0x0049u, 0x004au, 0x004bu, 0x004cu, 0x004du, 0x004eu, 0x004fu,
        0x0050u, 0x0051u, 0x0052u, 0x0053u, 0x0054u, 0x0055u, 0x0056u, 0x0057u,
        0x0058u, 0x0059u, 0x005au, 0x005bu, 0x005cu, 0x005du, 0x005eu, 0x005fu,
        0x0060u, 0x0061u, 0x0062u, 0x0063u, 0x0064u, 0x0065u, 0x0066u, 0x0067u,
        0x0068u, 0x0069u, 0x006au, 0x006bu, 0x006cu, 0x006du, 0x006eu, 0x006fu,
        0x0070u, 0x0071u, 0x0072u, 0x0073u, 0x0074u, 0x0075u, 0x0076u, 0x0077u,
        0x0078u, 0x0079u, 0x007au, 0x007bu, 0x007cu, 0x007du, 0x007eu, 0x2302u,
        0x00c7u, 0x00fcu, 0x00e9u, 0x00e2u, 0x00e4u, 0x00e0u, 0x00e5u, 0x00e7u,
        0x00eau, 0x00ebu, 0x00e8u, 0x00efu, 0x00eeu, 0x00ecu, 0x00c4u, 0x00c5u,
        0x00c9u, 0x00e6u, 0x00c6u, 0x00f4u, 0x00f6u, 0x00f2u, 0x00fbu, 0x00f9u,
        0x00ffu, 0x00d6u, 0x00dcu, 0x00a2u, 0x00a3u, 0x00a5u, 0x20a7u, 0x0192u,
        0x00e1u, 0x00edu, 0x00f3u, 0x00fau, 0x00f1u, 0x00d1u, 0x00aau, 0x00bau,
        0x00bfu, 0x2310u, 0x00acu, 0x00bdu, 0x00bcu, 0x00a1u, 0x00abu, 0x00bbu,
        0x2591u, 0x2592u, 0x2593u, 0x2502u, 0x2524u, 0x2561u, 0x2562u, 0x2556u,
        0x2555u, 0x2563u, 0x2551u, 0x2557u, 0x255du, 0x255cu, 0x255bu, 0x2510u,
        0x2514u, 0x2534u, 0x252cu, 0x251cu, 0x2500u, 0x253cu, 0x255eu, 0x255fu,
        0x255au, 0x2554u, 0x2569u, 0x2566u, 0x2560u, 0x2550u, 0x256cu, 0x2567u,
        0x2568u, 0x2564u, 0x2565u, 0x2559u, 0x2558u, 0x2552u, 0x2553u, 0x256bu,
        0x256au, 0x2518u, 0x250cu, 0x2588u, 0x2584u, 0x258cu, 0x2590u, 0x2580u,
        0x03b1u, 0x00dfu, 0x0393u, 0x03c0u, 0x03a3u, 0x03c3u, 0x00b5u, 0x03c4u,
        0x03a6u, 0x0398u, 0x03a9u, 0x03b4u, 0x221eu, 0x03c6u, 0x03b5u, 0x2229u,
        0x2261u, 0x00b1u, 0x2265u, 0x2264u, 0x2320u, 0x2321u, 0x00f7u, 0x2248u,
        0x00b0u, 0x2219u, 0x00b7u, 0x221au, 0x207fu, 0x00b2u, 0x25a0u, 0x00a0u,
    };
    return glyphs[value];
}
