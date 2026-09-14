#include "lib/console/console_interface.h"
#include "lib/console/binding_interface.h"

#include <assert.h>
#include <string.h>

typedef struct console_probe {
    lib_console_event event;
    unsigned int events;
    char output[32];
    lib_size output_length;
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
    memcpy(probe->output, text, length);
    probe->output[length] = '\0';
    probe->output_length = length;
    return LIB_STATUS_OK;
}

int main(void)
{
    lib_console *console = LIB_NULL;
    console_probe probe = { 0 };
    lib_console_event event = { 0 };

    assert(lib_console_create(&console) == LIB_STATUS_OK);
    assert(console != LIB_NULL);
    assert(lib_console_retain(console) == console);
    lib_console_release(console);
    assert(lib_console_write_text(console, "x", 1u) == LIB_STATUS_NOT_CURRENT);
    assert(lib_console_set_event_sink(console, console_probe_event, &probe) ==
        LIB_STATUS_OK);
    assert(lib_console_bind_generation(console, 7u) == LIB_STATUS_OK);
    event.kind = LIB_CONSOLE_EVENT_RAW_KEY;
    event.binding_generation = 7u;
    event.value.raw_key.key = 'P';
    event.value.raw_key.pressed = LIB_TRUE;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_OK);
    assert(probe.events == 1u);
    assert(probe.event.binding_generation == 7u);
    assert(probe.event.value.raw_key.key == 'P');
    event.binding_generation = 6u;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_NOT_CURRENT);
    event.binding_generation = 7u;
    event.kind = LIB_CONSOLE_EVENT_COOKED_LINE;
    event.value.line.length = LIB_CONSOLE_LINE_MAX;
    assert(lib_console_deliver_event(console, &event) == LIB_STATUS_INVALID_ARGUMENT);
    assert(lib_console_set_output_sink(console, console_probe_output, &probe) ==
        LIB_STATUS_OK);
    assert(lib_console_write_text(console, "hello", 5u) == LIB_STATUS_OK);
    assert(probe.output_length == 5u);
    assert(strcmp(probe.output, "hello") == 0);
    assert(lib_console_set_event_sink(console, LIB_NULL, LIB_NULL) == LIB_STATUS_OK);
    assert(lib_console_deliver_event(console, &probe.event) == LIB_STATUS_INVALID_STATE);
    lib_console_destroy(console);
    return 0;
}
