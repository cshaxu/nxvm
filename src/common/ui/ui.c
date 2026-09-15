#include "common/ui/ui_interface.h"

#include "lib/host/console_interface.h"
#include "lib/kvm-console/console_interface.h"
#include "lib/kvm-window/window_interface.h"


struct common_ui {
    lib_console *monitor;
    host_console_broker *broker;
    kvm_window *window;
    kvm_console *console;
    common_ui_options options;
    lib_atomic_i32 run_generation;
    lib_u32 window_delivered_frame_sequence;
    lib_u32 console_delivered_frame_sequence;
    lib_bool console_status_delivered;
};

static int common_ui_emit(common_ui *ui, const common_ui_event *event)
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
    event.run_generation = ui == NULL ? 0u : (lib_u32)lib_atomic_i32_load_explicit(
        &ui->run_generation, LIB_MEMORY_ORDER_SEQ_CST);
    event.value.delivery_failure.source_identity = source_identity;
    event.value.delivery_failure.status = status;
    (void)common_ui_emit(ui, &event);
}

static int common_ui_input(void *opaque, const kvm_input_event *input)
{
    common_ui *ui = (common_ui *)opaque;
    common_ui_event event = { 0 };
    if (ui == NULL || input == NULL) return 0;
    event.kind = COMMON_UI_EVENT_KVM_INPUT;
    event.run_generation = (lib_u32)lib_atomic_i32_load_explicit(
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
    event.run_generation = (lib_u32)lib_atomic_i32_load_explicit(
        &ui->run_generation, LIB_MEMORY_ORDER_SEQ_CST);
    event.value.component.component = component;
    event.value.component.exists = exists;
    return common_ui_emit(ui, &event) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

static lib_status common_ui_emit_broker(common_ui *ui, lib_bool vm_current)
{
    common_ui_event event = { 0 };
    event.kind = COMMON_UI_EVENT_BROKER_COMPLETED;
    event.run_generation = (lib_u32)lib_atomic_i32_load_explicit(
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
    options.component.input_context = ui;
    options.component.input_sink = common_ui_input;
    options.component.failure_context = ui;
    options.component.failure_sink = common_ui_delivery_failed;
    options.component.hotkeys = ui->options.hotkeys;
    options.initial_title = common_ui_title(ui, state);
    options.initial_frozen = state != COMMON_UI_STATE_RUNNING;
    status = kvm_window_create(&ui->window, &options);
    if (status != LIB_STATUS_OK) return status;
    ui->window_delivered_frame_sequence = 0u;
    return status;
}

static lib_status common_ui_create_console(common_ui *ui)
{
    kvm_console_options options = { 0 };
    lib_status status;
    if (ui == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (ui->console != NULL) return LIB_STATUS_OK;
    options.input_context = ui;
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
    if (out_ui == NULL || options == NULL || options->event_sink == NULL ||
        options->running_window_title == NULL || options->paused_window_title == NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_ui = NULL;
    ui = lib_allocate_zero(1u, sizeof(*ui));
    if (ui == NULL) return LIB_STATUS_NO_MEMORY;
    lib_atomic_i32_initialize(&ui->run_generation, 0);
    ui->options = *options;
    status = lib_console_create(&ui->monitor);
    if (status == LIB_STATUS_OK)
        status = lib_console_set_event_sink(ui->monitor, common_ui_monitor_receive, ui);
    if (status == LIB_STATUS_OK)
        status = host_console_broker_create(&ui->broker, ui->monitor,
            HOST_CONSOLE_COOKED_LINES);
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
    lib_status status = LIB_STATUS_OK;
    if (ui == NULL) return LIB_STATUS_OK;
    if (ui->console != NULL) {
        lib_status replace = host_console_broker_replace(ui->broker,
            kvm_console_get_console(ui->console), ui->monitor,
            HOST_CONSOLE_COOKED_LINES);
        if (replace != LIB_STATUS_OK) status = replace;
    }
    if (ui->window != NULL) {
        lib_status destroy = kvm_window_destroy(ui->window);
        if (status == LIB_STATUS_OK) status = destroy;
    }
    if (ui->console != NULL) {
        lib_status destroy = kvm_console_destroy(ui->console);
        if (status == LIB_STATUS_OK) status = destroy;
    }
    if (ui->broker != NULL) {
        lib_status destroy = host_console_broker_destroy(ui->broker);
        if (status == LIB_STATUS_OK) status = destroy;
    }
    if (ui->monitor != NULL) {
        (void)lib_console_set_event_sink(ui->monitor, NULL, NULL);
        lib_console_release(ui->monitor);
    }
    lib_release(ui);
    return status;
}

void common_ui_set_run_generation(common_ui *ui, lib_u32 run_generation)
{
    if (ui != NULL) lib_atomic_i32_store_explicit(&ui->run_generation,
        (lib_i32)run_generation, LIB_MEMORY_ORDER_SEQ_CST);
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
        status = host_console_broker_replace(ui->broker, ui->monitor,
            kvm_console_get_console(ui->console), HOST_CONSOLE_RAW_EVENTS);
        return status == LIB_STATUS_OK ? common_ui_emit_broker(ui, LIB_TRUE) : status;
    case COMMON_UI_ACTION_BIND_MONITOR:
        if (ui->console == NULL) return LIB_STATUS_INVALID_STATE;
        status = host_console_broker_replace(ui->broker, kvm_console_get_console(ui->console),
            ui->monitor, HOST_CONSOLE_COOKED_LINES);
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

static void common_ui_status_frame(kvm_frame *frame, const kvm_frame *source,
    const char *text)
{
    lib_size index;
    lib_size row = 0u, column = 0u;
    lib_memory_set(frame, 0, sizeof(*frame));
    frame->valid = 1u;
    frame->sequence = source->sequence;
    frame->text_columns = KVM_TEXT_COLUMNS;
    frame->text_rows = KVM_TEXT_ROWS;
    frame->cursor_column = -1;
    frame->cursor_row = -1;
    for (index = 0u; index < sizeof(frame->text); ++index) {
        frame->text[index] = ' ';
        frame->attributes[index] = 0x07u;
    }
    for (index = 0u; text != NULL && text[index] != '\0'; ++index) {
        if (text[index] == '\r') continue;
        if (text[index] == '\n') { ++row; column = 0u; continue; }
        if (row < KVM_TEXT_ROWS && column < KVM_TEXT_COLUMNS)
            frame->text[row * KVM_TEXT_COLUMNS + column] = (lib_u8)text[index];
        ++column;
    }
}

lib_status common_ui_publish_frame(common_ui *ui, const kvm_frame *frame,
    lib_bool window_actual, lib_bool vm_console_current,
    lib_bool console_status_surface)
{
    kvm_frame status_frame;
    const kvm_frame *console_frame = frame;
    lib_status status;
    lib_bool show_status;
    if (ui == NULL || frame == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    show_status = frame->graphics != 0u && console_status_surface;
    if (vm_console_current && ui->console != NULL &&
        (show_status != ui->console_status_delivered ||
         (!show_status && ui->console_delivered_frame_sequence != frame->sequence))) {
        if (show_status) {
            common_ui_status_frame(&status_frame, frame, ui->options.graphics_console_status_text);
            console_frame = &status_frame;
        }
        status = kvm_console_publish_frame(ui->console, console_frame);
        if (status != LIB_STATUS_OK) return status;
        ui->console_delivered_frame_sequence = frame->sequence;
        ui->console_status_delivered = show_status;
    }
    if (window_actual && ui->window != NULL &&
        ui->window_delivered_frame_sequence != frame->sequence) {
        status = kvm_window_publish_frame(ui->window, frame);
        if (status != LIB_STATUS_OK) return status;
        ui->window_delivered_frame_sequence = frame->sequence;
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
        host_console_broker_request_cooked_line(ui->broker, ui->monitor);
}

lib_status common_ui_cancel_monitor_line(common_ui *ui, lib_bool *out_completed)
{
    return ui == NULL ? LIB_STATUS_INVALID_ARGUMENT :
        host_console_broker_cancel_cooked_line(ui->broker, ui->monitor, out_completed);
}
