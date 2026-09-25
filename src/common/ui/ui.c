#include "common/ui/ui_interface.h"

#include "lib/console-broker/console_interface.h"
#include "lib/kvm-console/console_interface.h"
#include "lib/kvm-window/window_interface.h"


typedef struct common_ui_input_context {
    common_ui *ui;
    lib_bool vm_console;
} common_ui_input_context;

struct common_ui {
    lib_console *monitor;
    console_broker *broker;
    kvm_window *window;
    kvm_console *console;
    common_ui_options options;
    lib_atomic_u32 run_generation;
    lib_atomic_i32 window_live;
    lib_u32 window_delivered_frame_sequence;
    lib_u32 console_delivered_frame_sequence;
    lib_bool console_status_delivered;
    common_ui_input_context window_input;
    common_ui_input_context console_input;
};

static lib_bool common_ui_emit(common_ui *ui, const common_ui_event *event)
{
    return ui != NULL && ui->options.event_sink != NULL &&
        ui->options.event_sink(ui->options.event_context, event);
}

static void common_ui_delivery_failed(void *opaque, lib_u64 source_identity,
    lib_status status)
{
    common_ui *ui = (common_ui *)opaque;
    common_ui_event event = { 0 };
    event.kind = COMMON_UI_EVENT_KVM_DELIVERY_FAILED;
    event.run_generation = ui == NULL ? 0u : lib_atomic_u32_load_explicit(
        &ui->run_generation, LIB_MEMORY_ORDER_SEQ_CST);
    event.value.delivery_failure.source_identity = source_identity;
    event.value.delivery_failure.status = status;
    (void)common_ui_emit(ui, &event);
}

static lib_bool common_ui_input(void *opaque, const kvm_input_event *input)
{
    common_ui_input_context *context = (common_ui_input_context *)opaque;
    common_ui *ui = context == NULL ? NULL : context->ui;
    common_ui_event event = { 0 };
    if (ui == NULL || input == NULL) return LIB_FALSE;
    /* A graphical Window is the sole guest-mouse surface. The concurrent raw
       Console remains a keyboard/hotkey surface in Console display mode. */
    if (context->vm_console && input->type == KVM_EVENT_MOUSE &&
        lib_atomic_i32_load_explicit(&ui->window_live,
            LIB_MEMORY_ORDER_SEQ_CST) != 0) return LIB_TRUE;
    event.kind = COMMON_UI_EVENT_KVM_INPUT;
    event.run_generation = lib_atomic_u32_load_explicit(
        &ui->run_generation, LIB_MEMORY_ORDER_SEQ_CST);
    event.value.kvm = *input;
    return common_ui_emit(ui, &event);
}

static void common_ui_monitor_receive(void *opaque, const lib_console_event *source)
{
    common_ui *ui = (common_ui *)opaque;
    common_ui_event event = { 0 };
    if (ui == NULL || source == NULL) return;
    if (source->kind == LIB_CONSOLE_EVENT_IO_FAILURE) {
        event.kind = COMMON_UI_EVENT_CONSOLE_FAILED;
        (void)common_ui_emit(ui, &event);
        return;
    }
    if (source->kind != LIB_CONSOLE_EVENT_COOKED_LINE &&
        source->kind != LIB_CONSOLE_EVENT_REJECTED_LINE) return;
    event.kind = COMMON_UI_EVENT_MONITOR_LINE;
    event.monitor_line_rejected = source->kind == LIB_CONSOLE_EVENT_REJECTED_LINE;
    event.value.line = source->value.line;
    (void)common_ui_emit(ui, &event);
}

static lib_status common_ui_emit_component(common_ui *ui,
    common_ui_component component, lib_bool exists)
{
    common_ui_event event = { 0 };
    event.kind = COMMON_UI_EVENT_COMPONENT_COMPLETED;
    event.run_generation = lib_atomic_u32_load_explicit(
        &ui->run_generation, LIB_MEMORY_ORDER_SEQ_CST);
    event.value.component.component = component;
    event.value.component.exists = exists;
    return common_ui_emit(ui, &event) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

static lib_status common_ui_emit_broker(common_ui *ui, lib_bool vm_current)
{
    common_ui_event event = { 0 };
    event.kind = COMMON_UI_EVENT_BROKER_COMPLETED;
    event.run_generation = lib_atomic_u32_load_explicit(
        &ui->run_generation, LIB_MEMORY_ORDER_SEQ_CST);
    event.value.broker_vm_console_current = vm_current;
    return common_ui_emit(ui, &event) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

static const char *common_ui_title(const common_ui *ui, common_ui_state state)
{
    return state == COMMON_UI_STATE_PAUSED ? ui->options.paused_window_title :
        ui->options.running_window_title;
}

static lib_status common_ui_create_window(common_ui *ui, common_ui_state state)
{
    kvm_window_options options = { 0 };
    lib_status status;
    if (ui == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (ui->window != NULL) return LIB_STATUS_OK;
    options.component.input_context = &ui->window_input;
    options.component.input_sink = common_ui_input;
    options.component.failure_context = ui;
    options.component.failure_sink = common_ui_delivery_failed;
    options.component.hotkeys = ui->options.hotkeys;
    options.initial_title = common_ui_title(ui, state);
    options.initial_frozen = state != COMMON_UI_STATE_RUNNING;
    status = kvm_window_create(&ui->window, &options);
    if (status != LIB_STATUS_OK) return status;
    lib_atomic_i32_store_explicit(&ui->window_live, 1,
        LIB_MEMORY_ORDER_SEQ_CST);
    ui->window_delivered_frame_sequence = 0u;
    return status;
}

static lib_status common_ui_create_console(common_ui *ui)
{
    kvm_console_options options = { 0 };
    lib_status status;
    if (ui == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (ui->console != NULL) return LIB_STATUS_OK;
    options.input_context = &ui->console_input;
    options.input_sink = common_ui_input;
    options.failure_context = ui;
    options.failure_sink = common_ui_delivery_failed;
    options.hotkeys = ui->options.hotkeys;
    status = kvm_console_create(&ui->console, &options);
    if (status == LIB_STATUS_OK) {
        ui->console_delivered_frame_sequence = 0u;
        ui->console_status_delivered = LIB_FALSE;
    }
    return status;
}

lib_status common_ui_create(common_ui **out_ui, const common_ui_options *options)
{
    common_ui *ui;
    lib_status status;
    if (out_ui == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_ui = NULL;
    if (options == NULL || options->event_sink == NULL ||
        options->running_window_title == NULL || options->paused_window_title == NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    ui = lib_allocate_zero(1u, sizeof(*ui));
    if (ui == NULL) return LIB_STATUS_NO_MEMORY;
    lib_atomic_u32_initialize(&ui->run_generation, 0u);
    lib_atomic_i32_initialize(&ui->window_live, 0);
    ui->window_input.ui = ui;
    ui->console_input.ui = ui;
    ui->console_input.vm_console = LIB_TRUE;
    ui->options = *options;
    status = lib_console_create(&ui->monitor);
    if (status == LIB_STATUS_OK)
        status = lib_console_set_event_sink(ui->monitor, common_ui_monitor_receive, ui);
    if (status == LIB_STATUS_OK)
        status = console_broker_create(&ui->broker, ui->monitor,
            CONSOLE_BROKER_COOKED_LINES);
    if (status != LIB_STATUS_OK) {
        if (ui->monitor != NULL) {
            (void)lib_console_set_event_sink(ui->monitor, NULL, NULL);
            lib_console_release(ui->monitor);
        }
        lib_release(ui);
        return status;
    }
    *out_ui = ui;
    return LIB_STATUS_OK;
}

lib_status common_ui_destroy(common_ui *ui)
{
    lib_status status;
    if (ui == NULL) return LIB_STATUS_OK;
    /* Stop native input and detach output before releasing its consumers. */
    if (ui->broker != NULL) {
        status = console_broker_destroy(ui->broker);
        if (status != LIB_STATUS_OK) return status;
        ui->broker = NULL;
    }
    if (ui->window != NULL) {
        status = kvm_window_destroy(ui->window);
        if (status != LIB_STATUS_OK) return status;
        ui->window = NULL;
        lib_atomic_i32_store_explicit(
            &ui->window_live, 0, LIB_MEMORY_ORDER_SEQ_CST);
    }
    if (ui->console != NULL) {
        status = kvm_console_destroy(ui->console);
        if (status != LIB_STATUS_OK) return status;
        ui->console = NULL;
    }
    if (ui->monitor != NULL) {
        (void)lib_console_set_event_sink(ui->monitor, NULL, NULL);
        lib_console_release(ui->monitor);
    }
    lib_release(ui);
    return LIB_STATUS_OK;
}

void common_ui_set_run_generation(common_ui *ui, lib_u32 run_generation)
{
    if (ui != NULL) lib_atomic_u32_store_explicit(&ui->run_generation,
        run_generation, LIB_MEMORY_ORDER_SEQ_CST);
}

lib_status common_ui_apply_action(common_ui *ui, common_ui_action action,
    common_ui_state state)
{
    lib_status status = LIB_STATUS_OK;
    if (ui == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    switch (action) {
    case COMMON_UI_ACTION_NONE: return LIB_STATUS_OK;
    case COMMON_UI_ACTION_CREATE_WINDOW:
        status = common_ui_create_window(ui, state);
        return status == LIB_STATUS_OK ? common_ui_emit_component(ui,
            COMMON_UI_COMPONENT_WINDOW, LIB_TRUE) : status;
    case COMMON_UI_ACTION_CREATE_VM_CONSOLE:
        status = common_ui_create_console(ui);
        return status == LIB_STATUS_OK ? common_ui_emit_component(ui,
            COMMON_UI_COMPONENT_VM_CONSOLE, LIB_TRUE) : status;
    case COMMON_UI_ACTION_BIND_VM_CONSOLE:
        if (ui->console == NULL) return LIB_STATUS_INVALID_STATE;
        status = console_broker_replace(ui->broker, ui->monitor,
            kvm_console_get_console(ui->console), CONSOLE_BROKER_RAW_EVENTS);
        return status == LIB_STATUS_OK ? common_ui_emit_broker(ui, LIB_TRUE) : status;
    case COMMON_UI_ACTION_BIND_MONITOR:
        if (ui->console == NULL) return LIB_STATUS_INVALID_STATE;
        status = console_broker_replace(ui->broker, kvm_console_get_console(ui->console),
            ui->monitor, CONSOLE_BROKER_COOKED_LINES);
        return status == LIB_STATUS_OK ? common_ui_emit_broker(ui, LIB_FALSE) : status;
    case COMMON_UI_ACTION_DESTROY_VM_CONSOLE:
        if (ui->console == NULL) return LIB_STATUS_INVALID_STATE;
        status = kvm_console_destroy(ui->console);
        if (status != LIB_STATUS_OK) return status;
        ui->console = NULL;
        ui->console_delivered_frame_sequence = 0u;
        return common_ui_emit_component(ui, COMMON_UI_COMPONENT_VM_CONSOLE, LIB_FALSE);
    case COMMON_UI_ACTION_DESTROY_WINDOW:
        if (ui->window == NULL) return LIB_STATUS_INVALID_STATE;
        status = kvm_window_destroy(ui->window);
        if (status != LIB_STATUS_OK) return status;
        lib_atomic_i32_store_explicit(&ui->window_live, 0,
            LIB_MEMORY_ORDER_SEQ_CST);
        ui->window = NULL;
        ui->window_delivered_frame_sequence = 0u;
        return common_ui_emit_component(ui, COMMON_UI_COMPONENT_WINDOW, LIB_FALSE);
    }
    return LIB_STATUS_INVALID_ARGUMENT;
}

lib_status common_ui_set_state(common_ui *ui, common_ui_state state)
{
    lib_status status;
    if (ui == NULL || ui->window == NULL) return LIB_STATUS_OK;
    status = kvm_window_set_title(ui->window, common_ui_title(ui, state));
    if (status != LIB_STATUS_OK) return status;
    if (state == COMMON_UI_STATE_RUNNING)
        return kvm_window_unfreeze(ui->window);
    if (state == COMMON_UI_STATE_PAUSED)
        return kvm_window_freeze(ui->window);
    return LIB_STATUS_OK;
}

static void common_ui_status_frame(kvm_console_text_frame *frame,
    const char *text)
{
    lib_size index;
    lib_size row = 0u, column = 0u;
    lib_memory_set(frame, 0, sizeof(*frame));
    frame->base.text_columns = KVM_TEXT_COLUMNS;
    frame->base.text_rows = 25u; /* Status layout is independent of capacity. */
    frame->base.cursor_column = -1;
    frame->base.cursor_row = -1;
    frame->base.text_palette[7] = 0xc0c0c0u;
    for (index = 0u; index < KVM_TEXT_COLUMNS * KVM_TEXT_ROWS; ++index)
        frame->base.cells[index] = (kvm_text_cell){ ' ', 0u, 7u, 0u };
    for (index = 0u; index < 256u; ++index) {
        lib_u16 character = index >= 32u && index < 127u ? (lib_u16)index : ' ';
        frame->characters.primary[index] = character;
        frame->characters.secondary[index] = character;
    }
    for (index = 0u; text != NULL && text[index] != '\0'; ++index) {
        if (text[index] == '\r') continue;
        if (text[index] == '\n') { ++row; column = 0u; continue; }
        if (row < frame->base.text_rows && column < frame->base.text_columns)
            frame->base.cells[row * KVM_TEXT_COLUMNS + column].glyph_index = (lib_u8)text[index];
        ++column;
    }
}

lib_status common_ui_publish_frame(common_ui *ui, const kvm_window_frame *frame,
    const kvm_console_character_map *characters, lib_u32 sequence,
    lib_bool window_actual, lib_bool vm_console_current,
    lib_bool console_status_surface)
{
    kvm_console_text_frame console_frame;
    lib_status status;
    lib_bool show_status;
    if (ui == NULL || frame == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    show_status = frame->graphics != 0u && console_status_surface;
    if (vm_console_current && ui->console != NULL &&
        (show_status != ui->console_status_delivered ||
         (!show_status && ui->console_delivered_frame_sequence != sequence))) {
        if (show_status) {
            common_ui_status_frame(&console_frame, ui->options.graphics_console_status_text);
        } else {
            if (frame->graphics != 0u) return LIB_STATUS_UNSUPPORTED;
            if (characters == NULL) return LIB_STATUS_INVALID_ARGUMENT;
            console_frame.base = frame->text.base;
            console_frame.characters = *characters;
        }
        status = kvm_console_publish_frame(ui->console, &console_frame);
        if (status != LIB_STATUS_OK) return status;
        ui->console_delivered_frame_sequence = sequence;
        ui->console_status_delivered = show_status;
    }
    if (window_actual && ui->window != NULL &&
        ui->window_delivered_frame_sequence != sequence) {
        status = kvm_window_publish_frame(ui->window, frame);
        if (status != LIB_STATUS_OK) return status;
        ui->window_delivered_frame_sequence = sequence;
    }
    return LIB_STATUS_OK;
}

lib_status common_ui_release_window_mouse(common_ui *ui)
{
    return ui == NULL || ui->window == NULL ? LIB_STATUS_OK :
        kvm_window_release_mouse(ui->window);
}

lib_status common_ui_write_monitor(common_ui *ui, const char *text)
{
    return ui == NULL || text == NULL ? LIB_STATUS_INVALID_ARGUMENT :
        lib_console_write_text(ui->monitor, text, lib_text_length(text));
}

lib_status common_ui_request_monitor_line(common_ui *ui)
{
    return ui == NULL ? LIB_STATUS_INVALID_ARGUMENT :
        console_broker_request_cooked_line(ui->broker, ui->monitor);
}

lib_status common_ui_cancel_monitor_line(common_ui *ui, lib_bool *out_completed)
{
    return ui == NULL ? LIB_STATUS_INVALID_ARGUMENT :
        console_broker_cancel_cooked_line(ui->broker, ui->monitor, out_completed);
}
