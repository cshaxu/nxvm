#include "lib/console/console_interface.h"
#include "lib/console/binding_interface.h"

#include <assert.h>

typedef struct console_probe {
    lib_console_event event;
    lib_u32 events;
    char output[32];
    lib_size output_length;
    lib_u32 frames;
} console_probe;

static void console_probe_event(void *context, const lib_console_event *event)
{
    console_probe *probe = (console_probe *)context;
    probe->event = *event;
    ++probe->events;
}

static lib_status console_probe_output(void *context, const char *text,
    lib_size length)
{
    console_probe *probe = (console_probe *)context;
    if (length >= sizeof(probe->output)) return LIB_STATUS_LIMIT_EXCEEDED;
    lib_memory_copy(probe->output, text, length);
    probe->output[length] = '\0';
    probe->output_length = length;
    return LIB_STATUS_OK;
}

static lib_status console_probe_frame(void *context,
    const lib_console_text_frame *frame)
{
    console_probe *probe = (console_probe *)context;
    assert(frame->columns == 80u && frame->rows == 25u);
    ++probe->frames;
    return LIB_STATUS_OK;
}

int main(void)
{
    assert(sizeof(lib_console_text_frame) == 8084);
    lib_console *console = LIB_NULL;
    console_probe probe = { 0 };
    lib_console_event event = { 0 };
    console_probe next = { 0 };
    lib_console_text_frame frame = { .columns = 80u, .rows = 25u };
    lib_console_output_binding binding = {
        console_probe_output, console_probe_frame, &probe
    };

    assert(lib_console_create(&console) == LIB_STATUS_OK);
    assert(console != LIB_NULL);
    assert(lib_console_retain(console) == console);
    lib_console_release(console);
    assert(lib_console_write_text(console, "x", 1u) == LIB_STATUS_OK);
    assert(lib_console_set_event_sink(console, console_probe_event, &probe) ==
        LIB_STATUS_OK);
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 7u;
    event.value.raw_key.key = 'P';
    event.value.raw_key.pressed = LIB_TRUE;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    assert(lib_console_bind_generation(console, 0u) == LIB_STATUS_INVALID_ARGUMENT);
    assert(lib_console_bind_generation(console, 7u) == LIB_STATUS_OK);
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    assert(probe.events == 1u);
    assert(probe.event.binding_generation == 7u);
    assert(probe.event.value.raw_key.key == 'P');
    event.binding_generation = 6u;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    event.binding_generation = 0u;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    lib_console_invalidate_binding(console);
    event.binding_generation = 7u;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    assert(lib_console_bind_generation(console, 8u) == LIB_STATUS_OK);
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    event.binding_generation = 8u;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    assert(probe.events == 2u);
    event.kind = LIB_CONSOLE_EVENT_COOKED_LINE;
    event.value.line.length = LIB_CONSOLE_LINE_MAX;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_INVALID_ARGUMENT);
    assert(lib_console_set_output_binding(NULL, &binding) == LIB_STATUS_INVALID_ARGUMENT);
    assert(lib_console_set_output_binding(console, &binding) == LIB_STATUS_OK);
    binding.context = &next; /* Console owns a copy, not this descriptor. */
    assert(lib_console_write_text(console, "hello", 5u) == LIB_STATUS_OK);
    assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_OK);
    assert(probe.frames == 1u && next.frames == 0u);
    frame.columns = 81u;
    assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_UNSUPPORTED);
    frame.columns = 80u; frame.rows = 26u;
    assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_UNSUPPORTED);
    frame.rows = 0u;
    assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_INVALID_ARGUMENT);
    frame.rows = 25u; frame.text[1999] = 0xd800u;
    assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_INVALID_ARGUMENT);
    assert(probe.frames == 1u);
    frame.text[1999] = 0u;
    for (lib_u32 field = 0; field < 2; ++field) {
        lib_u8 *value = field == 0 ? &frame.foreground[1999] : &frame.background[1999];
        *value = 16;
        assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_INVALID_ARGUMENT);
        assert(probe.frames == 1u);
        *value = 0;
    }
    assert(probe.output_length == 5u);
    assert(lib_text_compare(probe.output, "hello") == 0);
    assert(lib_console_set_output_binding(console, &binding) == LIB_STATUS_OK);
    assert(lib_console_write_text(console, "next", 4u) == LIB_STATUS_OK);
    assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_OK);
    assert(next.frames == 1u && lib_text_compare(next.output, "next") == 0);
    assert(probe.frames == 1u && lib_text_compare(probe.output, "hello") == 0);
    assert(lib_console_set_output_binding(console, NULL) == LIB_STATUS_OK);
    assert(lib_console_write_text(console, "x", 1u) == LIB_STATUS_OK);
    assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_OK);
    binding.text = NULL;
    assert(lib_console_set_output_binding(console, &binding) == LIB_STATUS_OK);
    assert(lib_console_write_text(console, "x", 1u) == LIB_STATUS_OK);
    assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_OK);
    binding.text = console_probe_output;
    binding.frame = NULL;
    assert(lib_console_set_output_binding(console, &binding) == LIB_STATUS_OK);
    assert(lib_console_write_text(console, "x", 1u) == LIB_STATUS_OK);
    assert(lib_console_write_text_frame(console, &frame) == LIB_STATUS_OK);
    assert(lib_console_set_event_sink(console, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    assert(lib_console_deliver_event(console, &probe.event) == LIB_STATUS_INVALID_STATE);
    lib_console_release(console);
    return 0;
}
