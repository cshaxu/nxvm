#include "lib/types/test.h"
#include "lib/types/file.h"
#include "common/ui/ui_interface.h"
#include "lib/console-broker/console_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/kvm-window/window_interface.h"
#include "lib/kvm-console/console_interface.h"

static lib_u32 status_builds, unfreezes, freezes, console_frames, window_frames;
static lib_u32 received, received_run;
static lib_status publish_status = LIB_STATUS_OK;
static kvm_console_text_frame last_console;
static common_ui *tracked_ui;
static lib_u32 released_ui, destroy_failure, broker_destroys, window_destroys, console_destroys;
static void counted_release(void *memory)
{
    if (memory == tracked_ui) ++released_ui;
    lib_release(memory);
}
static void *counted_set(void *destination, lib_i32 value, lib_size size)
{
    if (size == sizeof(kvm_console_text_frame)) ++status_builds;
    return lib_memory_set(destination, value, size);
}
#define lib_memory_set counted_set
#define lib_release counted_release
#include "common/ui/ui.c"
#undef lib_memory_set
#undef lib_release

struct kvm_window { kvm_window_options options; };
struct kvm_console { lib_console *object; kvm_console_options options; };
struct console_broker { lib_console *current; };
static struct kvm_window window_fake;
static struct kvm_console console_fake;
static struct console_broker broker_fake;

lib_status kvm_window_create(kvm_window **out, const kvm_window_options *options)
{ window_fake.options = *options; *out = &window_fake; return LIB_STATUS_OK; }
lib_status kvm_window_destroy(kvm_window *window)
{ lib_test_assert(window == &window_fake); ++window_destroys;
  return destroy_failure == 2u ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK; }
lib_status kvm_window_set_title(kvm_window *window, const char *title)
{ lib_test_assert(window == &window_fake && title != LIB_NULL); return LIB_STATUS_OK; }
lib_status kvm_window_freeze(kvm_window *window)
{ lib_test_assert(window == &window_fake); ++freezes; return LIB_STATUS_OK; }
lib_status kvm_window_unfreeze(kvm_window *window)
{ lib_test_assert(window == &window_fake); ++unfreezes; return LIB_STATUS_OK; }
lib_status kvm_window_release_mouse(kvm_window *window)
{ lib_test_assert(window == &window_fake); return LIB_STATUS_OK; }
lib_status kvm_window_publish_frame(kvm_window *window, const kvm_window_frame *frame)
{ lib_test_assert(window == &window_fake && frame->valid); ++window_frames; return publish_status; }
lib_status kvm_console_create(kvm_console **out, const kvm_console_options *options)
{
    console_fake.options = *options;
    lib_test_assert(lib_console_create(&console_fake.object) == LIB_STATUS_OK);
    *out = &console_fake;
    return LIB_STATUS_OK;
}
lib_status kvm_console_destroy(kvm_console *console)
{ lib_test_assert(console == &console_fake); ++console_destroys;
  if (destroy_failure == 3u) return LIB_STATUS_IO_ERROR;
  lib_console_release(console->object); return LIB_STATUS_OK; }
lib_console *kvm_console_get_console(const kvm_console *console)
{ return console->object; }
lib_status kvm_console_publish_frame(kvm_console *console, const kvm_console_text_frame *frame)
{
    lib_test_assert(console == &console_fake);
    lib_status status = kvm_console_text_frame_validate(frame);
    if (status != LIB_STATUS_OK) return status;
    last_console = *frame;
    ++console_frames;
    return publish_status;
}
lib_status console_broker_create(console_broker **out,
    lib_console *initial, console_broker_mode mode)
{ lib_test_assert(mode == CONSOLE_BROKER_COOKED_LINES); broker_fake.current = initial;
  *out = &broker_fake; return LIB_STATUS_OK; }
lib_status console_broker_replace(console_broker *broker,
    lib_console *expected, lib_console *next, console_broker_mode mode)
{ (void)mode; lib_test_assert(broker->current == expected); broker->current = next; return LIB_STATUS_OK; }
lib_status console_broker_request_cooked_line(console_broker *broker,
    lib_console *expected)
{ lib_test_assert(broker->current == expected); return LIB_STATUS_OK; }
lib_status console_broker_destroy(console_broker *broker)
{ lib_test_assert(broker == &broker_fake); ++broker_destroys;
  return destroy_failure == 1u ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK; }
lib_status console_broker_cancel_cooked_line(console_broker *broker,
    lib_console *expected, lib_bool *out_completed)
{ lib_test_assert(broker->current == expected); *out_completed = LIB_FALSE; return LIB_STATUS_OK; }

static lib_i32 receive(void *context, const common_ui_event *event)
{
    (void)context;
    lib_test_assert(event->run_generation == 11u || event->run_generation == 12u ||
        event->run_generation == LIB_UINT32_MAX);
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
        lib_test_assert(options->input_sink(options->input_context, &input));
    options->failure_sink(options->failure_context, 1u, LIB_STATUS_IO_ERROR);
}

static void check_destroy(const common_ui_options *options, lib_u32 failure, lib_i32 raw)
{
    common_ui *ui;
    lib_test_assert(common_ui_create(&ui, options) == LIB_STATUS_OK);
    common_ui_set_run_generation(ui, 11u);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_WINDOW,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    if (raw) lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    tracked_ui = ui;
    released_ui = broker_destroys = window_destroys = console_destroys = 0u;
    destroy_failure = failure;
    lib_test_assert(common_ui_destroy(ui) == LIB_STATUS_IO_ERROR);
    lib_test_assert(released_ui == 0u && ui->monitor != LIB_NULL && ui->console != LIB_NULL);
    lib_test_assert(broker_destroys == 1u && window_destroys == (failure >= 2u) &&
        console_destroys == (failure == 3u));
    lib_test_assert((ui->broker != LIB_NULL) == (failure == 1u));
    lib_test_assert((ui->window != LIB_NULL) == (failure != 3u));
    /* A retained source can still safely call its retained UI/event context. */
    input_worker(&console_fake.options, LIB_NULL);
    if (ui->window) input_worker(&window_fake.options.component, LIB_NULL);
    /* End the scripted failure for fixture cleanup, not a product retry policy. */
    destroy_failure = 0u;
    lib_test_assert(common_ui_destroy(ui) == LIB_STATUS_OK && released_ui == 1u);
    lib_test_assert(broker_destroys == (failure == 1u ? 2u : 1u));
    lib_test_assert(window_destroys == (failure == 2u ? 2u : 1u));
    lib_test_assert(console_destroys == (failure == 3u ? 2u : 1u));
    tracked_ui = LIB_NULL;
}

int main(void)
{
    common_ui *ui = LIB_NULL;
    common_ui_options options = { 0 };
    base_sync_task *worker;
    static kvm_window_frame frame;
    kvm_console_character_map characters = { 0 };
    lib_u32 sequence;
    lib_test_assert(common_ui_create(LIB_NULL, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    ui = (common_ui *)&options;
    lib_test_assert(common_ui_create(&ui, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT && ui == LIB_NULL);
    ui = (common_ui *)&options;
    lib_test_assert(common_ui_create(&ui, &options) == LIB_STATUS_INVALID_ARGUMENT && ui == LIB_NULL);
    options.event_sink = receive;
    options.running_window_title = "running";
    options.paused_window_title = "paused";
    options.graphics_console_status_text = "Window active\n\nHotkeys\n\n";
    lib_test_assert(common_ui_create(&ui, &options) == LIB_STATUS_OK);
    common_ui_set_run_generation(ui, 11u);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_WINDOW,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(!window_fake.options.initial_frozen && unfreezes == 0u);
    lib_test_assert(common_ui_set_state(ui, COMMON_UI_STATE_PAUSED) == LIB_STATUS_OK);
    lib_test_assert(common_ui_set_state(ui, COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(freezes == 1u && unfreezes == 1u);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_DESTROY_WINDOW,
        COMMON_UI_STATE_PAUSED) == LIB_STATUS_OK);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_WINDOW,
        COMMON_UI_STATE_PAUSED) == LIB_STATUS_OK);
    lib_test_assert(window_fake.options.initial_frozen && unfreezes == 1u);
    lib_test_assert(common_ui_set_state(ui, COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(unfreezes == 2u);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    frame.valid = frame.graphics = 1u;
    frame.image.width = frame.image.height = frame.image.stride = 1u;
    sequence = 1u;
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 0, 1) == LIB_STATUS_OK);
    lib_test_assert(status_builds == 0u && console_frames == 0u && window_frames == 1u);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_OK);
    lib_test_assert(status_builds == 1u && console_frames == 1u && window_frames == 1u);
    lib_test_assert(last_console.base.text_columns == 80u);
    lib_test_assert(last_console.base.text_rows == 25u);
    lib_test_assert(last_console.base.text_palette[7] == 0xc0c0c0u &&
        last_console.base.text_palette[0] == 0u);
    for (lib_u32 i = 0; i < 13u; ++i)
        lib_test_assert(last_console.base.cells[i].glyph_index == "Window active"[i]);
    for (lib_u32 i = 0; i < 7u; ++i)
        lib_test_assert(last_console.base.cells[KVM_TEXT_COLUMNS * 2u + i].glyph_index == "Hotkeys"[i]);
    for (lib_u32 i = 0; i < KVM_TEXT_COLUMNS * KVM_TEXT_ROWS; ++i) {
        lib_test_assert(last_console.base.cells[i].foreground == 7u);
        lib_test_assert(last_console.base.cells[i].background == 0u && last_console.base.cells[i].glyph_bank == 0u);
    }
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_OK);
    lib_test_assert(status_builds == 1u && console_frames == 1u && window_frames == 1u);
    sequence = 2u;
    /* Window advances, but its fixed Console explanation is unchanged. */
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_OK);
    lib_test_assert(status_builds == 1u && console_frames == 1u && window_frames == 2u);
    sequence = 3u;
    frame.graphics = 0u;
    lib_memory_set(&frame.text, 0, sizeof(frame.text));
    frame.text.base.text_columns = 80u; frame.text.base.text_rows = 50u;
    frame.text.base.cells[3999].glyph_index = 'Z';
    characters.primary['T'] = 0x263au;
    characters.secondary['T'] = 0x2665u;
    frame.text.base.cells[0].glyph_index = 'T';
    publish_status = LIB_STATUS_IO_ERROR;
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_IO_ERROR);
    lib_test_assert(ui->console_status_delivered);
    publish_status = LIB_STATUS_OK;
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_OK);
    lib_test_assert(!ui->console_status_delivered && last_console.base.cells[0].glyph_index == 'T');
    lib_test_assert(last_console.base.text_rows == 50u &&
        last_console.base.cells[3999].glyph_index == 'Z');
    lib_test_assert(last_console.characters.primary['T'] == 0x263au &&
        last_console.characters.secondary['T'] == 0x2665u);
    lib_test_assert(status_builds == 1u && console_frames == 3u && window_frames == 3u);
    sequence = 4u;
    frame.text.base.cells[0].glyph_index = 'U';
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 0) == LIB_STATUS_OK);
    lib_test_assert(console_frames == 4u && last_console.base.cells[0].glyph_index == 'U');
    sequence = 5u;
    frame.graphics = 1u;
    publish_status = LIB_STATUS_IO_ERROR;
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_IO_ERROR);
    lib_test_assert(!ui->console_status_delivered);
    publish_status = LIB_STATUS_OK;
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_OK);
    lib_test_assert(status_builds == 3u && console_frames == 6u && window_frames == 5u);
    /* Unsupported output must not advance Common's completed-content cache. */
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 0) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(console_frames == 6u && ui->console_status_delivered);
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_OK);
    lib_test_assert(console_frames == 6u);
    /* Rebinding retains content in this existing KVM instance. */
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_MONITOR,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_OK);
    lib_test_assert(status_builds == 3u && console_frames == 6u);
    /* A fresh Console has no old status; even the same sequence is delivered. */
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_MONITOR,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_DESTROY_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_BIND_VM_CONSOLE,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 1) == LIB_STATUS_OK);
    lib_test_assert(status_builds == 4u && console_frames == 7u && window_frames == 5u);
    /* UI deduplication needs equality only, including wrap and recreation. */
    frame.graphics = 0u;
    for (lib_u32 i = 0u; i < 2u; ++i) {
        sequence = i == 0u ? LIB_UINT32_MAX : 1u;
        lib_u32 wc = window_frames, cc = console_frames;
        lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 0) == LIB_STATUS_OK);
        lib_test_assert(window_frames == wc + 1u && console_frames == cc + 1u);
        lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 0) == LIB_STATUS_OK);
        lib_test_assert(window_frames == wc + 1u && console_frames == cc + 1u);
    }
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_DESTROY_WINDOW,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_CREATE_WINDOW,
        COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
    lib_u32 wc = window_frames;
    lib_test_assert(common_ui_publish_frame(ui, &frame, &characters, sequence, 1, 1, 0) == LIB_STATUS_OK);
    lib_test_assert(window_frames == wc + 1u); /* Same serial, fresh presenter. */
    /* Real concurrent callback/control access uses the one existing atomic. */
    for (lib_i32 source = 0; source < 2; ++source) {
        kvm_component_options *component = source ? &console_fake.options :
            &window_fake.options.component;
        lib_u32 prior = received;
        lib_test_assert(base_sync_task_create(input_worker, component, &worker) == LIB_STATUS_OK);
        for (lib_u32 i = 0u; i < 10000u; ++i) common_ui_set_run_generation(ui, 11u + i % 2u);
        base_sync_task_destroy(worker);
        lib_test_assert(received == prior + 10001u);
    }
    common_ui_set_run_generation(ui, 12u);
    input_worker(&window_fake.options.component, LIB_NULL);
    lib_test_assert(received_run == 12u);
    common_ui_set_run_generation(ui, LIB_UINT32_MAX);
    input_worker(&window_fake.options.component, LIB_NULL);
    lib_test_assert(received_run == LIB_UINT32_MAX);
    {
        kvm_input_event input = { 0 };
        lib_u32 prior = received;
        input.type = KVM_EVENT_MOUSE;
        lib_test_assert(console_fake.options.input_sink(console_fake.options.input_context,
            &input));
        lib_test_assert(received == prior);
        lib_test_assert(window_fake.options.component.input_sink(
            window_fake.options.component.input_context, &input));
        lib_test_assert(received == prior + 1u);
        input.type = KVM_EVENT_KEY;
        lib_test_assert(console_fake.options.input_sink(console_fake.options.input_context,
            &input));
        lib_test_assert(received == prior + 2u);
        input.type = KVM_EVENT_HOTKEY;
        lib_test_assert(console_fake.options.input_sink(console_fake.options.input_context,
            &input));
        lib_test_assert(received == prior + 3u);
        lib_test_assert(common_ui_apply_action(ui, COMMON_UI_ACTION_DESTROY_WINDOW,
            COMMON_UI_STATE_RUNNING) == LIB_STATUS_OK);
        lib_test_assert(received == prior + 4u);
        input.type = KVM_EVENT_MOUSE;
        lib_test_assert(console_fake.options.input_sink(console_fake.options.input_context,
            &input));
        lib_test_assert(received == prior + 5u);
    }
    lib_test_assert(common_ui_destroy(ui) == LIB_STATUS_OK);
    for (lib_u32 failure = 1u; failure <= 3u; ++failure)
        for (lib_i32 raw = 0; raw <= 1; ++raw) check_destroy(&options, failure, raw);
    lib_test_assert(common_ui_destroy(LIB_NULL) == LIB_STATUS_OK);
    return 0;
}
