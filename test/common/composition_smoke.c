#include "common/ui/ui_interface.h"
#include "lib/console-broker/console_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/kvm-window/window_interface.h"
#include "lib/kvm-console/console_interface.h"
#include <assert.h>
#include <string.h>

static lib_u32 status_builds, unfreezes, freezes, console_frames, window_frames;
static lib_u32 received, received_run;
static lib_status publish_status = LIB_STATUS_OK;
static kvm_frame last_console;
static void *counted_set(void *destination, int value, lib_size size)
{
    if (size == sizeof(kvm_frame)) ++status_builds;
    return memset(destination, value, size);
}
#define lib_memory_set counted_set
#include "common/ui/ui.c"
#undef lib_memory_set

struct kvm_window { kvm_window_options options; };
struct kvm_console { lib_console *object; kvm_console_options options; };
struct console_broker { lib_console *current; };
static struct kvm_window window_fake;
static struct kvm_console console_fake;
static struct console_broker broker_fake;

lib_status kvm_window_create(kvm_window **out, const kvm_window_options *options)
{ window_fake.options = *options; *out = &window_fake; return LIB_STATUS_OK; }
lib_status kvm_window_destroy(kvm_window *window)
{ assert(window == &window_fake); return LIB_STATUS_OK; }
lib_status kvm_window_set_title(kvm_window *window, const char *title)
{ assert(window == &window_fake && title != NULL); return LIB_STATUS_OK; }
lib_status kvm_window_freeze(kvm_window *window)
{ assert(window == &window_fake); ++freezes; return LIB_STATUS_OK; }
lib_status kvm_window_unfreeze(kvm_window *window)
{ assert(window == &window_fake); ++unfreezes; return LIB_STATUS_OK; }
lib_status kvm_window_release_mouse(kvm_window *window)
{ assert(window == &window_fake); return LIB_STATUS_OK; }
lib_status kvm_window_publish_frame(kvm_window *window, const kvm_frame *frame)
{ assert(window == &window_fake && frame->valid); ++window_frames; return publish_status; }
lib_status kvm_console_create(kvm_console **out, const kvm_console_options *options)
{
    console_fake.options = *options;
    assert(lib_console_create(&console_fake.object) == LIB_STATUS_OK);
    *out = &console_fake;
    return LIB_STATUS_OK;
}
lib_status kvm_console_destroy(kvm_console *console)
{ assert(console == &console_fake); lib_console_release(console->object); return LIB_STATUS_OK; }
lib_console *kvm_console_get_console(const kvm_console *console)
{ return console->object; }
lib_status kvm_console_publish_frame(kvm_console *console, const kvm_frame *frame)
{
    assert(console == &console_fake);
    last_console = *frame;
    ++console_frames;
    return publish_status;
}
lib_status console_broker_create(console_broker **out,
    lib_console *initial, console_broker_mode mode)
{ assert(mode == CONSOLE_BROKER_COOKED_LINES); broker_fake.current = initial;
  *out = &broker_fake; return LIB_STATUS_OK; }
lib_status console_broker_replace(console_broker *broker,
    lib_console *expected, lib_console *next, console_broker_mode mode)
{ (void)mode; assert(broker->current == expected); broker->current = next; return LIB_STATUS_OK; }
lib_status console_broker_request_cooked_line(console_broker *broker,
    lib_console *expected)
{ assert(broker->current == expected); return LIB_STATUS_OK; }
lib_status console_broker_destroy(console_broker *broker)
{ assert(broker == &broker_fake); return LIB_STATUS_OK; }
lib_status console_broker_cancel_cooked_line(console_broker *broker,
    lib_console *expected, lib_bool *out_completed)
{ assert(broker->current == expected); *out_completed = LIB_FALSE; return LIB_STATUS_OK; }

static int receive(void *context, const common_ui_event *event)
{
    (void)context;
    assert(event->run_generation == 11u || event->run_generation == 12u);
    received_run = event->run_generation;
    ++received;
    return 1;
}
static void input_worker(void *context, const base_sync_task *task)
{
    const kvm_component_options *options = context;
    kvm_input_event input = { 0 };
    (void)task;
    input.type = KVM_EVENT_KEY;
    for (lib_u32 i = 0u; i < 10000u; ++i)
        assert(options->input_sink(options->input_context, &input));
    options->failure_sink(options->failure_context, 1u, LIB_STATUS_IO_ERROR);
}

int main(void)
{
    common_ui *ui = NULL;
    common_ui_options options = { 0 };
    base_sync_task *worker;
    static kvm_frame frame;
    options.event_sink = receive;
    options.running_window_title = "running";
    options.paused_window_title = "paused";
    options.graphics_console_status_text = "Window active\n\nHotkeys\n\n";
    assert(common_ui_create(&ui, &options) == LIB_STATUS_OK);
    common_ui_set_run_generation(ui, 11u);
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_WINDOW,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(!window_fake.options.initial_frozen && unfreezes == 0u);
    assert(common_ui_set_state(ui, COMMON_UI_STATE_PAUSED) == LIB_STATUS_OK);
    assert(common_ui_set_state(ui, COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(freezes == 1u && unfreezes == 1u);
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_DESTROY_WINDOW,
        COMMON_UI_STATE_PAUSED) == LIB_STATUS_OK);
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_WINDOW,
        COMMON_UI_STATE_PAUSED) == LIB_STATUS_OK);
    assert(window_fake.options.initial_frozen && unfreezes == 1u);
    assert(common_ui_set_state(ui, COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(unfreezes == 2u);
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    frame.valid = frame.graphics = 1u;
    frame.sequence = 1u;
    assert(common_ui_publish_frame(ui, &frame, 1, 0, 1) == LIB_STATUS_OK);
    assert(status_builds == 0u && console_frames == 0u && window_frames == 1u);
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_OK);
    assert(status_builds == 1u && console_frames == 1u && window_frames == 1u);
    assert(!last_console.graphics && last_console.sequence == 1u &&
        memcmp(last_console.text, "Window active", 13u) == 0 &&
        memcmp(last_console.text + KVM_TEXT_COLUMNS * 2u, "Hotkeys", 7u) == 0);
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_OK);
    assert(status_builds == 1u && console_frames == 1u && window_frames == 1u);
    frame.sequence = 2u;
    /* Window advances, but its fixed Console explanation is unchanged. */
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_OK);
    assert(status_builds == 1u && console_frames == 1u && window_frames == 2u);
    frame.sequence = 3u;
    frame.graphics = 0u;
    frame.text[0] = 'T';
    publish_status = LIB_STATUS_IO_ERROR;
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_IO_ERROR);
    assert(ui->console_status_delivered);
    publish_status = LIB_STATUS_OK;
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_OK);
    assert(!ui->console_status_delivered && last_console.text[0] == 'T');
    assert(status_builds == 1u && console_frames == 3u && window_frames == 3u);
    frame.sequence = 4u;
    frame.text[0] = 'U';
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 0) == LIB_STATUS_OK);
    assert(console_frames == 4u && last_console.text[0] == 'U');
    frame.sequence = 5u;
    frame.graphics = 1u;
    publish_status = LIB_STATUS_IO_ERROR;
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_IO_ERROR);
    assert(!ui->console_status_delivered);
    publish_status = LIB_STATUS_OK;
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_OK);
    assert(status_builds == 3u && console_frames == 6u && window_frames == 5u);
    /* Switching content category with the same source frame still submits. */
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 0) == LIB_STATUS_OK);
    assert(last_console.graphics && console_frames == 7u);
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_OK);
    assert(!last_console.graphics && console_frames == 8u);
    /* Rebinding retains content in this existing KVM instance. */
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_MONITOR,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_OK);
    assert(status_builds == 4u && console_frames == 8u);
    /* A fresh Console has no old status; even the same sequence is delivered. */
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_MONITOR,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_DESTROY_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    assert(common_ui_publish_frame(ui, &frame, 1, 1, 1) == LIB_STATUS_OK);
    assert(status_builds == 5u && console_frames == 9u && window_frames == 5u);
    /* Real concurrent callback/control access uses the one existing atomic. */
    for (int source = 0; source < 2; ++source) {
        kvm_component_options *component = source ? &console_fake.options :
            &window_fake.options.component;
        lib_u32 prior = received;
        assert(base_sync_task_create(input_worker, component, &worker) == LIB_STATUS_OK);
        for (lib_u32 i = 0u; i < 10000u; ++i) common_ui_set_run_generation(ui, 11u + i % 2u);
        base_sync_task_destroy(worker);
        assert(received == prior + 10001u);
    }
    common_ui_set_run_generation(ui, 12u);
    input_worker(&window_fake.options.component, NULL);
    assert(received_run == 12u);
    {
        kvm_input_event input = { 0 };
        lib_u32 prior = received;
        input.type = KVM_EVENT_MOUSE;
        assert(console_fake.options.input_sink(console_fake.options.input_context,
            &input));
        assert(received == prior);
        assert(window_fake.options.component.input_sink(
            window_fake.options.component.input_context, &input));
        assert(received == prior + 1u);
        input.type = KVM_EVENT_KEY;
        assert(console_fake.options.input_sink(console_fake.options.input_context,
            &input));
        assert(received == prior + 2u);
        input.type = KVM_EVENT_HOTKEY;
        assert(console_fake.options.input_sink(console_fake.options.input_context,
            &input));
        assert(received == prior + 3u);
        assert(common_ui_apply_action(ui, COMMON_UI_ACTION_DESTROY_WINDOW,
            COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
        assert(received == prior + 4u);
        input.type = KVM_EVENT_MOUSE;
        assert(console_fake.options.input_sink(console_fake.options.input_context,
            &input));
        assert(received == prior + 5u);
    }
    assert(common_ui_destroy(ui) == LIB_STATUS_OK);
    return 0;
}
